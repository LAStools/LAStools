/*
===============================================================================

  FILE:  lasinfo.cpp

  CONTENTS:

    This tool reads a LIDAR file in LAS or LAZ format and prints out the info
    from the standard header. It also prints out info from any variable length
    headers and gives detailed info on any geokeys that may be present (this
    can be disabled with the '-no_vrls' flag). The real bounding box of the
    points is computed and compared with the bounding box specified in the header
    (this can be disabled with the '-no_check' flag). It is also possible to
    change or repair some aspects of the header 

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-13, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    14 October 2017 -- WARN when bounding box miss-matches coordinate resolution
    16 May 2015 -- new option '-set_GUID F794F8A4-A23E-421E-A134-ACF7754E1C54'
     9 July 2012 -- fixed crash that occured when input had a corrupt VLRs
     7 January 2012 -- set bounding box / file source id / point type & size / ...
     6 January 2012 -- make area/density optional
     5 September 2011 -- also compute total area covered and point densities
    26 January 2011 -- added LAStransform because it allows quick previews
    21 January 2011 -- added LASreadOpener and reading of multiple LAS files 
     4 January 2011 -- added the LASfilter to drop or keep points 
    10 July 2009 -- '-auto_date' sets the day/year from the file creation date
    12 March 2009 -- updated to ask for input if started without arguments 
     9 March 2009 -- added output for size of user-defined header data
    17 September 2008 -- updated to deal with LAS format version 1.2
    13 July 2007 -- added the option to "repair" the header and change items
    11 June 2007 -- fixed number of return counts after Vinton found another bug
     6 June 2007 -- added lidardouble2string() after Vinton Valentine's bug report
    25 March 2007 -- sitting at the Pacific Coffee after doing the escalators

===============================================================================
*/

#include "geoprojectionconverter.hpp"
#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "lasutility.hpp"
#include "laswriter.hpp"
#include "lasquadtree.hpp"
#include "lasvlrpayload.hpp"
#include "lasindex.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif

static const char * LASpointClassification [32] = {
  "never classified",
  "unclassified",
  "ground",
  "low vegetation",
  "medium vegetation",
  "high vegetation",
  "building",
  "noise",
  "keypoint",
  "water",
  "rail",
  "road surface",
  "overlap",
  "wire guard",
  "wire conductor",
  "tower",
  "wire connector",
  "bridge deck",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition"
};

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasinfo -i lidar.las\n");
  fprintf(stderr,"lasinfo -i lidar.las -compute_density -o lidar_info.txt\n");
  fprintf(stderr,"lasinfo -i *.las\n");
  fprintf(stderr,"lasinfo -i *.las -single -otxt\n");
  fprintf(stderr,"lasinfo -no_header -no_vlrs -i lidar.laz\n");
  fprintf(stderr,"lasinfo -nv -nc -stdout -i lidar.las\n");
  fprintf(stderr,"lasinfo -nv -nc -stdout -i *.laz -single | grep version\n");
  fprintf(stderr,"lasinfo -i *.laz -subseq 100000 100100 -histo user_data 8\n");
  fprintf(stderr,"lasinfo -i *.las -repair\n");
  fprintf(stderr,"lasinfo -i *.laz -repair_bb -set_file_creation 8 2007\n");
  fprintf(stderr,"lasinfo -i *.las -repair_counters -set_version 1.2\n");
  fprintf(stderr,"lasinfo -i *.laz -set_system_identifier \"hello world!\" -set_generating_software \"this is a test (-:\"\n");
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

static inline void VecUpdateMinMax3dv(double min[3], double max[3], const double v[3])
{
  if (v[0]<min[0]) min[0]=v[0]; else if (v[0]>max[0]) max[0]=v[0];
  if (v[1]<min[1]) min[1]=v[1]; else if (v[1]>max[1]) max[1]=v[1];
  if (v[2]<min[2]) min[2]=v[2]; else if (v[2]>max[2]) max[2]=v[2];
}

static inline void VecCopy3dv(double v[3], const double a[3])
{
  v[0] = a[0];
  v[1] = a[1];
  v[2] = a[2];
}

static int lidardouble2string(char* string, double value)
{
  int len;
  len = sprintf(string, "%.15f", value) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
  return len;
}

static I32 lidardouble2string(char* string, double value, double precision)
{
  if (precision == 0.1)
    sprintf(string, "%.1f", value);
  else if (precision == 0.01)
    sprintf(string, "%.2f", value);
  else if (precision == 0.001 || precision == 0.002 || precision == 0.005 || precision == 0.025) 
    sprintf(string, "%.3f", value);
  else if (precision == 0.0001 || precision == 0.0002 || precision == 0.0005 || precision == 0.0025)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00001 || precision == 0.00002 || precision == 0.00005 || precision == 0.00025)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000001)
    sprintf(string, "%.6f", value);
  else if (precision == 0.0000001)
    sprintf(string, "%.7f", value);
  else if (precision == 0.00000001)
    sprintf(string, "%.8f", value);
  else if (precision == 0.5)
    sprintf(string, "%.1f", value);
  else if (precision == 0.25)
    sprintf(string, "%.2f", value);
  else if (precision == 0.125)
    sprintf(string, "%.3f", value);
  else
    return lidardouble2string(string, value);
  return (I32)strlen(string)-1;
}

static bool valid_resolution(F64 coordinate, F64 offset, F64 scale_factor)
{
  F64 coordinate_without_offset = coordinate - offset;
  F64 fixed_precision_multiplier = coordinate_without_offset / scale_factor;
  I64 quantized_fixed_precision_multiplier = I64_QUANTIZE(fixed_precision_multiplier);
  if ((fabs(fixed_precision_multiplier - quantized_fixed_precision_multiplier)) < 0.001)
  {
    return true;
  }
  return false;
}

#ifdef COMPILE_WITH_GUI
extern int lasinfo_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int lasinfo_multi_core(int argc, char *argv[], LASreadOpener* lasreadopener, LAShistogram* lashistogram, LASwriteOpener* laswriteopener, int cores, BOOL cpu64);
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
  bool wait = false;
  bool verbose = false;
  bool no_header = false;
  bool no_variable_header = false;
  bool no_returns = false;
  bool no_min_max = false;
  bool no_warnings = false;
  bool check_points = true;
  bool compute_density = false;
  bool gps_week = false;
  bool check_outside = true;
  bool report_outside = false;
  bool suppress_z = false;
  bool suppress_classification = false;
  bool suppress_flags = false;
  bool suppress_intensity = false;
  bool suppress_user_data = false;
  bool suppress_point_source = false;
  bool suppress_scan_angle = false;
  bool suppress_RGB = false;
  bool suppress_extra_bytes = false;
  bool repair_bb = false;
  bool repair_counters = false;
  bool edit_header = false;
  I32 set_file_source_ID = -1;
  bool set_file_source_ID_from_point_source_ID = false;
  I32 set_global_encoding = -1;
  I64 set_project_ID_GUID_data_1 = -1;
  I32 set_project_ID_GUID_data_2 = -1;
  I32 set_project_ID_GUID_data_3 = -1;
  I32 set_project_ID_GUID_data_4a = -1;
  I64 set_project_ID_GUID_data_4b = -1;
  I8 set_version_major = -1;
  I8 set_version_minor = -1;
	I8* set_system_identifier = 0;
	I8* set_generating_software = 0;
	I32 set_creation_day = -1;
	I32 set_creation_year = -1;
  U16 set_header_size = 0;
  U32 set_offset_to_point_data = 0;
  I32 set_number_of_variable_length_records = -1;
  I32 set_point_data_format = -1;
  I32 set_point_data_record_length = -1;
  I32 set_number_of_point_records = -1;
  I32 set_number_of_points_by_return[5] = {-1, -1, -1, -1, -1};
  F64* set_bounding_box = 0;
  F64* set_offset = 0;
  F64* set_scale = 0;
  I64 set_start_of_waveform_data_packet_record = -1;
  I32 set_geotiff_epsg = -1;
  bool auto_date_creation = false;
  FILE* file_out = stderr;
  U32 horizontal_units = 0; 
  // extract a subsequence
  I64 subsequence_start = 0;
  I64 subsequence_stop = I64_MAX;
  U32 progress = 0;
  // rename
  CHAR* base_name = 0;

  LAShistogram lashistogram;
  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasinfo_gui(argc, argv, 0);
#else
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
#endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '–') argv[i][0] = '-';
    }
    if (!lashistogram.parse(argc, argv)) byebye(true);
    if (!lasreadopener.parse(argc, argv)) byebye(true);
    if (!geoprojectionconverter.parse(argc, argv)) byebye(true);
    if (!laswriteopener.parse(argc, argv)) byebye(true);
  }

  if (laswriteopener.is_piped())
  {
    file_out = stdout;
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0)
    {
      verbose = true;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-wait") == 0)
    {
      wait = true;
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
      argv[i][0] = '\0';
      i++;
      cores = atoi(argv[i]);
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
    else if (strcmp(argv[i],"-quiet") == 0)
    {
      file_out = 0;
    }
    else if (strcmp(argv[i],"-otxt") == 0)
    {
      laswriteopener.set_appendix("_info");
      laswriteopener.set_format("txt");
    }
    else if (strcmp(argv[i],"-nh") == 0 || strcmp(argv[i],"-no_header") == 0)
    {
      no_header = true;
    }
    else if (strcmp(argv[i],"-nv") == 0 || strcmp(argv[i],"-no_vlrs") == 0)
    {
      no_variable_header = true;
    }
    else if (strcmp(argv[i],"-nr") == 0 || strcmp(argv[i],"-no_returns") == 0)
    {
      no_returns = true;
    }
    else if (strcmp(argv[i],"-nmm") == 0 || strcmp(argv[i],"-no_min_max") == 0)
    {
      no_min_max = true;
    }
    else if (strcmp(argv[i],"-nw") == 0 || strcmp(argv[i],"-no_warnings") == 0)
    {
      no_warnings = true;
    }
    else if (strcmp(argv[i],"-nc") == 0 || strcmp(argv[i],"-no_check") == 0)
    {
      check_points = false;
    }
    else if (strcmp(argv[i],"-cd") == 0 || strcmp(argv[i],"-compute_density") == 0)
    {
      compute_density = true;
    }
    else if (strcmp(argv[i],"-gw") == 0 || strcmp(argv[i],"-gps_week") == 0)
    {
      gps_week = true;
    }
    else if (strcmp(argv[i],"-nco") == 0 || strcmp(argv[i],"-no_check_outside") == 0)
    {
      check_outside = false;
    }
    else if (strcmp(argv[i],"-ro") == 0 || strcmp(argv[i],"-report_outside") == 0)
    {
      report_outside = true;
      check_outside = true;
    }
    else if (strcmp(argv[i],"-subseq") == 0)
    {
      if ((i+2) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1], "%I64d", &subsequence_start) != 1)
#else
      if (sscanf(argv[i+1], "%lld", &subsequence_start) != 1)
#endif
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%s' is not a valid start\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (subsequence_start < 0)
      {
#ifdef _WIN32
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%I64d' is not a valid start\n", argv[i], subsequence_start);
#else
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%lld' is not a valid start\n", argv[i], subsequence_start);
#endif
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+2], "%I64d", &subsequence_stop) != 1)
#else
      if (sscanf(argv[i+2], "%lld", &subsequence_stop) != 1)
#endif
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%s' is not a valid stop\n", argv[i], argv[i+2]);
        byebye(true);
      }
      if (subsequence_stop < 0)
      {
#ifdef _WIN32
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%I64d' is not a valid stop\n", argv[i], subsequence_stop);
#else
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%lld' is not a valid stop\n", argv[i], subsequence_stop);
#endif
        byebye(true);
      }
      if (subsequence_start >= subsequence_stop)
      {
#ifdef _WIN32
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%I64d' and '%I64d' are no valid start and stop combination \n", argv[i], subsequence_start, subsequence_stop);
#else
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: start stop but '%lld' and '%lld' are no valid start and stop combination \n", argv[i], subsequence_start, subsequence_stop);
#endif
        byebye(true);
      }
      i+=2;
    }
    else if (strcmp(argv[i],"-start_at_point") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1], "%I64d", &subsequence_start) != 1)
#else
      if (sscanf(argv[i+1], "%lld", &subsequence_start) != 1)
#endif
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%s' is not a valid start\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (subsequence_start < 0)
      {
#ifdef _WIN32
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%I64d' is not a valid start\n", argv[i], subsequence_start);
#else
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%lld' is not a valid start\n", argv[i], subsequence_start);
#endif
        byebye(true);
      }
      i+=1;
    }
    else if (strcmp(argv[i],"-stop_at_point") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: stop\n", argv[i]);
        byebye(true);
      }
#ifdef _WIN32
      if (sscanf(argv[i+1], "%I64d", &subsequence_stop) != 1)
#else
      if (sscanf(argv[i+1], "%lld", &subsequence_stop) != 1)
#endif
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%s' is not a valid stop\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (subsequence_stop < 0)
      {
#ifdef _WIN32
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%I64d' is not a valid stop\n", argv[i], subsequence_stop);
#else
        fprintf(stderr,"ERROR: '%s' needs 1 argument: start but '%lld' is not a valid stop\n", argv[i], subsequence_stop);
#endif
        byebye(true);
      }
      i+=1;
    }
    else if (strncmp(argv[i],"-repair", 7) == 0)
    {
      if (strcmp(argv[i],"-repair") == 0)
      {
        repair_bb = true;
        repair_counters = true;
      }
      else if (strcmp(argv[i],"-repair_bb") == 0)
      {
        repair_bb = true;
      }
      else if (strcmp(argv[i],"-repair_counters") == 0)
      {
        repair_counters = true;
      }
    }
    else if (strcmp(argv[i],"-auto_date") == 0 || strcmp(argv[i],"-auto_creation_date") == 0 || strcmp(argv[i],"-auto_creation") == 0)
    {
      auto_date_creation = true;
    }
    else if (strncmp(argv[i],"-set_", 5) == 0)
    {
      if (strcmp(argv[i],"-set_file_source_ID") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1], "%u", &set_file_source_ID) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index but '%s' is no valid index\n", argv[i], argv[i+1]);
          byebye(true);
        }
        if (set_file_source_ID > U16_MAX)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index between 0 and %u but %u is out of range\n", argv[i], U16_MAX, set_file_source_ID);
          byebye(true);
        }
			  i++;
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_file_source_ID_from_point_source_ID") == 0)
      {
			  set_file_source_ID_from_point_source_ID = true;
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_GUID") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: value1\n", argv[i]);
          byebye(true);
        }
			  i++;
#ifdef _WIN32
        if (sscanf(argv[i], "%I64x-%x-%x-%x-%I64x", &set_project_ID_GUID_data_1, &set_project_ID_GUID_data_2, &set_project_ID_GUID_data_3, &set_project_ID_GUID_data_4a, &set_project_ID_GUID_data_4b) != 5)
#else
        if (sscanf(argv[i], "%llx-%x-%x-%x-%llx", &set_project_ID_GUID_data_1, &set_project_ID_GUID_data_2, &set_project_ID_GUID_data_3, &set_project_ID_GUID_data_4a, &set_project_ID_GUID_data_4b) != 5)
#endif
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs hexadecimal GUID in 'F794F8A4-A23E-421E-A134-ACF7754E1C54' format\n", argv[i]);
            byebye(true);
          }
        }
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_system_identifier") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: name\n", argv[i]);
          byebye(true);
        }
			  i++;
			  set_system_identifier = new I8[32];
        memset(set_system_identifier, 0, 32);
        strncpy(set_system_identifier, argv[i], 32);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_generating_software") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: name\n", argv[i]);
          byebye(true);
        }
			  i++;
			  set_generating_software = new I8[32];
        memset(set_generating_software, 0, 32);
        strncpy(set_generating_software, argv[i], 32);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_bb") == 0 || strcmp(argv[i],"-set_bounding_box") == 0)
      {
        if ((i+6) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z\n", argv[i]);
          byebye(true);
        }
			  set_bounding_box = new F64[6];
			  i++;
        set_bounding_box[1] = atof(argv[i]);
			  i++;
        set_bounding_box[3] = atof(argv[i]);
			  i++;
        set_bounding_box[5] = atof(argv[i]);
			  i++;
        set_bounding_box[0] = atof(argv[i]);
			  i++;
        set_bounding_box[2] = atof(argv[i]);
			  i++;
        set_bounding_box[4] = atof(argv[i]);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_offset") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i]);
          byebye(true);
        }
			  set_offset = new F64[3];
			  i++;
        set_offset[0] = atof(argv[i]);
			  i++;
        set_offset[1] = atof(argv[i]);
			  i++;
        set_offset[2] = atof(argv[i]);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_scale") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i]);
          byebye(true);
        }
			  set_scale = new F64[3];
			  i++;
        set_scale[0] = atof(argv[i]);
			  i++;
        set_scale[1] = atof(argv[i]);
			  i++;
        set_scale[2] = atof(argv[i]);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_global_encoding") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_global_encoding = atoi(argv[i]);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_version") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: major.minor\n", argv[i]);
          byebye(true);
        }
        i++;
        int major;
        int minor;
        if (sscanf(argv[i],"%d.%d",&major,&minor) != 2)
        {
          fprintf(stderr,"ERROR: cannot understand argument '%s' of '%s'\n", argv[i], argv[i-1]);
          usage();
        }
        set_version_major = (I8)major;
        set_version_minor = (I8)minor;
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_creation_date") == 0 || strcmp(argv[i],"-set_file_creation") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: day year\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_creation_day = (U16)atoi(argv[i]);
			  i++;
        set_creation_year = (U16)atoi(argv[i]);
        edit_header = true;
		  }
      else if (strcmp(argv[i],"-set_number_of_point_records") == 0 )
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_number_of_point_records = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_number_of_points_by_return") == 0 )
      {
        if ((i+5) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 5 arguments: ret1 ret2 ret3 ret4 ret5\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_number_of_points_by_return[0] = atoi(argv[i]);
			  i++;
        set_number_of_points_by_return[1] = atoi(argv[i]);
			  i++;
        set_number_of_points_by_return[2] = atoi(argv[i]);
			  i++;
        set_number_of_points_by_return[3] = atoi(argv[i]);
			  i++;
        set_number_of_points_by_return[4] = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_header_size") == 0 )
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_header_size = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_offset_to_point_data") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_offset_to_point_data = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_number_of_variable_length_records") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_number_of_variable_length_records = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_point_data_format") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: type\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_point_data_format = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_point_data_record_length") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_point_data_record_length = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_start_of_waveform_data_packet_record") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: start\n", argv[i]);
          byebye(true);
        }
			  i++;
        set_start_of_waveform_data_packet_record = atoi(argv[i]);
        edit_header = true;
      }
      else if (strcmp(argv[i],"-set_geotiff_epsg") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: code\n", argv[i]);
          byebye(true);
        }
        if (sscanf(argv[i+1], "%u", &set_geotiff_epsg) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: code but '%s' is no valid code\n", argv[i], argv[i+1]);
          byebye(true);
        }
        if (set_geotiff_epsg > U16_MAX)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: code between 0 and %u but %u is out of range\n", argv[i], U16_MAX, set_geotiff_epsg);
          byebye(true);
        }
			  i++;
        edit_header = true;
		  }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        byebye(TRUE);
      }
    }
    else if (strncmp(argv[i],"-suppress_", 10) == 0)
    {
      if (strcmp(argv[i],"-suppress_z") == 0)
      {
        suppress_z = true;
      }
      else if (strcmp(argv[i],"-suppress_classification") == 0)
      {
        suppress_classification = true;
      }
      else if (strcmp(argv[i],"-suppress_flags") == 0)
      {
        suppress_flags = true;
      }
      else if (strcmp(argv[i],"-suppress_intensity") == 0)
      {
        suppress_intensity = true;
      }
      else if (strcmp(argv[i],"-suppress_user_data") == 0)
      {
        suppress_user_data = true;
      }
      else if (strcmp(argv[i],"-suppress_point_source") == 0)
      {
        suppress_point_source = true;
      }
      else if (strcmp(argv[i],"-suppress_scan_angle") == 0)
      {
        suppress_scan_angle = true;
      }
      else if (strcmp(argv[i],"-suppress_RGB") == 0)
      {
        suppress_RGB = true;
      }
      else if (strcmp(argv[i],"-suppress_extra_bytes") == 0)
      {
        suppress_extra_bytes = true;
      }
      else
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
        byebye(TRUE);
      }
    }
    else if (strcmp(argv[i],"-rename") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: base name\n", argv[i]);
        byebye(true);
      }
			i++;
      base_name = argv[i];
    }
    else if (strcmp(argv[i],"-progress") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: every\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i+1], "%u", &progress) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: every but '%s' is no valid number\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (progress == 0)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: every but '%u' is no valid number\n", argv[i], progress);
        byebye(true);
      }
			i++;
    }
    else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      byebye(true);
    }
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return lasinfo_gui(argc, argv, &lasreadopener);
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
      return lasinfo_multi_core(argc, argv, &lasreadopener, &lashistogram, &laswriteopener, cores, cpu64);
    }
  }
  if (cpu64)
  {
    return lasinfo_multi_core(argc, argv, &lasreadopener, &lashistogram, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf (stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  // omit "suppressed" layers from LAZ decompression (for new LAS 1.4 point types only)

  U32 decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_ALL;

  if (suppress_z)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_Z;
  }

  if (suppress_classification)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION;
  }

  if (suppress_flags)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_FLAGS;
  }
  
  if (suppress_intensity)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_INTENSITY;
  }

  if (suppress_user_data)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_USER_DATA;
  }

  if (suppress_point_source)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE;
  }

  if (suppress_scan_angle)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE;
  }
  
  if (suppress_RGB)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_RGB;
  }

  if (suppress_extra_bytes)
  {
    decompress_selective &= ~LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES;
  }
  
  lasreadopener.set_decompress_selective(decompress_selective);

  // possibly loop over multiple input files

  while (lasreadopener.active())
  {
    if (edit_header)
    {
      if (lasreadopener.is_piped())
      {
        fprintf(stderr, "ERROR: cannot edit header of piped input\n");
        edit_header = false;
      }
      else if (lasreadopener.is_merged())
      {
        fprintf(stderr, "ERROR: cannot edit header of merged input\n");
        edit_header = false;
      }
      else if (lasreadopener.is_buffered())
      {
        fprintf(stderr, "ERROR: cannot edit header of buffered input\n");
        edit_header = false;
      }
      const CHAR* file_name = lasreadopener.get_file_name(lasreadopener.get_file_name_current());
      if ((strstr(file_name, ".laz") == 0) && (strstr(file_name, ".las") == 0) && (strstr(file_name, ".LAZ") == 0) && (strstr(file_name, ".LAS") == 0))
      {
        fprintf(stderr, "ERROR: can only edit for LAS or LAZ files, not for '%s'\n", file_name);
        edit_header = false;
      }
      if (set_file_source_ID_from_point_source_ID)
      {
        LASreader* lasreader = lasreadopener.open(file_name, FALSE);
        if (lasreader == 0)
        {
          fprintf(stderr, "ERROR: cannot open lasreader for '%s'\n", file_name);
          byebye(true, argc==1);
        }
        if (lasreader->read_point())
        {
          set_file_source_ID = lasreader->point.get_point_source_ID();
        }
        else
        {
          set_file_source_ID = -1;
        }
        lasreader->close();
        delete lasreader;
      }
      I64 set_geotiff_vlr_geo_keys_pos = -1;
      U32 set_geotiff_vlr_geo_keys_length = 0;
      I64 set_geotiff_vlr_geo_double_pos = -1;
      U32 set_geotiff_vlr_geo_double_length = 0;
      I64 set_geotiff_vlr_geo_ascii_pos = -1;
      U32 set_geotiff_vlr_geo_ascii_length = 0;
      if (set_geotiff_epsg != -1)
      {
        LASreader* lasreader = lasreadopener.open(file_name, FALSE);
        if (lasreader == 0)
        {
          fprintf(stderr, "ERROR: cannot open lasreader for '%s'\n", file_name);
          byebye(true, argc==1);
        }
        I64 pos = lasreader->header.header_size;
        for (i = 0; i < (int)lasreader->header.number_of_variable_length_records; i++)
        {
          pos += 54;
          if (strcmp(lasreader->header.vlrs[i].user_id, "LASF_Projection") == 0)
          {
            if (lasreader->header.vlrs[i].record_id == 34735) // GeoKeyDirectoryTag
            {
              set_geotiff_vlr_geo_keys_pos = pos;
              set_geotiff_vlr_geo_keys_length = lasreader->header.vlrs[i].record_length_after_header;
            }
            else if (lasreader->header.vlrs[i].record_id == 34736) // GeoDoubleParamsTag
            {
              set_geotiff_vlr_geo_double_pos = pos;
              set_geotiff_vlr_geo_double_length = lasreader->header.vlrs[i].record_length_after_header;
            }
            else if (lasreader->header.vlrs[i].record_id == 34737) // GeoAsciiParamsTag
            {
              set_geotiff_vlr_geo_ascii_pos = pos;
              set_geotiff_vlr_geo_ascii_length = lasreader->header.vlrs[i].record_length_after_header;
            }
          }
          pos += lasreader->header.vlrs[i].record_length_after_header;
        }
        lasreader->close();
        delete lasreader;
      }
      FILE* file = fopen(file_name, "rb+");
      if (file == 0)
      {
        fprintf (stderr, "ERROR: could not open file '%s' for edit of header\n", file_name);
        edit_header = false;
      }
      else if (edit_header)
      {
        if (set_file_source_ID != -1)
        {
          U16 file_source_ID = U16_CLAMP(set_file_source_ID);
          fseek(file, 4, SEEK_SET);
          fwrite(&file_source_ID, sizeof(U16), 1, file);
        }
        if (set_global_encoding != -1)
        {
          U16 global_encoding = U16_CLAMP(set_global_encoding);
          fseek(file, 6, SEEK_SET);
          fwrite(&global_encoding, sizeof(U16), 1, file);
        }
        if (set_project_ID_GUID_data_1 != -1)
        {
          fseek(file, 8, SEEK_SET);
          U32 GUID_data_1 = U32_CLAMP(set_project_ID_GUID_data_1);
          U16 GUID_data_2 = U16_CLAMP(set_project_ID_GUID_data_2);
          U16 GUID_data_3 = U16_CLAMP(set_project_ID_GUID_data_3);
          U16 GUID_data_4a = U16_CLAMP(set_project_ID_GUID_data_4a);
          U16 GUID_data_4b_a = U16_CLAMP(set_project_ID_GUID_data_4b >> 32);
          U32 GUID_data_4b_b = U32_CLAMP(set_project_ID_GUID_data_4b & 0xFFFFFFFF);
          fwrite(&GUID_data_1, sizeof(U32), 1, file);
          fwrite(&GUID_data_2, sizeof(U16), 1, file);
          fwrite(&GUID_data_3, sizeof(U16), 1, file);
          fwrite(&GUID_data_4a, sizeof(U16), 1, file);
          fwrite(&GUID_data_4b_a, sizeof(U16), 1, file);
          fwrite(&GUID_data_4b_b, sizeof(U32), 1, file);
        }
        if (set_version_major != -1)
        {
          fseek(file, 24, SEEK_SET);
          fwrite(&set_version_major, sizeof(I8), 1, file);
        }
        if (set_version_minor != -1)
        {
          fseek(file, 25, SEEK_SET);
          fwrite(&set_version_minor, sizeof(I8), 1, file);
        }
        if (set_system_identifier)
        {
          fseek(file, 26, SEEK_SET);
          fwrite(set_system_identifier, sizeof(I8), 32, file);
        }
        if (set_generating_software)
        {
          fseek(file, 58, SEEK_SET);
          fwrite(set_generating_software, sizeof(I8), 32, file);
        }
        if (set_creation_day != -1)
        {
          U16 creation_day = U16_CLAMP(set_creation_day);
          fseek(file, 90, SEEK_SET);
          fwrite(&creation_day, sizeof(U16), 1, file);
        }
        if (set_creation_year != -1)
        {
          U16 creation_year = U16_CLAMP(set_creation_year);
          fseek(file, 92, SEEK_SET);
          fwrite(&creation_year, sizeof(U16), 1, file);
        }
        if (set_header_size)
        {
          fseek(file, 94, SEEK_SET);
          fwrite(&set_header_size, sizeof(U16), 1, file);
        }
        if (set_offset_to_point_data)
        {
          fseek(file, 96, SEEK_SET);
          fwrite(&set_offset_to_point_data, sizeof(U32), 1, file);
        }
        if (set_number_of_variable_length_records != -1)
        {
          fseek(file, 100, SEEK_SET);
          fwrite(&set_number_of_variable_length_records, sizeof(U32), 1, file);
        }
        if (set_point_data_format != -1)
        {
          U8 point_data_format = U8_CLAMP(set_point_data_format);
          fseek(file, 104, SEEK_SET);
          fwrite(&point_data_format, sizeof(U8), 1, file);
        }
        if (set_point_data_record_length != -1)
        {
          U16 point_data_record_length = U16_CLAMP(set_point_data_record_length);
          fseek(file, 105, SEEK_SET);
          fwrite(&point_data_record_length, sizeof(U16), 1, file);
        }
        if (set_number_of_point_records != -1)
        {
          fseek(file, 107, SEEK_SET);
          fwrite(&set_number_of_point_records, sizeof(I32), 1, file);
        }
        if (set_number_of_points_by_return[0] != -1)
        {
          fseek(file, 111, SEEK_SET);
          fwrite(&(set_number_of_points_by_return[0]), sizeof(I32), 1, file);
        }
        if (set_number_of_points_by_return[1] != -1)
        {
          fseek(file, 115, SEEK_SET);
          fwrite(&(set_number_of_points_by_return[1]), sizeof(I32), 1, file);
        }
        if (set_number_of_points_by_return[2] != -1)
        {
          fseek(file, 119, SEEK_SET);
          fwrite(&(set_number_of_points_by_return[2]), sizeof(I32), 1, file);
        }
        if (set_number_of_points_by_return[3] != -1)
        {
          fseek(file, 123, SEEK_SET);
          fwrite(&(set_number_of_points_by_return[3]), sizeof(I32), 1, file);
        }
        if (set_number_of_points_by_return[4] != -1)
        {
          fseek(file, 127, SEEK_SET);
          fwrite(&(set_number_of_points_by_return[4]), sizeof(I32), 1, file);
        }
        if (set_scale)
        {
          fseek(file, 131, SEEK_SET);
          fwrite(set_scale, 3*sizeof(F64), 1, file);
        }
        if (set_offset)
        {
          fseek(file, 155, SEEK_SET);
          fwrite(set_offset, 3*sizeof(F64), 1, file);
        }
        if (set_bounding_box)
        {
          fseek(file, 179, SEEK_SET);
          fwrite(set_bounding_box, 6*sizeof(F64), 1, file);
        }
        if (set_start_of_waveform_data_packet_record != -1)
        {
          fseek(file, 227, SEEK_SET);
          fwrite(&set_start_of_waveform_data_packet_record, sizeof(I64), 1, file);
        }
        if (set_geotiff_epsg != -1)
        {
          if (set_geotiff_vlr_geo_keys_pos != -1)
          {
            GeoProjectionConverter geo;
            if (geo.set_epsg_code(set_geotiff_epsg))
            {
              int number_of_keys;
              GeoProjectionGeoKeys* geo_keys = 0;
              int num_geo_double_params;
              double* geo_double_params = 0;
              if (geo.get_geo_keys_from_projection(number_of_keys, &geo_keys, num_geo_double_params, &geo_double_params))
              {
                U32 set_geotiff_vlr_geo_keys_new_length = sizeof(GeoProjectionGeoKeys)*(number_of_keys+1);

                if (set_geotiff_vlr_geo_keys_new_length <= set_geotiff_vlr_geo_keys_length)
                {
                  fseek(file,(long)set_geotiff_vlr_geo_keys_pos, SEEK_SET);
                  LASvlr_geo_keys vlr_geo_key_directory;
                  vlr_geo_key_directory.key_directory_version = 1;
                  vlr_geo_key_directory.key_revision = 1;
                  vlr_geo_key_directory.minor_revision = 0;
                  vlr_geo_key_directory.number_of_keys = number_of_keys;
                  fwrite(&vlr_geo_key_directory, sizeof(LASvlr_geo_keys), 1, file);
                  fwrite(geo_keys, sizeof(GeoProjectionGeoKeys), number_of_keys, file);
                  for (i = set_geotiff_vlr_geo_keys_new_length; i < (int)set_geotiff_vlr_geo_keys_length; i++)
                  {
                    fputc(0, file);
                  }

                  if (set_geotiff_vlr_geo_double_pos != -1)
                  {
                    fseek(file,(long)set_geotiff_vlr_geo_double_pos, SEEK_SET);
                    for (i = 0; i < (int)set_geotiff_vlr_geo_double_length; i++)
                    {
                      fputc(0, file);
                    }
                  }

                  if (set_geotiff_vlr_geo_ascii_pos != -1)
                  {
                    fseek(file,(long)set_geotiff_vlr_geo_ascii_pos, SEEK_SET);
                    for (i = 0; i < (int)set_geotiff_vlr_geo_ascii_length; i++)
                    {
                      fputc(0, file);
                    }
                  }
                }
                else
                {
                  fprintf(stderr, "WARNING: cannot set EPSG to %u because file '%s' has not enough header space for GeoTIFF tags\n", set_geotiff_epsg, file_name);
                }
              }
              else
              {
                fprintf(stderr, "WARNING: cannot set EPSG in GeoTIFF tags of because no GeoTIFF tags available for code %u\n", set_geotiff_epsg);
                set_geotiff_epsg = -1;
              }
            }
            else
            {
              fprintf(stderr, "WARNING: cannot set EPSG in GeoTIFF tags of because code %u is unknown\n", set_geotiff_epsg);
              set_geotiff_epsg = -1;
            }
          }
          else
          {
            fprintf(stderr, "WARNING: cannot set EPSG to %u because file '%s' has no GeoTIFF tags\n", set_geotiff_epsg, file_name);
          }
        }
        if (verbose) fprintf(stderr, "edited '%s' ...\n", file_name);
        fclose(file);
      }
    }

    // open lasreader

    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: cannot open lasreader\n");
      byebye(true, argc==1);
    }
    LASheader* lasheader = &lasreader->header;

    if (base_name && lasreadopener.get_file_name())
    {
      lasreader->close();

#ifdef _WIN32
      if (verbose) fprintf(stderr, "renaming '%s' with %I64d points\n", lasreadopener.get_file_name(), lasreader->npoints);
#else
      fprintf(stderr, "ERROR: renaming implemented ...\n");
      byebye(true, argc==1);
#endif

      char command[2048];
      if (strlen(base_name))
      {
        sprintf(command, "rename \"%s\" \"%s_%d_%d.xxx\"", lasreadopener.get_file_name(), base_name, I32_QUANTIZE(lasheader->min_x), I32_QUANTIZE(lasheader->min_y));
      }
      else
      {
        sprintf(command, "rename \"%s\" \"%d_%d.xxx\"", lasreadopener.get_file_name(), I32_QUANTIZE(lasheader->min_x), I32_QUANTIZE(lasheader->min_y));
      }
      int len1 = (int)strlen(lasreadopener.get_file_name());
      int len2 = (int)strlen(command);
      command[len2-4] = lasreadopener.get_file_name()[len1-3];
      command[len2-3] = lasreadopener.get_file_name()[len1-2];
      command[len2-2] = lasreadopener.get_file_name()[len1-1];
      delete lasreader;
      
      if (verbose) fprintf(stderr, "executing '%s'\n", command);

      if (system(command) != 0)
      {
        fprintf(stderr, "ERROR: failed to execute '%s'\n", command);
        byebye(true);
      }
      continue;
    }


#ifdef _WIN32
    if (verbose) fprintf(stderr, "%s '%s' with %I64d points\n", (repair_bb || repair_counters ? "repairing" : "reading"), (lasreadopener.get_file_name() ? lasreadopener.get_file_name() : "stdin"), lasreader->npoints);
#else
    if (verbose) fprintf(stderr, "%s '%s' with %lld points\n", (repair_bb || repair_counters ? "repairing" : "reading"), (lasreadopener.get_file_name() ? lasreadopener.get_file_name() : "stdin"), lasreader->npoints);
#endif

    if (auto_date_creation && lasreadopener.get_file_name())
    {
#ifdef _WIN32
      WIN32_FILE_ATTRIBUTE_DATA attr;
	    SYSTEMTIME creation;
      GetFileAttributesEx(lasreadopener.get_file_name(), GetFileExInfoStandard, &attr);
	    FileTimeToSystemTime(&attr.ftCreationTime, &creation);
      int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
      set_creation_day = startday[creation.wMonth] + creation.wDay;
      set_creation_year = creation.wYear;
      // leap year handling
      if ((((creation.wYear)%4) == 0) && (creation.wMonth > 2)) set_creation_day++;
      edit_header = true;
#endif
    }

    if (laswriteopener.get_file_name() == 0)
    {
      if (lasreadopener.get_file_name() && (laswriteopener.get_format() == LAS_TOOLS_FORMAT_TXT))
      {
        laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
      }
    }

    if (laswriteopener.get_file_name())
    {
      // make sure we do not corrupt the input file
      if (lasreadopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
      {
        fprintf(stderr, "ERROR: input and output file name for '%s' are identical\n", lasreadopener.get_file_name());
        usage(true);
      }
      // open the text output file
      file_out = fopen(laswriteopener.get_file_name(), "w");
      if (file_out == 0)
      {
        fprintf (stderr, "WARNING: could not open output text file '%s'\n", laswriteopener.get_file_name());
        file_out = stderr;
      }
    }

    // print name of input

    if (file_out)
    {
      if (lasreadopener.is_merged())
      {
        fprintf(file_out, "lasinfo (%u) report for %u merged files\012", LAS_TOOLS_VERSION, lasreadopener.get_file_name_number());
      }
      else if (lasreadopener.is_piped())
      {
        fprintf(file_out, "lasinfo (%u) report for piped input\012", LAS_TOOLS_VERSION);
      }
      else if (lasreadopener.get_file_name())
      {
        fprintf(file_out, "lasinfo (%u) report for '%s'\012", LAS_TOOLS_VERSION, lasreadopener.get_file_name());
      }
    }

    U32 number_of_point_records = lasheader->number_of_point_records;
    U32 number_of_points_by_return0 = lasheader->number_of_points_by_return[0];

    // print header info

    CHAR printstring[4096];

    if (file_out && !no_header)
    {
      if (lasreadopener.is_merged() && (lasreader->header.version_minor < 4))
      {
#ifdef _WIN32
        if (lasreader->npoints > number_of_point_records) fprintf(file_out, "WARNING: merged file has %I64d points, more than the 32 bits counters of LAS 1.%d can handle.\012", lasreader->npoints, lasreader->header.version_minor);
#else
        if (lasreader->npoints > number_of_point_records) fprintf(file_out, "WARNING: merged file has %lld points, more than the 32 bits counters of LAS 1.%d can handle.\012", lasreader->npoints, lasreader->header.version_minor);
#endif
      }
      fprintf(file_out, "reporting all LAS header entries:\012");
      fprintf(file_out, "  file signature:             '%.4s'\012", lasheader->file_signature);
      fprintf(file_out, "  file source ID:             %d\012", lasheader->file_source_ID);
      fprintf(file_out, "  global_encoding:            %d\012", lasheader->global_encoding);
      fprintf(file_out, "  project ID GUID data 1-4:   %08X-%04X-%04X-%04X-%04X%08X\012", lasheader->project_ID_GUID_data_1, lasheader->project_ID_GUID_data_2, lasheader->project_ID_GUID_data_3, *((U16*)(lasheader->project_ID_GUID_data_4)), *((U16*)(lasheader->project_ID_GUID_data_4+2)), *((U32*)(lasheader->project_ID_GUID_data_4+4)));
      fprintf(file_out, "  version major.minor:        %d.%d\012", lasheader->version_major, lasheader->version_minor);
      fprintf(file_out, "  system identifier:          '%.32s'\012", lasheader->system_identifier);
      fprintf(file_out, "  generating software:        '%.32s'\012", lasheader->generating_software);
      fprintf(file_out, "  file creation day/year:     %d/%d\012", lasheader->file_creation_day, lasheader->file_creation_year);
      fprintf(file_out, "  header size:                %d\012", lasheader->header_size);
      fprintf(file_out, "  offset to point data:       %u\012", lasheader->offset_to_point_data);
      fprintf(file_out, "  number var. length records: %u\012", lasheader->number_of_variable_length_records);
      fprintf(file_out, "  point data format:          %d\012", lasheader->point_data_format);
      fprintf(file_out, "  point data record length:   %d\012", lasheader->point_data_record_length);
      fprintf(file_out, "  number of point records:    %u\012", lasheader->number_of_point_records);
      fprintf(file_out, "  number of points by return: %u %u %u %u %u\012", lasheader->number_of_points_by_return[0], lasheader->number_of_points_by_return[1], lasheader->number_of_points_by_return[2], lasheader->number_of_points_by_return[3], lasheader->number_of_points_by_return[4]);
      fprintf(file_out, "  scale factor x y z:         "); lidardouble2string(printstring, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
      fprintf(file_out, "  offset x y z:               "); lidardouble2string(printstring, lasheader->x_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->y_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->z_offset); fprintf(file_out, "%s\012", printstring);
      fprintf(file_out, "  min x y z:                  "); lidardouble2string(printstring, lasheader->min_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
      fprintf(file_out, "  max x y z:                  "); lidardouble2string(printstring, lasheader->max_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
      if (!no_warnings && !valid_resolution(lasheader->min_x, lasheader->x_offset, lasheader->x_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of min_x not compatible with x_offset and x_scale_factor: "); lidardouble2string(printstring, lasheader->min_x); fprintf(file_out, "%s\n", printstring);
      }
      if (!no_warnings && !valid_resolution(lasheader->min_y, lasheader->y_offset, lasheader->y_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of min_y not compatible with y_offset and y_scale_factor: "); lidardouble2string(printstring, lasheader->min_y); fprintf(file_out, "%s\n", printstring);
      }
      if (!no_warnings && !valid_resolution(lasheader->min_z, lasheader->z_offset, lasheader->z_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of min_z not compatible with z_offset and z_scale_factor: "); lidardouble2string(printstring, lasheader->min_z); fprintf(file_out, "%s\n", printstring);
      }
      if (!no_warnings && !valid_resolution(lasheader->max_x, lasheader->x_offset, lasheader->x_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of max_x not compatible with x_offset and x_scale_factor: "); lidardouble2string(printstring, lasheader->max_x); fprintf(file_out, "%s\n", printstring);
      }
      if (!no_warnings && !valid_resolution(lasheader->max_y, lasheader->y_offset, lasheader->y_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of max_y not compatible with y_offset and y_scale_factor: "); lidardouble2string(printstring, lasheader->max_y); fprintf(file_out, "%s\n", printstring);
      }
      if (!no_warnings && !valid_resolution(lasheader->max_z, lasheader->z_offset, lasheader->z_scale_factor))
      {
        fprintf(file_out, "WARNING: stored resolution of max_z not compatible with z_offset and z_scale_factor: "); lidardouble2string(printstring, lasheader->max_z); fprintf(file_out, "%s\n", printstring);
      }
      if ((lasheader->version_major == 1) && (lasheader->version_minor >= 3))
      {
#ifdef _WIN32
        fprintf(file_out, "  start of waveform data packet record: %I64d\012", lasheader->start_of_waveform_data_packet_record);
#else
        fprintf(file_out, "  start of waveform data packet record: %lld\012", lasheader->start_of_waveform_data_packet_record);
#endif
      }
      if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4))
      {
#ifdef _WIN32
        fprintf(file_out, "  start of first extended variable length record: %I64d\012", lasheader->start_of_first_extended_variable_length_record);
#else
        fprintf(file_out, "  start of first extended variable length record: %lld\012", lasheader->start_of_first_extended_variable_length_record);
#endif
        fprintf(file_out, "  number of extended_variable length records: %d\012", lasheader->number_of_extended_variable_length_records);
#ifdef _WIN32
        fprintf(file_out, "  extended number of point records: %I64d\012", lasheader->extended_number_of_point_records);
        fprintf(file_out, "  extended number of points by return: %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d\012", lasheader->extended_number_of_points_by_return[0], lasheader->extended_number_of_points_by_return[1], lasheader->extended_number_of_points_by_return[2], lasheader->extended_number_of_points_by_return[3], lasheader->extended_number_of_points_by_return[4], lasheader->extended_number_of_points_by_return[5], lasheader->extended_number_of_points_by_return[6], lasheader->extended_number_of_points_by_return[7], lasheader->extended_number_of_points_by_return[8], lasheader->extended_number_of_points_by_return[9], lasheader->extended_number_of_points_by_return[10], lasheader->extended_number_of_points_by_return[11], lasheader->extended_number_of_points_by_return[12], lasheader->extended_number_of_points_by_return[13], lasheader->extended_number_of_points_by_return[14]);
#else
        fprintf(file_out, "  extended number of point records: %lld\012", lasheader->extended_number_of_point_records);
        fprintf(file_out, "  extended number of points by return: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\012", lasheader->extended_number_of_points_by_return[0], lasheader->extended_number_of_points_by_return[1], lasheader->extended_number_of_points_by_return[2], lasheader->extended_number_of_points_by_return[3], lasheader->extended_number_of_points_by_return[4], lasheader->extended_number_of_points_by_return[5], lasheader->extended_number_of_points_by_return[6], lasheader->extended_number_of_points_by_return[7], lasheader->extended_number_of_points_by_return[8], lasheader->extended_number_of_points_by_return[9], lasheader->extended_number_of_points_by_return[10], lasheader->extended_number_of_points_by_return[11], lasheader->extended_number_of_points_by_return[12], lasheader->extended_number_of_points_by_return[13], lasheader->extended_number_of_points_by_return[14]);
#endif
      }
      if (lasheader->user_data_in_header_size) fprintf(file_out, "the header contains %u user-defined bytes\012", lasheader->user_data_in_header_size);
    }

    // maybe print variable header

    if (file_out && !no_variable_header)
    {
      for (int i = 0; i < (int)lasheader->number_of_variable_length_records; i++)
      {
        fprintf(file_out, "variable length header record %d of %d:\012", i+1, (int)lasheader->number_of_variable_length_records);
        fprintf(file_out, "  reserved             %d\012", lasreader->header.vlrs[i].reserved);
        fprintf(file_out, "  user ID              '%.16s'\012", lasreader->header.vlrs[i].user_id);
        fprintf(file_out, "  record ID            %d\012", lasreader->header.vlrs[i].record_id);
        fprintf(file_out, "  length after header  %d\012", lasreader->header.vlrs[i].record_length_after_header);
        fprintf(file_out, "  description          '%.32s'\012", lasreader->header.vlrs[i].description);

        // special handling for known variable header tags

        if ((strcmp(lasheader->vlrs[i].user_id, "LASF_Projection") == 0) && (lasheader->vlrs[i].data != 0))
        {
          if (lasheader->vlrs[i].record_id == 34735) // GeoKeyDirectoryTag
          {
            fprintf(file_out, "    GeoKeyDirectoryTag version %d.%d.%d number of keys %d\012", lasheader->vlr_geo_keys->key_directory_version, lasheader->vlr_geo_keys->key_revision, lasheader->vlr_geo_keys->minor_revision, lasheader->vlr_geo_keys->number_of_keys);
            for (int j = 0; j < lasheader->vlr_geo_keys->number_of_keys; j++)
            {
              if (file_out)
              {
                fprintf(file_out, "      key %d tiff_tag_location %d count %d value_offset %d - ", lasheader->vlr_geo_key_entries[j].key_id, lasheader->vlr_geo_key_entries[j].tiff_tag_location, lasheader->vlr_geo_key_entries[j].count, lasheader->vlr_geo_key_entries[j].value_offset);
                switch (lasreader->header.vlr_geo_key_entries[j].key_id)
                {
                case 1024: // GTModelTypeGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 1: // ModelTypeProjected   
                    fprintf(file_out, "GTModelTypeGeoKey: ModelTypeProjected\012");
                    break;
                  case 2:
                    fprintf(file_out, "GTModelTypeGeoKey: ModelTypeGeographic\012");
                    break;
                  case 3:
                    fprintf(file_out, "GTModelTypeGeoKey: ModelTypeGeocentric\012");
                    break;
                  case 0: // ModelTypeUndefined   
                    fprintf(file_out, "GTModelTypeGeoKey: ModelTypeUndefined\012");
                    break;
                  default:
                    fprintf(file_out, "GTModelTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                 }
                  break;
                case 1025: // GTRasterTypeGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 1: // RasterPixelIsArea   
                    fprintf(file_out, "GTRasterTypeGeoKey: RasterPixelIsArea\012");
                    break;
                  case 2: // RasterPixelIsPoint
                    fprintf(file_out, "GTRasterTypeGeoKey: RasterPixelIsPoint\012");
                    break;
                 default:
                    fprintf(file_out, "GTRasterTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 1026: // GTCitationGeoKey
                  if (lasreader->header.vlr_geo_ascii_params)
                  {
                    char dummy[256];
                    strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                    dummy[lasreader->header.vlr_geo_key_entries[j].count-1] = '\0';
                    fprintf(file_out, "GTCitationGeoKey: %s\012",dummy);
                  }
                  break;
                case 2048: // GeographicTypeGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 32767: // user-defined
                    fprintf(file_out, "GeographicTypeGeoKey: user-defined\012");
                    break;
                  case 4001: // GCSE_Airy1830
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Airy1830\012");
                    break;
                  case 4002: // GCSE_AiryModified1849 
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_AiryModified1849\012");
                    break;
                  case 4003: // GCSE_AustralianNationalSpheroid
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_AustralianNationalSpheroid\012");
                    break;
                  case 4004: // GCSE_Bessel1841
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Bessel1841\012");
                    break;
                  case 4005: // GCSE_Bessel1841Modified
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Bessel1841Modified\012");
                    break;
                  case 4006: // GCSE_BesselNamibia
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_BesselNamibia\012");
                    break;
                  case 4008: // GCSE_Clarke1866
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1866\012");
                    break;
                  case 4009: // GCSE_Clarke1866Michigan
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1866Michigan\012");
                    break;
                  case 4010: // GCSE_Clarke1880_Benoit
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_Benoit\012");
                    break;
                  case 4011: // GCSE_Clarke1880_IGN
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_IGN\012");
                    break;
                  case 4012: // GCSE_Clarke1880_RGS
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_RGS\012");
                    break;
                  case 4013: // GCSE_Clarke1880_Arc
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_Arc\012");
                    break;
                  case 4014: // GCSE_Clarke1880_SGA1922
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_SGA1922\012");
                    break;
                  case 4015: // GCSE_Everest1830_1937Adjustment
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1937Adjustment\012");
                    break;
                  case 4016: // GCSE_Everest1830_1967Definition
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1967Definition\012");
                    break;
                  case 4017: // GCSE_Everest1830_1975Definition
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1975Definition\012");
                    break;
                  case 4018: // GCSE_Everest1830Modified
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830Modified\012");
                    break;
                  case 4019: // GCSE_GRS1980
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_GRS1980\012");
                    break;
                  case 4020: // GCSE_Helmert1906
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Helmert1906\012");
                    break;
                  case 4022: // GCSE_International1924
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_International1924\012");
                    break;
                  case 4023: // GCSE_International1967
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_International1967\012");
                    break;
                  case 4024: // GCSE_Krassowsky1940
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Krassowsky1940\012");
                    break;
                  case 4030: // GCSE_WGS84
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_WGS84\012");
                    break;
                  case 4034: // GCSE_Clarke1880
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880\012");
                    break;
                  case 4140: // GCSE_NAD83_CSRS
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_NAD83_CSRS\012");
                    break;
                  case 4167: // GCSE_New_Zealand_Geodetic_Datum_2000
                    fprintf(file_out, "GeographicTypeGeoKey: GCSE_New_Zealand_Geodetic_Datum_2000\012");
                    break;
                  case 4267: // GCS_NAD27
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD27\012");
                    break;
                  case 4269: // GCS_NAD83
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83\012");
                    break;
                  case 4283: // GCS_GDA94
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_GDA94\012");
                    break;
                  case 4322: // GCS_WGS_72
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_WGS_72\012");
                    break;
                  case 4326: // GCS_WGS_84
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_WGS_84\012");
                    break;
                  case 4289: // GCS_Amersfoort
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_Amersfoort\012");
                    break;
                  case 4617: // GCS_NAD83_CSRS
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_CSRS\012");
                    break;
                  case 4619: // GCS_SWEREF99
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_SWEREF99\012");
                    break;
                  case 6318: // GCS_NAD83_2011
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_2011\012");
                    break;
                  case 6322: // GCS_NAD83_PA11
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_PA11\012");
                    break;
                  case 7844: // GCS_GDA2020
                    fprintf(file_out, "GeographicTypeGeoKey: GCS_GDA2020\012");
                    break;
                  default:
                    fprintf(file_out, "GeographicTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2049: // GeogCitationGeoKey
                  if (lasreader->header.vlr_geo_ascii_params)
                  {
                    char dummy[256];
                    strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                    dummy[lasreader->header.vlr_geo_key_entries[j].count-1] = '\0';
                    fprintf(file_out, "GeogCitationGeoKey: %s\012",dummy);
                  }
                  break;
                case 2050: // GeogGeodeticDatumGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 32767: // user-defined
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: user-defined\012");
                    break;
                  case 6202: // Datum_Australian_Geodetic_Datum_1966
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Australian_Geodetic_Datum_1966\012");
                    break;
                  case 6203: // Datum_Australian_Geodetic_Datum_1984
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Australian_Geodetic_Datum_1984\012");
                    break;
                  case 6267: // Datum_North_American_Datum_1927
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_North_American_Datum_1927\012");
                    break;
                  case 6269: // Datum_North_American_Datum_1983
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_North_American_Datum_1983\012");
                    break;
                  case 6283: // Datum_Geocentric_Datum_of_Australia_1994
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Geocentric_Datum_of_Australia_1994\012");
                    break;
                  case 6322: // Datum_WGS72
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_WGS72\012");
                    break;
                  case 6326: // Datum_WGS84
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_WGS84\012");
                    break;
                  case 6140: // Datum_WGS84
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_NAD83_CSRS\012");
                    break;
                  case 6619: // Datum_SWEREF99
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_SWEREF99\012");
                    break;
                  case 6289: // Datum_Amersfoort
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Amersfoort\012");
                    break;
                  case 6167: // Datum_NZGD2000
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_NZGD2000\012");
                    break;
                  case 6001: // DatumE_Airy1830
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Airy1830\012");
                    break;
                  case 6002: // DatumE_AiryModified1849
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_AiryModified1849\012");
                    break;
                  case 6003: // DatumE_AustralianNationalSpheroid
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_AustralianNationalSpheroid\012");
                    break;
                  case 6004: // DatumE_Bessel1841
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Bessel1841\012");
                    break;
                  case 6005: // DatumE_BesselModified
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_BesselModified\012");
                    break;
                  case 6006: // DatumE_BesselNamibia
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_BesselNamibia\012");
                    break;
                  case 6008: // DatumE_Clarke1866
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1866\012");
                    break;
                  case 6009: // DatumE_Clarke1866Michigan
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1866Michigan\012");
                    break;
                  case 6010: // DatumE_Clarke1880_Benoit
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_Benoit\012");
                    break;
                  case 6011: // DatumE_Clarke1880_IGN
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_IGN\012");
                    break;
                  case 6012: // DatumE_Clarke1880_RGS
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_RGS\012");
                    break;
                  case 6013: // DatumE_Clarke1880_Arc
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_Arc\012");
                    break;
                  case 6014: // DatumE_Clarke1880_SGA1922
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_SGA1922\012");
                    break;
                  case 6015: // DatumE_Everest1830_1937Adjustment
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1937Adjustment\012");
                    break;
                  case 6016: // DatumE_Everest1830_1967Definition
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1967Definition\012");
                    break;
                  case 6017: // DatumE_Everest1830_1975Definition
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1975Definition\012");
                    break;
                  case 6018: // DatumE_Everest1830Modified
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830Modified\012");
                    break;
                  case 6019: // DatumE_GRS1980
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_GRS1980\012");
                    break;
                  case 6020: // DatumE_Helmert1906
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Helmert1906\012");
                    break;
                  case 6022: // DatumE_International1924
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_International1924\012");
                    break;
                  case 6023: // DatumE_International1967
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_International1967\012");
                    break;
                  case 6024: // DatumE_Krassowsky1940
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Krassowsky1940\012");
                    break;
                  case 6030: // DatumE_WGS84
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_WGS84\012");
                    break;
                  case 6034: // DatumE_Clarke1880
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880\012");
                    break;
                  default:
                    fprintf(file_out, "GeogGeodeticDatumGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2051: // GeogPrimeMeridianGeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 32767: // user-defined
                    fprintf(file_out, "GeogPrimeMeridianGeoKey: user-defined\012");
                    break;
                  case 8901: // PM_Greenwich
                    fprintf(file_out, "GeogPrimeMeridianGeoKey: PM_Greenwich\012");
                    break;
                  case 8902: // PM_Lisbon
                    fprintf(file_out, "GeogPrimeMeridianGeoKey: PM_Lisbon\012");
                    break;
                  default:
                    fprintf(file_out, "GeogPrimeMeridianGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2052: // GeogLinearUnitsGeoKey 
                  horizontal_units = lasreader->header.vlr_geo_key_entries[j].value_offset;
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 9001: // Linear_Meter
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Meter\012");
                    break;
                  case 9002: // Linear_Foot
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot\012");
                    break;
                  case 9003: // Linear_Foot_US_Survey
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_US_Survey\012");
                    break;
                  case 9004: // Linear_Foot_Modified_American
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Modified_American\012");
                    break;
                  case 9005: // Linear_Foot_Clarke
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Clarke\012");
                    break;
                  case 9006: // Linear_Foot_Indian
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Indian\012");
                    break;
                  case 9007: // Linear_Link
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link\012");
                    break;
                  case 9008: // Linear_Link_Benoit
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link_Benoit\012");
                    break;
                  case 9009: // Linear_Link_Sears
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link_Sears\012");
                    break;
                  case 9010: // Linear_Chain_Benoit
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Chain_Benoit\012");
                    break;
                  case 9011: // Linear_Chain_Sears
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Chain_Sears\012");
                    break;
                  case 9012: // Linear_Yard_Sears
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Yard_Sears\012");
                    break;
                  case 9013: // Linear_Yard_Indian
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Yard_Indian\012");
                    break;
                  case 9014: // Linear_Fathom
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Fathom\012");
                    break;
                  case 9015: // Linear_Mile_International_Nautical
                    fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Mile_International_Nautical\012");
                    break;
                  default:
                    fprintf(file_out, "GeogLinearUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2053: // GeogLinearUnitSizeGeoKey  
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogLinearUnitSizeGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2054: // GeogAngularUnitsGeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 9101: // Angular_Radian
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Radian\012");
                    break;
                  case 9102: // Angular_Degree
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Degree\012");
                    break;
                  case 9103: // Angular_Arc_Minute
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Arc_Minute\012");
                    break;
                  case 9104: // Angular_Arc_Second
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Arc_Second\012");
                    break;
                  case 9105: // Angular_Grad
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Grad\012");
                    break;
                  case 9106: // Angular_Gon
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Gon\012");
                    break;
                  case 9107: // Angular_DMS
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_DMS\012");
                    break;
                  case 9108: // Angular_DMS_Hemisphere
                    fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_DMS_Hemisphere\012");
                    break;
                  default:
                    fprintf(file_out, "GeogAngularUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2055: // GeogAngularUnitSizeGeoKey 
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogAngularUnitSizeGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2056: // GeogEllipsoidGeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 32767: // user-defined
                    fprintf(file_out, "GeogEllipsoidGeoKey: user-defined\012");
                    break;
                  case 7001: // Ellipse_Airy_1830
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Airy_1830\012");
                    break;
                  case 7002: // Ellipse_Airy_Modified_1849
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Airy_Modified_1849\012");
                    break;
                  case 7003: // Ellipse_Australian_National_Spheroid
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Australian_National_Spheroid\012");
                    break;
                  case 7004: // Ellipse_Bessel_1841
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_1841\012");
                    break;
                  case 7005: // Ellipse_Bessel_Modified
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_Modified\012");
                    break;
                  case 7006: // Ellipse_Bessel_Namibia
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_Namibia\012");
                    break;
                  case 7008: // Ellipse_Clarke_1866
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1866\012");
                    break;
                  case 7009: // Ellipse_Clarke_1866_Michigan
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1866_Michigan\012");
                    break;
                  case 7010: // Ellipse_Clarke1880_Benoit
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_Benoit\012");
                    break;
                  case 7011: // Ellipse_Clarke1880_IGN
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_IGN\012");
                    break;
                  case 7012: // Ellipse_Clarke1880_RGS
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_RGS\012");
                    break;
                  case 7013: // Ellipse_Clarke1880_Arc
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_Arc\012");
                    break;
                  case 7014: // Ellipse_Clarke1880_SGA1922
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_SGA1922\012");
                    break;
                  case 7015: // Ellipse_Everest1830_1937Adjustment
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1937Adjustment\012");
                    break;
                  case 7016: // Ellipse_Everest1830_1967Definition
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1967Definition\012");
                    break;
                  case 7017: // Ellipse_Everest1830_1975Definition
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1975Definition\012");
                    break;
                  case 7018: // Ellipse_Everest1830Modified
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830Modified\012");
                    break;
                  case 7019: // Ellipse_GRS_1980
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_GRS_1980\012");
                    break;
                  case 7020: // Ellipse_Helmert1906
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Helmert1906\012");
                    break;
                  case 7022: // Ellipse_International1924
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_International1924\012");
                    break;
                  case 7023: // Ellipse_International1967
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_International1967\012");
                    break;
                  case 7024: // Ellipse_Krassowsky1940
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Krassowsky1940\012");
                    break;
                  case 7030: // Ellipse_WGS_84
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_WGS_84\012");
                    break;
                  case 7034: // Ellipse_Clarke_1880
                    fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1880\012");
                    break;
                  default:
                    fprintf(file_out, "GeogEllipsoidGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2057: // GeogSemiMajorAxisGeoKey 
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogSemiMajorAxisGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2058: // GeogSemiMinorAxisGeoKey 
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogSemiMinorAxisGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2059: // GeogInvFlatteningGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogInvFlatteningGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2060: // GeogAzimuthUnitsGeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 9101: // Angular_Radian
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Radian\012");
                    break;
                  case 9102: // Angular_Degree
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Degree\012");
                    break;
                  case 9103: // Angular_Arc_Minute
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Arc_Minute\012");
                    break;
                  case 9104: // Angular_Arc_Second
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Arc_Second\012");
                    break;
                  case 9105: // Angular_Grad
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Grad\012");
                    break;
                  case 9106: // Angular_Gon
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Gon\012");
                    break;
                  case 9107: // Angular_DMS
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_DMS\012");
                    break;
                  case 9108: // Angular_DMS_Hemisphere
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_DMS_Hemisphere\012");
                    break;
                  default:
                    fprintf(file_out, "GeogAzimuthUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 2061: // GeogPrimeMeridianLongGeoKey  
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "GeogPrimeMeridianLongGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 2062: // GeogTOWGS84GeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].count)
                  {
                  case 3:
                    if (lasreader->header.vlr_geo_double_params)
                    {
                      fprintf(file_out, "GeogTOWGS84GeoKey: TOWGS84[%.10g,%.10g,%.10g]\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+1], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+2]);
                    }
                    else
                    {
                      fprintf(file_out, "GeogTOWGS84GeoKey: no vlr_geo_double_params. cannot look up the three parameters.\012");
                    }
                    break;
                  case 7:
                    if (lasreader->header.vlr_geo_double_params)
                    {
                      fprintf(file_out, "GeogTOWGS84GeoKey: TOWGS84[%.10g,%.10g,%.10g,%.10g,%.10g,%.10g,%.10g]\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+1], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+2], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+3], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+4], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+5], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset+6]);
                    }
                    else
                    {
                      fprintf(file_out, "GeogTOWGS84GeoKey: no vlr_geo_double_params. cannot look up the seven parameters.\012");
                    }
                    break;
                  default:
                    fprintf(file_out, "GeogTOWGS84GeoKey: look-up for type %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].count);
                  }
                  break;
                case 3072: // ProjectedCSTypeGeoKey
                  if (geoprojectionconverter.set_ProjectedCSTypeGeoKey(lasreader->header.vlr_geo_key_entries[j].value_offset, printstring))
                  {
                    horizontal_units = geoprojectionconverter.get_ProjLinearUnitsGeoKey();
                    fprintf(file_out, "ProjectedCSTypeGeoKey: %s\012", printstring);
                    break;
                  }
                  else
                  {
                    fprintf(file_out, "ProjectedCSTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 3073: // PCSCitationGeoKey
                  if (lasreader->header.vlr_geo_ascii_params)
                  {
                    char dummy[256];
                    strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                    dummy[lasreader->header.vlr_geo_key_entries[j].count-1] = '\0';
                    fprintf(file_out, "PCSCitationGeoKey: %s\012",dummy);
                  }
                  break;
                case 3074: // ProjectionGeoKey
                  if ((16001 <= lasreader->header.vlr_geo_key_entries[j].value_offset) && (lasreader->header.vlr_geo_key_entries[j].value_offset <= 16060))
                  {
                    fprintf(file_out, "ProjectionGeoKey: Proj_UTM_zone_%dN\012", lasreader->header.vlr_geo_key_entries[j].value_offset - 16000);
                  }
                  else if ((16101 <= lasreader->header.vlr_geo_key_entries[j].value_offset) && (lasreader->header.vlr_geo_key_entries[j].value_offset <= 16160))
                  {
                    fprintf(file_out, "ProjectionGeoKey: Proj_UTM_zone_%dS\012", lasreader->header.vlr_geo_key_entries[j].value_offset - 16100);
                  }
                  else
                  {
                    switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                    {
                    case 32767: // user-defined
                      fprintf(file_out, "ProjectionGeoKey: user-defined\012");
                      break;
                    case 10101: // Proj_Alabama_CS27_East
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS27_East\012");
                      break;
                    case 10102: // Proj_Alabama_CS27_West
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS27_West\012");
                      break;
                    case 10131: // Proj_Alabama_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS83_East\012");
                      break;
                    case 10132: // Proj_Alabama_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS83_West\012");
                      break;
                    case 10201: // Proj_Arizona_Coordinate_System_east			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_east\012");
                      break;
                    case 10202: // Proj_Arizona_Coordinate_System_Central		
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_Central\012");
                      break;
                    case 10203: // Proj_Arizona_Coordinate_System_west			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_west\012");
                      break;
                    case 10231: // Proj_Arizona_CS83_east				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_east\012");
                      break;
                    case 10232: // Proj_Arizona_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_Central\012");
                      break;
                    case 10233: // Proj_Arizona_CS83_west				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_west\012");
                      break;
                    case 10301: // Proj_Arkansas_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS27_North\012");
                      break;
                    case 10302: // Proj_Arkansas_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS27_South\012");
                      break;
                    case 10331: // Proj_Arkansas_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS83_North\012");
                      break;
                    case 10332: // Proj_Arkansas_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS83_South\012");
                      break;
                    case 10401: // Proj_California_CS27_I				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_I\012");
                      break;
                    case 10402: // Proj_California_CS27_II				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_II\012");
                      break;
                    case 10403: // Proj_California_CS27_III				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_III\012");
                      break;
                    case 10404: // Proj_California_CS27_IV				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_IV\012");
                      break;
                    case 10405: // Proj_California_CS27_V				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_V\012");
                      break;
                    case 10406: // Proj_California_CS27_VI				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_VI\012");
                      break;
                    case 10407: // Proj_California_CS27_VII				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_VII\012");
                      break;
                    case 10431: // Proj_California_CS83_1				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_1\012");
                      break;
                    case 10432: // Proj_California_CS83_2				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_2\012");
                      break;
                    case 10433: // Proj_California_CS83_3				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_3\012");
                      break;
                    case 10434: // Proj_California_CS83_4				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_4\012");
                      break;
                    case 10435: // Proj_California_CS83_5				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_5\012");
                      break;
                    case 10436: // Proj_California_CS83_6				
                      fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_6\012");
                      break;
                    case 10501: // Proj_Colorado_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_North\012");
                      break;
                    case 10502: // Proj_Colorado_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_Central\012");
                      break;
                    case 10503: // Proj_Colorado_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_South\012");
                      break;
                    case 10531: // Proj_Colorado_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_North\012");
                      break;
                    case 10532: // Proj_Colorado_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_Central\012");
                      break;
                    case 10533: // Proj_Colorado_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_South\012");
                      break;
                    case 10600: // Proj_Connecticut_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Connecticut_CS27\012");
                      break;
                    case 10630: // Proj_Connecticut_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Connecticut_CS83\012");
                      break;
                    case 10700: // Proj_Delaware_CS27					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Delaware_CS27\012");
                      break;
                    case 10730: // Proj_Delaware_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Delaware_CS83\012");
                      break;
                    case 10901: // Proj_Florida_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_East\012");
                      break;
                    case 10902: // Proj_Florida_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_West\012");
                      break;
                    case 10903: // Proj_Florida_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_North\012");
                      break;
                    case 10931: // Proj_Florida_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_East\012");
                      break;
                    case 10932: // Proj_Florida_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_West\012");
                      break;
                    case 10933: // Proj_Florida_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_North\012");
                      break;
                    case 11001: // Proj_Georgia_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS27_East\012");
                      break;
                    case 11002: // Proj_Georgia_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS27_West\012");
                      break;
                    case 11031: // Proj_Georgia_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS83_East\012");
                      break;
                    case 11032: // Proj_Georgia_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS83_West\012");
                      break;
                    case 11101: // Proj_Idaho_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_East\012");
                      break;
                    case 11102: // Proj_Idaho_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_Central\012");
                      break;
                    case 11103: // Proj_Idaho_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_West\012");
                      break;
                    case 11131: // Proj_Idaho_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_East\012");
                      break;
                    case 11132: // Proj_Idaho_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_Central\012");
                      break;
                    case 11133: // Proj_Idaho_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_West\012");
                      break;
                    case 11201: // Proj_Illinois_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS27_East\012");
                      break;
                    case 11202: // Proj_Illinois_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS27_West\012");
                      break;
                    case 11231: // Proj_Illinois_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS83_East\012");
                      break;
                    case 11232: // Proj_Illinois_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS83_West\012");
                      break;
                    case 11301: // Proj_Indiana_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS27_East\012");
                      break;
                    case 11302: // Proj_Indiana_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS27_West\012");
                      break;
                    case 11331: // Proj_Indiana_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS83_East\012");
                      break;
                    case 11332: // Proj_Indiana_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS83_West\012");
                      break;
                    case 11401: // Proj_Iowa_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS27_North\012");
                      break;
                    case 11402: // Proj_Iowa_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS27_South\012");
                      break;
                    case 11431: // Proj_Iowa_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS83_North\012");
                      break;
                    case 11432: // Proj_Iowa_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS83_South\012");
                      break;
                    case 11501: // Proj_Kansas_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS27_North\012");
                      break;
                    case 11502: // Proj_Kansas_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS27_South\012");
                      break;
                    case 11531: // Proj_Kansas_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS83_North\012");
                      break;
                    case 11532: // Proj_Kansas_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS83_South\012");
                      break;
                    case 11601: // Proj_Kentucky_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS27_North\012");
                      break;
                    case 11602: // Proj_Kentucky_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS27_South\012");
                      break;
                    case 11631: // Proj_Kentucky_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS83_North\012");
                      break;
                    case 11632: // Proj_Kentucky_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS83_South\012");
                      break;
                    case 11701: // Proj_Louisiana_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS27_North\012");
                      break;
                    case 11702: // Proj_Louisiana_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS27_South\012");
                      break;
                    case 11731: // Proj_Louisiana_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS83_North\012");
                      break;
                    case 11732: // Proj_Louisiana_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS83_South\012");
                      break;
                    case 11801: // Proj_Maine_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS27_East\012");
                      break;
                    case 11802: // Proj_Maine_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS27_West\012");
                      break;
                    case 11831: // Proj_Maine_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS83_East\012");
                      break;
                    case 11832: // Proj_Maine_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS83_West\012");
                      break;
                    case 11900: // Proj_Maryland_CS27					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maryland_CS27\012");
                      break;
                    case 11930: // Proj_Maryland_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Maryland_CS83\012");
                      break;
                    case 12001: // Proj_Massachusetts_CS27_Mainland			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS27_Mainland\012");
                      break;
                    case 12002: // Proj_Massachusetts_CS27_Island			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS27_Island\012");
                      break;
                    case 12031: // Proj_Massachusetts_CS83_Mainland			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS83_Mainland\012");
                      break;
                    case 12032: // Proj_Massachusetts_CS83_Island			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS83_Island\012");
                      break;
                    case 12101: // Proj_Michigan_State_Plane_East			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_East\012");
                      break;
                    case 12102: // Proj_Michigan_State_Plane_Old_Central		
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_Old_Central\012");
                      break;
                    case 12103: // Proj_Michigan_State_Plane_West			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_West\012");
                      break;
                    case 12111: // Proj_Michigan_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_North\012");
                      break;
                    case 12112: // Proj_Michigan_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_Central\012");
                      break;
                    case 12113: // Proj_Michigan_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_South\012");
                      break;
                    case 12141: // Proj_Michigan_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_North\012");
                      break;
                    case 12142: // Proj_Michigan_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_Central\012");
                      break;
                    case 12143: // Proj_Michigan_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_South\012");
                      break;
                    case 12201: // Proj_Minnesota_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_North\012");
                      break;
                    case 12202: // Proj_Minnesota_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_Central\012");
                      break;
                    case 12203: // Proj_Minnesota_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_South\012");
                      break;
                    case 12231: // Proj_Minnesota_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_North\012");
                      break;
                    case 12232: // Proj_Minnesota_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_Central\012");
                      break;
                    case 12233: // Proj_Minnesota_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_South\012");
                      break;
                    case 12301: // Proj_Mississippi_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS27_East\012");
                      break;
                    case 12302: // Proj_Mississippi_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS27_West\012");
                      break;
                    case 12331: // Proj_Mississippi_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS83_East\012");
                      break;
                    case 12332: // Proj_Mississippi_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS83_West\012");
                      break;
                    case 12401: // Proj_Missouri_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_East\012");
                      break;
                    case 12402: // Proj_Missouri_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_Central\012");
                      break;
                    case 12403: // Proj_Missouri_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_West\012");
                      break;
                    case 12431: // Proj_Missouri_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_East\012");
                      break;
                    case 12432: // Proj_Missouri_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_Central\012");
                      break;
                    case 12433: // Proj_Missouri_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_West\012");
                      break;
                    case 12501: // Proj_Montana_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_North\012");
                      break;
                    case 12502: // Proj_Montana_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_Central\012");
                      break;
                    case 12503: // Proj_Montana_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_South\012");
                      break;
                    case 12530: // Proj_Montana_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS83\012");
                      break;
                    case 12601: // Proj_Nebraska_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS27_North\012");
                      break;
                    case 12602: // Proj_Nebraska_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS27_South\012");
                      break;
                    case 12630: // Proj_Nebraska_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS83\012");
                      break;
                    case 12701: // Proj_Nevada_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_East\012");
                      break;
                    case 12702: // Proj_Nevada_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_Central\012");
                      break;
                    case 12703: // Proj_Nevada_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_West\012");
                      break;
                    case 12731: // Proj_Nevada_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_East\012");
                      break;
                    case 12732: // Proj_Nevada_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_Central\012");
                      break;
                    case 12733: // Proj_Nevada_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_West\012");
                      break;
                    case 12800: // Proj_New_Hampshire_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Hampshire_CS27\012");
                      break;
                    case 12830: // Proj_New_Hampshire_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Hampshire_CS83\012");
                      break;
                    case 12900: // Proj_New_Jersey_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Jersey_CS27\012");
                      break;
                    case 12930: // Proj_New_Jersey_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Jersey_CS83\012");
                      break;
                    case 13001: // Proj_New_Mexico_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_East\012");
                      break;
                    case 13002: // Proj_New_Mexico_CS27_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_Central\012");
                      break;
                    case 13003: // Proj_New_Mexico_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_West\012");
                      break;
                    case 13031: // Proj_New_Mexico_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_East\012");
                      break;
                    case 13032: // Proj_New_Mexico_CS83_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_Central\012");
                      break;
                    case 13033: // Proj_New_Mexico_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_West\012");
                      break;
                    case 13101: // Proj_New_York_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_East\012");
                      break;
                    case 13102: // Proj_New_York_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_Central\012");
                      break;
                    case 13103: // Proj_New_York_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_West\012");
                      break;
                    case 13104: // Proj_New_York_CS27_Long_Island			
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_Long_Island\012");
                      break;
                    case 13131: // Proj_New_York_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_East\012");
                      break;
                    case 13132: // Proj_New_York_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_Central\012");
                      break;
                    case 13133: // Proj_New_York_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_West\012");
                      break;
                    case 13134: // Proj_New_York_CS83_Long_Island			
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_Long_Island\012");
                      break;
                    case 13200: // Proj_North_Carolina_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Carolina_CS27\012");
                      break;
                    case 13230: // Proj_North_Carolina_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Carolina_CS83\012");
                      break;
                    case 13301: // Proj_North_Dakota_CS27_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS27_North\012");
                      break;
                    case 13302: // Proj_North_Dakota_CS27_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS27_South\012");
                      break;
                    case 13331: // Proj_North_Dakota_CS83_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS83_North\012");
                      break;
                    case 13332: // Proj_North_Dakota_CS83_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS83_South\012");
                      break;
                    case 13401: // Proj_Ohio_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS27_North\012");
                      break;
                    case 13402: // Proj_Ohio_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS27_South\012");
                      break;
                    case 13431: // Proj_Ohio_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS83_North\012");
                      break;
                    case 13432: // Proj_Ohio_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS83_South\012");
                      break;
                    case 13501: // Proj_Oklahoma_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS27_North\012");
                      break;
                    case 13502: // Proj_Oklahoma_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS27_South\012");
                      break;
                    case 13531: // Proj_Oklahoma_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS83_North\012");
                      break;
                    case 13532: // Proj_Oklahoma_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS83_South\012");
                      break;
                    case 13601: // Proj_Oregon_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS27_North\012");
                      break;
                    case 13602: // Proj_Oregon_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS27_South\012");
                      break;
                    case 13631: // Proj_Oregon_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS83_North\012");
                      break;
                    case 13632: // Proj_Oregon_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS83_South\012");
                      break;
                    case 13701: // Proj_Pennsylvania_CS27_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS27_North\012");
                      break;
                    case 13702: // Proj_Pennsylvania_CS27_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS27_South\012");
                      break;
                    case 13731: // Proj_Pennsylvania_CS83_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS83_North\012");
                      break;
                    case 13732: // Proj_Pennsylvania_CS83_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS83_South\012");
                      break;
                    case 13800: // Proj_Rhode_Island_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Rhode_Island_CS27\012");
                      break;
                    case 13830: // Proj_Rhode_Island_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Rhode_Island_CS83\012");
                      break;
                    case 13901: // Proj_South_Carolina_CS27_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS27_North\012");
                      break;
                    case 13902: // Proj_South_Carolina_CS27_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS27_South\012");
                      break;
                    case 13930: // Proj_South_Carolina_CS83				
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS83\012");
                      break;
                    case 14001: // Proj_South_Dakota_CS27_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS27_North\012");
                      break;
                    case 14002: // Proj_South_Dakota_CS27_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS27_South\012");
                      break;
                    case 14031: // Proj_South_Dakota_CS83_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS83_North\012");
                      break;
                    case 14032: // Proj_South_Dakota_CS83_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS83_South\012");
                      break;
                    case 14100: // Proj_Tennessee_CS27					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Tennessee_CS27\012");
                      break;
                    case 14130: // Proj_Tennessee_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Tennessee_CS83\012");
                      break;
                    case 14201: // Proj_Texas_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_North\012");
                      break;
                    case 14202: // Proj_Texas_CS27_North_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_North_Central\012");
                      break;
                    case 14203: // Proj_Texas_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_Central\012");
                      break;
                    case 14204: // Proj_Texas_CS27_South_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_South_Central\012");
                      break;
                    case 14205: // Proj_Texas_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_South\012");
                      break;
                    case 14231: // Proj_Texas_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_North\012");
                      break;
                    case 14232: // Proj_Texas_CS83_North_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_North_Central\012");
                      break;
                    case 14233: // Proj_Texas_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_Central\012");
                      break;
                    case 14234: // Proj_Texas_CS83_South_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_South_Central\012");
                      break;
                    case 14235: // Proj_Texas_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_South\012");
                      break;
                    case 14301: // Proj_Utah_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_North\012");
                      break;
                    case 14302: // Proj_Utah_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_Central\012");
                      break;
                    case 14303: // Proj_Utah_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_South\012");
                      break;
                    case 14331: // Proj_Utah_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_North\012");
                      break;
                    case 14332: // Proj_Utah_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_Central\012");
                      break;
                    case 14333: // Proj_Utah_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_South\012");
                      break;
                    case 14400: // Proj_Vermont_CS27					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Vermont_CS27\012");
                      break;
                    case 14430: // Proj_Vermont_CS83					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Vermont_CS83\012");
                      break;
                    case 14501: // Proj_Virginia_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS27_North\012");
                      break;
                    case 14502: // Proj_Virginia_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS27_South\012");
                      break;
                    case 14531: // Proj_Virginia_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS83_North\012");
                      break;
                    case 14532: // Proj_Virginia_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS83_South\012");
                      break;
                    case 14601: // Proj_Washington_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS27_North\012");
                      break;
                    case 14602: // Proj_Washington_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS27_South\012");
                      break;
                    case 14631: // Proj_Washington_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS83_North\012");
                      break;
                    case 14632: // Proj_Washington_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS83_South\012");
                      break;
                    case 14701: // Proj_West_Virginia_CS27_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS27_North\012");
                      break;
                    case 14702: // Proj_West_Virginia_CS27_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS27_South\012");
                      break;
                    case 14731: // Proj_West_Virginia_CS83_North			
                      fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS83_North\012");
                      break;
                    case 14732: // Proj_West_Virginia_CS83_South			
                      fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS83_South\012");
                      break;
                    case 14801: // Proj_Wisconsin_CS27_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_North\012");
                      break;
                    case 14802: // Proj_Wisconsin_CS27_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_Central\012");
                      break;
                    case 14803: // Proj_Wisconsin_CS27_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_South\012");
                      break;
                    case 14831: // Proj_Wisconsin_CS83_North				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_North\012");
                      break;
                    case 14832: // Proj_Wisconsin_CS83_Central				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_Central\012");
                      break;
                    case 14833: // Proj_Wisconsin_CS83_South				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_South\012");
                      break;
                    case 14901: // Proj_Wyoming_CS27_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_East\012");
                      break;
                    case 14902: // Proj_Wyoming_CS27_East_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_East_Central\012");
                      break;
                    case 14903: // Proj_Wyoming_CS27_West_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_West_Central\012");
                      break;
                    case 14904: // Proj_Wyoming_CS27_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_West\012");
                      break;
                    case 14931: // Proj_Wyoming_CS83_East				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_East\012");
                      break;
                    case 14932: // Proj_Wyoming_CS83_East_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_East_Central\012");
                      break;
                    case 14933: // Proj_Wyoming_CS83_West_Central			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_West_Central\012");
                      break;
                    case 14934: // Proj_Wyoming_CS83_West				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_West\012");
                      break;
                    case 15001: // Proj_Alaska_CS27_1					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_1\012");
                      break;
                    case 15002: // Proj_Alaska_CS27_2					
                      fprintf(file_out, "ProjectionGeoKey: ProjectionGeoKey\012");
                      break;
                    case 15003: // Proj_Alaska_CS27_3					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_3\012");
                      break;
                    case 15004: // Proj_Alaska_CS27_4					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_4\012");
                      break;
                    case 15005: // Proj_Alaska_CS27_5					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_5\012");
                      break;
                    case 15006: // Proj_Alaska_CS27_6					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_6\012");
                      break;
                    case 15007: // Proj_Alaska_CS27_7					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_7\012");
                      break;
                    case 15008: // Proj_Alaska_CS27_8					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_8\012");
                      break;
                    case 15009: // Proj_Alaska_CS27_9					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_9\012");
                      break;
                    case 15010: // Proj_Alaska_CS27_10					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_10\012");
                      break;
                    case 15031: // Proj_Alaska_CS83_1					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_1\012");
                      break;
                    case 15032: // Proj_Alaska_CS83_2					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_2\012");
                      break;
                    case 15033: // Proj_Alaska_CS83_3					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_3\012");
                      break;
                    case 15034: // Proj_Alaska_CS83_4					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_4\012");
                      break;
                    case 15035: // Proj_Alaska_CS83_5					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_5\012");
                      break;
                    case 15036: // Proj_Alaska_CS83_6					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_6\012");
                      break;
                    case 15037: // Proj_Alaska_CS83_7					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_7\012");
                      break;
                    case 15038: // Proj_Alaska_CS83_8					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_8\012");
                      break;
                    case 15039: // Proj_Alaska_CS83_9					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_9\012");
                      break;
                    case 15040: // Proj_Alaska_CS83_10					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_10\012");
                      break;
                    case 15101: // Proj_Hawaii_CS27_1					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_1\012");
                      break;
                    case 15102: // Proj_Hawaii_CS27_2					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_2\012");
                      break;
                    case 15103: // Proj_Hawaii_CS27_3					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_3\012");
                      break;
                    case 15104: // Proj_Hawaii_CS27_4					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_4\012");
                      break;
                    case 15105: // Proj_Hawaii_CS27_5					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_5\012");
                      break;
                    case 15131: // Proj_Hawaii_CS83_1					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_1\012");
                      break;
                    case 15132: // Proj_Hawaii_CS83_2					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_2\012");
                      break;
                    case 15133: // Proj_Hawaii_CS83_3					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_3\012");
                      break;
                    case 15134: // Proj_Hawaii_CS83_4					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_4\012");
                      break;
                    case 15135: // Proj_Hawaii_CS83_5					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_5\012");
                      break;
                    case 15201: // Proj_Puerto_Rico_CS27				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Puerto_Rico_CS27\012");
                      break;
                    case 15202: // Proj_St_Croix					
                      fprintf(file_out, "ProjectionGeoKey: Proj_St_Croix\012");
                      break;
                    case 15230: // Proj_Puerto_Rico_Virgin_Is				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Puerto_Rico_Virgin_Is\012");
                      break;
                    case 15914: // Proj_BLM_14N_feet					
                      fprintf(file_out, "ProjectionGeoKey: Proj_BLM_14N_feet\012");
                      break;
                    case 15915: // Proj_BLM_15N_feet					
                      fprintf(file_out, "ProjectionGeoKey: Proj_BLM_15N_feet\012");
                      break;
                    case 15916: // Proj_BLM_16N_feet					
                      fprintf(file_out, "ProjectionGeoKey: Proj_BLM_16N_feet\012");
                      break;
                    case 15917: // Proj_BLM_17N_feet					
                      fprintf(file_out, "ProjectionGeoKey: Proj_BLM_17N_feet\012");
                      break;
                    case 17333: // Proj_SWEREF99_TM		
                      fprintf(file_out, "ProjectionGeoKey: Proj_SWEREF99_TM\012");
                      break;
                    case 17348: // Proj_Map_Grid_of_Australia_48			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_48\012");
                      break;
                    case 17349: // Proj_Map_Grid_of_Australia_49			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_49\012");
                      break;
                    case 17350: // Proj_Map_Grid_of_Australia_50			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_50\012");
                      break;
                    case 17351: // Proj_Map_Grid_of_Australia_51			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_51\012");
                      break;
                    case 17352: // Proj_Map_Grid_of_Australia_52			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_52\012");
                      break;
                    case 17353: // Proj_Map_Grid_of_Australia_53			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_53\012");
                      break;
                    case 17354: // Proj_Map_Grid_of_Australia_54			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_54\012");
                      break;
                    case 17355: // Proj_Map_Grid_of_Australia_55			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_55\012");
                      break;
                    case 17356: // Proj_Map_Grid_of_Australia_56			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_56\012");
                      break;
                    case 17357: // Proj_Map_Grid_of_Australia_57			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_57\012");
                      break;
                    case 17358: // Proj_Map_Grid_of_Australia_58			
                      fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_58\012");
                      break;
                    case 17448: // Proj_Australian_Map_Grid_48				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_48\012");
                      break;
                    case 17449: // Proj_Australian_Map_Grid_49				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_49\012");
                      break;
                    case 17450: // Proj_Australian_Map_Grid_50				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_50\012");
                      break;
                    case 17451: // Proj_Australian_Map_Grid_51				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_51\012");
                      break;
                    case 17452: // Proj_Australian_Map_Grid_52				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_52\012");
                      break;
                    case 17453: // Proj_Australian_Map_Grid_53				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_53\012");
                      break;
                    case 17454: // Proj_Australian_Map_Grid_54				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_54\012");
                      break;
                    case 17455: // Proj_Australian_Map_Grid_55				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_55\012");
                      break;
                    case 17456: // Proj_Australian_Map_Grid_56				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_56\012");
                      break;
                    case 17457: // Proj_Australian_Map_Grid_57				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_57\012");
                      break;
                    case 17458: // Proj_Australian_Map_Grid_58				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_58\012");
                      break;
                    case 18031: // Proj_Argentina_1					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_1\012");
                      break;
                    case 18032: // Proj_Argentina_2					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_2\012");
                      break;
                    case 18033: // Proj_Argentina_3					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_3\012");
                      break;
                    case 18034: // Proj_Argentina_4					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_4\012");
                      break;
                    case 18035: // Proj_Argentina_5					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_5\012");
                      break;
                    case 18036: // Proj_Argentina_6					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_6\012");
                      break;
                    case 18037: // Proj_Argentina_7					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_7\012");
                      break;
                    case 18051: // Proj_Colombia_3W					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_3W\012");
                      break;
                    case 18052: // Proj_Colombia_Bogota				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_Bogota\012");
                      break;
                    case 18053: // Proj_Colombia_3E					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_3E\012");
                      break;
                    case 18054: // Proj_Colombia_6E					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_6E\012");
                      break;
                    case 18072: // Proj_Egypt_Red_Belt					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Egypt_Red_Belt\012");
                      break;
                    case 18073: // Proj_Egypt_Purple_Belt				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Egypt_Purple_Belt\012");
                      break;
                    case 18074: // Proj_Extended_Purple_Belt				
                      fprintf(file_out, "ProjectionGeoKey: Proj_Extended_Purple_Belt\012");
                      break;
                    case 18141: // Proj_New_Zealand_North_Island_Nat_Grid		
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Zealand_North_Island_Nat_Grid\012");
                      break;
                    case 18142: // Proj_New_Zealand_South_Island_Nat_Grid		
                      fprintf(file_out, "ProjectionGeoKey: Proj_New_Zealand_South_Island_Nat_Grid\012");
                      break;
                    case 19900: // Proj_Bahrain_Grid					
                      fprintf(file_out, "ProjectionGeoKey: Proj_Bahrain_Grid\012");
                      break;
                    case 19905: // Proj_Netherlands_E_Indies_Equatorial		
                      fprintf(file_out, "ProjectionGeoKey: Proj_Netherlands_E_Indies_Equatorial\012");
                      break;
                    case 19912: // Proj_RSO_Borneo
                      fprintf(file_out, "ProjectionGeoKey: Proj_RSO_Borneo\012");
                      break;
                    default:
                      fprintf(file_out, "ProjectionGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  }
                  break;
                case 3075: // ProjCoordTransGeoKey
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 1: // CT_TransverseMercator
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_TransverseMercator\012");
                    break;
                  case 2: // CT_TransvMercator_Modified_Alaska
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_TransvMercator_Modified_Alaska\012");
                    break;
                  case 3: // CT_ObliqueMercator
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator\012");
                    break;
                  case 4: // CT_ObliqueMercator_Laborde
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Laborde\012");
                    break;
                  case 5: // CT_ObliqueMercator_Rosenmund
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Rosenmund\012");
                    break;
                  case 6: // CT_ObliqueMercator_Spherical
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Spherical\012");
                    break;
                  case 7: // CT_Mercator
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Mercator\012");
                    break;
                  case 8: // CT_LambertConfConic_2SP
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertConfConic_2SP\012");
                    break;
                  case 9: // CT_LambertConfConic_Helmert
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertConfConic_Helmert\012");
                    break;
                  case 10: // CT_LambertAzimEqualArea
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertAzimEqualArea\012");
                    break;
                  case 11: // CT_AlbersEqualArea
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_AlbersEqualArea\012");
                    break;
                  case 12: // CT_AzimuthalEquidistant
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_AzimuthalEquidistant\012");
                    break;
                  case 13: // CT_EquidistantConic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_EquidistantConic\012");
                    break;
                  case 14: // CT_Stereographic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Stereographic\012");
                    break;
                  case 15: // CT_PolarStereographic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_PolarStereographic\012");
                    break;
                  case 16: // CT_ObliqueStereographic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueStereographic\012");
                    break;
                  case 17: // CT_Equirectangular
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Equirectangular\012");
                    break;
                  case 18: // CT_CassiniSoldner
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_CassiniSoldner\012");
                    break;
                  case 19: // CT_Gnomonic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Gnomonic\012");
                    break;
                  case 20: // CT_MillerCylindrical
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_MillerCylindrical\012");
                    break;
                  case 21: // CT_Orthographic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Orthographic\012");
                    break;
                  case 22: // CT_Polyconic
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Polyconic\012");
                    break;
                  case 23: // CT_Robinson
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Robinson\012");
                    break;
                  case 24: // CT_Sinusoidal
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_Sinusoidal\012");
                    break;
                  case 25: // CT_VanDerGrinten
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_VanDerGrinten\012");
                    break;
                  case 26: // CT_NewZealandMapGrid
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_NewZealandMapGrid\012");
                    break;
                  case 27: // CT_TransvMercator_SouthOriented
                    fprintf(file_out, "ProjCoordTransGeoKey: CT_TransvMercator_SouthOriented\012");
                    break;
                  default:
                    fprintf(file_out, "ProjCoordTransGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 3076: // ProjLinearUnitsGeoKey
                  horizontal_units = lasreader->header.vlr_geo_key_entries[j].value_offset;
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 9001: // Linear_Meter
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Meter\012");
                    break;
                  case 9002: // Linear_Foot
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot\012");
                    break;
                  case 9003: // Linear_Foot_US_Survey
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_US_Survey\012");
                    break;
                  case 9004: // Linear_Foot_Modified_American
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Modified_American\012");
                    break;
                  case 9005: // Linear_Foot_Clarke
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Clarke\012");
                    break;
                  case 9006: // Linear_Foot_Indian
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Indian\012");
                    break;
                  case 9007: // Linear_Link
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link\012");
                    break;
                  case 9008: // Linear_Link_Benoit
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link_Benoit\012");
                    break;
                  case 9009: // Linear_Link_Sears
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link_Sears\012");
                    break;
                  case 9010: // Linear_Chain_Benoit
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Chain_Benoit\012");
                    break;
                  case 9011: // Linear_Chain_Sears
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Chain_Sears\012");
                    break;
                  case 9012: // Linear_Yard_Sears
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Yard_Sears\012");
                    break;
                  case 9013: // Linear_Yard_Indian
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Yard_Indian\012");
                    break;
                  case 9014: // Linear_Fathom
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Fathom\012");
                    break;
                  case 9015: // Linear_Mile_International_Nautical
                    fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Mile_International_Nautical\012");
                    break;
                  default:
                    fprintf(file_out, "ProjLinearUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                case 3077: // ProjLinearUnitSizeGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjLinearUnitSizeGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3078: // ProjStdParallel1GeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjStdParallel1GeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3079: // ProjStdParallel2GeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjStdParallel2GeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;        
                case 3080: // ProjNatOriginLongGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjNatOriginLongGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3081: // ProjNatOriginLatGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjNatOriginLatGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3082: // ProjFalseEastingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseEastingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3083: // ProjFalseNorthingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseNorthingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3084: // ProjFalseOriginLongGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseOriginLongGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3085: // ProjFalseOriginLatGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseOriginLatGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3086: // ProjFalseOriginEastingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseOriginEastingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3087: // ProjFalseOriginNorthingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjFalseOriginNorthingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3088: // ProjCenterLongGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjCenterLongGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3089: // ProjCenterLatGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjCenterLatGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3090: // ProjCenterEastingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjCenterEastingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3091: // ProjCenterNorthingGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjCenterNorthingGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3092: // ProjScaleAtNatOriginGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjScaleAtNatOriginGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3093: // ProjScaleAtCenterGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjScaleAtCenterGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3094: // ProjAzimuthAngleGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjAzimuthAngleGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 3095: // ProjStraightVertPoleLongGeoKey
                  if (lasreader->header.vlr_geo_double_params)
                  {
                    fprintf(file_out, "ProjStraightVertPoleLongGeoKey: %.10g\012",lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                  }
                  break;
                case 4096: // VerticalCSTypeGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 1127: // VertCS_Canadian_Geodetic_Vertical_Datum_2013
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Canadian_Geodetic_Vertical_Datum_2013\012");
                    break;
                  case 5001: // VertCS_Airy_1830_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Airy_1830_ellipsoid\012");
                    break;
                  case 5002: // VertCS_Airy_Modified_1849_ellipsoid 
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Airy_Modified_1849_ellipsoid\012");
                    break;
                  case 5003: // VertCS_ANS_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_ANS_ellipsoid\012");
                    break;
                  case 5004: // VertCS_Bessel_1841_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_1841_ellipsoid\012");
                    break;
                  case 5005: // VertCS_Bessel_Modified_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_Modified_ellipsoid\012");
                    break;
                  case 5006: // VertCS_Bessel_Namibia_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_Namibia_ellipsoid\012");
                    break;
                  case 5007: // VertCS_Clarke_1858_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1858_ellipsoid\012");
                    break;
                  case 5008: // VertCS_Clarke_1866_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1866_ellipsoid\012");
                    break;
                  case 5010: // VertCS_Clarke_1880_Benoit_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_Benoit_ellipsoid\012");
                    break;
                  case 5011: // VertCS_Clarke_1880_IGN_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_IGN_ellipsoid\012");
                    break;
                  case 5012: // VertCS_Clarke_1880_RGS_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_RGS_ellipsoid\012");
                    break;
                  case 5013: // VertCS_Clarke_1880_Arc_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_Arc_ellipsoid\012");
                    break;
                  case 5014: // VertCS_Clarke_1880_SGA_1922_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_SGA_1922_ellipsoid\012");
                    break;
                  case 5015: // VertCS_Everest_1830_1937_Adjustment_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1937_Adjustment_ellipsoid\012");
                    break;
                  case 5016: // VertCS_Everest_1830_1967_Definition_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1967_Definition_ellipsoid\012");
                    break;
                  case 5017: // VertCS_Everest_1830_1975_Definition_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1975_Definition_ellipsoid\012");
                    break;
                  case 5018: // VertCS_Everest_1830_Modified_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_Modified_ellipsoid\012");
                    break;
                  case 5019: // VertCS_GRS_1980_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_GRS_1980_ellipsoid\012");
                    break;
                  case 5020: // VertCS_Helmert_1906_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Helmert_1906_ellipsoid\012");
                    break;
                  case 5021: // VertCS_INS_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_INS_ellipsoid\012");
                    break;
                  case 5022: // VertCS_International_1924_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_International_1924_ellipsoid\012");
                    break;
                  case 5023: // VertCS_International_1967_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_International_1967_ellipsoid\012");
                    break;
                  case 5024: // VertCS_Krassowsky_1940_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Krassowsky_1940_ellipsoid\012");
                    break;
                  case 5025: // VertCS_NWL_9D_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_NWL_9D_ellipsoid\012");
                    break;
                  case 5026: // VertCS_NWL_10D_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_NWL_10D_ellipsoid\012");
                    break;
                  case 5027: // VertCS_Plessis_1817_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Plessis_1817_ellipsoid\012");
                    break;
                  case 5028: // VertCS_Struve_1860_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Struve_1860_ellipsoid\012");
                    break;
                  case 5029: // VertCS_War_Office_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_War_Office_ellipsoid\012");
                    break;
                  case 5030: // VertCS_WGS_84_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_WGS_84_ellipsoid\012");
                    break;
                  case 5031: // VertCS_GEM_10C_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_GEM_10C_ellipsoid\012");
                    break;
                  case 5032: // VertCS_OSU86F_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_OSU86F_ellipsoid\012");
                    break;
                  case 5033: // VertCS_OSU91A_ellipsoid
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_OSU91A_ellipsoid\012");
                    break;
                  case 5101: // VertCS_Newlyn
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Newlyn\012");
                    break;
                  case 5102: // VertCS_North_American_Vertical_Datum_1929
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_North_American_Vertical_Datum_1929\012");
                    break;
                  case 5103: // VertCS_North_American_Vertical_Datum_1988
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_North_American_Vertical_Datum_1988\012");
                    break;
                  case 5104: // VertCS_Yellow_Sea_1956
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Yellow_Sea_1956\012");
                    break;
                  case 5105: // VertCS_Baltic_Sea
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Baltic_Sea\012");
                    break;
                  case 5106: // VertCS_Caspian_Sea
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Caspian_Sea\012");
                    break;
                  case 5114: // VertCS_Canadian_Geodetic_Vertical_Datum_1928
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Canadian_Geodetic_Vertical_Datum_1928\012");
                    break;
                  case 5206: // VertCS_Dansk_Vertikal_Reference_1990
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Dansk_Vertikal_Reference_1990\012");
                    break;
                  case 5215: // VertCS_European_Vertical_Reference_Frame_2007
                    fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_European_Vertical_Reference_Frame_2007\012");
                    break;
                  case 5701: // ODN height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: ODN height (Reserved EPSG)\012");
                    break;
                  case 5702: // NGVD29 height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: NGVD29 height (Reserved EPSG)\012");
                    break;
                  case 5703: // NAVD88 height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: NAVD88 height (Reserved EPSG)\012");
                    break;
                  case 5704: // Yellow Sea (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: Yellow Sea (Reserved EPSG)\012");
                    break;
                  case 5705: // Baltic height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: Baltic height (Reserved EPSG)\012");
                    break;
                  case 5706: // Caspian depth (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: Caspian depth (Reserved EPSG)\012");
                    break;
                  case 5707: // NAP height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: NAP height (Reserved EPSG)\012");
                    break;
                  case 5710: // Oostende height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: Oostende height (Reserved EPSG)\012");
                    break;
                  case 5711: // AHD height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: AHD height (Reserved EPSG)\012");
                    break;
                  case 5712: // AHD (Tasmania) height (Reserved EPSG)
                    fprintf(file_out, "VerticalCSTypeGeoKey: AHD (Tasmania) height (Reserved EPSG)\012");
                    break;
                  case 5776: // Norway Normal Null 1954
                    fprintf(file_out, "VerticalCSTypeGeoKey: Norway Normal Null 1954\012");
                    break;
                  case 5783: // Deutches Haupthohennetz 1992
                    fprintf(file_out, "VerticalCSTypeGeoKey: Deutsches Haupthoehennetz 1992\012");
                    break;
                  case 5941: // Norway Normal Null 2000
                    fprintf(file_out, "VerticalCSTypeGeoKey: Norway Normal Null 2000\012");
                    break;
                  case 6647: // Canadian Geodetic Vertical Datum of 2013
                    fprintf(file_out, "VerticalCSTypeGeoKey: Canadian Geodetic Vertical Datum of 2013\012");
                    break;
                  case 7837: // Deutches Haupthohennetz 2016
                    fprintf(file_out, "VerticalCSTypeGeoKey: Deutsches Haupthoehennetz 2016\012");
                    break;
                  default:
                    if (geoprojectionconverter.set_VerticalCSTypeGeoKey(lasreader->header.vlr_geo_key_entries[j].value_offset, printstring))
                    {
                      fprintf(file_out, "VerticalCSTypeGeoKey: %s\012", printstring);
                    }
                    else
                    {
                      fprintf(file_out, "VerticalCSTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                    }
                  }
                  break;
                case 4097: // VerticalCitationGeoKey
                  if (lasreader->header.vlr_geo_ascii_params)
                  {
                    char dummy[256];
                    strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                    dummy[lasreader->header.vlr_geo_key_entries[j].count-1] = '\0';
                    fprintf(file_out, "VerticalCitationGeoKey: %s\012",dummy);
                  }
                  break;
                case 4098: // VerticalDatumGeoKey 
                  fprintf(file_out, "VerticalDatumGeoKey: Vertical Datum Codes %d\012",lasreader->header.vlr_geo_key_entries[j].value_offset);
                  break;
                case 4099: // VerticalUnitsGeoKey 
                  switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                  {
                  case 9001: // Linear_Meter
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Meter\012");
                    break;
                  case 9002: // Linear_Foot
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot\012");
                    break;
                  case 9003: // Linear_Foot_US_Survey
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_US_Survey\012");
                    break;
                  case 9004: // Linear_Foot_Modified_American
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Modified_American\012");
                    break;
                  case 9005: // Linear_Foot_Clarke
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Clarke\012");
                    break;
                  case 9006: // Linear_Foot_Indian
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Indian\012");
                    break;
                  case 9007: // Linear_Link
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link\012");
                    break;
                  case 9008: // Linear_Link_Benoit
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link_Benoit\012");
                    break;
                  case 9009: // Linear_Link_Sears
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link_Sears\012");
                    break;
                  case 9010: // Linear_Chain_Benoit
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Chain_Benoit\012");
                    break;
                  case 9011: // Linear_Chain_Sears
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Chain_Sears\012");
                    break;
                  case 9012: // Linear_Yard_Sears
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Yard_Sears\012");
                    break;
                  case 9013: // Linear_Yard_Indian
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Yard_Indian\012");
                    break;
                  case 9014: // Linear_Fathom
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Fathom\012");
                    break;
                  case 9015: // Linear_Mile_International_Nautical
                    fprintf(file_out, "VerticalUnitsGeoKey: Linear_Mile_International_Nautical\012");
                    break;
                  default:
                    fprintf(file_out, "VerticalUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                  }
                  break;
                default:
                  fprintf(file_out, "key ID %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].key_id);
                }
              }
            }
          }
          else if (lasheader->vlrs[i].record_id == 34736) // GeoDoubleParamsTag
          {
            fprintf(file_out, "    GeoDoubleParamsTag (number of doubles %d)\012", lasreader->header.vlrs[i].record_length_after_header/8);
            fprintf(file_out, "      ");
            for (int j = 0; j < lasreader->header.vlrs[i].record_length_after_header/8; j++)
            {
              fprintf(file_out, "%lg ", lasheader->vlr_geo_double_params[j]);
            }
            fprintf(file_out, "\012");
          }
          else if (lasheader->vlrs[i].record_id == 34737) // GeoAsciiParamsTag
          {
            fprintf(file_out, "    GeoAsciiParamsTag (number of characters %d)\012", lasreader->header.vlrs[i].record_length_after_header);
            fprintf(file_out, "      ");
            for (int j = 0; j < lasreader->header.vlrs[i].record_length_after_header; j++)
            {
              if (lasheader->vlr_geo_ascii_params[j] >= ' ')
              {
                fprintf(file_out, "%c", lasheader->vlr_geo_ascii_params[j]);
              }
              else
              {
                fprintf(file_out, " ");
              }
            }
            fprintf(file_out, "\012");
          }
          else if (lasheader->vlrs[i].record_id == 2111) // WKT OGC MATH TRANSFORM
          {
            fprintf(file_out, "    WKT OGC MATH TRANSFORM:\012");
            fprintf(file_out, "    %s\012", lasreader->header.vlrs[i].data);
          }
          else if (lasheader->vlrs[i].record_id == 2112) // WKT OGC COORDINATE SYSTEM
          {
            fprintf(file_out, "    WKT OGC COORDINATE SYSTEM:\012");
            fprintf(file_out, "    %s\012", lasreader->header.vlrs[i].data);
          }
        }
        else if ((strcmp(lasheader->vlrs[i].user_id, "LASF_Spec") == 0) && (lasheader->vlrs[i].data != 0))
        {
          if (lasheader->vlrs[i].record_id == 0) // ClassificationLookup
          {
            LASvlr_classification* vlr_classification = (LASvlr_classification*)lasheader->vlrs[i].data;
            int num = lasheader->vlrs[i].record_length_after_header / sizeof(LASvlr_classification);
            for (int j = 0; j < num; j++)
            {
              fprintf(file_out, "    %d %.15s", vlr_classification[j].class_number, vlr_classification[j].description);
            }
            if (num) fprintf(file_out, "\012");
          }
          else if (lasheader->vlrs[i].record_id == 2) // Histogram
          {
          }
          else if (lasheader->vlrs[i].record_id == 3) // TextAreaDescription
          {
            fprintf(file_out, "    ");
            for (int j = 0; j < lasheader->vlrs[i].record_length_after_header; j++)
            {
              if (lasheader->vlrs[i].data[j] != '\0')
              {
                fprintf(file_out, "%c", lasheader->vlrs[i].data[j]);
              }
              else
              {
                fprintf(file_out, " ");
              }
            }
            fprintf(file_out, "\012");
          }
          else if (lasheader->vlrs[i].record_id == 4) // ExtraBytes
          {
            const char* name_table[10] = { "unsigned char", "char", "unsigned short", "short", "unsigned long", "long", "unsigned long long", "long long", "float", "double" };
            fprintf(file_out, "    Extra Byte Descriptions\012");
            for (int j = 0; j < lasheader->vlrs[i].record_length_after_header; j += 192)
            {
              if (lasheader->vlrs[i].data[j+2])
              {
                int type = ((I32)(lasheader->vlrs[i].data[j+2])-1)%10;
                int dim = ((I32)(lasheader->vlrs[i].data[j+2])-1)/10+1;
                if (file_out)
                {
                  fprintf(file_out, "      data type: %d (%s), name \"%s\", description: \"%s\"", (I32)(lasheader->vlrs[i].data[j+2]), name_table[type], (char*)(lasheader->vlrs[i].data + j + 4), (char*)(lasheader->vlrs[i].data + j + 160));
                  if (lasheader->vlrs[i].data[j+3] & 0x02) // if min is set
                  {
                    fprintf(file_out, ", min:");
                    for (int k = 0; k < dim; k++)
                    {
                      if (type < 8)
                      {
#ifdef _WIN32
                        fprintf(file_out, " %I64d", ((I64*)(lasheader->vlrs[i].data + j + 64))[k]);
#else
                        fprintf(file_out, ", %lld", ((I64*)(lasheader->vlrs[i].data + j + 64))[k]);
#endif
                      }
                      else
                      {
                        fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 64))[k]);
                      }
                    }
                  }
                  if (lasheader->vlrs[i].data[j+3] & 0x04) // if max is set
                  {
                    fprintf(file_out, ", max:");
                    for (int k = 0; k < dim; k++)
                    {
                      if (type < 8)
                      {
#ifdef _WIN32
                        fprintf(file_out, " %I64d", ((I64*)(lasheader->vlrs[i].data + j + 88))[k]);
#else
                        fprintf(file_out, ", %lld", ((I64*)(lasheader->vlrs[i].data + j + 88))[k]);
#endif
                      }
                      else
                      {
                        fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 88))[k]);
                      }
                    }
                  }
                  if (lasheader->vlrs[i].data[j+3] & 0x08) // if scale is set
                  {
                    fprintf(file_out, ", scale:");
                    for (int k = 0; k < dim; k++)
                    {
                      fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 112))[k]);
                    }
                  }
                  else
                  {
                    fprintf(file_out, ", scale: 1 (not set)");
                  }
                  if (lasheader->vlrs[i].data[j+3] & 0x10) // if offset is set
                  {
                    fprintf(file_out, ", offset:");
                    for (int k = 0; k < dim; k++)
                    {
                      fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 136))[k]);
                    }
                  }
                  else
                  {
                    fprintf(file_out, ", offset: 0 (not set)");
                  }
                  fprintf(file_out, "\012");
                }
              }
              else
              {
                fprintf(file_out, "      data type: 0 (untyped bytes), size: %d\012", lasheader->vlrs[i].data[j+3]);
              }
            }
          }
          else if ((lasheader->vlrs[i].record_id >= 100) && (lasheader->vlrs[i].record_id < 355)) // WavePacketDescriptor
          {
            LASvlr_wave_packet_descr* vlr_wave_packet_descr = (LASvlr_wave_packet_descr*)lasheader->vlrs[i].data;
            fprintf(file_out, "  index %d bits/sample %d compression %d samples %u temporal %u gain %lg, offset %lg\012", lasheader->vlrs[i].record_id-99, vlr_wave_packet_descr->getBitsPerSample(), vlr_wave_packet_descr->getCompressionType(), vlr_wave_packet_descr->getNumberOfSamples(), vlr_wave_packet_descr->getTemporalSpacing(), vlr_wave_packet_descr->getDigitizerGain(), vlr_wave_packet_descr->getDigitizerOffset());
          }
        }
        else if ((strcmp(lasheader->vlrs[i].user_id, "Raster LAZ") == 0) && (lasheader->vlrs[i].record_id == 7113))
        {
          LASvlrRasterLAZ vlrRasterLAZ;
          if (vlrRasterLAZ.set_payload(lasheader->vlrs[i].data, lasheader->vlrs[i].record_length_after_header))
          {
            fprintf(file_out, "    ncols %6d\012", vlrRasterLAZ.ncols);
            fprintf(file_out, "    nrows %6d\012", vlrRasterLAZ.nrows);
            fprintf(file_out, "    llx   %.10g\012", vlrRasterLAZ.llx);
            fprintf(file_out, "    lly   %.10g\012", vlrRasterLAZ.lly);
            fprintf(file_out, "    stepx    %g\012", vlrRasterLAZ.stepx);
            fprintf(file_out, "    stepy    %g\012", vlrRasterLAZ.stepy);
            if (vlrRasterLAZ.sigmaxy)
            {
              fprintf(file_out, "    sigmaxy %g\012", vlrRasterLAZ.sigmaxy);
            }
            else
            {
              fprintf(file_out, "    sigmaxy <not set>\012");
            }
          }
          else
          {
            fprintf(file_out,"WARNING: corrupt RasterLAZ VLR\n");
          }
        }
      }
    }

    if (file_out && !no_variable_header)
    {
      for (int i = 0; i < (int)lasheader->number_of_extended_variable_length_records; i++)
      {
        fprintf(file_out, "extended variable length header record %d of %d:\012", i+1, (int)lasheader->number_of_extended_variable_length_records);
        fprintf(file_out, "  reserved             %d\012", lasreader->header.evlrs[i].reserved);
        fprintf(file_out, "  user ID              '%.16s'\012", lasreader->header.evlrs[i].user_id);
        fprintf(file_out, "  record ID            %d\012", lasreader->header.evlrs[i].record_id);
#ifdef _WIN32
        fprintf(file_out, "  length after header  %I64d\012", lasreader->header.evlrs[i].record_length_after_header);
#else
        fprintf(file_out, "  length after header  %lld\012", lasreader->header.evlrs[i].record_length_after_header);
#endif
        fprintf(file_out, "  description          '%.32s'\012", lasreader->header.evlrs[i].description);
        if (strcmp(lasheader->evlrs[i].user_id, "LASF_Projection") == 0)
        {
          if (lasheader->evlrs[i].record_id == 2111) // OGC MATH TRANSFORM WKT
          {
            fprintf(file_out, "    OGC MATH TRANSFORM WKT:\012");
            fprintf(file_out, "    %s\012", lasreader->header.evlrs[i].data);
          }
          else if (lasheader->evlrs[i].record_id == 2112) // OGC COORDINATE SYSTEM WKT
          {
            fprintf(file_out, "    OGC COORDINATE SYSTEM WKT:\012");
            fprintf(file_out, "    %s\012", lasreader->header.evlrs[i].data);
          }
        }
      }
    }

    if (file_out && !no_variable_header)
    {
      const LASindex* index = lasreader->get_index();
      if (index)
      {
        fprintf(file_out, "has spatial indexing LAX file\012"); // index->start, index->end, index->full, index->total, index->cells);
      }
    }

    if (file_out && !no_header)
    {
      if (lasheader->user_data_after_header_size) fprintf(file_out, "the header is followed by %u user-defined bytes\012", lasheader->user_data_after_header_size);

      if (lasheader->laszip)
      {
        fprintf(file_out, "LASzip compression (version %d.%dr%d c%d", lasheader->laszip->version_major, lasheader->laszip->version_minor, lasheader->laszip->version_revision, lasheader->laszip->compressor);
        if ((lasheader->laszip->compressor == LASZIP_COMPRESSOR_CHUNKED) || (lasheader->laszip->compressor == LASZIP_COMPRESSOR_LAYERED_CHUNKED)) fprintf(file_out, " %d):", lasheader->laszip->chunk_size);
        else fprintf(file_out, "):");
        for (i = 0; i < (int)lasheader->laszip->num_items; i++) fprintf(file_out, " %s %d", lasheader->laszip->items[i].get_name(), lasheader->laszip->items[i].version);
        fprintf(file_out, "\012");
      }
      if (lasheader->vlr_lastiling)
      {
        LASquadtree lasquadtree;
        lasquadtree.subtiling_setup(lasheader->vlr_lastiling->min_x, lasheader->vlr_lastiling->max_x, lasheader->vlr_lastiling->min_y, lasheader->vlr_lastiling->max_y, lasheader->vlr_lastiling->level, lasheader->vlr_lastiling->level_index, 0);
        F32 min[2], max[2];
        lasquadtree.get_cell_bounding_box(lasheader->vlr_lastiling->level_index, min, max);
        F32 buffer = 0.0f;
        if (lasheader->vlr_lastiling->buffer)
        {
          buffer = (F32)(min[0] - lasheader->min_x);
          if ((F32)(min[1] - lasheader->min_y) > buffer) buffer = (F32)(min[1] - lasheader->min_y);
          if ((F32)(lasheader->max_x - max[0]) > buffer) buffer = (F32)(lasheader->max_x - max[0]);
          if ((F32)(lasheader->max_y - max[1]) > buffer) buffer = (F32)(lasheader->max_y - max[1]);
        }
        fprintf(file_out, "LAStiling (idx %d, lvl %d, sub %d, bbox %.10g %.10g %.10g %.10g%s%s) (size %g x %g, buffer %g)\n", 
          lasheader->vlr_lastiling->level_index, 
          lasheader->vlr_lastiling->level,
          lasheader->vlr_lastiling->implicit_levels,
          lasheader->vlr_lastiling->min_x,
          lasheader->vlr_lastiling->min_y,
          lasheader->vlr_lastiling->max_x,
          lasheader->vlr_lastiling->max_y,
          (lasheader->vlr_lastiling->buffer ? ", buffer" : ""),
          (lasheader->vlr_lastiling->reversible ? ", reversible" : ""),
          max[0]-min[0],
          max[1]-min[1],
          buffer);
      }
      if (lasheader->vlr_lasoriginal)
      {
        fprintf(file_out, "LASoriginal (npoints %u, bbox %.10g %.10g %.10g %.10g %.10g %.10g)\n", 
          (U32)lasheader->vlr_lasoriginal->number_of_point_records, 
          lasheader->vlr_lasoriginal->min_x,
          lasheader->vlr_lasoriginal->min_y,
          lasheader->vlr_lasoriginal->min_z,
          lasheader->vlr_lasoriginal->max_x,
          lasheader->vlr_lasoriginal->max_y,
          lasheader->vlr_lasoriginal->max_z);
      }
    }

    // loop over the points
    F64 enlarged_min_x = lasreader->header.min_x - 0.25 * lasreader->header.x_scale_factor;
    F64 enlarged_max_x = lasreader->header.max_x + 0.25 * lasreader->header.x_scale_factor;
    F64 enlarged_min_y = lasreader->header.min_y - 0.25 * lasreader->header.y_scale_factor;
    F64 enlarged_max_y = lasreader->header.max_y + 0.25 * lasreader->header.y_scale_factor;
    F64 enlarged_min_z = lasreader->header.min_z - 0.25 * lasreader->header.z_scale_factor;
    F64 enlarged_max_z = lasreader->header.max_z + 0.25 * lasreader->header.z_scale_factor;
    LASsummary lassummary;

    if (check_points)
    {
      I64 num_first_returns = 0;
      I64 num_intermediate_returns = 0;
      I64 num_last_returns = 0;
      I64 num_single_returns = 0;
      I64 num_all_returns = 0;
      I64 outside_bounding_box = 0;
      LASoccupancyGrid* lasoccupancygrid = 0;

      if (compute_density)
      {
        lasoccupancygrid = new LASoccupancyGrid(horizontal_units > 9001 ? 6.0f : 2.0f);
      }

      if (file_out && !no_min_max) fprintf(file_out, "reporting minimum and maximum for all LAS point record entries ...\012");

      // maybe seek to start position

      if (subsequence_start) lasreader->seek(subsequence_start);

      while (lasreader->read_point())
      {
        if (lasreader->p_count > subsequence_stop) break;

        if (check_outside)
        {
          if (!lasreader->point.inside_bounding_box(enlarged_min_x, enlarged_min_y, enlarged_min_z, enlarged_max_x, enlarged_max_y, enlarged_max_z))
          {
            outside_bounding_box++;
            if (file_out && report_outside)
            {
              fprintf(file_out, "%u t %g x %g y %g z %g i %d (%d of %d) d %d e %d c %d s %d %u p %d \012", (U32)(lasreader->p_count-1), lasreader->point.get_gps_time(), lasreader->point.get_x(), lasreader->point.get_y(), lasreader->point.get_z(), lasreader->point.get_intensity(), lasreader->point.get_return_number(), lasreader->point.get_number_of_returns(), lasreader->point.get_scan_direction_flag(), lasreader->point.get_edge_of_flight_line(), lasreader->point.get_classification(), lasreader->point.get_scan_angle_rank(), lasreader->point.get_user_data(), lasreader->point.get_point_source_ID());
            }
          }
        }

        lassummary.add(&lasreader->point);

        if (lasoccupancygrid)
        {
          lasoccupancygrid->add(&lasreader->point);
        }

        if (lasreader->point.is_first())
        {
          num_first_returns++;
        }
        if (lasreader->point.is_intermediate())
        {
          num_intermediate_returns++;
        }
        if (lasreader->point.is_last())
        {
          num_last_returns++;
        }
        if (lasreader->point.is_single())
        {
          num_single_returns++;
        }
        num_all_returns++;

        if (lashistogram.active())
        {
          lashistogram.add(&lasreader->point);
        }

        if (file_out && progress && (lasreader->p_count % progress) == 0)
        {
#ifdef _WIN32
          fprintf(file_out, " ... processed %I64d points ...\012", lasreader->p_count);
#else
          fprintf(file_out, " ... processed %lld points ...\012", lasreader->p_count);
#endif
        }
      }
      if (file_out && !no_min_max)
      {
        fprintf(file_out, "  X          %10d %10d\012",lassummary.min.get_X(), lassummary.max.get_X());
        fprintf(file_out, "  Y          %10d %10d\012",lassummary.min.get_Y(), lassummary.max.get_Y());
        fprintf(file_out, "  Z          %10d %10d\012",lassummary.min.get_Z(), lassummary.max.get_Z());
        fprintf(file_out, "  intensity  %10d %10d\012",lassummary.min.intensity, lassummary.max.intensity);
        fprintf(file_out, "  return_number       %d %10d\012",lassummary.min.return_number, lassummary.max.return_number);
        fprintf(file_out, "  number_of_returns   %d %10d\012",lassummary.min.number_of_returns, lassummary.max.number_of_returns);
        fprintf(file_out, "  edge_of_flight_line %d %10d\012",lassummary.min.edge_of_flight_line, lassummary.max.edge_of_flight_line);
        fprintf(file_out, "  scan_direction_flag %d %10d\012",lassummary.min.scan_direction_flag, lassummary.max.scan_direction_flag);
        fprintf(file_out, "  classification  %5d %10d\012",lassummary.min.classification, lassummary.max.classification);
        fprintf(file_out, "  scan_angle_rank %5d %10d\012",lassummary.min.scan_angle_rank, lassummary.max.scan_angle_rank);
        fprintf(file_out, "  user_data       %5d %10d\012",lassummary.min.user_data, lassummary.max.user_data);
        fprintf(file_out, "  point_source_ID %5d %10d\012",lassummary.min.point_source_ID, lassummary.max.point_source_ID);
        if (lasreader->point.have_gps_time)
        {
          fprintf(file_out, "  gps_time %f %f\012",lassummary.min.gps_time, lassummary.max.gps_time);
          if ((lasreader->header.global_encoding & 1) == 0)
          {
            if (!no_warnings && (lassummary.min.gps_time < 0.0 || lassummary.max.gps_time > 604800.0))
            {
              fprintf(file_out, "WARNING: range violates GPS week time specified by global encoding bit 0\012");
            }
          }
          else if (gps_week)
          {
            I32 week_min = (I32)(lassummary.min.gps_time/604800.0 + 1653.4391534391534391534391534392);
            I32 week_max = (I32)(lassummary.max.gps_time/604800.0 + 1653.4391534391534391534391534392);
            I32 secs_min = week_min*604800 - 1000000000;
            I32 secs_max = week_max*604800 - 1000000000;
            fprintf(file_out, "  gps_week %d %d\012", week_min, week_max);
            fprintf(file_out, "  gps_secs_of_week %f %f\012", (lassummary.min.gps_time-secs_min), (lassummary.max.gps_time-secs_max));
          }
        }
        if (lasreader->point.have_rgb)
        {
          fprintf(file_out, "  Color R %d %d\012", lassummary.min.rgb[0], lassummary.max.rgb[0]);
          fprintf(file_out, "        G %d %d\012", lassummary.min.rgb[1], lassummary.max.rgb[1]);
          fprintf(file_out, "        B %d %d\012", lassummary.min.rgb[2], lassummary.max.rgb[2]);
        }
        if (lasreader->point.have_nir)
        {
          fprintf(file_out, "      NIR %d %d\012", lassummary.min.rgb[3], lassummary.max.rgb[3]);
        }
        if (lasreader->point.have_wavepacket)
        {
          fprintf(file_out, "  Wavepacket Index    %d %d\012", lassummary.min.wavepacket.getIndex(), lassummary.max.wavepacket.getIndex());
#ifdef _WIN32
          fprintf(file_out, "             Offset   %I64d %I64d\012", lassummary.min.wavepacket.getOffset(), lassummary.max.wavepacket.getOffset());
#else
          fprintf(file_out, "             Offset   %lld %lld\012", lassummary.min.wavepacket.getOffset(), lassummary.max.wavepacket.getOffset());
#endif
          fprintf(file_out, "             Size     %d %d\012", lassummary.min.wavepacket.getSize(), lassummary.max.wavepacket.getSize());
          fprintf(file_out, "             Location %g %g\012", lassummary.min.wavepacket.getLocation(), lassummary.max.wavepacket.getLocation());
          fprintf(file_out, "             Xt       %g %g\012", lassummary.min.wavepacket.getXt(), lassummary.max.wavepacket.getXt());
          fprintf(file_out, "             Yt       %g %g\012", lassummary.min.wavepacket.getYt(), lassummary.max.wavepacket.getYt());
          fprintf(file_out, "             Zt       %g %g\012", lassummary.min.wavepacket.getZt(), lassummary.max.wavepacket.getZt());
        }
        if (lasreader->point.extended_point_type)
        {
          fprintf(file_out, "  extended_return_number     %6d %6d\012", lassummary.min.extended_return_number, lassummary.max.extended_return_number);
          fprintf(file_out, "  extended_number_of_returns %6d %6d\012", lassummary.min.extended_number_of_returns, lassummary.max.extended_number_of_returns);
          fprintf(file_out, "  extended_classification    %6d %6d\012", lassummary.min.extended_classification, lassummary.max.extended_classification);
          fprintf(file_out, "  extended_scan_angle        %6d %6d\012", lassummary.min.extended_scan_angle, lassummary.max.extended_scan_angle);
          fprintf(file_out, "  extended_scanner_channel   %6d %6d\012", lassummary.min.extended_scanner_channel, lassummary.max.extended_scanner_channel);
        }
        if (lasreader->point.extra_bytes_number && lasreader->point.attributer)
        {
          lassummary.min.attributer = lasreader->point.attributer;
          lassummary.max.attributer = lasreader->point.attributer;
          I32 a;
          for (a = 0; a < lasreader->point.attributer->number_attributes; a++)
          {
            fprintf(file_out, "  attribute%d %10g %10g  ('%s')\012", a, lassummary.min.get_attribute_as_float(a), lassummary.max.get_attribute_as_float(a), lasreader->point.attributer->get_attribute_name(a));
          }
          lassummary.min.attributer = 0;
          lassummary.max.attributer = 0;
        }
        if (((number_of_point_records == 0) && (lasheader->number_of_point_records > 0)) || ((number_of_points_by_return0 == 0) && (lasheader->number_of_points_by_return[0] > 0)))
        {
          fprintf(file_out, "re-reporting LAS header entries populated during read pass:\012");
          if ((number_of_point_records == 0) && (lasheader->number_of_point_records > 0)) fprintf(file_out, "  number of point records    %u\012", lasheader->number_of_point_records);
          if ((number_of_points_by_return0 == 0) && (lasheader->number_of_points_by_return[0] > 0)) fprintf(file_out, "  number of points by return %u %u %u %u %u\012", lasheader->number_of_points_by_return[0], lasheader->number_of_points_by_return[1], lasheader->number_of_points_by_return[2], lasheader->number_of_points_by_return[3], lasheader->number_of_points_by_return[4]);
          fprintf(file_out, "  min x y z                  "); lidardouble2string(printstring, lasheader->min_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
          fprintf(file_out, "  max x y z                  "); lidardouble2string(printstring, lasheader->max_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
        }
      }
      if (!no_warnings && file_out && outside_bounding_box)
      {
#ifdef _WIN32
        fprintf(file_out, "WARNING: %I64d points outside of header bounding box\012", outside_bounding_box);
#else
        fprintf(file_out, "WARNING: %lld points outside of header bounding box\012", outside_bounding_box);
#endif
      }
      if (!no_warnings && file_out && lassummary.has_fluff())
      {
        fprintf(file_out, "WARNING: there is coordinate resolution fluff (x10) in %s%s%s\012", (lassummary.has_fluff(0) ? "X" : ""), (lassummary.has_fluff(1) ? "Y" : ""), (lassummary.has_fluff(2) ? "Z" : ""));
        if (lassummary.has_serious_fluff())
        {
          fprintf(file_out, "WARNING: there is serious coordinate resolution fluff (x100) in %s%s%s\012", (lassummary.has_serious_fluff(0) ? "X" : ""), (lassummary.has_serious_fluff(1) ? "Y" : ""), (lassummary.has_serious_fluff(2) ? "Z" : ""));
          if (lassummary.has_very_serious_fluff())
          {
            fprintf(file_out, "WARNING: there is very serious coordinate resolution fluff (x1000) in %s%s%s\012", (lassummary.has_very_serious_fluff(0) ? "X" : ""), (lassummary.has_very_serious_fluff(1) ? "Y" : ""), (lassummary.has_very_serious_fluff(2) ? "Z" : ""));
            if (lassummary.has_extremely_serious_fluff())
            {
              fprintf(file_out, "WARNING: there is extremely serious coordinate resolution fluff (x10000) in %s%s%s\012", (lassummary.has_extremely_serious_fluff(0) ? "X" : ""), (lassummary.has_extremely_serious_fluff(1) ? "Y" : ""), (lassummary.has_extremely_serious_fluff(2) ? "Z" : ""));
            }
          }
        }
      }
      if (file_out && !no_returns)
      {
#ifdef _WIN32
        fprintf(file_out, "number of first returns:        %I64d\012", num_first_returns);
        fprintf(file_out, "number of intermediate returns: %I64d\012", num_intermediate_returns);
        fprintf(file_out, "number of last returns:         %I64d\012", num_last_returns);
        fprintf(file_out, "number of single returns:       %I64d\012", num_single_returns);
#else
        fprintf(file_out, "number of first returns:        %lld\012", num_first_returns);
        fprintf(file_out, "number of intermediate returns: %lld\012", num_intermediate_returns);
        fprintf(file_out, "number of last returns:         %lld\012", num_last_returns);
        fprintf(file_out, "number of single returns:       %lld\012", num_single_returns);
#endif
      }
      if (file_out && lasoccupancygrid)
      {
        if (num_last_returns)
        {
          if (horizontal_units == 9001)
          {
            fprintf(file_out, "covered area in square meters/kilometers: %d/%.2f\012", 4*lasoccupancygrid->get_num_occupied(), 0.000004*lasoccupancygrid->get_num_occupied());
            fprintf(file_out, "point density: all returns %.2f last only %.2f (per square meter)\012", ((F64)num_all_returns/(4.0*lasoccupancygrid->get_num_occupied())), ((F64)num_last_returns/(4.0*lasoccupancygrid->get_num_occupied())));
            fprintf(file_out, "      spacing: all returns %.2f last only %.2f (in meters)\012", sqrt(4.0*lasoccupancygrid->get_num_occupied()/(F64)num_all_returns), sqrt(4.0*lasoccupancygrid->get_num_occupied()/(F64)num_last_returns));
          }
          else if (horizontal_units == 9002)
          {
            fprintf(file_out, "covered area in square feet/miles: %d/%.2f\012", 36*lasoccupancygrid->get_num_occupied(),1.2913223e-6*lasoccupancygrid->get_num_occupied());
            fprintf(file_out, "point density: all returns %.2f last only %.2f (per square foot)\012", ((F64)num_all_returns/(36.0*lasoccupancygrid->get_num_occupied())), ((F64)num_last_returns/(36.0*lasoccupancygrid->get_num_occupied())));
            fprintf(file_out, "      spacing: all returns %.2f last only %.2f (in feet)\012", sqrt(36.0*lasoccupancygrid->get_num_occupied()/(F64)num_all_returns), sqrt(36.0*lasoccupancygrid->get_num_occupied()/(F64)num_last_returns));
          }
          else if (horizontal_units == 9003)
          {
            fprintf(file_out, "covered area in square survey feet: %d\012", 36*lasoccupancygrid->get_num_occupied());
            fprintf(file_out, "point density: all returns %.2f last only %.2f (per square survey foot)\012", ((F64)num_all_returns/(36.0*lasoccupancygrid->get_num_occupied())), ((F64)num_last_returns/(36.0*lasoccupancygrid->get_num_occupied())));
            fprintf(file_out, "      spacing: all returns %.2f last only %.2f (in survey feet)\012", sqrt(36.0*lasoccupancygrid->get_num_occupied()/(F64)num_all_returns), sqrt(36.0*lasoccupancygrid->get_num_occupied()/(F64)num_last_returns));
          }
          else
          {
            fprintf(file_out, "covered area in square units/kilounits: %d/%.2f\012", 4*lasoccupancygrid->get_num_occupied(), 0.000004*lasoccupancygrid->get_num_occupied());
            fprintf(file_out, "point density: all returns %.2f last only %.2f (per square units)\012", ((F64)num_all_returns/(4.0*lasoccupancygrid->get_num_occupied())), ((F64)num_last_returns/(4.0*lasoccupancygrid->get_num_occupied())));
            fprintf(file_out, "      spacing: all returns %.2f last only %.2f (in units)\012", sqrt(4.0*lasoccupancygrid->get_num_occupied()/(F64)num_all_returns), sqrt(4.0*lasoccupancygrid->get_num_occupied()/(F64)num_last_returns));
          }
        }
        delete lasoccupancygrid;
      }
    }

    lasreader->close();

    FILE* file = 0;

    if (repair_bb || repair_counters)
    {
      if (lasreadopener.is_piped())
      {
        fprintf(stderr, "ERROR: cannot repair header of piped input\n");
        repair_bb = repair_counters = false;
      }
      else if (lasreadopener.is_merged())
      {
        fprintf(stderr, "ERROR: cannot repair header of merged input\n");
        repair_bb = repair_counters = false;
      }
      else if (lasreadopener.is_buffered())
      {
        fprintf(stderr, "ERROR: cannot repair header of buffered input\n");
        repair_bb = repair_counters = false;
      }
      else if (lasreader->get_format() > LAS_TOOLS_FORMAT_LAZ)
      {
        fprintf(stderr, "ERROR: can only repair header for LAS or LAZ files, not for '%s'\n", lasreadopener.get_file_name());
        repair_bb = repair_counters = false;
      }
      file = fopen(lasreadopener.get_file_name(), "rb+");
      if (file == 0)
      {
        fprintf (stderr, "ERROR: could not reopen file '%s' for repair of header\n", lasreadopener.get_file_name());
        repair_bb = repair_counters = false;
      }
    }

    if (check_points)
    {
      // check number_of_point_records

      if ((lasheader->point_data_format < 6) && (lassummary.number_of_point_records != lasheader->number_of_point_records))
      {
        if (repair_counters)
        {
          if (lassummary.number_of_point_records <= U32_MAX)
          {
            U32 number_of_point_records = (U32)lassummary.number_of_point_records;;
            fseek(file, 107, SEEK_SET);
            fwrite(&number_of_point_records, sizeof(U32), 1, file);
            if (file_out)
            {
              fprintf(file_out, "WARNING: real number of point records (%u) is different from header entry (%u). it was repaired. \n", number_of_point_records, lasheader->number_of_point_records);
            }
          }
          else if (lasheader->version_minor < 4)
          {
            if (file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. cannot repair. too big.\n", lassummary.number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. cannot repair. too big.\n", lassummary.number_of_point_records);
#endif
            }
          }
          else if (lasheader->number_of_point_records != 0)
          {
            U32 number_of_point_records = 0;
            fseek(file, 107, SEEK_SET);
            fwrite(&number_of_point_records, sizeof(U32), 1, file);
            if (file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. but header entry is %u instead zero. it was repaired.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. but header entry is %u instead zero. it was repaired.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
          }
          else
          {
            if (file_out)
            {
              fprintf(file_out, "number of point records in header is correct.\n");
            }
          }
        }
        else
        {
          if (!no_warnings && file_out)
          {
            if (lassummary.number_of_point_records <= U32_MAX)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) is different from header entry (%u).\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) is different from header entry (%u).\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
            else if (lasheader->version_minor < 4)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295.\n", lassummary.number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295.\n", lassummary.number_of_point_records);
#endif
            }
            else if (lasheader->number_of_point_records != 0)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. but header entry is %u instead of zero.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. but header entry is %u instead of zero.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
          }
        }
      }
      else if ((lasheader->point_data_format >= 6) && (lasheader->number_of_point_records != 0))
      {
        if (repair_counters)
        {
          U32 number_of_point_records = 0;
          fseek(file, 107, SEEK_SET);
          fwrite(&number_of_point_records, sizeof(U32), 1, file);
        }
        if (!no_warnings && file_out)
        {
          fprintf(file_out, "WARNING: point type is %d but (legacy) number of point records in header is %u instead zero.%s\n", lasheader->point_data_format, lasheader->number_of_point_records, (repair_counters ? "it was repaired." : ""));
        }
      }
      else
      {
        if (repair_counters)
        {
          if (file_out)
          {
            fprintf(file_out, "number of point records in header is correct.\n");
          }
        }
      }

      // check extended_number_of_point_records

      if (lasheader->version_minor > 3)
      {
        if (lassummary.number_of_point_records != (I64)lasheader->extended_number_of_point_records)
        {
          if (repair_counters)
          {
            I64 extended_number_of_point_records = lassummary.number_of_point_records;
            fseek(file, 235 + 12, SEEK_SET);
            fwrite(&extended_number_of_point_records, sizeof(I64), 1, file);
          }
          if (!no_warnings && file_out)
          {
#ifdef _WIN32
            fprintf(file_out, "WARNING: real number of point records (%I64d) is different from extended header entry (%I64d).%s\n", lassummary.number_of_point_records, lasheader->extended_number_of_point_records, (repair_counters ? " it was repaired." : ""));
#else
            fprintf(file_out, "WARNING: real number of point records (%lld) is different from extended header entry (%lld).%s\n", lassummary.number_of_point_records, lasheader->extended_number_of_point_records, (repair_counters ? " it was repaired." : ""));
#endif
          }
        }
        else
        {
          if (repair_counters)
          {
            if (file_out)
            {
              fprintf(file_out, "extended number of point records in header is correct.\n");
            }
          }
        }
      }

      // check number_of_points_by_return[5]

      bool was_set = false;
      for (i = 1; i < 6; i++) if (lasheader->number_of_points_by_return[i-1]) was_set = true;

      bool wrong_entry = false;

      U32 number_of_points_by_return[5];
      for (i = 1; i < 6; i++)
      {
        if ((lasheader->point_data_format < 6) && ((I64)(lasheader->number_of_points_by_return[i-1]) != lassummary.number_of_points_by_return[i]))
        {
          if (lassummary.number_of_points_by_return[i] <= U32_MAX)
          {
            number_of_points_by_return[i-1] = (U32)lassummary.number_of_points_by_return[i];
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
              if (was_set)
              {
                fprintf(file_out, "WARNING: for return %d real number of points by return (%u) is different from header entry (%u).%s\n", i, number_of_points_by_return[i-1], lasheader->number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
              }
              else
              {
                fprintf(file_out, "WARNING: for return %d real number of points by return is %u but header entry was not set.%s\n", i, number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
              }
            }
          }
          else if (lasheader->version_minor < 4)
          {
            if (!no_warnings && file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: for return %d real number of points by return (%I64d) exceeds 4,294,967,295.%s\n", i, lassummary.number_of_points_by_return[i], (repair_counters ? " cannot repair. too big." : ""));
#else
              fprintf(file_out, "WARNING: for return %d real number of points by return (%lld) exceeds 4,294,967,295.%s\n", i, lassummary.number_of_points_by_return[i], (repair_counters ? " cannot repair. too big." : ""));
#endif
            }
          }
          else if (lasheader->number_of_points_by_return[i-1] != 0)
          {
            number_of_points_by_return[i-1] = 0;
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: for return %d real number of points by return (%I64d) exceeds 4,294,967,295. but header entry is %u instead zero.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
#else
              fprintf(file_out, "WARNING: for return %d real number of points by return (%lld) exceeds 4,294,967,295. but header entry is %u instead zero.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
#endif
            }
          }
          else
          {
            number_of_points_by_return[i-1] = 0;
          }
        }
        else if ((lasheader->point_data_format >= 6) && (lasheader->number_of_points_by_return[i-1] != 0))
        {
          number_of_points_by_return[i-1] = 0;
          wrong_entry = true;
          if (!no_warnings && file_out)
          {
            fprintf(file_out, "WARNING: point type is %d but (legacy) number of points by return [%d] in header is %u instead zero.%s\n", lasheader->point_data_format, i, lasheader->number_of_points_by_return[i-1], (repair_counters ? "it was repaired." : ""));
          }
        }
        else
        {
          number_of_points_by_return[i-1] = (U32)lassummary.number_of_points_by_return[i];
        }
      }

      if (repair_counters)
      {
        if (wrong_entry)
        {
          fseek(file, 111, SEEK_SET);
          fwrite(&(number_of_points_by_return[0]), sizeof(U32), 5, file);
        }
        else if (file_out)
        {
          fprintf(file_out, "number of points by return in header is correct.\n");
        }
      }
      
      // check extended_number_of_points_by_return[15]

      if (lasheader->version_minor > 3)
      {
        bool was_set = false;
        for (i = 1; i < 15; i++) if (lasheader->extended_number_of_points_by_return[i-1]) was_set = true;

        bool wrong_entry = false;

        I64 extended_number_of_points_by_return[15];

        for (i = 1; i < 16; i++)
        {
          extended_number_of_points_by_return[i-1] = lassummary.number_of_points_by_return[i];
          if ((I64)lasheader->extended_number_of_points_by_return[i-1] != lassummary.number_of_points_by_return[i])
          {
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
              if (was_set)
              {
#ifdef _WIN32
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %I64d - different from header entry %I64d.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->extended_number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
#else
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %lld - different from header entry %lld.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->extended_number_of_points_by_return[i-1], (repair_counters ? " it was repaired." : ""));
#endif
              }
              else
              {
#ifdef _WIN32
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %I64d but header entry was not set.%s\n", i, lassummary.number_of_points_by_return[i], (repair_counters ? " it was repaired." : ""));
#else
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %lld but header entry was not set.%s\n", i, lassummary.number_of_points_by_return[i], (repair_counters ? " it was repaired." : ""));
#endif
              }
            }
          }
        }

        if (repair_counters)
        {
          if (wrong_entry)
          {
            fseek(file, 235 + 20, SEEK_SET);
            fwrite(&(extended_number_of_points_by_return[0]), sizeof(I64), 15, file);
          }
          else if (file_out)
          {
            fprintf(file_out, "extended number of points by return in header is correct.\n");
          }
        }
      }

      if (!no_warnings && file_out && !no_returns)
      {
#ifdef _WIN32
        if (lassummary.number_of_points_by_return[0]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 0\n", (lassummary.number_of_points_by_return[0] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[0], (lassummary.number_of_points_by_return[0] > 1 ? "s" : ""));
        if (lasheader->version_minor < 4)
        {
          if (lassummary.number_of_points_by_return[6]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 6\n", (lassummary.number_of_points_by_return[6] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[6], (lassummary.number_of_points_by_return[6] > 1 ? "s" : "")); 
          if (lassummary.number_of_points_by_return[7]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 7\n", (lassummary.number_of_points_by_return[7] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[7], (lassummary.number_of_points_by_return[7] > 1 ? "s" : "")); 
        }
#else
        if (lassummary.number_of_points_by_return[0]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 0\n", (lassummary.number_of_points_by_return[0] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[0], (lassummary.number_of_points_by_return[0] > 1 ? "s" : "")); 
        if (lasheader->version_minor < 4)
        {
          if (lassummary.number_of_points_by_return[6]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 6\n", (lassummary.number_of_points_by_return[6] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[6], (lassummary.number_of_points_by_return[6] > 1 ? "s" : ""));
          if (lassummary.number_of_points_by_return[7]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 7\n", (lassummary.number_of_points_by_return[7] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[7], (lassummary.number_of_points_by_return[7] > 1 ? "s" : ""));
        }
#endif

        wrong_entry = false;

        if (lasheader->version_minor > 3)
        {
          for (i = 1; i < 16; i++) if (lassummary.number_of_returns[i]) wrong_entry = true;
          if (wrong_entry)
          {
           fprintf(file_out, "overview over extended number of returns of given pulse:"); 
#ifdef _WIN32
            for (i = 1; i < 16; i++) fprintf(file_out, " %I64d", lassummary.number_of_returns[i]);
#else
            for (i = 1; i < 16; i++) fprintf(file_out, " %lld", lassummary.number_of_returns[i]);
#endif
            fprintf(file_out, "\n"); 
          }
        }
        else
        {
          for (i = 1; i < 8; i++) if (lassummary.number_of_returns[i]) wrong_entry = true;
          if (wrong_entry)
          {
           fprintf(file_out, "overview over number of returns of given pulse:"); 
#ifdef _WIN32
            for (i = 1; i < 8; i++) fprintf(file_out, " %I64d", lassummary.number_of_returns[i]);
#else
            for (i = 1; i < 8; i++) fprintf(file_out, " %lld", lassummary.number_of_returns[i]);
#endif
            fprintf(file_out, "\n"); 
          }
        }

#ifdef _WIN32
        if (lassummary.number_of_returns[0]) fprintf(file_out, "WARNING: there are %I64d points with a number of returns of given pulse of 0\n", lassummary.number_of_returns[0]); 
#else
        if (lassummary.number_of_returns[0]) fprintf(file_out, "WARNING: there are %lld points with a number of returns of given pulse of 0\n", lassummary.number_of_returns[0]); 
#endif
      }

      if (file_out && !no_min_max)
      {
        wrong_entry = false;
        for (i = 0; i < 32; i++) if (lassummary.classification[i]) wrong_entry = true;
        if (lassummary.flagged_synthetic || lassummary.flagged_keypoint || lassummary.flagged_withheld) wrong_entry = true;

        if (wrong_entry)
        {
          fprintf(file_out, "histogram of classification of points:\n"); 
#ifdef _WIN32
          for (i = 0; i < 32; i++) if (lassummary.classification[i]) fprintf(file_out, " %15I64d  %s (%u)\n", lassummary.classification[i], LASpointClassification[i], i);
          if (lassummary.flagged_synthetic)
          {
            fprintf(file_out, " +-> flagged as synthetic: %I64d\n", lassummary.flagged_synthetic);
            for (i = 0; i < 32; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_synthetic_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_synthetic_classification[i], i);
          }
          if (lassummary.flagged_keypoint)
          {
            fprintf(file_out,  " +-> flagged as keypoints: %I64d\n", lassummary.flagged_keypoint);
            for (i = 0; i < 32; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_keypoint_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_keypoint_classification[i], i);
          }
          if (lassummary.flagged_withheld)
          {
            fprintf(file_out,  " +-> flagged as withheld:  %I64d\n", lassummary.flagged_withheld);
            for (i = 0; i < 32; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_withheld_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_withheld_classification[i], i);
          }
#else
          for (i = 0; i < 32; i++) if (lassummary.classification[i]) fprintf(file_out, " %15lld  %s (%u)\n", lassummary.classification[i], LASpointClassification[i], i);
          if (lassummary.flagged_synthetic)
          {
            fprintf(file_out, " +-> flagged as synthetic: %lld\n", lassummary.flagged_synthetic);
            for (i = 0; i < 32; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_synthetic_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_synthetic_classification[i], i);
          }
          if (lassummary.flagged_keypoint)
          {
            fprintf(file_out,  " +-> flagged as keypoints: %lld\n", lassummary.flagged_keypoint);
            for (i = 0; i < 32; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_keypoint_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_keypoint_classification[i], i);
          }
          if (lassummary.flagged_withheld)
          {
            fprintf(file_out,  " +-> flagged as withheld:  %lld\n", lassummary.flagged_withheld);
            for (i = 0; i < 32; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_withheld_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_withheld_classification[i], i);
          }
#endif
        }

        if (lasreader->point.extended_point_type)
        {
#ifdef _WIN32
          if (lassummary.flagged_extended_overlap)
          {
            fprintf(file_out, " +-> flagged as extended overlap: %I64d\n", lassummary.flagged_extended_overlap);
            for (i = 0; i < 32; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_extended_overlap_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_extended_overlap_classification[i], i);
          }
#else
          if (lassummary.flagged_extended_overlap)
          {
            fprintf(file_out, " +-> flagged as extended overlap: %lld\n", lassummary.flagged_extended_overlap);
            for (i = 0; i < 32; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_extended_overlap_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_extended_overlap_classification[i], i);
          }
#endif

          wrong_entry = false;
          for (i = 32; i < 256; i++) if (lassummary.extended_classification[i]) wrong_entry = true;

          if (wrong_entry)
          {
            fprintf(file_out, "histogram of extended classification of points:\n"); 
  #ifdef _WIN32
            for (i = 32; i < 256; i++) if (lassummary.extended_classification[i]) fprintf(file_out, " %15I64d  extended classification (%u)\n", lassummary.extended_classification[i], i);
  #else
            for (i = 32; i < 256; i++) if (lassummary.extended_classification[i]) fprintf(file_out, " %15lld  extended classification (%u)\n", lassummary.extended_classification[i], i);
  #endif
          }
        }
      }

      if (lashistogram.active())
      {
        lashistogram.report(file_out);
        lashistogram.reset();
      }

      double value;
      if (repair_bb)
      {
        wrong_entry = false;
        if (lasheader->get_x(lassummary.max.get_X()) != lasheader->max_x) wrong_entry = true;
        if (lasheader->get_x(lassummary.min.get_X()) != lasheader->min_x) wrong_entry = true;
        if (lasheader->get_y(lassummary.max.get_Y()) != lasheader->max_y) wrong_entry = true;
        if (lasheader->get_y(lassummary.min.get_Y()) != lasheader->min_y) wrong_entry = true;
        if (lasheader->get_z(lassummary.max.get_Z()) != lasheader->max_z) wrong_entry = true;
        if (lasheader->get_z(lassummary.min.get_Z()) != lasheader->min_z) wrong_entry = true;
        if (wrong_entry)
        {
          fseek(file, 179, SEEK_SET);
          value = lasheader->get_x(lassummary.max.get_X()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_x(lassummary.min.get_X()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_y(lassummary.max.get_Y()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_y(lassummary.min.get_Y()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_z(lassummary.max.get_Z()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_z(lassummary.min.get_Z()); fwrite(&value, sizeof(double), 1, file);
          if (file_out) fprintf(file_out, "bounding box was repaired.\n");
        }
        else
        {
          if (file_out) fprintf(file_out, "bounding box is correct.\n");
        }
      }
      else
      {
        value = lasheader->get_x(lassummary.max.get_X());
        if (value > enlarged_max_x)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max x larger than header max x by %lf\n", value - lasheader->max_x);
        }
        value = lasheader->get_x(lassummary.min.get_X());
        if (value < enlarged_min_x)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min x smaller than header min x by %lf\n", lasheader->min_x - value);
        }
        value = lasheader->get_y(lassummary.max.get_Y());
        if (value > enlarged_max_y)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max y larger than header max y by %lf\n", value - lasheader->max_y);
        }
        value = lasheader->get_y(lassummary.min.get_Y());
        if (value < enlarged_min_y)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min y smaller than header min y by %lf\n", lasheader->min_y - value);
        }
        value = lasheader->get_z(lassummary.max.get_Z());
        if (value > enlarged_max_z)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max z larger than header max z by %lf\n", value - lasheader->max_z);
        }
        value = lasheader->get_z(lassummary.min.get_Z());
        if (value < enlarged_min_z)
        {
          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min z smaller than header min z by %lf\n", lasheader->min_z - value);
        }
      }
    }

    if (file_out && (file_out != stdout) && (file_out != stderr)) fclose(file_out);
    laswriteopener.set_file_name(0);

    delete lasreader;
    if (file) fclose(file);
  }

  if (set_system_identifier) delete [] set_system_identifier;
  if (set_generating_software) delete [] set_generating_software;
  if (set_bounding_box) delete [] set_bounding_box;
  if (set_offset) delete [] set_offset;
  if (set_scale) delete [] set_scale;

  byebye(false, wait || (argc==1));

  return 0;
}
