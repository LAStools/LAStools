/*
===============================================================================

  FILE:  laszip.cpp
  
  CONTENTS:
  
    This tool compresses and uncompresses LiDAR data in the LAS format to our
    losslessly compressed LAZ format.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2014, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    9 September 2014 -- prototyping forward-compatible coding of LAS 1.4 points  
    5 August 2011 -- possible to add/change projection info in command line
    23 June 2011 -- turned on LASzip version 2.0 compressor with chunking 
    17 May 2011 -- enabling batch processing with wildcards or multiple file names
    25 April 2011 -- added chunking for random access decompression
    23 January 2011 -- added LASreadOpener and LASwriteOpener 
    16 December 2010 -- updated to use the new library
    12 March 2009 -- updated to ask for input if started without arguments 
    17 September 2008 -- updated to deal with LAS format version 1.2
    14 February 2007 -- created after picking flowers for the Valentine dinner
  
===============================================================================
*/

#include <time.h>
#include <stdlib.h>

#include <map>
using namespace std;

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "laswaveform13reader.hpp"
#include "laswaveform13writer.hpp"
#include "bytestreamin.hpp"
#include "bytestreamout_array.hpp"
#include "bytestreamin_array.hpp"
#include "geoprojectionconverter.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"

class OffsetSize
{
public:
  OffsetSize(U64 o, U32 s) { offset = o; size = s; };
  U64 offset;
  U32 size;
};

typedef map<U64, OffsetSize> my_offset_size_map;

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"laszip *.las\n");
  fprintf(stderr,"laszip *.laz\n");
  fprintf(stderr,"laszip *.txt -iparse xyztiarn\n");
  fprintf(stderr,"laszip lidar.las\n");
  fprintf(stderr,"laszip lidar.laz -v\n");
  fprintf(stderr,"laszip -i lidar.las -o lidar_zipped.laz\n");
  fprintf(stderr,"laszip -i lidar.laz -o lidar_unzipped.las\n");
  fprintf(stderr,"laszip -i lidar.las -stdout -olaz > lidar.laz\n");
  fprintf(stderr,"laszip -stdin -o lidar.laz < lidar.las\n");
  fprintf(stderr,"laszip -h\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static void byebye(bool error=false, bool wait=false)
{
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_GUI
extern int laszip_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int laszip_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores);
#endif

int main(int argc, char *argv[])
{
  int i;
  BOOL dry = FALSE;
#ifdef COMPILE_WITH_GUI
  BOOL gui = FALSE;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
#endif
  BOOL verbose = FALSE;
  bool waveform = false;
  bool waveform_with_map = false;
  bool report_file_size = false;
  bool check_integrity = false;
  I32 end_of_points = -1;
  bool projection_was_set = false;
  bool format_not_specified = false;
  BOOL lax = FALSE;
  BOOL append = FALSE;
  U32 compatible = 0;
  U16 compatible_version = 0;
  F32 tile_size = 100.0f;
  U32 threshold = 1000;
  U32 minimum_points = 100000;
  I32 maximum_intervals = -20;
  double start_time = 0.0;
  double total_start_time = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return laszip_gui(argc, argv, 0);
#else
    fprintf(stderr,"laszip.exe is better run in the command line or via the lastool.exe GUI\n");
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
#endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '–') argv[i][0] = '-';
    }
    if (!geoprojectionconverter.parse(argc, argv)) byebye(true);
    if (!lasreadopener.parse(argc, argv)) byebye(true);
    if (!laswriteopener.parse(argc, argv)) byebye(true);
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = TRUE;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-gui") == 0)
    {
#ifdef COMPILE_WITH_GUI
      gui = TRUE;
#else
      fprintf(stderr, "WARNING: not compiled with GUI support. ignoring '-gui' ...\n");
#endif
    }
    else if (strcmp(argv[i],"-cores") == 0)
    {
#ifdef COMPILE_WITH_MULTI_CORE
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      argv[i][0] = '\0';
      i++;
      cores = atoi(argv[i]);
      argv[i][0] = '\0';
#else
      fprintf(stderr, "WARNING: not compiled with multi-core batching. ignoring '-cores' ...\n");
      i++;
#endif
    }
    else if (strcmp(argv[i],"-dry") == 0)
    {
      dry = TRUE;
    }
    else if (strcmp(argv[i],"-lax") == 0)
    {
      lax = TRUE;
    }
    else if (strcmp(argv[i],"-append") == 0)
    {
      append = TRUE;
    }
    else if (strcmp(argv[i],"-compatible") == 0)
    {
      compatible = 1;
      if (((i+1) < argc) && (argv[i+1][0] != '-') && (argv[i+1][0] != '\0'))
      {
        i++;
        compatible_version = (U16)atoi(argv[i]);
      }
      else
      {
        compatible_version = 2;
      }
    }
    else if (strcmp(argv[i],"-eop") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: char\n", argv[i]);
        usage(true);
      }
      i++;
      end_of_points = atoi(argv[i]);
      if ((end_of_points < 0) || (end_of_points > 255))
      {
        fprintf(stderr,"ERROR: end of points value needs to be between 0 and 255\n");
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-tile_size") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        usage(true);
      }
      i++;
      tile_size = (F32)atof(argv[i]);
    }
    else if (strcmp(argv[i],"-maximum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      i++;
      maximum_intervals = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-minimum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      i++;
      minimum_points = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-threshold") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
        usage(true);
      }
      i++;
      threshold = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-size") == 0)
    {
      report_file_size = true;
    }
    else if (strcmp(argv[i],"-check") == 0)
    {
      check_integrity = true;
    }
    else if (strcmp(argv[i],"-waveform") == 0 || strcmp(argv[i],"-waveforms") == 0)
    {
      waveform = true;
    }
    else if (strcmp(argv[i],"-waveform_with_map") == 0 || strcmp(argv[i],"-waveforms_with_map") == 0)
    {
      waveform = true;
      waveform_with_map = true;
    }
    else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage(true);
    }
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return laszip_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (cores > 1)
  {
    if (lasreadopener.get_file_name_number() < 2)
    {
      fprintf(stderr,"WARNING: only %u input files. ignoring '-cores %d' ...\n", lasreadopener.get_file_name_number(), cores);
    }
    else if (lasreadopener.is_merged())
    {
      fprintf(stderr,"WARNING: input files merged on-the-fly. ignoring '-cores %d' ...\n", cores);
    }
    else
    {
      return laszip_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores);
    }
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(true, argc==1);
  }

  // check output

  if (laswriteopener.is_piped())
  {
    if (lax)
    {
      fprintf(stderr,"WARNING: disabling LAX generation for piped output\n");
      lax = FALSE;
      append = FALSE;
    }
  }

  // make sure we do not corrupt the input file

  if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
  {
    fprintf(stderr, "ERROR: input and output file name are identical\n");
    usage(true);
  }

  // check if projection info was set in the command line

  int number_of_keys;
  GeoProjectionGeoKeys* geo_keys = 0;
  int num_geo_double_params;
  double* geo_double_params = 0;

  if (geoprojectionconverter.has_projection())
  {
    projection_was_set = geoprojectionconverter.get_geo_keys_from_projection(number_of_keys, &geo_keys, num_geo_double_params, &geo_double_params);
  }

  // check if the output format was *not* specified in the command line

  format_not_specified = !laswriteopener.format_was_specified();

  if (verbose) total_start_time = taketime();

  // loop over multiple input files

  while (lasreadopener.active())
  {
    if (verbose) start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();

    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      usage(true, argc==1);
    }

    // check if compatible is on and how it makes sense

    I32 compatible_start_scan_angle = -1;
    I32 compatible_start_extended_returns = -1;
    I32 compatible_start_classification = -1;
    I32 compatible_start_flags_and_channel = -1;
    I32 compatible_start_NIR_band = -1;

    if (compatible) 
    {
      fprintf(stderr, "WARNING: the new 'compatibility mode' encoding for the new LAS 1.4\n");
      fprintf(stderr, "         points is not complete. this is just an awesome prototype\n");
      fprintf(stderr, "         as proof of concept as we gather community input ... (-;\n");

      I32 index_scan_angle;
      I32 index_extended_returns;
      I32 index_classification;
      I32 index_flags_and_channel;
      I32 index_NIR_band;
      compatible = 1; // none
      if (lasreader->header.point_data_format > 5)
      {
        // only 6, 7, 8, 9, and 10 are supported
        if (lasreader->header.point_data_format <= 10)
        {
          compatible = 2; // down
          // if LAS 1.4 make it LAS 1.2
          if (lasreader->header.version_minor == 4)
          {
            // check for EVLRs and turns them into VLRs
            if (lasreader->header.number_of_extended_variable_length_records)
            {
              U32 i;
              for (i = 0; i < lasreader->header.number_of_extended_variable_length_records; i++)
              {
                if (U16_FITS_IN_RANGE(lasreader->header.evlrs[i].record_length_after_header))
                {
                  lasreader->header.add_vlr(lasreader->header.evlrs[i].user_id, lasreader->header.evlrs[i].record_id, U16_CLAMP(lasreader->header.evlrs[i].record_length_after_header), lasreader->header.evlrs[i].data, FALSE, lasreader->header.evlrs[i].description);
                  lasreader->header.evlrs[i].record_length_after_header = 0;
                  lasreader->header.evlrs[i].data = 0;
                }
                else
                {
                  fprintf(stderr, "WARNING: compatibility does not support large EVLR %u of %u. deleting '%16s' with %u bytes\n", i, lasreader->header.number_of_extended_variable_length_records, lasreader->header.evlrs[i].user_id, (U32)lasreader->header.evlrs[i].record_length_after_header);
                }
              }
              lasreader->header.clean_evlrs();
            }
            if ((compatible_version == 1) || (compatible_version == 2) || (compatible_version == 3))
            {
              if (lasreader->header.point_data_format <= 8)
              {
                lasreader->header.version_minor = 2;
                // subtract 148 byte difference between LAS 1.4 and LAS 1.2 header sizes
                lasreader->header.header_size -= 148;
                lasreader->header.offset_to_point_data -= 148;
              }
              else
              {
                lasreader->header.version_minor = 3;
                // subtract 140 byte difference between LAS 1.4 and LAS 1.3 header sizes
                lasreader->header.header_size -= 140;
                lasreader->header.offset_to_point_data -= 140;
              }
              // create 2+2+4+148 bytes payload for compatibility VLR
              ByteStreamOutArray* out;
              if (IS_LITTLE_ENDIAN())
                out = new ByteStreamOutArrayLE();
              else
                out = new ByteStreamOutArrayBE();
              // write control info
              U16 lastools_version = (U16)LAS_TOOLS_VERSION;
              out->put16bitsLE((U8*)&lastools_version);
              out->put16bitsLE((U8*)&compatible_version);
              U32 unused = 0;
              out->put32bitsLE((U8*)&unused);
              // write the 148 bytes of the extended LAS 1.4 header
              out->put64bitsLE((U8*)&(lasreader->header.start_of_waveform_data_packet_record));
              out->put64bitsLE((U8*)&(lasreader->header.start_of_first_extended_variable_length_record));
              out->put32bitsLE((U8*)&(lasreader->header.number_of_extended_variable_length_records));
              out->put64bitsLE((U8*)&(lasreader->header.extended_number_of_point_records));
              for (i = 0; i < 15; i++)
              {
                out->put64bitsLE((U8*)&(lasreader->header.extended_number_of_points_by_return[i]));
              }
              // add the compatibility VLR
              lasreader->header.add_vlr("LAS 1.4 mode", 22204, 2+2+4+148, out->takeData());
              delete out;
            }
            else
            {
              fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
              byebye(true);
            }
          }
          // old point type is two bytes shorter
          lasreader->header.point_data_record_length -= 2;
          if ((compatible_version == 1) || (compatible_version == 2) || (compatible_version == 3))
          {
            // but we add 5 bytes of attributes
            lasreader->header.point_data_record_length += 5;
            // scan_angle (absolute or difference) is stored as a I16
            LASattribute lasattribute_scan_angle(3, "LAS 1.4 scan angle", "additional attributes");
            index_scan_angle = lasreader->header.add_attribute(lasattribute_scan_angle);
            // extended returns stored as a U8
            LASattribute lasattribute_extended_returns(0, "LAS 1.4 extended returns", "additional attributes");
            index_extended_returns = lasreader->header.add_attribute(lasattribute_extended_returns);
            // classification stored as a U8
            LASattribute lasattribute_classification(0, "LAS 1.4 classification", "additional attributes");
            index_classification = lasreader->header.add_attribute(lasattribute_classification);
            // flags and channel stored as a U8
            LASattribute lasattribute_flags_and_channel(0, "LAS 1.4 flags and channel", "additional attributes");
            index_flags_and_channel = lasreader->header.add_attribute(lasattribute_flags_and_channel);
          }
          else
          {
            fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
            byebye(true);
          }
          // maybe store the NIR band as a U16
          if (lasreader->header.point_data_format == 8 || lasreader->header.point_data_format == 10)
          {
            // the NIR band is stored as a U16
            LASattribute lasattribute_NIR_band(2, "LAS 1.4 NIR band", "additional attributes");
            index_NIR_band = lasreader->header.add_attribute(lasattribute_NIR_band);
          }
          // update VLR
          lasreader->header.update_extra_bytes_vlr(TRUE);
          // update point type
          if (lasreader->header.point_data_format == 6)
          {
            lasreader->header.point_data_format = 1;
          }
          else if (lasreader->header.point_data_format <= 8)
          {
            lasreader->header.point_data_format = 3;
          }
          else // 9->4 and 10->5 
          {
            lasreader->header.point_data_format -= 5;
          }
        }
        else
        {
          fprintf(stderr, "ERROR: compatibility for %d not supported yet\n", lasreader->header.point_data_format);        
        }
      }
      else if ((lasreader->header.get_vlr("LAS 1.4 mode", 22204)) && ((index_scan_angle = lasreader->header.get_attribute_index("LAS 1.4 scan angle")) >= 0) && ((index_extended_returns = lasreader->header.get_attribute_index("LAS 1.4 extended returns")) >= 0) && ((index_classification = lasreader->header.get_attribute_index("LAS 1.4 classification")) >= 0) && ((index_flags_and_channel = lasreader->header.get_attribute_index("LAS 1.4 flags and channel")) >= 0))
      {
        // only 1, 3, 4 and 5 are supported
        if ((lasreader->header.point_data_format == 1) || (lasreader->header.point_data_format == 3) || (lasreader->header.point_data_format == 4) || (lasreader->header.point_data_format == 5))
        {
          compatible = 3; // up
          // make it LAS 1.4
          if (lasreader->header.version_minor < 3)
          {
            // add the 148 byte difference between LAS 1.4 and LAS 1.2 header sizes
            lasreader->header.header_size += 148;
            lasreader->header.offset_to_point_data += 148;
          }
          else if (lasreader->header.version_minor == 3)
          {
            // add the 140 byte difference between LAS 1.4 and LAS 1.3 header sizes
            lasreader->header.header_size += 140;
            lasreader->header.offset_to_point_data += 140;
          }
          else
          {
            fprintf(stderr, "WARNING: LAS header version %d.%d when upgrading to LAS 1.4 in compatibility mode\n", lasreader->header.version_major, lasreader->header.version_minor);        
          }
          lasreader->header.version_minor = 4;
          // get the compatibility VLR
          const LASvlr* compatibility_vlr = lasreader->header.get_vlr("LAS 1.4 mode", 22204);
          // make sure it has the right length
          if (compatibility_vlr->record_length_after_header != (2+2+4+148))
          {
            fprintf(stderr, "ERROR: compatibility VLR has %u instead of %u bytes in payload\n", compatibility_vlr->record_length_after_header, 2+2+4+148);
            byebye(true);
          }
          // read the 2+2+4+148 bytes payload from the compatibility VLR
          ByteStreamInArray* in;
          if (IS_LITTLE_ENDIAN())
            in = new ByteStreamInArrayLE(compatibility_vlr->data, compatibility_vlr->record_length_after_header);
          else
            in = new ByteStreamInArrayBE(compatibility_vlr->data, compatibility_vlr->record_length_after_header);
          // read the 2+2+4+148 bytes of the extended LAS 1.4 header
          U16 lastools_version;
          in->get16bitsLE((U8*)&lastools_version);
          in->get16bitsLE((U8*)&compatible_version);
          if ((compatible_version != 1) && (compatible_version != 2) && (compatible_version != 3))
          {
            fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
            byebye(true);
          }
          U32 unused;
          in->get32bitsLE((U8*)&unused);
          if (unused != 0)
          {
            fprintf(stderr, "WARNING: unused is %u instead of 0\n", unused);
          }
          in->get64bitsLE((U8*)&(lasreader->header.start_of_waveform_data_packet_record));
          in->get64bitsLE((U8*)&(lasreader->header.start_of_first_extended_variable_length_record));
          in->get32bitsLE((U8*)&(lasreader->header.number_of_extended_variable_length_records));
          in->get64bitsLE((U8*)&(lasreader->header.extended_number_of_point_records));
          for (i = 0; i < 15; i++)
          {
            in->get64bitsLE((U8*)&(lasreader->header.extended_number_of_points_by_return[i]));
          }
          // delete the compatibility VLR
          lasreader->header.remove_vlr("LAS 1.4 mode", 22204);
          delete in;
          // new point type is two bytes longer
          lasreader->header.point_data_record_length += 2;
          // but we subtract 5 bytes of attributes
          lasreader->header.point_data_record_length -= 5;
          // get start of attributes in point
          compatible_start_scan_angle = lasreader->header.get_attribute_start(index_scan_angle);
          compatible_start_extended_returns = lasreader->header.get_attribute_start(index_extended_returns);
          compatible_start_classification = lasreader->header.get_attribute_start(index_classification);
          compatible_start_flags_and_channel = lasreader->header.get_attribute_start(index_flags_and_channel);
          // maybe we also have a NIR band?
          if ((lasreader->header.point_data_format == 3) || (lasreader->header.point_data_format == 5))
          {
            index_NIR_band = lasreader->header.get_attribute_index("LAS 1.4 NIR band");
            if (index_NIR_band != -1)
            {
              compatible_start_NIR_band = lasreader->header.get_attribute_start(index_NIR_band);
              lasreader->header.remove_attribute(index_NIR_band);
            }
          }
          // remove attributes from Extra Bytes VLR
          lasreader->header.remove_attribute(index_flags_and_channel);
          lasreader->header.remove_attribute(index_classification);
          lasreader->header.remove_attribute(index_extended_returns);
          lasreader->header.remove_attribute(index_scan_angle);
          // update VLR
          lasreader->header.update_extra_bytes_vlr(TRUE);
          // update point type
          if (lasreader->header.point_data_format == 1)
          {
            lasreader->header.point_data_format = 6;
          }
          else if (lasreader->header.point_data_format == 3)
          {
            if (compatible_start_NIR_band != -1)
            {
              lasreader->header.point_data_format = 8;
            }
            else
            {
              lasreader->header.point_data_format = 7;
            }
          }
          else
          {
            lasreader->header.point_data_format += 5;
          }
          // remove old LASzip
          lasreader->header.clean_laszip();
        }
      }
    }

    // switch

    if (report_file_size)
    {
      // maybe only report uncompressed file size
      I64 uncompressed_file_size = (I64)lasreader->header.number_of_point_records * (I64)lasreader->header.point_data_record_length + lasreader->header.offset_to_point_data;
      if (uncompressed_file_size == (I64)((U32)uncompressed_file_size))
        fprintf(stderr,"uncompressed file size is %u bytes or %.2f MB for '%s'\n", (U32)uncompressed_file_size, (F64)uncompressed_file_size/1024.0/1024.0, lasreadopener.get_file_name());
      else
        fprintf(stderr,"uncompressed file size is %.2f MB or %.2f GB for '%s'\n", (F64)uncompressed_file_size/1024.0/1024.0, (F64)uncompressed_file_size/1024.0/1024.0/1024.0, lasreadopener.get_file_name());
    }
    else if (dry || check_integrity)
    {
      // maybe only a dry read pass
      start_time = taketime();
      while (lasreader->read_point());
      if (check_integrity)
      {
        if (lasreader->p_count != lasreader->npoints)
        {
#ifdef _WIN32
          fprintf(stderr,"FAILED integrity check for '%s' after %I64d of %I64d points\n", lasreadopener.get_file_name(), lasreader->p_count, lasreader->npoints);
#else
          fprintf(stderr,"FAILED integrity check for '%s' after %lld of %lld points\n", lasreadopener.get_file_name(), lasreader->p_count, lasreader->npoints);
#endif
        }
        else
        {
          fprintf(stderr,"SUCCESS for '%s'\n", lasreadopener.get_file_name());
        }
      }
      else
      {
        fprintf(stderr,"needed %g secs to read '%s'\n", taketime()-start_time, lasreadopener.get_file_name());
      }
    }
    else
    {
      I64 start_of_waveform_data_packet_record = 0;

      // create output file name if no output was specified 
      if (!laswriteopener.active())
      {
        if (lasreadopener.get_file_name() == 0)
        {
          fprintf(stderr,"ERROR: no output specified\n");
          usage(true, argc==1);
        }
        laswriteopener.set_force(TRUE);
        if (format_not_specified)
        {
          if (lasreader->get_format() == LAS_TOOLS_FORMAT_LAZ)
          {
            laswriteopener.set_format(LAS_TOOLS_FORMAT_LAS);
          }
          else
          {
            laswriteopener.set_format(LAS_TOOLS_FORMAT_LAZ);
          }
        }
        laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
      }

      // maybe set projection

      if (projection_was_set)
      {
        lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
        if (geo_double_params)
        {
          lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
        }
        else
        {
          lasreader->header.del_geo_double_params();
        }
        lasreader->header.del_geo_ascii_params();
      }

      // almost never open laswaveform13reader and laswaveform13writer (-:

      LASwaveform13reader* laswaveform13reader = 0;
      LASwaveform13writer* laswaveform13writer = 0;

      if (waveform)
      {
        laswaveform13reader = lasreadopener.open_waveform13(&lasreader->header);
        if (laswaveform13reader)
        {
          // switch compression on/off
          U8 compression_type = (laswriteopener.get_format() == LAS_TOOLS_FORMAT_LAZ ? 1 : 0);
          for (i = 0; i < 255; i++) if (lasreader->header.vlr_wave_packet_descr[i]) lasreader->header.vlr_wave_packet_descr[i]->setCompressionType(compression_type);
          // create laswaveform13writer
          laswaveform13writer = laswriteopener.open_waveform13(&lasreader->header);
          if (laswaveform13writer == 0)
          {
            delete [] laswaveform13reader;
            laswaveform13reader = 0;
            waveform = 0;
            // switch compression on/off back
            U8 compression_type = (laswriteopener.get_format() == LAS_TOOLS_FORMAT_LAZ ? 0 : 1);
            for (i = 0; i < 255; i++) if (lasreader->header.vlr_wave_packet_descr[i]) lasreader->header.vlr_wave_packet_descr[i]->setCompressionType(compression_type);
          }
        }
        else
        {
          waveform = false;
        }
      }

      // special check for LAS 1.3+ files that contain waveform data

      if ((lasreader->header.version_major == 1) && (lasreader->header.version_minor >= 3))
      {
        if (lasreader->header.global_encoding & 2) // if bit # 1 is set we have internal waveform data
        {
          lasreader->header.global_encoding &= ~((U16)2); // remove internal bit
          if (lasreader->header.start_of_waveform_data_packet_record) // offset to 
          {
            start_of_waveform_data_packet_record = lasreader->header.start_of_waveform_data_packet_record;
            lasreader->header.start_of_waveform_data_packet_record = 0;
            lasreader->header.global_encoding |= ((U16)4); // set external bit
          }
        }
      }

      I64 bytes_written = 0;

      // open laswriter

      LASwriter* laswriter = laswriteopener.open(&lasreader->header);

      if (laswriter == 0)
      {
        fprintf(stderr, "ERROR: could not open laswriter\n");
        usage(true, argc==1);
      }

      // should we also deal with waveform data

      if (waveform)
      {
        U8 compression_type = (laswaveform13reader->is_compressed() ? 1 : 0);
        for (i = 0; i < 255; i++) if (lasreader->header.vlr_wave_packet_descr[i]) lasreader->header.vlr_wave_packet_descr[i]->setCompressionType(compression_type);

        U64 last_offset = 0;
        U32 last_size = 60;
        U64 new_offset = 0;
        U32 new_size = 0;
        U32 waves_written = 0;
        U32 waves_referenced = 0;

        my_offset_size_map offset_size_map;

        LASindex lasindex;
        if (lax) // should we also create a spatial indexing file
        {
          // setup the quadtree
          LASquadtree* lasquadtree = new LASquadtree;
          lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);

          // create lax index
          lasindex.prepare(lasquadtree, threshold);
        }

        // loop over points

        while (lasreader->read_point())
        {
          if (lasreader->point.wavepacket.getIndex()) // if point is attached to a waveform
          {
            waves_referenced++;
            if (lasreader->point.wavepacket.getOffset() == last_offset)
            {
              lasreader->point.wavepacket.setOffset(new_offset);
              lasreader->point.wavepacket.setSize(new_size);
            }
            else if (lasreader->point.wavepacket.getOffset() > last_offset)
            {
              if (lasreader->point.wavepacket.getOffset() > (last_offset + last_size))
              {
                if (!waveform_with_map)
                {
                  fprintf(stderr,"WARNING: gap in waveform offsets.\n");
#ifdef _WIN32
                  fprintf(stderr,"WARNING: last offset plus size was %I64d but new offset is %I64d (for point %I64d)\n", (last_offset + last_size), lasreader->point.wavepacket.getOffset(), lasreader->p_count);
#else
                  fprintf(stderr,"WARNING: last offset plus size was %lld but new offset is %lld (for point %lld)\n", (last_offset + last_size), lasreader->point.wavepacket.getOffset(), lasreader->p_count);
#endif
                }
              }
              waves_written++;
              last_offset = lasreader->point.wavepacket.getOffset();
              last_size = lasreader->point.wavepacket.getSize();
              laswaveform13reader->read_waveform(&lasreader->point);
              laswaveform13writer->write_waveform(&lasreader->point, laswaveform13reader->samples);
              new_offset = lasreader->point.wavepacket.getOffset();
              new_size = lasreader->point.wavepacket.getSize();
              if (waveform_with_map)
              {
                offset_size_map.insert(my_offset_size_map::value_type(last_offset, OffsetSize(new_offset,new_size)));
              }
            }
            else
            {
              if (waveform_with_map)
              {
                my_offset_size_map::iterator map_element;
                map_element = offset_size_map.find(lasreader->point.wavepacket.getOffset());
                if (map_element == offset_size_map.end())
                {
                  waves_written++;
                  last_offset = lasreader->point.wavepacket.getOffset();
                  last_size = lasreader->point.wavepacket.getSize();
                  laswaveform13reader->read_waveform(&lasreader->point);
                  laswaveform13writer->write_waveform(&lasreader->point, laswaveform13reader->samples);
                  new_offset = lasreader->point.wavepacket.getOffset();
                  new_size = lasreader->point.wavepacket.getSize();
                  offset_size_map.insert(my_offset_size_map::value_type(last_offset, OffsetSize(new_offset,new_size)));
                }
                else
                {
                  lasreader->point.wavepacket.setOffset((*map_element).second.offset);
                  lasreader->point.wavepacket.setSize((*map_element).second.size);
                }
              }
              else
              {
                fprintf(stderr,"ERROR: waveform offsets not in monotonically increasing order.\n");
#ifdef _WIN32
                fprintf(stderr,"ERROR: last offset was %I64d but new offset is %I64d (for point %I64d)\n", last_offset, lasreader->point.wavepacket.getOffset(), lasreader->p_count);
#else
                fprintf(stderr,"ERROR: last offset was %lld but new offset is %lld (for point %lld)\n", last_offset, lasreader->point.wavepacket.getOffset(), lasreader->p_count);
#endif
                fprintf(stderr,"ERROR: use option '-waveforms_with_map' to compress.\n");
                byebye(true, argc==1);
              }
            }
          }
          laswriter->write_point(&lasreader->point);
          if (lax)
          {
            lasindex.add(&lasreader->point, (U32)(laswriter->p_count));
          }
          if (!lasreadopener.is_header_populated())
          {
            laswriter->update_inventory(&lasreader->point);
          }
        }

        if (verbose && ((laswriter->p_count % 1000000) == 0)) fprintf(stderr,"written %d referenced %d of %d points\n", waves_written, waves_referenced, (I32)laswriter->p_count);

        if (!lasreadopener.is_header_populated())
        {
          laswriter->update_header(&lasreader->header, TRUE);
        }

        // flush the writer
        bytes_written = laswriter->close();

        if (lax)
        {
          // adaptive coarsening
          lasindex.complete(minimum_points, maximum_intervals);

          if (append)
          {
            // append lax to file
            lasindex.append(laswriteopener.get_file_name());
          }
          else
          {
            // write lax to file
            lasindex.write(laswriteopener.get_file_name());
          }
        }
      }
      else
      {
        // loop over points

        if (lasreadopener.is_header_populated())
        {
          if (lax) // should we also create a spatial indexing file
          {
            // setup the quadtree
            LASquadtree* lasquadtree = new LASquadtree;
            lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);

            // create lax index
            LASindex lasindex;
            lasindex.prepare(lasquadtree, threshold);
  
            // compress points and add to index
            while (lasreader->read_point())
            {
              lasindex.add(&lasreader->point, (U32)(laswriter->p_count));
              laswriter->write_point(&lasreader->point);
            }

            // flush the writer
            bytes_written = laswriter->close();

            // adaptive coarsening
            lasindex.complete(minimum_points, maximum_intervals);

            if (append)
            {
              // append lax to file
              lasindex.append(laswriteopener.get_file_name());
            }
            else
            {
              // write lax to file
              lasindex.write(laswriteopener.get_file_name());
            }
          }
          else
          {
            if (end_of_points > -1)
            {
              U8 point10[20];
              memset(point10, end_of_points, 20);

              if (verbose) fprintf(stderr, "writing with end_of_points value %d\n", end_of_points);

              while (lasreader->read_point())
              {
                if (memcmp(point10, &lasreader->point, 20) == 0)
                {
                  break;
                }
                laswriter->write_point(&lasreader->point);
                laswriter->update_inventory(&lasreader->point);
              }
              laswriter->update_header(&lasreader->header, TRUE);
            }
            else
            {
              if (compatible == 2) // down
              {
                I32 scan_angle_remainder;
                I16 extended_scan_angle;
                I32 return_count_difference;
                I32 number_of_returns_increment;
                I32 return_number_increment;
                I32 overlap_bit;
                I32 scanner_channel;
                LASpoint laspoint;
                laspoint.init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length, &lasreader->header);
                if (compatible_version == 1)
                {
                  I32 start_scan_angle = lasreader->header.get_attribute_start("LAS 1.4 scan angle");
                  I32 start_extended_returns = lasreader->header.get_attribute_start("LAS 1.4 extended returns");
                  I32 start_classification = lasreader->header.get_attribute_start("LAS 1.4 classification");
                  I32 start_flags_and_channel = lasreader->header.get_attribute_start("LAS 1.4 flags and channel");
                  I32 start_NIR_band = lasreader->header.get_attribute_start("LAS 1.4 NIR band");
                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // distill individual attributes
                    scan_angle_remainder = laspoint.extended_scan_angle - I16_QUANTIZE(((F32)laspoint.scan_angle_rank)/0.006f);
                    if (laspoint.extended_number_of_returns <= 7)
                    {
                      laspoint.number_of_returns = laspoint.extended_number_of_returns;
                      if (laspoint.extended_return_number <= 7)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        laspoint.return_number = 7;
                      }
                    }
                    else
                    {
                      laspoint.number_of_returns = 7;
                      if (laspoint.extended_return_number < 7)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        if (laspoint.extended_return_number < laspoint.extended_number_of_returns)
                        {
                          laspoint.return_number = 6;
                        }
                        else
                        {
                          laspoint.return_number = 7;
                        }
                      }
                    }
                    return_number_increment = laspoint.extended_return_number - laspoint.return_number;
                    assert(return_number_increment >= 0);
                    number_of_returns_increment = laspoint.extended_number_of_returns - laspoint.number_of_returns;
                    assert(number_of_returns_increment >= 0);
                    if (laspoint.extended_classification > 31)
                    {
                      laspoint.set_classification(0);
                    }
                    else
                    {
                      laspoint.extended_classification = 0;
                    }
                    scanner_channel = laspoint.extended_scanner_channel;
                    overlap_bit = (laspoint.extended_classification_flags >> 3);
                    // compose
                    laspoint.set_attribute(start_scan_angle, ((I16)scan_angle_remainder));
                    laspoint.set_attribute(start_extended_returns, (U8)((return_number_increment << 4) | number_of_returns_increment));
                    laspoint.set_attribute(start_classification, (U8)(laspoint.extended_classification));
                    laspoint.set_attribute(start_flags_and_channel, (U8)((scanner_channel << 1) | overlap_bit));
                    if (start_NIR_band != -1)
                    {
                      laspoint.set_attribute(start_NIR_band, laspoint.rgb[3]);
                    }
                    laswriter->write_point(&laspoint);
                  }
                }
                else if (compatible_version == 2)
                {
                  I32 start_scan_angle = lasreader->header.get_attribute_start("LAS 1.4 scan angle");
                  I32 start_extended_returns = lasreader->header.get_attribute_start("LAS 1.4 extended returns");
                  I32 start_classification = lasreader->header.get_attribute_start("LAS 1.4 classification");
                  I32 start_flags_and_channel = lasreader->header.get_attribute_start("LAS 1.4 flags and channel");
                  I32 start_NIR_band = lasreader->header.get_attribute_start("LAS 1.4 NIR band");
                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // distill individual attributes
                    scan_angle_remainder = laspoint.extended_scan_angle - I16_QUANTIZE(((F32)laspoint.scan_angle_rank)/0.006f);
                    if (laspoint.extended_number_of_returns <= 7)
                    {
                      laspoint.number_of_returns = laspoint.extended_number_of_returns;
                      if (laspoint.extended_return_number <= 7)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        laspoint.return_number = 7;
                      }
                    }
                    else
                    {
                      laspoint.number_of_returns = 7;
                      if (laspoint.extended_return_number <= 4)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        return_count_difference = laspoint.extended_number_of_returns - laspoint.extended_return_number;
                        if (return_count_difference <= 0)
                        {
                          laspoint.return_number = 7;
                        }
                        else if (return_count_difference >= 3)
                        {
                          laspoint.return_number = 4;
                        }
                        else
                        {
                          laspoint.return_number = 7 - return_count_difference;
                        }
                      }
                    }
                    return_number_increment = laspoint.extended_return_number - laspoint.return_number;
                    assert(return_number_increment >= 0);
                    number_of_returns_increment = laspoint.extended_number_of_returns - laspoint.number_of_returns;
                    assert(number_of_returns_increment >= 0);
                    if (laspoint.extended_classification > 31)
                    {
                      laspoint.set_classification(0);
                    }
                    else
                    {
                      laspoint.extended_classification = 0;
                    }
                    scanner_channel = laspoint.extended_scanner_channel;
                    overlap_bit = (laspoint.extended_classification_flags >> 3);
                    // compose
                    laspoint.set_attribute(start_scan_angle, ((I16)scan_angle_remainder));
                    laspoint.set_attribute(start_extended_returns, (U8)((return_number_increment << 4) | number_of_returns_increment));
                    laspoint.set_attribute(start_classification, (U8)(laspoint.extended_classification));
                    laspoint.set_attribute(start_flags_and_channel, (U8)((scanner_channel << 1) | overlap_bit));
                    if (start_NIR_band != -1)
                    {
                      laspoint.set_attribute(start_NIR_band, laspoint.rgb[3]);
                    }
                    laswriter->write_point(&laspoint);
                  }
                }
                else if (compatible_version == 3)
                {
                  I32 start_scan_angle = lasreader->header.get_attribute_start("LAS 1.4 scan angle");
                  I32 start_extended_returns = lasreader->header.get_attribute_start("LAS 1.4 extended returns");
                  I32 start_classification = lasreader->header.get_attribute_start("LAS 1.4 classification");
                  I32 start_flags_and_channel = lasreader->header.get_attribute_start("LAS 1.4 flags and channel");
                  I32 start_NIR_band = lasreader->header.get_attribute_start("LAS 1.4 NIR band");
                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // distill individual attributes
                    extended_scan_angle = laspoint.extended_scan_angle;
                    if (laspoint.extended_number_of_returns <= 7)
                    {
                      laspoint.number_of_returns = laspoint.extended_number_of_returns;
                      if (laspoint.extended_return_number <= 7)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        laspoint.return_number = 7;
                      }
                    }
                    else
                    {
                      laspoint.number_of_returns = 7;
                      if (laspoint.extended_return_number <= 4)
                      {
                        laspoint.return_number = laspoint.extended_return_number;
                      }
                      else
                      {
                        return_count_difference = laspoint.extended_number_of_returns - laspoint.extended_return_number;
                        if (return_count_difference <= 0)
                        {
                          laspoint.return_number = 7;
                        }
                        else if (return_count_difference >= 3)
                        {
                          laspoint.return_number = 4;
                        }
                        else
                        {
                          laspoint.return_number = 7 - return_count_difference;
                        }
                      }
                    }
                    return_number_increment = laspoint.extended_return_number - laspoint.return_number;
                    assert(return_number_increment >= 0);
                    number_of_returns_increment = laspoint.extended_number_of_returns - laspoint.number_of_returns;
                    assert(number_of_returns_increment >= 0);
                    if (laspoint.extended_classification > 31)
                    {
                      laspoint.set_classification(0);
                    }
                    else
                    {
                      laspoint.extended_classification = 0;
                    }
                    scanner_channel = laspoint.extended_scanner_channel;
                    overlap_bit = (laspoint.extended_classification_flags >> 3);
                    // compose
                    laspoint.set_attribute(start_scan_angle, extended_scan_angle);
                    laspoint.set_attribute(start_extended_returns, (U8)((return_number_increment << 4) | number_of_returns_increment));
                    laspoint.set_attribute(start_classification, (U8)(laspoint.extended_classification));
                    laspoint.set_attribute(start_flags_and_channel, (U8)((scanner_channel << 1) | overlap_bit));
                    if (start_NIR_band != -1)
                    {
                      laspoint.set_attribute(start_NIR_band, laspoint.rgb[3]);
                    }
                    laswriter->write_point(&laspoint);
                  }
                }
                else
                {
                  fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
                  byebye(true);
                }
              }
              else if (compatible == 3) // up
              {
                LASpoint laspoint;
                laspoint.init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length, &lasreader->header);
                I32 number_of_returns_increment;
                I32 return_number_increment;
                I32 overlap_bit;
                I32 scanner_channel;
                if (compatible_version == 1)
                {
                  I16 scan_angle;
                  U8 extended_returns;
                  U8 classification;
                  U8 flags_and_channel;

                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // get extra_attributes
                    lasreader->point.get_attribute(compatible_start_scan_angle, scan_angle);
                    lasreader->point.get_attribute(compatible_start_extended_returns, extended_returns);
                    lasreader->point.get_attribute(compatible_start_classification, classification);
                    lasreader->point.get_attribute(compatible_start_flags_and_channel, flags_and_channel);
                    if (compatible_start_NIR_band != -1)
                    {
                      lasreader->point.get_attribute(compatible_start_NIR_band, laspoint.rgb[3]);
                    }
                    // decompose into individual attributes
                    return_number_increment = (extended_returns >> 4) & 0x0F;
                    number_of_returns_increment = extended_returns & 0x0F;
                    scanner_channel = (flags_and_channel >> 1) & 0x03;
                    overlap_bit = flags_and_channel & 0x01;
                    // instill into point
                    laspoint.extended_scan_angle = scan_angle + I16_QUANTIZE(((F32)laspoint.scan_angle_rank) / 0.006f);
                    laspoint.extended_return_number = return_number_increment + laspoint.return_number;
                    laspoint.extended_number_of_returns = number_of_returns_increment + laspoint.number_of_returns;
                    laspoint.extended_classification = classification + laspoint.get_classification();
                    laspoint.extended_scanner_channel = scanner_channel;
                    laspoint.extended_classification_flags = (overlap_bit << 3) | (laspoint.classification >> 5);
                    laswriter->write_point(&laspoint);
                  }
                }
                else if (compatible_version == 2)
                {
                  I16 scan_angle;
                  U8 extended_returns;
                  U8 classification;
                  U8 flags_and_channel;

                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // get extra_attributes
                    lasreader->point.get_attribute(compatible_start_scan_angle, scan_angle);
                    lasreader->point.get_attribute(compatible_start_extended_returns, extended_returns);
                    lasreader->point.get_attribute(compatible_start_classification, classification);
                    lasreader->point.get_attribute(compatible_start_flags_and_channel, flags_and_channel);
                    if (compatible_start_NIR_band != -1)
                    {
                      lasreader->point.get_attribute(compatible_start_NIR_band, laspoint.rgb[3]);
                    }
                    // decompose into individual attributes
                    return_number_increment = (extended_returns >> 4) & 0x0F;
                    number_of_returns_increment = extended_returns & 0x0F;
                    scanner_channel = (flags_and_channel >> 1) & 0x03;
                    overlap_bit = flags_and_channel & 0x01;
                    // instill into point
                    laspoint.extended_scan_angle = scan_angle + I16_QUANTIZE(((F32)laspoint.scan_angle_rank) / 0.006f);
                    laspoint.extended_return_number = return_number_increment + laspoint.return_number;
                    laspoint.extended_number_of_returns = number_of_returns_increment + laspoint.number_of_returns;
                    laspoint.extended_classification = classification + laspoint.get_classification();
                    laspoint.extended_scanner_channel = scanner_channel;
                    laspoint.extended_classification_flags = (overlap_bit << 3) | (laspoint.classification >> 5);
                    laswriter->write_point(&laspoint);
                  }
                }
                else if (compatible_version == 3)
                {
                  I16 extended_scan_angle;
                  U8 extended_returns;
                  U8 classification;
                  U8 flags_and_channel;

                  while (lasreader->read_point())
                  {
                    // copy point
                    laspoint = lasreader->point;
                    // get extra_attributes
                    lasreader->point.get_attribute(compatible_start_scan_angle, extended_scan_angle);
                    lasreader->point.get_attribute(compatible_start_extended_returns, extended_returns);
                    lasreader->point.get_attribute(compatible_start_classification, classification);
                    lasreader->point.get_attribute(compatible_start_flags_and_channel, flags_and_channel);
                    if (compatible_start_NIR_band != -1)
                    {
                      lasreader->point.get_attribute(compatible_start_NIR_band, laspoint.rgb[3]);
                    }
                    // decompose into individual attributes
                    return_number_increment = (extended_returns >> 4) & 0x0F;
                    number_of_returns_increment = extended_returns & 0x0F;
                    scanner_channel = (flags_and_channel >> 1) & 0x03;
                    overlap_bit = flags_and_channel & 0x01;
                    // instill into point
                    laspoint.extended_scan_angle = extended_scan_angle;
                    laspoint.extended_return_number = return_number_increment + laspoint.return_number;
                    laspoint.extended_number_of_returns = number_of_returns_increment + laspoint.number_of_returns;
                    laspoint.extended_classification = classification + laspoint.get_classification();
                    laspoint.extended_scanner_channel = scanner_channel;
                    laspoint.extended_classification_flags = (overlap_bit << 3) | (laspoint.classification >> 5);
                    laswriter->write_point(&laspoint);
                  }
                }
                else 
                {
                  fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
                  byebye(true);
                }
              }
              else
              {
                while (lasreader->read_point())
                {
                  laswriter->write_point(&lasreader->point);
                }
              }
            }
            // flush the writer
            bytes_written = laswriter->close();
          }
        }
        else
        {
          if (lax && (lasreader->header.min_x < lasreader->header.max_x) && (lasreader->header.min_y < lasreader->header.max_y))
          {
            // setup the quadtree
            LASquadtree* lasquadtree = new LASquadtree;
            lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);

            // create lax index
            LASindex lasindex;
            lasindex.prepare(lasquadtree, threshold);
  
            // compress points and add to index
            while (lasreader->read_point())
            {
              lasindex.add(&lasreader->point, (U32)(laswriter->p_count));
              laswriter->write_point(&lasreader->point);
              laswriter->update_inventory(&lasreader->point);
            }

            // flush the writer
            bytes_written = laswriter->close();

            // adaptive coarsening
            lasindex.complete(minimum_points, maximum_intervals);

            if (append)
            {
              // append lax to file
              lasindex.append(laswriteopener.get_file_name());
            }
            else
            {
              // write lax to file
              lasindex.write(laswriteopener.get_file_name());
            }
          }
          else
          {
            if (end_of_points > -1)
            {
              U8 point10[20];
              memset(point10, end_of_points, 20);

              if (verbose) fprintf(stderr, "writing with end_of_points value %d\n", end_of_points);

              while (lasreader->read_point())
              {
                if (memcmp(point10, &lasreader->point, 20) == 0)
                {
                  break;
                }
                laswriter->write_point(&lasreader->point);
                laswriter->update_inventory(&lasreader->point);
              }
            }
            else
            {
              while (lasreader->read_point())
              {
                laswriter->write_point(&lasreader->point);
                laswriter->update_inventory(&lasreader->point);
              }
            }
          }

          // update the header
          laswriter->update_header(&lasreader->header, TRUE);

          // flush the writer
          bytes_written = laswriter->close();
        }
      }

      delete laswriter;
  
#ifdef _WIN32
      if (verbose) fprintf(stderr,"%g secs to write %I64d bytes for '%s' with %I64d points of type %d\n", taketime()-start_time, bytes_written, laswriteopener.get_file_name(), lasreader->p_count, lasreader->header.point_data_format);
#else
      if (verbose) fprintf(stderr,"%g secs to write %lld bytes for '%s' with %lld points of type %d\n", taketime()-start_time, bytes_written, laswriteopener.get_file_name(), lasreader->p_count, lasreader->header.point_data_format);
#endif

      if (start_of_waveform_data_packet_record && !waveform)
      {
        lasreader->close(FALSE);
        ByteStreamIn* stream = lasreader->get_stream();
        stream->seek(start_of_waveform_data_packet_record);
        char* wave_form_file_name;
        if (laswriteopener.get_file_name())
        {
          wave_form_file_name = strdup(laswriteopener.get_file_name());
          int len = strlen(wave_form_file_name);
          if (wave_form_file_name[len-3] == 'L')
          {
            wave_form_file_name[len-3] = 'W';
            wave_form_file_name[len-2] = 'D';
            wave_form_file_name[len-1] = 'P';
          }
          else
          {
            wave_form_file_name[len-3] = 'w';
            wave_form_file_name[len-2] = 'd';
            wave_form_file_name[len-1] = 'p';
          }
        }
        else
        {
          wave_form_file_name = strdup("wave_form.wdp");
        }
        FILE* file = fopen(wave_form_file_name, "wb");
        if (file)
        {
          if (verbose) fprintf(stderr,"writing waveforms to '%s'\n", wave_form_file_name);
          try
          {
            int byte;
            while (true)
            {
              byte = stream->getByte();
              fputc(byte, file);
            }
          }
          catch (...)
          {
            fclose(file);
          }
        }
      }

      laswriteopener.set_file_name(0);
      if (format_not_specified)
      {
        laswriteopener.set_format((const CHAR*)NULL);
      }
    }
  
    lasreader->close();

    delete lasreader;
  }

  if (projection_was_set)
  {
    free(geo_keys);
    if (geo_double_params)
    {
      free(geo_double_params);
    }
  }

  if (verbose && lasreadopener.get_file_name_number() > 1) fprintf(stderr,"needed %g sec for %u files\n", taketime()-total_start_time, lasreadopener.get_file_name_number());

  byebye(false, argc==1);

  return 0;
}
