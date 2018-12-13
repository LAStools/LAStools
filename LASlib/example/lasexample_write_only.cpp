/*
===============================================================================

  FILE:  lasexample_write_only.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    write to the LAS format or - its compressed, but identical twin - the
    LAZ format.

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
  
    26 August 2014 -- created after question from Tristan Allouis of YellowScan
  
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
  lasheader.x_scale_factor = 0.1;
  lasheader.y_scale_factor = 0.01;
  lasheader.z_scale_factor = 0.001;
  lasheader.x_offset = 1000.0;
  lasheader.y_offset = 2000.0;
  lasheader.z_offset = 0.0;
  lasheader.point_data_format = 1;
  lasheader.point_data_record_length = 28;

  // add a VLR with an empty payload that has only meaning to you and your users

  lasheader.add_vlr("my_one_VLR", 12345, 0, 0, FALSE, "this has no payload");

  // add a VLR with a small payload that has only meaning to you and your users

  U8* payload = new U8[64];
  memset(payload, 0, 64);
  strcpy((char*)payload, "this is a small payload followed by zeros");

  // note that LASheader takes over the memory control / deallocation for payload

  lasheader.add_vlr("my_other_VLR", 23456, 64, payload, FALSE, "this has a small payload");

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

  for (i = 0; i < 100; i++)
  {
    // populate the point

    laspoint.set_X(i);
    laspoint.set_Y(i);
    laspoint.set_Z(i);
    laspoint.set_intensity((U16)i);
    laspoint.set_gps_time(0.0006*i);

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
