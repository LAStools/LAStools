/*
===============================================================================

  FILE:  lasexample_write_only_with_extra_bytes.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    write to the LAS format or - its compressed, but identical twin - the
    LAZ format and include additional per point attributes that are formally
    documented using the "extra bytes" functionality introduced in the 1.4
    version of the LAS specification.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2018, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    22 December 2017 -- created after question of Jung (from China?)
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "laswriter.hpp"

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasexample_write_only out.las\n");
  fprintf(stderr,"lasexample_write_only -o out.las -verbose\n");
  fprintf(stderr,"lasexample_write_only > out.las\n");
  fprintf(stderr,"lasexample_write_only -h\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(1);
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

int main(int argc, char *argv[])
{
  int i;
  bool verbose = false;
  double start_time = 0.0;

  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
  }
  else
  {
    laswriteopener.parse(argc, argv);
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
      verbose = true;
    }
    else if (i == argc - 1 && !laswriteopener.active())
    {
      laswriteopener.set_file_name(argv[i]);
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage();
    }
  }

  if (verbose) start_time = taketime();

  // check output

  if (!laswriteopener.active())
  {
    fprintf(stderr,"ERROR: no output specified\n");
    usage(argc == 1);
  }

  // init header

  LASheader lasheader;
  lasheader.global_encoding = 1;
  lasheader.x_scale_factor = 0.01;
  lasheader.y_scale_factor = 0.01;
  lasheader.z_scale_factor = 0.01;
  lasheader.x_offset =  500000.0;
  lasheader.y_offset = 4100000.0;
  lasheader.z_offset = 0.0;
  lasheader.point_data_format = 1;
  lasheader.point_data_record_length = 28;

  // add two attributes

  // type = 0 : unsigned char
  // type = 1 : char
  // type = 2 : unsigned short
  // type = 3 : short
  // type = 4 : unsigned int
  // type = 5 : int
  // type = 6 : unsigned int64
  // type = 7 : int64
  // type = 8 : float  (try not to use)
  // type = 9 : double (try not to use)

  I32 attribute_index_echo_width = -1;
  I32 attribute_index_height_above_ground = -1;

  // first an unsigned short that expresses echo-width in nanoseconds [ns]

  F64 echo_width_scale = 0.1;
  F64 echo_width_offset = 0.0;

  try {
    I32 type = 2; // unsigned short
    LASattribute attribute(type, "echo width", "full width at half maximum [ns]");
    attribute.set_scale(echo_width_scale);
    attribute.set_offset(echo_width_offset);
    attribute_index_echo_width = lasheader.add_attribute(attribute);
  }
  catch(...) {
    fprintf(stderr,"ERROR: initializing first additional attribute\n");
    usage(argc == 1);
  }

  // second an signed short that expresses height above ground in meters [m]

  F64 height_above_ground_scale = 0.01;
  F64 height_above_ground_offset = 300.0;

  try {
    I32 type = 3; // signed short
    LASattribute attribute(type, "height above ground", "vertical distance to TIN [m]");
    attribute.set_scale(height_above_ground_scale);
    attribute.set_offset(height_above_ground_offset);
    attribute_index_height_above_ground = lasheader.add_attribute(attribute);
  }
  catch(...) {
    fprintf(stderr,"ERROR: initializing second additional attribute\n");
    usage(argc == 1);
  }

  // create extra bytes VLR

  lasheader.update_extra_bytes_vlr();

  // add number of extra bytes to the point size
  
  lasheader.point_data_record_length += lasheader.get_attributes_size();

  // get indices for fast extra bytes access

  I32 attribute_start_echo_width = lasheader.get_attribute_start(attribute_index_echo_width);
  I32 attribute_start_height_above_ground = lasheader.get_attribute_start(attribute_index_height_above_ground);

  // init point 

  LASpoint laspoint;
  laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&lasheader);
  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }

  if (verbose) fprintf(stderr, "writing 100 points to '%s'.\n", laswriteopener.get_file_name());

  // write points

  F32 echo_width;
  F32 height_above_ground;
  I32 temp_i;

  for (i = 0; i < 100; i++)
  {
    // populate the point

    laspoint.set_X(i);
    laspoint.set_Y(i);
    laspoint.set_Z(i);
    laspoint.set_intensity((U16)i);
    laspoint.set_gps_time(23365829.0 + 0.0006*i);

    // create additional attributes 

    echo_width = 4.0f + 0.1f*(rand()%20);
    height_above_ground = -2.0f + 0.01f*(rand()%3000);

    // populate the point's extra bytes
    
    temp_i = I32_QUANTIZE((echo_width-echo_width_offset)/echo_width_scale);
    if ((temp_i < U16_MIN) || (temp_i > U16_MAX))
    {
      fprintf(stderr, "WARNING: attribute 'echo width' of type U16 is %d. clamped to [%d %d] range.\n", temp_i, U16_MIN, U16_MAX);
      laspoint.set_attribute(attribute_start_echo_width, U16_CLAMP(temp_i));
    }
    else
    {
      laspoint.set_attribute(attribute_start_echo_width, ((U16)temp_i));
    }

    temp_i = I32_QUANTIZE((height_above_ground - height_above_ground_offset)/height_above_ground_scale);
    if ((temp_i < I16_MIN) || (temp_i > I16_MAX))
    {
      fprintf(stderr, "WARNING: attribute 'height above ground' of type I16 is %d. clamped to [%d %d] range.\n", temp_i, I16_MIN, I16_MAX);
      laspoint.set_attribute(attribute_start_height_above_ground, I16_CLAMP(temp_i));
    }
    else
    {
      laspoint.set_attribute(attribute_start_height_above_ground, ((I16)temp_i));
    }

    // write the point

    laswriter->write_point(&laspoint);

    // add it to the inventory

    laswriter->update_inventory(&laspoint);
  }

  // update the header

  laswriter->update_header(&lasheader, TRUE);

  // close the writer

  I64 total_bytes = laswriter->close();

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#endif

  delete laswriter;

  return 0;
}
