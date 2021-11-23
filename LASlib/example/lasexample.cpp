/*
===============================================================================

  FILE:  lasexample.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    write your own processing tools or how to import from and export to the
    LAS format or - its compressed, but identical twin - the LAZ format.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2014, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    3 January 2011 -- created while too homesick to go to Salzburg with Silke
  
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
  fprintf(stderr,"lasexample in.las out.las\n");
  fprintf(stderr,"lasexample -i in.las -o out.las -verbose\n");
  fprintf(stderr,"lasexample -ilas -olas < in.las > out.las\n");
  fprintf(stderr,"lasexample -h\n");
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

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&lasreader->header);
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
    // modify the point
    lasreader->point.set_point_source_ID(1020);
    lasreader->point.set_user_data(42);
    if (lasreader->point.get_classification() == 12) lasreader->point.set_classification(1);
    lasreader->point.set_Z(lasreader->point.get_Z() + 10);
    // write the modified point
    laswriter->write_point(&lasreader->point);
    // add it to the inventory
    laswriter->update_inventory(&lasreader->point);
  } 

  laswriter->update_header(&lasreader->header, TRUE);

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
