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
  
    (c) 2007-12, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    4 December 2011 -- added option to set classification with '-set_class 2'
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
  fprintf(stderr,"txt2las -parse xyzairn -i lidar.zip -utm 17T -vertical_navd88 -olaz -set_class 2 -quiet\n");
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
  fprintf(stderr,"color, G - green channel, B - blue channel\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"Other parameters are\n");
  fprintf(stderr,"'-set_scale 0.05 0.05 0.001'\n");
  fprintf(stderr,"'-set_offset 500000 2000000 0'\n");
  fprintf(stderr,"'-set_file_creation 67 2011'\n");
  fprintf(stderr,"'-set_system_identifier \"Riegl 500,000 Hz\"'\n");
  fprintf(stderr,"'-set_generating_software \"LAStools\"'\n");
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
extern int txt2las_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, I32 cores);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
#endif
  bool verbose = false;
  bool projection_was_set = false;
  bool quiet = false;
  int file_creation_day = -1;
  int file_creation_year = -1;
  int set_version_major = -1;
  int set_version_minor = -1;
  int set_classification = -1;
  char* set_system_identifier = 0;
  char* set_generating_software = 0;
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
        lasreadopener.set_scale_intensity(atof(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-translate_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_translate_intensity(atof(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-translate_then_scale_intensity") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset factor\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_translate_intensity(atof(argv[i+1]));
        lasreadopener.set_scale_intensity(atof(argv[i+2]));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
      }
      else if (strcmp(argv[i],"-scale_scan_angle") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: factor\n", argv[i]);
          usage(true);
        }
        lasreadopener.set_scale_scan_angle(atof(argv[i+1]));
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
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
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
    else if (strcmp(argv[i],"-quiet") == 0)
    {
      quiet = true;
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
      sscanf(argv[i], "%lf", &(scale_factor[0]));
      i++;
      sscanf(argv[i], "%lf", &(scale_factor[1]));
      i++;
      sscanf(argv[i], "%lf", &(scale_factor[2]));
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
      sscanf(argv[i], "%lf", &(offset[0]));
      i++;
      sscanf(argv[i], "%lf", &(offset[1]));
      i++;
      sscanf(argv[i], "%lf", &(offset[2]));
      lasreadopener.set_offset(offset);
    }
    else if (strcmp(argv[i],"-add_extra") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 3 arguments: data_type name description\n", argv[i]);
        usage(true);
      }
      if (((i+4) < argc) && (atof(argv[i+4]) != 0.0))
      {
        if (((i+5) < argc) && (atof(argv[i+5]) != 0.0))
        {
          lasreadopener.add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]));
          i+=5;
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
      sscanf(argv[i], "%d", &file_creation_day);
      i++;
      sscanf(argv[i], "%d", &file_creation_year);
    }
    else if (strcmp(argv[i],"-set_class") == 0 || strcmp(argv[i],"-set_classification") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
        usage(true);
      }
      i++;
      set_classification = atoi(argv[i]);
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
      return txt2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores);
    }
  }
#endif

  // make sure we have input

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  // make sure that input and output are not *both* piped

  if (lasreadopener.is_piped() && laswriteopener.is_piped())
  {
    fprintf(stderr, "ERROR: input and output cannot both be piped\n");
    byebye(true, argc==1);
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

  // loop over multiple input files

  while (lasreadopener.active())
  {
    if (verbose) start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();

    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

    // check output

    if (!laswriteopener.active())
    {
      // create name from input name
      laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
    }

    // if the output was piped we need to precompute the bounding box, etc ...

    if (laswriteopener.is_piped())
    {
      // because the output goes to a pipe we have to precompute the header
      // information with an additional pass.

      if (verbose) { fprintf(stderr, "piped output. extra read pass over file '%s' ...\n", lasreadopener.get_file_name()); }

      while (lasreader->read_point());
      lasreader->close();

      // output some stats
    
      if (verbose)
      {
#ifdef _WIN32
        fprintf(stderr, "npoints %I64d min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#else
        fprintf(stderr, "npoints %lld min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#endif
        fprintf(stderr, "return histogram %d %d %d %d %d\n", lasreader->header.number_of_points_by_return[0], lasreader->header.number_of_points_by_return[1], lasreader->header.number_of_points_by_return[2], lasreader->header.number_of_points_by_return[3], lasreader->header.number_of_points_by_return[4]);
        fprintf(stderr,"took %g sec.\n", taketime()-start_time); start_time = taketime();
      }

      // reopen lasreader for the second pass

      if (!lasreadopener.reopen(lasreader))
      {
        fprintf(stderr, "ERROR: could not reopen '%s' for main pass\n", lasreadopener.get_file_name());
        byebye(true, argc==1);
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

    if (set_generating_software)
    {
      strncpy(lasreader->header.generating_software, set_generating_software, 32);
      lasreader->header.generating_software[31] = '\0';
    }
    else
    {
      char temp[64];
      sprintf(temp, "txt2las (version %d)", LAS_TOOLS_VERSION);
      strncpy(lasreader->header.generating_software, temp, 32);
      lasreader->header.generating_software[31] = '\0';
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
      lasreader->header.file_creation_day = (U16)333;
      lasreader->header.file_creation_year = (U16)2011;
    }
    else
    {
      lasreader->header.file_creation_day = (U16)file_creation_day;
      lasreader->header.file_creation_year = (U16)file_creation_year;
    }

    // maybe set version

    if (set_version_major != -1) lasreader->header.version_major = (U8)set_version_major;
    if (set_version_minor != -1) lasreader->header.version_minor = (U8)set_version_minor;

    if (set_version_minor == 3)
    {
      lasreader->header.header_size = 235;
      lasreader->header.offset_to_point_data = 235;
    }
    else if (set_version_minor == 4)
    {
      lasreader->header.header_size = 375;
      lasreader->header.offset_to_point_data = 375;
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

    // open the output

    LASwriter* laswriter = laswriteopener.open(&lasreader->header);

    if (laswriter == 0)
    {
      fprintf(stderr, "ERROR: could not open laswriter\n");
      byebye(true, argc==1);
    }

    if (verbose) fprintf(stderr, "reading file '%s' and writing to '%s'\n", lasreadopener.get_file_name(), laswriteopener.get_file_name());

    // loop over points

    while (lasreader->read_point())
    {
      // maybe set classification
      if (set_classification != -1)
      {
        lasreader->point.set_classification(set_classification);
      }
      // write the point
      laswriter->write_point(&lasreader->point);
    }
    lasreader->close();

    if (!laswriteopener.is_piped())
    {
      laswriter->update_header(&lasreader->header, FALSE, TRUE);
      if (verbose)
      {
#ifdef _WIN32
        fprintf(stderr, "npoints %I64d min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#else
        fprintf(stderr, "npoints %lld min %g %g %g max %g %g %g\n", lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z);
#endif
        fprintf(stderr, "return histogram %d %d %d %d %d\n", lasreader->header.number_of_points_by_return[0], lasreader->header.number_of_points_by_return[1], lasreader->header.number_of_points_by_return[2], lasreader->header.number_of_points_by_return[3], lasreader->header.number_of_points_by_return[4]);
     }
    }
    laswriter->close();

    delete laswriter;
    delete lasreader;

    laswriteopener.set_file_name(0);

    if (verbose) fprintf(stderr,"took %g sec.\n", taketime()-start_time);
  }

  byebye(false, argc==1);

  return 0;
}
