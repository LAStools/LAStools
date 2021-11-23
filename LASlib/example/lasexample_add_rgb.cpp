/*
===============================================================================

  FILE:  lasexample_add_rgb.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    read one LAS / LAZ file without RGB colors and write a LAS / LAZ file 
    with RGB colors.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2016, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    9 April 2016 -- created at Manila's oldest mall, Ali Mall, yes, like the boxer
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasexample_with_rgb in.las out.las\n");
  fprintf(stderr,"lasexample_with_rgb -i in.laz -o out_with_rgb.laz -verbose\n");
  fprintf(stderr,"lasexample_with_rgb -ilas -olaz < in.las > out_with_rgb.laz\n");
  fprintf(stderr,"lasexample_with_rgb -h\n");
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

  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
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
    lasreadopener.parse(argc, argv);
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
    else if (i == argc - 2 && !lasreadopener.active() && !laswriteopener.active())
    {
      lasreadopener.set_file_name(argv[i]);
    }
    else if (i == argc - 1 && !lasreadopener.active() && !laswriteopener.active())
    {
      lasreadopener.set_file_name(argv[i]);
    }
    else if (i == argc - 1 && lasreadopener.active() && !laswriteopener.active())
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

  // check input & output

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(argc == 1);
  }

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

  // make sure it does not have colors yet

  if (lasreader->header.point_data_format == 2 || lasreader->header.point_data_format == 3 || lasreader->header.point_data_format == 5)
  {
    fprintf(stderr, "ERROR: input file '%s' already has RGB colors\n", lasreadopener.get_file_name());
    byebye(argc==1);
  }

  // copy header for output

  LASheader header = lasreader->header;

  // zero the pointers of the other header so they don't get deallocated twice

  lasreader->header.unlink();

  // we need a new LAS point type for adding RGB

  U8 point_type = header.point_data_format;
  U16 point_size = header.point_data_record_length;

  switch (point_type)
  {
  case 0:
    point_type = 2;
    point_size += 6;
    break;
  case 1:
    point_type = 3;
    point_size += 6;
    break;
  case 4:
    point_type = 5;
    point_size += 6;
    break;
  default:
    fprintf(stderr, "ERROR: point type %d not supported\n", (I32)point_type);
    byebye(true, argc==1);
  }

  header.point_data_format = point_type;
  header.point_data_record_length = point_size;

  LASpoint point;

  if (header.laszip)
  {
    LASzip* laszip = new LASzip;
    laszip->setup(point_type, point_size);
    point.init(&header, laszip->num_items, laszip->items, &header);
    delete header.laszip;
    header.laszip = laszip;
  }
  else
  {
    point.init(&header, point_type, point_size, &header);
  }

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&header);

  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }

#ifdef _WIN32
  if (verbose) fprintf(stderr, "reading %I64d points from '%s' and writing them modified to '%s'.\n", lasreader->npoints, lasreadopener.get_file_name(), laswriteopener.get_file_name());
#else
  if (verbose) fprintf(stderr, "reading %lld points from '%s' and writing them modified to '%s'.\n", lasreader->npoints, lasreadopener.get_file_name(), laswriteopener.get_file_name());
#endif

  // loop over points and modify them

  // where there is a point to read
  while (lasreader->read_point())
  {
    // copy the point
    point = lasreader->point;
    // change RGB
    point.rgb[0] = point.rgb[1] = point.rgb[2] = U16_QUANTIZE(((point.get_z() - header.min_z)*65535.0)/(header.max_z - header.min_z));
    if (lasreader->point.get_classification() == 12) lasreader->point.set_classification(1);
    // write the modified point
    laswriter->write_point(&point);
  } 

  laswriter->update_header(&header);

  I64 total_bytes = laswriter->close();
  delete laswriter;

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, lasreader->p_count);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, lasreader->p_count);
#endif

  lasreader->close();
  delete lasreader;

  return 0;
}
