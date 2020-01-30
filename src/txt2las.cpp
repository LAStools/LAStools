/*
===============================================================================

  FILE:  txt2las.cpp
  
  CONTENTS:
  
    This tool parses LIDAR data as it is typically stored in standard ASCII
    formats and converts it into the more efficient binary LAS format. If 
    the LAS/LAZ output goes into a file the tool operates in one pass and
    updtes the header in the end. If the LAS/LAZ output is piped the tool
    operates in two passes because it has to precompute the information that
    is stored in the LAS header. The first pass counts the points, measures
    their bounding box, and - if applicable - creates the histogram for the
    number of returns. The second pass writes the points.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:

    21 November 2017 -- allow adding up to 32 (from 10) additional attributes
     7 April 2017 -- new option to '-set_point_type 6' for new LAS 1.4 point types 
    17 January 2016 -- pre-scaling and pre-offsetting of "extra bytes" attributes
     1 January 2016 -- option '-set_ogc_wkt' to store CRS as OGC WKT string
     4 December 2011 -- added option to set classification with '-set_classification 2'
    22 April 2011 -- added command-line flags to specify the projection VLRs
    20 March 2011 -- added capability to read *.zip, *.rar, and *.7z directly
    22 February 2011 -- added option to scale the intensity and scan_angle
    19 Juni 2009 -- added option to skip a number of lines in the text file
    12 March 2009 -- updated to ask for input if started without arguments 
    17 September 2008 -- updated to deal with LAS format version 1.2
    13 July 2007 -- single pass if output is to file by using fopen("rb+" ...) 
    25 June 2007 -- added warning in case that quantization causes a sign flip
    13 June 2007 -- added 'e' and 'd' for the parse string
    26 February 2007 -- created sitting in the SFO lounge waiting for LH 455
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "geoprojectionconverter.hpp"

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"Supported ASCII Inputs:\n");
  fprintf(stderr,"  -i lidar.txt\n");
  fprintf(stderr,"  -i lidar.txt.gz\n");
  fprintf(stderr,"  -i lidar.zip\n");
  fprintf(stderr,"  -i lidar.rar\n");
  fprintf(stderr,"  -i lidar.7z\n");
  fprintf(stderr,"  -stdin (pipe from stdin)\n");
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"txt2las -parse tsxyz -i lidar.txt.gz\n");
  fprintf(stderr,"txt2las -parse xyzairn -i lidar.zip -utm 17T -vertical_navd88 -olaz -set_classification 2 -quiet\n");
  fprintf(stderr,"unzip -p lidar.zip | txt2las -parse xyz -stdin -o lidar.las -longlat -elevation_survey_feet\n");
  fprintf(stderr,"txt2las -i lidar.zip -parse txyzar -scale_scan_angle 57.3 -o lidar.laz\n");
  fprintf(stderr,"txt2las -skip 5 -parse xyz -i lidar.rar -set_file_creation 28 2011 -o lidar.las\n");
  fprintf(stderr,"txt2las -parse xyzsst -verbose -set_scale 0.001 0.001 0.001 -i lidar.txt\n");
  fprintf(stderr,"txt2las -parse xsysz -set_scale 0.1 0.1 0.01 -i lidar.txt.gz -sp83 OH_N -feet\n");
  fprintf(stderr,"las2las -parse tsxyzRGB -i lidar.txt -set_version 1.2 -scale_intensity 65535 -o lidar.las\n");
  fprintf(stderr,"txt2las -h\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"The '-parse tsxyz' flag specifies how to interpret\n");
  fprintf(stderr,"each line of the ASCII file. For example, 'tsxyzssa'\n");
  fprintf(stderr,"means that the first number is the gpstime, the next\n");
  fprintf(stderr,"number should be skipped, the next three numbers are\n");
  fprintf(stderr,"the x, y, and z coordinate, the next two should be\n");
  fprintf(stderr,"skipped, and the next number is the scan angle.\n");
  fprintf(stderr,"The other supported entries are i - intensity,\n");
  fprintf(stderr,"n - number of returns of given pulse, r - number\n");
  fprintf(stderr,"of return, c - classification, u - user data, and\n");
  fprintf(stderr,"p - point source ID, e - edge of flight line flag, and\n");
  fprintf(stderr,"d - direction of scan flag, R - red channel of RGB\n");
  fprintf(stderr,"color, G - green channel, B - blue channel, I - NIR channel,\n");
  fprintf(stderr,"l - scanner channel, o - overlap flag, h - withheld\n");
  fprintf(stderr,"flag, k - keypoint flag, g - synthetic flag, 0 - first\n");
  fprintf(stderr,"additional attribute specified, 1 - second additional\n");
  fprintf(stderr,"attribute specified, 2 - third ...\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"Other parameters are\n");
  fprintf(stderr,"'-set_point_type 6\n");
  fprintf(stderr,"'-set_version 1.4\n");
  fprintf(stderr,"'-set_scale 0.05 0.05 0.001'\n");
  fprintf(stderr,"'-set_offset 500000 2000000 0'\n");
  fprintf(stderr,"'-set_file_creation 67 2011'\n");
  fprintf(stderr,"'-set_system_identifier \"Riegl 500,000 Hz\"'\n");
  fprintf(stderr,"'-set_generating_software \"LAStools\"'\n");
  fprintf(stderr,"'-set_global_encoding 1\n");  
  fprintf(stderr,"'-utm 14T'\n");
  fprintf(stderr,"'-sp83 CA_I -feet -elevation_survey_feet'\n");
  fprintf(stderr,"'-longlat -elevation_feet'\n");
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

extern "C" FILE* fopen_compressed(const char* filename, const char* mode, bool* piped=0);

#ifdef COMPILE_WITH_GUI
extern int txt2las_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int txt2las_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, I32 cores, BOOL cpu64);
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
  bool quiet = false;
  bool projection_was_set = false;
  int file_creation_day = -1;
  int file_creation_year = -1;
  int set_version_major = -1;
  int set_version_minor = -1;
  int set_global_encoding = -1;
  char* set_system_identifier = 0;
  char* set_generating_software = 0;
  bool set_ogc_wkt = false;
  U32 progress = 0;
  double full_start_time = 0.0;
  double start_time = 0.0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return txt2las_gui(argc, argv, 0);
#else
    char file_name[256];
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
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
    // need to get those before lastransform->parse() routine gets them 
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '–') argv[i][0] = '-';
      if (strcmp(argv[i],"-scale_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: factor\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_scale_intensity((F32)atof(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-translate_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_translate_intensity((F32)atof(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-translate_then_scale_intensity") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset factor\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_translate_intensity((F32)atof(argv[i+1]));
        lasreadopener.set_scale_intensity((F32)atof(argv[i+2]));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
      }
      else if (strcmp(argv[i],"-scale_scan_angle") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: factor\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_scale_scan_angle((F32)atof(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
    }
    if (!lasreadopener.parse(argc, argv)) byebye(true);
    if (!geoprojectionconverter.parse(argc, argv)) byebye(true);
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
      verbose = true;
      very_verbose = true;
    }
    else if (strcmp(argv[i],"-quiet") == 0)
    {
      quiet = true;
      verbose = false;
      very_verbose = false;
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
    else if (strcmp(argv[i],"-parse") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: parse_string\n", argv[i]);
        usage(true);
      }
      i++;
      lasreadopener.set_parse_string(argv[i]);
    }
    else if (strcmp(argv[i],"-set_point_type") == 0 || strcmp(argv[i],"-set_point_data_format") == 0 || strcmp(argv[i],"-point_type") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: point_type\n", argv[i]);
        usage(true);
      }
      i++;
      I32 point_type;
      if (sscanf(argv[i],"%d", &point_type) != 1)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' of '%s'\n", argv[i], argv[i-1]);
        usage(true);
      }
      if (point_type < 0 || point_type > 8 || point_type == 4 || point_type == 5)
      {
        fprintf(stderr, "ERROR: point type %d not supported\n", point_type);
        usage(true);
      }
      lasreadopener.set_point_type((U8)point_type);
    }
    else if (strcmp(argv[i],"-skip") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number_of_lines\n", argv[i]);
        usage(true);
      }
      i++;
      lasreadopener.set_skip_lines(atoi(argv[i]));
    }
    else if (strcmp(argv[i],"-set_scale") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i]);
        usage(true);
      }
      F64 scale_factor[3];
      i++;
      if (sscanf(argv[i], "%lf", &(scale_factor[0])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-1]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%lf", &(scale_factor[1])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-2]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%lf", &(scale_factor[2])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-3]);
        usage(true);
      }
      lasreadopener.set_scale_factor(scale_factor);
    }
    else if (strcmp(argv[i],"-set_offset") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i]);
        usage(true);
      }
      F64 offset[3];
      i++;
      if (sscanf(argv[i], "%lf", &(offset[0])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-1]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%lf", &(offset[1])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-2]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%lf", &(offset[2])) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: x y z\n", argv[i-3]);
        usage(true);
      }
      lasreadopener.set_offset(offset);
    }
    else if (strcmp(argv[i],"-add_extra") == 0 || strcmp(argv[i],"-add_attribute") == 0)
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
            if (((i+7) < argc) && ((atof(argv[i+7]) != 0.0) || (strcmp(argv[i+7], "0") == 0) || (strcmp(argv[i+7], "0.0") == 0)))
            {
              if (((i+8) < argc) && ((atof(argv[i+8]) != 0.0) || (strcmp(argv[i+8], "0") == 0) || (strcmp(argv[i+8], "0.0") == 0)))
              {
                lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]), atof(argv[i+7]), atof(argv[i+8]));
                i+=8;
              }
              else
              {
                lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]), atof(argv[i+7]));
                i+=7;
              }
            }
            else
            { 
              lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]));
              i+=6;
            }
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
    else if (strcmp(argv[i],"-set_creation_date") == 0 || strcmp(argv[i],"-set_file_creation") == 0)
    {
      if ((i+2) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: day year\n", argv[i]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%d", &file_creation_day) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: day year\n", argv[i-1]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%d", &file_creation_year) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: day year\n", argv[i-2]);
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-set_global_encoding") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i], "%d", &set_global_encoding) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i-1]);
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-set_system_identifier") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: name\n", argv[i]);
        usage(true);
      }
      i++;
      set_system_identifier = argv[i];
    }
    else if (strcmp(argv[i],"-set_generating_software") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: name\n", argv[i]);
        usage(true);
      }
      i++;
      set_generating_software = argv[i];
    }
    else if (strcmp(argv[i],"-set_ogc_wkt") == 0)
    {
      set_ogc_wkt = true;
    }
    else if (strcmp(argv[i],"-set_version") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: major.minor\n", argv[i]);
        usage(true);
      }
      i++;
      if (sscanf(argv[i],"%d.%d",&set_version_major,&set_version_minor) != 2)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' of '%s'\n", argv[i], argv[i-1]);
        usage(true);
      }
      if (set_version_major != 1)
      {
        fprintf(stderr, "ERROR: major version %d not supported\n", set_version_major);
        usage(true);
      }
      if ((set_version_minor < 0) || (set_version_minor > 4))
      {
        fprintf(stderr, "ERROR: minor version %d not supported\n", set_version_minor);
        usage(true);
      }
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
      usage(true);
    }
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return txt2las_gui(argc, argv, &lasreadopener);
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
      return txt2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores, cpu64);
    }
  }
  if (cpu64)
  {
    return txt2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // make sure we have input

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  // for piped output some checks are needed

  if (laswriteopener.is_piped())
  {
    // make sure that input and output are not *both* piped

    if (lasreadopener.is_piped())
    {
      fprintf(stderr, "ERROR: input and output cannot both be piped\n");
      byebye(true, argc==1);
    }

    // make sure there is only one input file

    if ((lasreadopener.get_file_name_number() > 1) && (!lasreadopener.is_merged()))
    {
      fprintf(stderr, "ERROR: multiple (unmerged) input files cannot produce piped output\n");
      byebye(true, argc==1);
    }

    lasreadopener.set_populate_header(TRUE);
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

  if (!quiet)
  {
    full_start_time = taketime();
  }

  // loop over multiple input files

  while (lasreadopener.active())
  {
    // open lasreader

    LASreader* lasreader = lasreadopener.open();

    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

    if (verbose)
    {
      fprintf(stderr,"opening reader took %g sec.\n", taketime()-start_time);
      start_time = taketime();
    }

    // check output

    if (!laswriteopener.active())
    {
      // create name from input name
      laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
    }

    if (!quiet)
    {
      start_time = taketime();
      if (verbose)
      {
        fprintf(stderr, "reading from '%s' and writing to '%s'\n", (lasreadopener.is_piped() ? "stdin" : lasreadopener.get_file_name()), (laswriteopener.is_piped() ? "stdout" : laswriteopener.get_file_name()));
      }
    }

    // populate header

    for (i = 0; i < 32; i++)
    {
      lasreader->header.system_identifier[i] = '\0';
      lasreader->header.generating_software[i] = '\0';
    }

    if (set_system_identifier)
    {
      strncpy(lasreader->header.system_identifier, set_system_identifier, 32);
      lasreader->header.system_identifier[31] = '\0';
    }
    else
    {
      strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
      lasreader->header.system_identifier[31] = '\0';
    }

    memset(lasreader->header.generating_software, 0, 32);
    if (set_generating_software)
    {
      strncpy(lasreader->header.generating_software, set_generating_software, 32);
    }
    else
    {
      char temp[64];
#ifdef _WIN64
      sprintf(temp, "txt2las64 (version %d)", LAS_TOOLS_VERSION);
#else // _WIN64
      sprintf(temp, "txt2las (version %d)", LAS_TOOLS_VERSION);
#endif // _WIN64
      strncpy(lasreader->header.generating_software, temp, 32);
    }
    lasreader->header.generating_software[31] = '\0';

    // maybe set global encoding

    if (set_global_encoding != -1)
    {
      lasreader->header.global_encoding = set_global_encoding;
    }

    // maybe set creation date

#ifdef _WIN32
    if (lasreadopener.get_file_name() && file_creation_day == -1 && file_creation_year == -1)
    {
      WIN32_FILE_ATTRIBUTE_DATA attr;
	    SYSTEMTIME creation;
      GetFileAttributesEx(lasreadopener.get_file_name(), GetFileExInfoStandard, &attr);
	    FileTimeToSystemTime(&attr.ftCreationTime, &creation);
      int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
      file_creation_day = startday[creation.wMonth] + creation.wDay;
      file_creation_year = creation.wYear;
      // leap year handling
      if ((((creation.wYear)%4) == 0) && (creation.wMonth > 2)) file_creation_day++;
    }
#endif
    if (file_creation_day == -1 && file_creation_year == -1)
    {
      lasreader->header.file_creation_day = (U16)111;
      lasreader->header.file_creation_year = (U16)2018;
    }
    else
    {
      lasreader->header.file_creation_day = (U16)file_creation_day;
      lasreader->header.file_creation_year = (U16)file_creation_year;
    }

    // maybe set version

    if (set_version_major != -1)
    {
      lasreader->header.version_major = (U8)set_version_major;
    }
    if (set_version_minor != -1)
    {
      if (lasreader->header.version_minor < 3)
      {
        if (set_version_minor == 3)
        {
          lasreader->header.header_size += 8;
          lasreader->header.offset_to_point_data += 8;
        }
        else if (set_version_minor == 4)
        {
          lasreader->header.header_size += 148;
          lasreader->header.offset_to_point_data += 148;
        }
      }
      else if (lasreader->header.version_minor == 3)
      {
        if (set_version_minor < 3)
        {
          lasreader->header.header_size -= 8;
          lasreader->header.offset_to_point_data -= 8;
        }
        else if (set_version_minor == 4)
        {
          lasreader->header.header_size += 140;
          lasreader->header.offset_to_point_data += 140;
        }
      }
      else if (lasreader->header.version_minor == 4)
      {
        if (set_version_minor < 3)
        {
          lasreader->header.header_size -= 148;
          lasreader->header.offset_to_point_data -= 148;
        }
        else if (set_version_minor == 3)
        {
          lasreader->header.header_size -= 140;
          lasreader->header.offset_to_point_data -= 140;
        }
      }
      lasreader->header.version_minor = (U8)set_version_minor;
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

      if (set_ogc_wkt) // maybe also set the OCG WKT 
      {
        I32 len = 0;
        CHAR* ogc_wkt = 0;
        if (geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt, !geoprojectionconverter.has_projection(false)))
        {
          lasreader->header.set_geo_ogc_wkt(len, ogc_wkt);
          free(ogc_wkt);
          if ((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6))
          {
            lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
          }
        }
        else
        {
          if (!quiet) fprintf(stderr, "WARNING: cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
        }
      }
    }

    // open the output

    LASwriter* laswriter = laswriteopener.open(&lasreader->header);

    if (laswriter == 0)
    {
      fprintf(stderr, "ERROR: could not open laswriter\n");
      byebye(true, argc==1);
    }

    // loop over points

    while (lasreader->read_point())
    {
      // write the point
      laswriter->write_point(&lasreader->point);
      if (progress && ((lasreader->p_count % progress) == 0))
      {
#ifdef _WIN32
        fprintf(stderr, " ... processed %I64d points ...\012", lasreader->p_count);
#else
        fprintf(stderr, " ... processed %lld points ...\012", lasreader->p_count);
#endif
      }
    }
    lasreader->close();

    if (verbose)
    {
      fprintf(stderr,"main pass took %g sec.\n", taketime()-start_time);
    }

    if (!laswriteopener.is_piped())
    {
      laswriter->update_header(&lasreader->header, FALSE, TRUE);
      if (very_verbose)
      {
#ifdef _WIN32
        fprintf(stderr, "npoints %I64d min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#else
        fprintf(stderr, "npoints %lld min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#endif
        if (lasreader->header.point_data_format > 5)
        {
#ifdef _WIN32
          fprintf(stderr, "return histogram %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d\n", lasreader->header.extended_number_of_points_by_return[0], lasreader->header.extended_number_of_points_by_return[1], lasreader->header.extended_number_of_points_by_return[2], lasreader->header.extended_number_of_points_by_return[3], lasreader->header.extended_number_of_points_by_return[4], lasreader->header.extended_number_of_points_by_return[5], lasreader->header.extended_number_of_points_by_return[6], lasreader->header.extended_number_of_points_by_return[7], lasreader->header.extended_number_of_points_by_return[8], lasreader->header.extended_number_of_points_by_return[9], lasreader->header.extended_number_of_points_by_return[10], lasreader->header.extended_number_of_points_by_return[11], lasreader->header.extended_number_of_points_by_return[12], lasreader->header.extended_number_of_points_by_return[13], lasreader->header.extended_number_of_points_by_return[14]); 
#else
          fprintf(stderr, "return histogram %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", lasreader->header.extended_number_of_points_by_return[0], lasreader->header.extended_number_of_points_by_return[1], lasreader->header.extended_number_of_points_by_return[2], lasreader->header.extended_number_of_points_by_return[3], lasreader->header.extended_number_of_points_by_return[4], lasreader->header.extended_number_of_points_by_return[5], lasreader->header.extended_number_of_points_by_return[6], lasreader->header.extended_number_of_points_by_return[7], lasreader->header.extended_number_of_points_by_return[8], lasreader->header.extended_number_of_points_by_return[9], lasreader->header.extended_number_of_points_by_return[10], lasreader->header.extended_number_of_points_by_return[11], lasreader->header.extended_number_of_points_by_return[12], lasreader->header.extended_number_of_points_by_return[13], lasreader->header.extended_number_of_points_by_return[14]); 
#endif
        }
        else
        {
          fprintf(stderr, "return histogram %d %d %d %d %d\n", lasreader->header.number_of_points_by_return[0], lasreader->header.number_of_points_by_return[1], lasreader->header.number_of_points_by_return[2], lasreader->header.number_of_points_by_return[3], lasreader->header.number_of_points_by_return[4]);
        }
      }
    }
    laswriter->close();

    delete laswriter;
    delete lasreader;

    if (!quiet) { fprintf(stderr, "done with '%s'. total time %g sec.\n", (laswriteopener.is_piped() ? lasreadopener.get_file_name() : laswriteopener.get_file_name()), taketime()-start_time); start_time = taketime(); }
    laswriteopener.set_file_name(0);
  }

  if (!quiet && (lasreadopener.get_file_name_number() > 1)) fprintf(stderr, "done with %u files. total time %g sec.\n", lasreadopener.get_file_name_number(), taketime()-full_start_time);

  byebye(false, argc==1);

  return 0;
}
