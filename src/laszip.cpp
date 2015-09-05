/*
===============================================================================

  FILE:  laszip.cpp
  
  CONTENTS:
  
    This tool compresses and uncompresses LiDAR data in the LAS format to our
    losslessly compressed LAZ format.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    29 March 2015 -- using LASwriterCompatible for LAS 1.4 compatibility mode  
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
#include "laswritercompatible.hpp"
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
  BOOL remain_compatible = FALSE;
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
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
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
    else if (strcmp(argv[i],"-remain_compatible") == 0)
    {
      remain_compatible = TRUE;
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

    // switch

    if (report_file_size)
    {
      // maybe only report uncompressed file size
      I64 uncompressed_file_size = lasreader->npoints * lasreader->header.point_data_record_length + lasreader->header.offset_to_point_data;
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

      LASwriter* laswriter = 0;
      
      if (lasreader->header.point_data_format > 5)
      {
        LASwriterCompatibleDown* laswritercompatibledown = new LASwriterCompatibleDown();
        if (laswritercompatibledown->open(&lasreader->header, &laswriteopener))
        {
          laswriter = laswritercompatibledown;
        }
        else
        {
          delete laswritercompatibledown;
          fprintf(stderr, "ERROR: could not open laswritercompatibledown\n");
        }
      }
      else if (!remain_compatible && (lasreader->header.point_data_format != 0) && (lasreader->header.point_data_format != 2) && lasreader->header.get_vlr("lascompatible", 22204) && (lasreader->header.get_attribute_index("LAS 1.4 scan angle") >= 0) && (lasreader->header.get_attribute_index("LAS 1.4 extended returns") >= 0) && (lasreader->header.get_attribute_index("LAS 1.4 classification") >= 0) && (lasreader->header.get_attribute_index("LAS 1.4 flags and channel") >= 0))
      {
        LASwriterCompatibleUp* laswritercompatibleup = new LASwriterCompatibleUp();
        if (laswritercompatibleup->open(&lasreader->header, &laswriteopener))
        {
          laswriter = laswritercompatibleup;
        }
        else
        {
          delete laswritercompatibleup;
          fprintf(stderr, "ERROR: could not open laswritercompatibleup\n");
        }
      }
      else
      {
        laswriter = laswriteopener.open(&lasreader->header);
      }

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
            lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(laswriter->p_count));
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
              lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(laswriter->p_count));
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
              while (lasreader->read_point())
              {
                laswriter->write_point(&lasreader->point);
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
              lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(laswriter->p_count));
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
