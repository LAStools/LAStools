/*
===============================================================================

  FILE:  lasexample_recover_returns_from_trajectory_in_extra_bytes.cpp
  
  CONTENTS:
  
    This source code serves as an example how easily you can write your own
    LAS to LAS converter in case you need a complex conversion that is not
    accomplished with the options of las2las.exe and that would be too slow
    using some kind of scripting language.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2020, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    29 January 2020 -- created at Marco Pollo Cafe, Robinson North in Tacloban
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "lasreader.hpp"
#include "laswriter.hpp"

static void usage(bool wait=false)
{
}

static void byebye(bool wait=false)
{
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(1);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

int main(int argc, char *argv[])
{
  I32 i;
  BOOL verbose = FALSE;
  I32 sensor_x_index = -1;
  I32 sensor_y_index = -1;
  I32 sensor_z_index = -1;
  F64 gps_time = 100000000.0;
  F64 gps_time_increment = 0.000001;
  double start_time = 0;

  fprintf(stderr, "Please license from 'martin.isenburg@gmail.com' to use LAStools commercially.\n");

  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
    fprintf(stderr,"lasrecover.exe is better run in the command line\n");
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
  }
  else
  {
    if (!lasreadopener.parse(argc, argv)) byebye();
    if (!laswriteopener.parse(argc, argv)) byebye();
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = TRUE;
    }
    else if (strcmp(argv[i],"-gps_time_start") == 0 || strcmp(argv[i],"-gpstime_start") == 0)
    {
      i++;
      gps_time = atof(argv[i]);
    }
    else if (strcmp(argv[i],"-gps_time_increment") == 0 || strcmp(argv[i],"-gpstime_increment") == 0)
    {
      i++;
      gps_time_increment = atof(argv[i]);
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage();
    }
  }

  if (verbose) start_time = taketime();

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(argc == 1);
  }

  if (lasreadopener.get_file_name_number() > 1)
  {
    fprintf(stderr,"ERROR: multiple inputs specified\n");
    usage(argc == 1);
  }

  // check output

  if (!laswriteopener.active())
  {
    fprintf(stderr,"ERROR: no output specified\n");
    usage(argc == 1);
  }

  // open lasreader

  LASreader* lasreader = lasreadopener.open();
  if (lasreader == 0)
  {
    fprintf(stderr, "ERROR: could not open lasreader\n");
    byebye(argc==1);
  }

  sensor_x_index = lasreader->header.get_attribute_index("sensor x coord");

  if (sensor_x_index == -1)
  {
    fprintf(stderr, "ERROR: could not find additional attribute 'sensor x coord'\n");
    byebye(argc==1);
  }

  sensor_y_index = lasreader->header.get_attribute_index("sensor y coord");

  if (sensor_y_index == -1)
  {
    fprintf(stderr, "ERROR: could not find additional attribute 'sensor y coord'\n");
    byebye(argc==1);
  }

  sensor_z_index = lasreader->header.get_attribute_index("sensor z coord");

  if (sensor_z_index == -1)
  {
    fprintf(stderr, "ERROR: could not find additional attribute 'sensor z coord'\n");
    byebye(argc==1);
  }

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&lasreader->header);
  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }

  if (verbose) start_time = taketime();

  // process the points

  U32 count = 0;
  F64 sensor_x = F64_MIN;
  F64 sensor_y = F64_MIN;
  F64 sensor_z = F64_MIN;
  U8 points[16][256];
  I32 point_num = 0;

  while (lasreader->read_point())
  {
    lasreader->point.copy_to(points[point_num]);
    if ((sensor_x != lasreader->point.get_attribute_as_float(sensor_x_index)) ||
        (sensor_y != lasreader->point.get_attribute_as_float(sensor_y_index)) ||
        (sensor_z != lasreader->point.get_attribute_as_float(sensor_z_index)))
    {
      if (point_num)
      {
        for (i = 0; i < point_num; i++)
        {
          lasreader->point.copy_from(points[i]);
          if (lasreader->point.extended_point_type)
          {
            lasreader->point.set_extended_return_number((U8)i+1);
            lasreader->point.set_extended_number_of_returns((U8)point_num);
          }
          else
          {
            lasreader->point.set_return_number((U8)i+1);
            lasreader->point.set_number_of_returns((U8)point_num);
          }
          lasreader->point.set_gps_time(gps_time);
          laswriter->write_point(&lasreader->point);
          laswriter->update_inventory(&lasreader->point);
        }
        lasreader->point.copy_from(points[point_num]);
        lasreader->point.copy_to(points[0]);
        gps_time += gps_time_increment;
      }
      sensor_x = lasreader->point.get_attribute_as_float(sensor_x_index);
      sensor_y = lasreader->point.get_attribute_as_float(sensor_y_index);
      sensor_z = lasreader->point.get_attribute_as_float(sensor_z_index);
      point_num = 1;
    }
    else
    {
      point_num++;
    }
    count++;
  }

  // write last set of points

  for (i = 0; i < point_num; i++)
  {
    lasreader->point.copy_from(points[i]);
    if (lasreader->point.extended_point_type)
    {
      lasreader->point.set_extended_return_number((U8)i+1);
      lasreader->point.set_extended_number_of_returns((U8)point_num);
    }
    else
    {
      lasreader->point.set_return_number((U8)i+1);
      lasreader->point.set_number_of_returns((U8)point_num);
    }
    lasreader->point.set_gps_time(gps_time);
    laswriter->write_point(&lasreader->point);
    laswriter->update_inventory(&lasreader->point);
  }

  laswriter->update_header(&lasreader->header, TRUE);
  laswriter->close();
  delete laswriter;

  lasreader->close();
  delete lasreader;

  if (verbose) fprintf(stderr, "processed %u points in %g sec\n", count, taketime()-start_time);

  byebye(argc==1);

  return 0;
}
