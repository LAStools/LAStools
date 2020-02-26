/*
===============================================================================

  FILE:  las2las.cpp
  
  CONTENTS:
  
    This tool reads and writes LIDAR data in the LAS format and is typically
    used to modify the contents of a LAS file. Examples are keeping or dropping
    those points lying within a certain region (-keep_xy, -drop_x_above, ...),
    or points with a certain elevation (-keep_z, -drop_z_below, -drop_z_above)
    eliminating points that are the second return (-drop_return 2), that have a
    scan angle above a certain threshold (-drop_scan_angle_above 5), or that are
    below a certain intensity (-drop_intensity_below 15).
    Another typical use may be to extract only first (-first_only) returns or
    only last returns (-last_only). Extracting the first return is actually the
    same as eliminating all others (e.g. -keep_return 2 -keep_return 3, ...).

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2019, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     9 September 2019 -- warn if modifying x or y coordinates for tiles with VLR
    30 November 2017 -- set OGC WKT with '-set_ogc_wkt "PROJCS[\"WGS84\",GEOGCS[\"GCS_ ..."
    10 October 2017 -- allow piped input *and* output if no filter or coordinate change 
    14 July 2017 -- fixed missing 'comma' in compound (COMPD_CS) OGC WKT string
    23 October 2016 -- OGC WKT string stores COMPD_CS for projection + vertical
    22 October 2016 -- new '-set_ogc_wkt_in_evlr' store to EVLR instead of VLR
     1 January 2016 -- option '-set_ogc_wkt' to store CRS as OGC WKT string
     3 May 2015 -- improved up-conversion via '-set_version 1.4 -set_point_type 6'
     5 July 2012 -- added option to '-remove_original_vlr' 
     6 May 2012 -- added option to '-remove_tiling_vlr' 
     5 January 2012 -- added option to clip points to the bounding box
    17 May 2011 -- enabling batch processing with wildcards or multiple file names
    13 May 2011 -- moved indexing, filtering, transforming into LASreader
    18 April 2011 -- can set projection tags or reproject horizontally
    26 January 2011 -- added LAStransform for simply manipulations of points 
    21 January 2011 -- added LASreadOpener and reading of multiple LAS files 
     3 January 2011 -- added -reoffset & -rescale + -keep_circle via LASfilter
    10 January 2010 -- added -subseq for selecting a [start, end] interval
    10 June 2009 -- added -scale_rgb_down and -scale_rgb_up to scale rgb values
    12 March 2009 -- updated to ask for input if started without arguments 
     9 March 2009 -- added ability to remove user defined headers or vlrs
    17 September 2008 -- updated to deal with LAS format version 1.2
    17 September 2008 -- dropping or keeping in double precision and based on z
    10 July 2007 -- created after talking with Linda about the H1B process
  
===============================================================================
*/

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lastransform.hpp"
#include "geoprojectionconverter.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamin_file.hpp"

static void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"las2las -i *.las -utm 13N\n");
  fprintf(stderr,"las2las -i *.laz -first_only -olaz\n");
  fprintf(stderr,"las2las -i *.las -drop_return 4 5 -olaz\n");
  fprintf(stderr,"las2las -latlong -target_utm 12T -i in.las -o out.las\n");
  fprintf(stderr,"las2las -i in.laz -target_epsg 2972 -o out.laz\n");
  fprintf(stderr,"las2las -set_point_type 0 -lof file_list.txt -merged -o out.las\n");
  fprintf(stderr,"las2las -remove_vlr 2 -scale_rgb_up -i in.las -o out.las\n");
  fprintf(stderr,"las2las -i in.las -keep_xy 630000 4834500 630500 4835000 -keep_z 10 100 -o out.las\n");
  fprintf(stderr,"las2las -i in.txt -iparse xyzit -keep_circle 630200 4834750 100 -oparse xyzit -o out.txt\n");
  fprintf(stderr,"las2las -i in.las -remove_padding -keep_scan_angle -15 15 -o out.las\n");
  fprintf(stderr,"las2las -i in.las -rescale 0.01 0.01 0.01 -reoffset 0 300000 0 -o out.las\n");
  fprintf(stderr,"las2las -i in.las -set_version 1.2 -keep_gpstime 46.5 47.5 -o out.las\n");
  fprintf(stderr,"las2las -i in.las -drop_intensity_below 10 -olaz -stdout > out.laz\n");
  fprintf(stderr,"las2las -i in.las -last_only -drop_gpstime_below 46.75 -otxt -oparse xyzt -stdout > out.txt\n");
  fprintf(stderr,"las2las -i in.las -remove_all_vlrs -keep_class 2 3 4 -olas -stdout > out.las\n");
  fprintf(stderr,"las2las -h\n");
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

static bool save_vlrs_to_file(const LASheader* header)
{
  U32 i;
  FILE* file = fopen("vlrs.vlr", "wb");
  if (file == 0)
  {
    return false;
  }
  ByteStreamOut* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);
  // write number of VLRs
  if (!out->put32bitsLE((U8*)&(header->number_of_variable_length_records)))
  {
    fprintf(stderr,"ERROR: writing header->number_of_variable_length_records\n");
    return false;
  }
  // loop over VLRs
  for (i = 0; i < header->number_of_variable_length_records; i++)
  {
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].reserved)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].reserved\n", i);
      return false;
    }
    if (!out->putBytes((U8*)header->vlrs[i].user_id, 16))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].user_id\n", i);
      return false;
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_id)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].record_id\n", i);
      return false;
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_length_after_header)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].record_length_after_header\n", i);
      return false;
    }
    if (!out->putBytes((U8*)header->vlrs[i].description, 32))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].description\n", i);
      return false;
    }

    // write the data following the header of the variable length record

    if (header->vlrs[i].record_length_after_header)
    {
      if (header->vlrs[i].data)
      {
        if (!out->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          fprintf(stderr,"ERROR: writing %d bytes of data from header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
          return false;
        }
      }
      else
      {
        fprintf(stderr,"ERROR: there should be %d bytes of data in header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
        return false;
      }
    }
  }
  delete out;
  fclose(file);
  return true;
}

static bool load_vlrs_from_file(LASheader* header)
{
  U32 i;
  FILE* file = fopen("vlrs.vlr", "rb");
  if (file == 0)
  {
    return false;
  }
  ByteStreamIn* in;
  if (IS_LITTLE_ENDIAN())
    in = new ByteStreamInFileLE(file);
  else
    in = new ByteStreamInFileBE(file);
  // read number of VLRs
  U32 number_of_variable_length_records;
  try { in->get32bitsLE((U8*)&number_of_variable_length_records); } catch (...)
  {
    fprintf(stderr,"ERROR: reading number_of_variable_length_records\n");
    return false;
  }
  // loop over VLRs
  LASvlr vlr;
  for (i = 0; i < number_of_variable_length_records; i++)
  {
    try { in->get16bitsLE((U8*)&(vlr.reserved)); } catch (...)
    {
      fprintf(stderr,"ERROR: reading vlr.reserved\n");
      return false;
    }
    try { in->getBytes((U8*)vlr.user_id, 16); } catch (...)
    {
      fprintf(stderr,"ERROR: reading vlr.user_id\n");
      return false;
    }
    try { in->get16bitsLE((U8*)&(vlr.record_id)); } catch (...)
    {
      fprintf(stderr,"ERROR: reading vlr.record_id\n");
      return false;
    }
    try { in->get16bitsLE((U8*)&(vlr.record_length_after_header)); } catch (...)
    {
      fprintf(stderr,"ERROR: reading vlr.record_length_after_header\n");
      return false;
    }
    try { in->getBytes((U8*)vlr.description, 32); } catch (...)
    {
      fprintf(stderr,"ERROR: reading vlr.description\n");
      return false;
    }

    // write the data following the header of the variable length record

    if (vlr.record_length_after_header)
    {
      vlr.data = new U8[vlr.record_length_after_header];
      try { in->getBytes((U8*)vlr.data, vlr.record_length_after_header); } catch (...)
      {
        fprintf(stderr,"ERROR: reading %d bytes into vlr.data\n", vlr.record_length_after_header);
        return false;
      }
    }
    else
    {
      vlr.data = 0;
    }
    header->add_vlr(vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.data, TRUE, vlr.description);
  }
  delete in;
  fclose(file);
  return true;
}

// for point type conversions
const U8 convert_point_type_from_to[11][11] = 
{
  {  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  0,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  0,  1,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 },
};

#ifdef COMPILE_WITH_GUI
extern int las2las_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int las2las_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores, BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
  BOOL cpu64 = FALSE;
#endif
  bool verbose = false;
  bool very_verbose = false;
  bool force = false;
  // fixed header changes 
  int set_version_major = -1;
  int set_version_minor = -1;
  int set_point_data_format = -1;
  int set_point_data_record_length = -1;
  int set_global_encoding_gps_bit = -1;
  int set_lastiling_buffer_flag = -1;
  // variable header changes
  bool set_ogc_wkt = false;
  bool set_ogc_wkt_in_evlr = false;
  CHAR* set_ogc_wkt_string = 0;
  bool remove_header_padding = false;
  bool remove_all_variable_length_records = false;
  int remove_variable_length_record = -1;
  int remove_variable_length_record_from = -1;
  int remove_variable_length_record_to = -1;
  bool remove_all_extended_variable_length_records = false;
  int remove_extended_variable_length_record = -1;
  int remove_extended_variable_length_record_from = -1;
  int remove_extended_variable_length_record_to = -1;
  bool move_evlrs_to_vlrs = false;
  bool save_vlrs = false;
  bool load_vlrs = false;
  int set_attribute_scales = 0;
  int set_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
  double set_attribute_scale_scale[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
  int set_attribute_offsets = 0;
  int set_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
  double set_attribute_offset_offset[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
  int unset_attribute_scales = 0;
  int unset_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
  int unset_attribute_offsets = 0;
  int unset_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
  bool remove_tiling_vlr = false;
  bool remove_original_vlr = false;
  bool remove_empty_files = true;
  // extract a subsequence
  I64 subsequence_start = 0;
  I64 subsequence_stop = I64_MAX;
  // fix files with corrupt points
  bool clip_to_bounding_box = false;
  double start_time = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return las2las_gui(argc, argv, 0);
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
      if (strcmp(argv[i],"-week_to_adjusted") == 0)
      {
        set_global_encoding_gps_bit = 1;
      }
      else if (strcmp(argv[i],"-adjusted_to_week") == 0)
      {
        set_global_encoding_gps_bit = 0;
      }
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
      verbose = true;
    }
    else if (strcmp(argv[i],"-vv") == 0 || strcmp(argv[i],"-very_verbose") == 0)
    {
      very_verbose = true;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-fail") == 0)
    {
    }
    else if (strcmp(argv[i],"-gui") == 0)
    {
#ifdef COMPILE_WITH_GUI
      gui = true;
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
      if (sscanf(argv[i+1],"%u",&cores) != 1)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
        usage(true);
      }
      argv[i][0] = '\0';
      i++;
      argv[i][0] = '\0';
#else
      fprintf(stderr, "WARNING: not compiled with multi-core batching. ignoring '-cores' ...\n");
      i++;
#endif
    }
    else if (strcmp(argv[i],"-cpu64") == 0)
    {
#ifdef COMPILE_WITH_MULTI_CORE
      cpu64 = TRUE;
#else
      fprintf(stderr, "WARNING: not compiled with 64 bit support. ignoring '-cpu64' ...\n");
#endif
      argv[i][0] = '\0';
    }
    else if (strcmp(argv[i],"-force") == 0)
    {
      force = true;
    }
    else if (strcmp(argv[i],"-subseq") == 0)
    {
      if ((i+2) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1],"%I64d",&subsequence_start) != 1)
#else
      if (sscanf(argv[i+1],"%lld",&subsequence_start) != 1)
#endif // _WIN32
      {
        fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i+1], argv[i]);
        usage(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+2],"%I64d",&subsequence_stop) != 1)
#else
      if (sscanf(argv[i+2],"%lld",&subsequence_stop) != 1)
#endif // _WIN32
      {
        fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i+2], argv[i]);
        usage(true);
      }
      i+=2;
    }
    else if (strcmp(argv[i],"-start_at_point") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: index of start point\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1],"%I64d",&subsequence_start) != 1)
#else
      if (sscanf(argv[i+1],"%lld",&subsequence_start) != 1)
#endif // _WIN32
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
        usage(true);
      }
      i+=1;
    }
    else if (strcmp(argv[i],"-stop_at_point") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: index of stop point\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1],"%I64d",&subsequence_stop) != 1)
#else
      if (sscanf(argv[i+1],"%lld",&subsequence_stop) != 1)
#endif // _WIN32
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
        usage(true);
      }
      i+=1;
    }
    else if (strncmp(argv[i],"-set_", 5) == 0)
    {
      if (strncmp(argv[i],"-set_point_", 11) == 0)
      {
        if (strcmp(argv[i],"-set_point_type") == 0 || strcmp(argv[i],"-set_point_data_format") == 0) 
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: type\n", argv[i]);
            byebye(true);
          }
          if (sscanf(argv[i+1],"%u",&set_point_data_format) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          i++;
        }
        else if (strcmp(argv[i],"-set_point_data_record_length") == 0 || strcmp(argv[i],"-set_point_size") == 0) 
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
            byebye(true);
          }
          if (sscanf(argv[i+1],"%u",&set_point_data_record_length) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          i++;
        }
        else
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
          usage(true);
        }
      }
      else if (strcmp(argv[i],"-set_global_encoding_gps_bit") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1],"%u",&set_global_encoding_gps_bit) != 1)
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
          usage(true);
        }
        i+=1;
      }
      else if (strcmp(argv[i],"-set_version") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: major.minor\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1],"%u.%u",&set_version_major,&set_version_minor) != 2)
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
          usage(true);
        }
        i+=1;
      }
      else if (strcmp(argv[i],"-set_version_major") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: major\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1],"%u",&set_version_major) != 1)
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
          usage(true);
        }
        i+=1;
      }
      else if (strcmp(argv[i],"-set_version_minor") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: minor\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1],"%u",&set_version_minor) != 1)
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
          usage(true);
        }
        i+=1;
      }
      else if (strcmp(argv[i],"-set_lastiling_buffer_flag") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1],"%u",&set_lastiling_buffer_flag) != 1)
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
          usage(true);
        }
        if (set_lastiling_buffer_flag > 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
          byebye(true);
        }
        i+=1;
      }
      else if (strncmp(argv[i],"-set_ogc_wkt", 12) == 0)
      {
        if (strcmp(argv[i],"-set_ogc_wkt") == 0)
        {
          set_ogc_wkt = true;
          set_ogc_wkt_in_evlr = false;
        }
        else if (strcmp(argv[i],"-set_ogc_wkt_in_evlr") == 0)
        {
          set_ogc_wkt = true;
          set_ogc_wkt_in_evlr = true;
        }
        else
        {
          fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
          usage(true);
        }
        if ((i+1) < argc)
        {
          if ((argv[i+1][0] != '-') && (argv[i+1][0] != '\0'))
          {
            set_ogc_wkt_string = argv[i+1];
            i++;
          }
        }
      }
      else if (strcmp(argv[i],"-set_attribute_scale") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: index scale\n", argv[i]);
          byebye(true);
        }
        if (set_attribute_scales < 5)
        {
          if (sscanf(argv[i+1],"%u",&(set_attribute_scale_index[set_attribute_scales])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          if (sscanf(argv[i+2],"%lf",&(set_attribute_scale_scale[set_attribute_scales])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i+2], argv[i]);
            usage(true);
          }
          set_attribute_scales++;
        }
        else
        {
          fprintf(stderr,"ERROR: cannot '%s' more than 5 times\n", argv[i]);
          byebye(true);
        }
        i+=2;
      }
      else if (strcmp(argv[i],"-set_attribute_offset") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: index offset\n", argv[i]);
          byebye(true);
        }
        if (set_attribute_offsets < 5)
        {
          if (sscanf(argv[i+1],"%u",&(set_attribute_offset_index[set_attribute_offsets])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          if (sscanf(argv[i+2],"%lf",&(set_attribute_offset_offset[set_attribute_offsets])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i+2], argv[i]);
            usage(true);
          }
        }
        else
        {
          fprintf(stderr,"ERROR: cannot '%s' more than 5 times\n", argv[i]);
          byebye(true);
        }
        i+=2;
      }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        usage(true);
      }
    }
    else if (strncmp(argv[i],"-remove_", 8) == 0)
    {
      if (strcmp(argv[i],"-remove_padding") == 0)
      {
        remove_header_padding = true;
      }
      else if (strcmp(argv[i],"-remove_all_vlrs") == 0)
      {
        remove_all_variable_length_records = true;
      }
      else if (strcmp(argv[i],"-remove_vlr") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          byebye(true);
        }
        remove_variable_length_record = atoi(argv[i+1]);
        remove_variable_length_record_from = -1;
        remove_variable_length_record_to = -1;
        i++;
      }
      else if (strcmp(argv[i],"-remove_vlrs_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: start end\n", argv[i]);
          byebye(true);
        }
        remove_variable_length_record = -1;
        remove_variable_length_record_from = atoi(argv[i+1]);
        remove_variable_length_record_to = atoi(argv[i+2]);
        i+=2;
      }
      else if (strcmp(argv[i],"-remove_all_evlrs") == 0)
      {
        remove_all_extended_variable_length_records = true;
      }
      else if (strcmp(argv[i],"-remove_evlr") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          byebye(true);
        }
        remove_extended_variable_length_record = atoi(argv[i+1]);
        remove_extended_variable_length_record_from = -1;
        remove_extended_variable_length_record_to = -1;
        i++;
      }
      else if (strcmp(argv[i],"-remove_evlrs_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: start end\n", argv[i]);
          byebye(true);
        }
        remove_extended_variable_length_record = -1;
        remove_extended_variable_length_record_from = atoi(argv[i+1]);
        remove_extended_variable_length_record_to = atoi(argv[i+2]);
        i+=2;
      }
      else if (strcmp(argv[i],"-remove_tiling_vlr") == 0)
      {
        remove_tiling_vlr = true;
      }
      else if (strcmp(argv[i],"-remove_original_vlr") == 0)
      {
        remove_original_vlr = true;
      }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        usage(true);
      }
    }
    else if (strncmp(argv[i],"-add_", 5) == 0)
    {
      if (strcmp(argv[i],"-add_attribute") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs at least 3 arguments: data_type name description\n", argv[i]);
          usage(true);
        }
        if (((i+4) < argc) && (atof(argv[i+4]) != 0.0))
        {
          if (((i+5) < argc) && ((atof(argv[i+5]) != 0.0) || (strcmp(argv[i+5], "0") == 0) || (strcmp(argv[i+5], "0.0") == 0)))
          {
            if (((i+6) < argc) && ((atof(argv[i+6]) != 0.0) || (strcmp(argv[i+6], "0") == 0) || (strcmp(argv[i+6], "0.0") == 0)))
            {
              lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), 1.0, 0.0, atof(argv[i+6]));
              i+=6;
            }
            else
            {
              lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]));
              i+=5;
            }
          }
          else
          {
            lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]));
            i+=4;
          }
        }
        else
        {
          lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3]);
          i+=3;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        usage(true);
      }
    }
    else if (strncmp(argv[i],"-unset_", 7) == 0)
    {
      if (strcmp(argv[i],"-unset_attribute_scale") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
          byebye(true);
        }
        if (unset_attribute_scales < 5)
        {
          if (sscanf(argv[i+1],"%u",&(unset_attribute_scale_index[unset_attribute_scales])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          unset_attribute_scales++;
        }
        else
        {
          fprintf(stderr,"ERROR: cannot '%s' more than 5 times\n", argv[i]);
          byebye(true);
        }
        i+=1;
      }
      else if (strcmp(argv[i],"-unset_attribute_offset") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
          byebye(true);
        }
        if (unset_attribute_offsets < 5)
        {
          if (sscanf(argv[i+1],"%u",&(unset_attribute_offset_index[unset_attribute_offsets])) != 1)
          {
            fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i+1], argv[i]);
            usage(true);
          }
          unset_attribute_offsets++;
        }
        else
        {
          fprintf(stderr,"ERROR: cannot '%s' more than 5 times\n", argv[i]);
          byebye(true);
        }
        i+=1;
      }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-move_evlrs_to_vlrs") == 0)
    {
      move_evlrs_to_vlrs = true;
    }
    else if (strcmp(argv[i],"-save_vlrs") == 0)
    {
      save_vlrs = true;
    }
    else if (strcmp(argv[i],"-load_vlrs") == 0)
    {
      load_vlrs = true;
    }
    else if (strcmp(argv[i],"-dont_remove_empty_files") == 0)
    {
      remove_empty_files = false;
    }
    else if (strcmp(argv[i],"-clip_to_bounding_box") == 0 || strcmp(argv[i],"-clip_to_bb") == 0) 
    {
      clip_to_bounding_box = true;
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
    return las2las_gui(argc, argv, &lasreadopener);
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
      return las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores, cpu64);
    }
  }
  if (cpu64)
  {
    return las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(true, argc==1);
  }
  
  BOOL extra_pass = laswriteopener.is_piped();

  // we only really need an extra pass if the coordinates are altered or if points are filtered

  if (extra_pass)
  {
    if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
    {
      extra_pass = FALSE;
    }
  }

  // for piped output we need an extra pass

  if (extra_pass)
  {
    if (lasreadopener.is_piped())
    {
      fprintf(stderr, "ERROR: input and output cannot both be piped\n");
      usage(true);
    }
  }

  // only save or load

  if (save_vlrs && load_vlrs)
  {
    fprintf(stderr, "ERROR: cannot save and load VLRs at the same time\n");
    usage(true);
  }
    
  // possibly loop over multiple input files

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

    // store the inventory for the header

    LASinventory lasinventory;

    // the point we write sometimes needs to be copied

    LASpoint* point = 0;

    // prepare the header for output

    if (set_global_encoding_gps_bit != -1)
    {
      if (set_global_encoding_gps_bit == 0)
      {
        if ((lasreader->header.global_encoding & 1) == 0)
        {
          fprintf(stderr, "WARNING: global encoding indicates file already in GPS week time\n");
          if (force)
          {
            fprintf(stderr, "         forced conversion.\n");
          }
          else
          {
            fprintf(stderr, "         use '-force' to force conversion.\n");
            byebye(true);
          }
        }
        else
        {
          lasreader->header.global_encoding &= ~1;
        }
      }
      else if (set_global_encoding_gps_bit == 1)
      {
        if ((lasreader->header.global_encoding & 1) == 1)
        {
          fprintf(stderr, "WARNING: global encoding indicates file already in Adjusted Standard GPS time\n");
          if (force)
          {
            fprintf(stderr, "         forced conversion.\n");
          }
          else
          {
            fprintf(stderr, "         use '-force' to force conversion.\n");
            byebye(true);
          }
        }
        else
        {
          lasreader->header.global_encoding |= 1;
        }
      }
      else
      {
        fprintf(stderr, "WARNING: ignoring invalid option '-set_global_encoding_gps_bit %d'\n", set_global_encoding_gps_bit);
      }
    }

    if (set_attribute_scales)
    {
      for (i = 0; i < set_attribute_scales; i++)
      {
        if (set_attribute_scale_index[i] != -1)
        {
          if (set_attribute_scale_index[i] < lasreader->header.number_attributes)
          {
            lasreader->header.attributes[set_attribute_scale_index[i]].set_scale(set_attribute_scale_scale[i]);
          }
          else
          {
            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", set_attribute_scale_index[i], lasreader->header.number_attributes);
          }
        }
      }
    }

    if (set_attribute_offsets)
    {
      for (i = 0; i < set_attribute_offsets; i++)
      {
        if (set_attribute_offset_index[i] != -1)
        {
          if (set_attribute_offset_index[i] < lasreader->header.number_attributes)
          {
            lasreader->header.attributes[set_attribute_offset_index[i]].set_offset(set_attribute_offset_offset[i]);
          }
          else
          {
            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", set_attribute_offset_index[i], lasreader->header.number_attributes);
          }
        }
      }
    }

    if (unset_attribute_scales)
    {
      for (i = 0; i < unset_attribute_scales; i++)
      {
        if (unset_attribute_scale_index[i] != -1)
        {
          if (unset_attribute_scale_index[i] < lasreader->header.number_attributes)
          {
            lasreader->header.attributes[unset_attribute_scale_index[i]].unset_scale();
          }
          else
          {
            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", unset_attribute_scale_index[i], lasreader->header.number_attributes);
          }
        }
      }
    }

    if (unset_attribute_offsets)
    {
      for (i = 0; i < unset_attribute_offsets; i++)
      {
        if (unset_attribute_offset_index[i] != -1)
        {
          if (unset_attribute_offset_index[i] < lasreader->header.number_attributes)
          {
            lasreader->header.attributes[unset_attribute_offset_index[i]].unset_offset();
          }
          else
          {
            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", unset_attribute_offset_index[i], lasreader->header.number_attributes);
          }
        }
      }
    }

    if (set_attribute_scales || set_attribute_offsets || unset_attribute_scales || unset_attribute_offsets)
    {
      lasreader->header.update_extra_bytes_vlr();
    }

    if (set_point_data_format > 5)
    {
      if (set_version_minor == -1)
      {
        set_version_minor = 4;
      }
    }

    if (set_version_major != -1)
    {
      if (set_version_major != 1)
      {
        fprintf(stderr, "ERROR: unknown version_major %d\n", set_version_major);
        byebye(true);
      }
      lasreader->header.version_major = (U8)set_version_major;
    }

    if (set_version_minor >= 0)
    {
      if (set_version_minor > 4)
      {
        fprintf(stderr, "ERROR: unknown version_minor %d\n", set_version_minor);
        byebye(true);
      }
      if (set_version_minor < 3)
      {
        if (lasreader->header.version_minor == 3)
        {
          lasreader->header.header_size -= 8;
          lasreader->header.offset_to_point_data -= 8;
        }
        else if (lasreader->header.version_minor >= 4)
        {
          lasreader->header.header_size -= (8 + 140);
          lasreader->header.offset_to_point_data -= (8 + 140);
        }
      }
      else if (set_version_minor == 3)
      {
        if (lasreader->header.version_minor < 3)
        {
          lasreader->header.header_size += 8;
          lasreader->header.offset_to_point_data += 8;
          lasreader->header.start_of_waveform_data_packet_record = 0;
        }
        else if (lasreader->header.version_minor >= 4)
        {
          lasreader->header.header_size -= 140;
          lasreader->header.offset_to_point_data -= 140;
        }
      }
      else if (set_version_minor == 4) 
      {
        if (lasreader->header.version_minor < 3)
        {
          lasreader->header.header_size += (8 + 140);
          lasreader->header.offset_to_point_data += (8 + 140);
          lasreader->header.start_of_waveform_data_packet_record = 0;
        }
        else if (lasreader->header.version_minor == 3)
        {
          lasreader->header.header_size += 140;
          lasreader->header.offset_to_point_data += 140;
        }

        if (lasreader->header.version_minor < 4)
        {
          if (set_point_data_format > 5)
          {
            lasreader->header.extended_number_of_point_records = lasreader->header.number_of_point_records;
            lasreader->header.number_of_point_records = 0;
            for (i = 0; i < 5; i++)
            {
              lasreader->header.extended_number_of_points_by_return[i] = lasreader->header.number_of_points_by_return[i];
              lasreader->header.number_of_points_by_return[i] = 0;
            }
          }
        }
      }

      if ((set_version_minor <= 3) && (lasreader->header.version_minor >= 4))
      {
        if (lasreader->header.point_data_format > 5)
        {
          switch (lasreader->header.point_data_format)
          {
          case 6:
            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 1\n", lasreader->header.point_data_format);
            lasreader->header.point_data_format = 1;
            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
            lasreader->header.point_data_record_length -= 2;
            break;
          case 7:
            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 3\n", lasreader->header.point_data_format);
            lasreader->header.point_data_format = 3;
            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
            lasreader->header.point_data_record_length -= 2;
            break;
          case 8:
            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 3\n", lasreader->header.point_data_format);
            lasreader->header.point_data_format = 3;
            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
            lasreader->header.point_data_record_length -= 4;
            break;
          case 9:
            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 4\n", lasreader->header.point_data_format);
            lasreader->header.point_data_format = 4;
            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
            lasreader->header.point_data_record_length -= 2;
            break;
          case 10:
            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 5\n", lasreader->header.point_data_format);
            lasreader->header.point_data_format = 5;
            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
            lasreader->header.point_data_record_length -= 4;
            break;
          default:
            fprintf(stderr, "ERROR: unknown point_data_format %d\n", lasreader->header.point_data_format);
            byebye(true);
          }
          point = new LASpoint;
          lasreader->header.clean_laszip();
        }
        if (lasreader->header.get_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS))
        {
          fprintf(stderr, "WARNING: unsetting global encoding bit %d when downgrading from version 1.%d to version 1.%d\n", LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS, lasreader->header.version_minor, set_version_minor);
          lasreader->header.unset_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
        }
        if (lasreader->header.number_of_extended_variable_length_records)
        {
          fprintf(stderr, "WARNING: loosing %d EVLR%s when downgrading from version 1.%d to version 1.%d\n         attempting to move %s to the VLR section ...\n", lasreader->header.number_of_extended_variable_length_records, (lasreader->header.number_of_extended_variable_length_records > 1 ? "s" : ""), lasreader->header.version_minor, set_version_minor, (lasreader->header.number_of_extended_variable_length_records > 1 ? "them" : "it"));

          U32 u;
          for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
          {
            if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
            {
              lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
              lasreader->header.evlrs[u].data = 0;
#ifdef _WIN32
              fprintf(stderr, "         moved EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
              fprintf(stderr, "         moved EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
            }
            else
            {
#ifdef _WIN32
              fprintf(stderr, "         lost EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
              fprintf(stderr, "         lost EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
            }
          }
        }
      }

      lasreader->header.version_minor = (U8)set_version_minor;
    }

    // are we supposed to change the point data format

    if (set_point_data_format != -1)
    {
      if (set_point_data_format < 0 || set_point_data_format > 10)
      {
        fprintf(stderr, "ERROR: unknown point_data_format %d\n", set_point_data_format);
        byebye(true);
      }
      // depending on the conversion we may need to copy the point
      if (convert_point_type_from_to[lasreader->header.point_data_format][set_point_data_format])
      {
        if (point == 0) point = new LASpoint;
      }
      // were there extra bytes before
      I32 num_extra_bytes = 0;
      switch (lasreader->header.point_data_format)
      {
      case 0:
        num_extra_bytes = lasreader->header.point_data_record_length - 20;
        break;
      case 1:
        num_extra_bytes = lasreader->header.point_data_record_length - 28;
        break;
      case 2:
        num_extra_bytes = lasreader->header.point_data_record_length - 26;
        break;
      case 3:
        num_extra_bytes = lasreader->header.point_data_record_length - 34;
        break;
      case 4:
        num_extra_bytes = lasreader->header.point_data_record_length - 57;
        break;
      case 5:
        num_extra_bytes = lasreader->header.point_data_record_length - 63;
        break;
      case 6:
        num_extra_bytes = lasreader->header.point_data_record_length - 30;
        break;
      case 7:
        num_extra_bytes = lasreader->header.point_data_record_length - 36;
        break;
      case 8:
        num_extra_bytes = lasreader->header.point_data_record_length - 38;
        break;
      case 9:
        num_extra_bytes = lasreader->header.point_data_record_length - 59;
        break;
      case 10:
        num_extra_bytes = lasreader->header.point_data_record_length - 67;
        break;
      }
      if (num_extra_bytes < 0)
      {
        fprintf(stderr, "ERROR: point record length has %d fewer bytes than needed\n", -num_extra_bytes);
        byebye(true);
      }
      lasreader->header.point_data_format = (U8)set_point_data_format;
      lasreader->header.clean_laszip();
      switch (lasreader->header.point_data_format)
      {
      case 0:
        lasreader->header.point_data_record_length = 20 + num_extra_bytes;
        break;
      case 1:
        lasreader->header.point_data_record_length = 28 + num_extra_bytes;
        break;
      case 2:
        lasreader->header.point_data_record_length = 26 + num_extra_bytes;
        break;
      case 3:
        lasreader->header.point_data_record_length = 34 + num_extra_bytes;
        break;
      case 4:
        lasreader->header.point_data_record_length = 57 + num_extra_bytes;
        break;
      case 5:
        lasreader->header.point_data_record_length = 63 + num_extra_bytes;
        break;
      case 6:
        lasreader->header.point_data_record_length = 30 + num_extra_bytes;
        break;
      case 7:
        lasreader->header.point_data_record_length = 36 + num_extra_bytes;
        break;
      case 8:
        lasreader->header.point_data_record_length = 38 + num_extra_bytes;
        break;
      case 9:
        lasreader->header.point_data_record_length = 59 + num_extra_bytes;
        break;
      case 10:
        lasreader->header.point_data_record_length = 67 + num_extra_bytes;
        break;
      }
    }

    // are we supposed to change the point data record length

    if (set_point_data_record_length != -1)
    {
      I32 num_extra_bytes = 0;
      switch (lasreader->header.point_data_format)
      {
      case 0:
        num_extra_bytes = set_point_data_record_length - 20;
        break;
      case 1:
        num_extra_bytes = set_point_data_record_length - 28;
        break;
      case 2:
        num_extra_bytes = set_point_data_record_length - 26;
        break;
      case 3:
        num_extra_bytes = set_point_data_record_length - 34;
        break;
      case 4:
        num_extra_bytes = set_point_data_record_length - 57;
        break;
      case 5:
        num_extra_bytes = set_point_data_record_length - 63;
        break;
      case 6:
        num_extra_bytes = set_point_data_record_length - 30;
        break;
      case 7:
        num_extra_bytes = set_point_data_record_length - 36;
        break;
      case 8:
        num_extra_bytes = set_point_data_record_length - 38;
        break;
      case 9:
        num_extra_bytes = set_point_data_record_length - 59;
        break;
      case 10:
        num_extra_bytes = set_point_data_record_length - 67;
        break;
      }
      if (num_extra_bytes < 0)
      {
        fprintf(stderr, "ERROR: point_data_format %d needs record length of at least %d\n", lasreader->header.point_data_format, set_point_data_record_length - num_extra_bytes);
        byebye(true);
      }
      if (lasreader->header.point_data_record_length < set_point_data_record_length)
      {
        if (!point) point = new LASpoint;
      }
      lasreader->header.point_data_record_length = (U16)set_point_data_record_length;
      lasreader->header.clean_laszip();
    }

    // are we supposed to add attributes

    if (lasreadopener.get_number_attributes())
    {
      I32 attibutes_before_size = lasreader->header.get_attributes_size();
      for (i = 0; i < lasreadopener.get_number_attributes(); i++)
      {
        I32 type = (lasreadopener.get_attribute_data_type(i)-1)%10;
        try {
          LASattribute attribute(type, lasreadopener.get_attribute_name(i), lasreadopener.get_attribute_description(i));
          if (lasreadopener.get_attribute_scale(i) != 1.0 || lasreadopener.get_attribute_offset(i) != 0.0)
          {
            attribute.set_scale(lasreadopener.get_attribute_scale(i));
          }
          if (lasreadopener.get_attribute_offset(i) != 0.0)
          {
            attribute.set_offset(lasreadopener.get_attribute_offset(i));
          }
          if (lasreadopener.get_attribute_no_data(i) != F64_MAX)
          {
            attribute.set_no_data(lasreadopener.get_attribute_no_data(i));
          }
          lasreader->header.add_attribute(attribute);
        }
        catch(...) {
          fprintf(stderr, "ERROR: initializing attribute %s\n", lasreadopener.get_attribute_name(i));
          byebye(true);
        }
      }
      I32 attibutes_after_size = lasreader->header.get_attributes_size();
      if (!point) point = new LASpoint;
      lasreader->header.update_extra_bytes_vlr();
      lasreader->header.point_data_record_length += (attibutes_after_size-attibutes_before_size);
      lasreader->header.clean_laszip();
    }

    if (set_lastiling_buffer_flag != -1)
    {
      if (lasreader->header.vlr_lastiling)
      {
        lasreader->header.vlr_lastiling->buffer = set_lastiling_buffer_flag;
      }
      else
      {
        fprintf(stderr, "WARNING: file '%s' has no LAStiling VLR. cannot set buffer flag.\n", lasreadopener.get_file_name());
      }
    }

    if (move_evlrs_to_vlrs)
    {
      if (lasreader->header.number_of_extended_variable_length_records > 0)
      {
        U32 u;
        for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
        {
          if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
          {
            lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
            lasreader->header.evlrs[u].data = 0;
#ifdef _WIN32
            if (very_verbose) fprintf(stderr, "         moved EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
            if (very_verbose) fprintf(stderr, "         moved EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
          }
        }
        U32 remaining = 0;
        for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
        {
          if (lasreader->header.evlrs[u].record_length_after_header > U16_MAX)
          {
            lasreader->header.evlrs[remaining] = lasreader->header.evlrs[u];
            remaining++;
          }
        }
        if (verbose) fprintf(stderr, "moved %u EVLRs to VLRs. %u EVLRs with large payload remain.\n", u-remaining, remaining);
        lasreader->header.number_of_extended_variable_length_records = remaining;
      }
    }

    // if the point needs to be copied set up the data fields

    if (point)
    {
      point->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);
    }

    // reproject or just set the projection?

    LASquantizer* reproject_quantizer = 0;
    LASquantizer* saved_quantizer = 0;
    bool set_projection_in_header = false;

    if (geoprojectionconverter.has_projection(false)) // reproject because a target projection was provided in the command line
    {
      if (!geoprojectionconverter.has_projection(true))      // if no source projection was provided in the command line ...
      {
        if (lasreader->header.vlr_geo_ogc_wkt)               // ... try to get it from the OGC WKT string ...
        {
          geoprojectionconverter.set_projection_from_ogc_wkt(lasreader->header.vlr_geo_ogc_wkt);
        }
        if (!geoprojectionconverter.has_projection(true))    // ... nothing ... ? ...
        {
          if (lasreader->header.vlr_geo_keys)                // ... try to get it from the geo keys.
          {
            geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
          }
        }
      }
      if (!geoprojectionconverter.has_projection(true))
      {
        fprintf(stderr, "WARNING: cannot determine source projection of '%s'. not reprojecting ... \n", lasreadopener.get_file_name());

        set_projection_in_header = false;
      }
      else
      {
        geoprojectionconverter.check_horizontal_datum_before_reprojection();

        reproject_quantizer = new LASquantizer();
        double point[3];
        point[0] = (lasreader->header.min_x+lasreader->header.max_x)/2;
        point[1] = (lasreader->header.min_y+lasreader->header.max_y)/2;
        point[2] = (lasreader->header.min_z+lasreader->header.max_z)/2;
        geoprojectionconverter.to_target(point);
        reproject_quantizer->x_scale_factor = geoprojectionconverter.get_target_precision();
        reproject_quantizer->y_scale_factor = geoprojectionconverter.get_target_precision();
        reproject_quantizer->z_scale_factor = geoprojectionconverter.get_target_elevation_precision();
        reproject_quantizer->x_offset = ((I64)((point[0]/reproject_quantizer->x_scale_factor)/10000000))*10000000*reproject_quantizer->x_scale_factor;
        reproject_quantizer->y_offset = ((I64)((point[1]/reproject_quantizer->y_scale_factor)/10000000))*10000000*reproject_quantizer->y_scale_factor;
        reproject_quantizer->z_offset = ((I64)((point[2]/reproject_quantizer->z_scale_factor)/10000000))*10000000*reproject_quantizer->z_scale_factor;

        set_projection_in_header = true;
      }
    }
    else if (geoprojectionconverter.has_projection(true)) // set because only a source projection was provided in the command line
    {
      set_projection_in_header = true;
    }

    if (set_projection_in_header)
    {
      int number_of_keys;
      GeoProjectionGeoKeys* geo_keys = 0;
      int num_geo_double_params;
      double* geo_double_params = 0;

      if (geoprojectionconverter.get_geo_keys_from_projection(number_of_keys, &geo_keys, num_geo_double_params, &geo_double_params, !geoprojectionconverter.has_projection(false)))
      {
        lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
        free(geo_keys);
        if (geo_double_params)
        {
          lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
          free(geo_double_params);
        }
        else
        {
          lasreader->header.del_geo_double_params();
        }
        lasreader->header.del_geo_ascii_params();
        lasreader->header.del_geo_ogc_wkt();
      }

      if (set_ogc_wkt || (lasreader->header.point_data_format >= 6)) // maybe also set the OCG WKT
      {
        CHAR* ogc_wkt = set_ogc_wkt_string;
        I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);
        if (ogc_wkt == 0)
        { 
          if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt, !geoprojectionconverter.has_projection(false)))
          {
            fprintf(stderr, "WARNING: cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
            if (ogc_wkt) free(ogc_wkt);
            ogc_wkt = 0;
          }
        }
        if (ogc_wkt)
        {
          if (set_ogc_wkt_in_evlr)
          {
            if (lasreader->header.version_minor >= 4)
            {
              lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
            }
            else
            {
              fprintf(stderr, "WARNING: input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...\n", lasreader->header.version_minor);
              lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
            }
          }
          else
          {
            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt);
          }
          if (!set_ogc_wkt_string) free(ogc_wkt);
          if ((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6))
          {
            lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
          }
        }
      }
    }
    else if (set_ogc_wkt) // maybe only set the OCG WKT 
    {
      CHAR* ogc_wkt = set_ogc_wkt_string;
      I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);

      if (ogc_wkt == 0)
      {
        if (lasreader->header.vlr_geo_keys)
        {
          geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
          if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt))
          {
            fprintf(stderr, "WARNING: cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
            if (ogc_wkt) free(ogc_wkt);
            ogc_wkt = 0;
          }
        }
        else
        {
          fprintf(stderr, "WARNING: no projection information. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
        }
      }

      if (ogc_wkt)
      {
        if (set_ogc_wkt_in_evlr)
        {
          if (lasreader->header.version_minor >= 4)
          {
            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
          }
          else
          {
            fprintf(stderr, "WARNING: input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...\n", lasreader->header.version_minor);
            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
          }
        }
        else
        {
          lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
        }

        if (!set_ogc_wkt_string) free(ogc_wkt);

        if ((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6))
        {
          lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
        }
      }
    }

    // maybe we should remove some stuff

    if (remove_header_padding)
    {
      lasreader->header.clean_user_data_in_header();
      lasreader->header.clean_user_data_after_header();
    }

    if (remove_all_variable_length_records)
    {
      lasreader->header.clean_vlrs();
    }
    else
    {
      if (remove_variable_length_record != -1)
      {
        lasreader->header.remove_vlr(remove_variable_length_record);
      }
    
      if (remove_variable_length_record_from != -1)
      {
        for (i = remove_variable_length_record_to; i >= remove_variable_length_record_from; i--)
        {
          lasreader->header.remove_vlr(i);
        }
      }
    }

    if (remove_all_extended_variable_length_records)
    {
      lasreader->header.clean_evlrs();
    }
    else
    {
      if (remove_extended_variable_length_record != -1)
      {
        lasreader->header.remove_evlr(remove_extended_variable_length_record);
      }
    
      if (remove_extended_variable_length_record_from != -1)
      {
        for (i = remove_extended_variable_length_record_to; i >= remove_extended_variable_length_record_from; i--)
        {
          lasreader->header.remove_evlr(i);
        }
      }
    }

    if (remove_tiling_vlr)
    {
      lasreader->header.clean_lastiling();
    }

    if (remove_original_vlr)
    {
      lasreader->header.clean_lasoriginal();
    }

    if (lasreader->header.vlr_lastiling || lasreader->header.vlr_lasoriginal)
    {
      if (lasreader->get_transform()) 
      {
        if (lasreader->get_transform()->transformed_fields & (LASTRANSFORM_X_COORDINATE | LASTRANSFORM_Y_COORDINATE))
        {
          fprintf(stderr, "WARNING: transforming x or y coordinates of file with %s VLR invalidates this VLR\n", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
        }
      }
      if (reproject_quantizer) 
      {
        fprintf(stderr, "WARNING: reprojecting file with %s VLR invalidates this VLR\n", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
      }
    }

    if (save_vlrs)
    {
      save_vlrs_to_file(&lasreader->header);
      save_vlrs = false;
    }

    if (load_vlrs)
    {
      load_vlrs_from_file(&lasreader->header);
    }

    // do we need an extra pass

    BOOL extra_pass = laswriteopener.is_piped();

    // we only really need an extra pass if the coordinates are altered or if points are filtered

    if (extra_pass)
    {
      if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (reproject_quantizer == 0) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
      {
        extra_pass = FALSE;
      }
    }

    // for piped output we need an extra pass

    if (extra_pass)
    {
      if (lasreadopener.is_piped())
      {
        fprintf(stderr, "ERROR: input and output cannot both be piped\n");
        usage(true);
      }

#ifdef _WIN32
      if (verbose) fprintf(stderr, "extra pass for piped output: reading %I64d points ...\n", lasreader->npoints);
#else
      if (verbose) fprintf(stderr, "extra pass for piped output: reading %lld points ...\n", lasreader->npoints);
#endif

      // maybe seek to start position

      if (subsequence_start) lasreader->seek(subsequence_start);

      while (lasreader->read_point())
      {
        if (lasreader->p_count > subsequence_stop) break;

        if (clip_to_bounding_box)
        {
          if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
          {
            continue;
          }
        }

        if (reproject_quantizer)
        {
          lasreader->point.compute_coordinates();
          geoprojectionconverter.to_target(lasreader->point.coordinates);
          lasreader->point.compute_XYZ(reproject_quantizer);
        }
        lasinventory.add(&lasreader->point);
      }
      lasreader->close();

      if (reproject_quantizer) lasreader->header = *reproject_quantizer;

      lasinventory.update_header(&lasreader->header);

      if (verbose) { fprintf(stderr,"extra pass took %g sec.\n", taketime()-start_time); start_time = taketime(); }
#ifdef _WIN32
      if (verbose) fprintf(stderr, "piped output: reading %I64d and writing %I64d points ...\n", lasreader->npoints, lasinventory.extended_number_of_point_records);
#else
      if (verbose) fprintf(stderr, "piped output: reading %lld and writing %lld points ...\n", lasreader->npoints, lasinventory.extended_number_of_point_records);
#endif
    }
    else
    {
      if (reproject_quantizer)
      {
        saved_quantizer = new LASquantizer();
        *saved_quantizer = lasreader->header;
        lasreader->header = *reproject_quantizer;
      }
#ifdef _WIN32
      if (verbose) fprintf(stderr, "reading %I64d and writing all surviving points ...\n", lasreader->npoints);
#else
      if (verbose) fprintf(stderr, "reading %lld and writing all surviving points ...\n", lasreader->npoints);
#endif
    }

    // check output

    if (!laswriteopener.active())
    {
      // create name from input name
      laswriteopener.make_file_name(lasreadopener.get_file_name());
    }
    else
    {
      // make sure we do not corrupt the input file

      if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
      {
        fprintf(stderr, "ERROR: input and output file name are identical: '%s'\n", lasreadopener.get_file_name());
        usage(true);
      }
    }

    // prepare the header for the surviving points

    strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
    lasreader->header.system_identifier[31] = '\0';
    char temp[64];
#ifdef _WIN64
    sprintf(temp, "las2las64 (version %d)", LAS_TOOLS_VERSION);
#else // _WIN64
    sprintf(temp, "las2las (version %d)", LAS_TOOLS_VERSION);
#endif // _WIN64
    memset(lasreader->header.generating_software, 0, 32);
    strncpy(lasreader->header.generating_software, temp, 32);
    lasreader->header.generating_software[31] = '\0';

    // open laswriter

    LASwriter* laswriter = laswriteopener.open(&lasreader->header);

    if (laswriter == 0)
    {
      fprintf(stderr, "ERROR: could not open laswriter\n");
      byebye(true, argc==1);
    }

    // for piped output we need to re-open the input file

    if (extra_pass)
    {
      if (!lasreadopener.reopen(lasreader))
      {
        fprintf(stderr, "ERROR: could not re-open lasreader\n");
        byebye(true);
      }
    }
    else
    {
      if (reproject_quantizer)
      {
        lasreader->header = *saved_quantizer;
        delete saved_quantizer;
      }
    }

    // maybe seek to start position

    if (subsequence_start) lasreader->seek(subsequence_start);

    // loop over points

    if (point)
    {
      while (lasreader->read_point())
      {
        if (lasreader->p_count > subsequence_stop) break;

        if (clip_to_bounding_box)
        {
          if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
          {
            continue;
          }
        }

        if (reproject_quantizer)
        {
          lasreader->point.compute_coordinates();
          geoprojectionconverter.to_target(lasreader->point.coordinates);
          lasreader->point.compute_XYZ(reproject_quantizer);
        }
        *point = lasreader->point;
        laswriter->write_point(point);
        // without extra pass we need inventory of surviving points
        if (!extra_pass) laswriter->update_inventory(point);
      }
      delete point;
      point = 0;
    }
    else
    {
      while (lasreader->read_point())
      {
        if (lasreader->p_count > subsequence_stop) break;

        if (clip_to_bounding_box)
        {
          if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
          {
            continue;
          }
        }

        if (reproject_quantizer)
        {
          lasreader->point.compute_coordinates();
          geoprojectionconverter.to_target(lasreader->point.coordinates);
          lasreader->point.compute_XYZ(reproject_quantizer);
        }
        laswriter->write_point(&lasreader->point);
        // without extra pass we need inventory of surviving points
        if (!extra_pass) laswriter->update_inventory(&lasreader->point);
      }
    }

    // without the extra pass we need to fix the header now

    if (!extra_pass)
    {
      if (reproject_quantizer) lasreader->header = *reproject_quantizer;
      laswriter->update_header(&lasreader->header, TRUE);
      if (verbose) { fprintf(stderr,"total time: %g sec. written %u surviving points to '%s'.\n", taketime()-start_time, (U32)laswriter->p_count, laswriteopener.get_file_name()); }
    }
    else
    {
      if (verbose) { fprintf(stderr,"main pass took %g sec.\n", taketime()-start_time); }
    }

    laswriter->close();
    // delete empty output files
    if (remove_empty_files && (laswriter->npoints == 0) && laswriteopener.get_file_name())
    {        
      remove(laswriteopener.get_file_name());
      if (verbose) fprintf(stderr,"removing empty output file '%s'\n", laswriteopener.get_file_name());
    }
    delete laswriter;

    lasreader->close();
    delete lasreader;

    if (reproject_quantizer) delete reproject_quantizer;

    laswriteopener.set_file_name(0);
  }

  byebye(false, argc==1);

  return 0;
}
