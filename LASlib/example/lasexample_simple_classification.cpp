/*
===============================================================================

  FILE:  lasexample_simple_classification.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    read one LAS / LAZ file and then perform your own classification of the 
    points based on some (geometric) criteria (here we make it dependent on
    the range in z values within a certain 2D cell size) and then write the
    resulting LAS / LAZ file with new classifiations to disk.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    19 October 2017 -- created at Cyient in Hyderabad, India as an example
  
===============================================================================
*/

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasexample_simple_classification -i in.las -o out.las\n");
  fprintf(stderr,"lasexample_simple_classification -i in.laz -step 2.0 -range 0.5 -o out.laz\n");
  fprintf(stderr,"lasexample_simple_classification -h\n");
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

int main(int argc, char *argv[])
{
  int i;
  bool verbose = false;
  double step = 1.0;
  double range = 1.0;
  double start_time = 0;

  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.add_file_name(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '–') argv[i][0] = '-';
    }
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
    else if (strcmp(argv[i],"-step") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: cell size\n", argv[i]);
        byebye(true);
      }
      step = atof(argv[i+1]);
      i+=1;
    }
    else if (strcmp(argv[i],"-range") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: vertical range\n", argv[i]);
        byebye(true);
      }
      range = atof(argv[i+1]);
      i+=1;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
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

  // we need to precompute the bounding box

  lasreadopener.set_populate_header(TRUE);

  // check input and output

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  if (verbose) start_time = taketime();

  // possibly loop over multiple input files

  while (lasreadopener.active())
  {
    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

    // create highest and lowest grid

    I32 xdim = I32_FLOOR((lasreader->header.max_x - lasreader->header.min_x)/step)+1;
    I32 ydim = I32_FLOOR((lasreader->header.max_y - lasreader->header.min_y)/step)+1;

    I32* grid_minz = new I32[xdim*ydim];
    I32* grid_maxz = new I32[xdim*ydim];

    for (i = 0; i < xdim*ydim; i++)
    {
      grid_minz[i] = I32_MAX;
      grid_maxz[i] = I32_MIN;
    }

    // first pass over the points

    I32 pos, xpos, ypos;
    while (lasreader->read_point())
    {
      xpos = I32_FLOOR((lasreader->point.get_x() - lasreader->header.min_x)/step);
      ypos = I32_FLOOR((lasreader->point.get_y() - lasreader->header.min_y)/step);

      if (ypos < 0)
      {
        fprintf(stderr, "ERROR: ypos = %d\n", ypos); 
      }

      pos = ypos*xdim + xpos;

      if (pos < 0)
      {
        fprintf(stderr, "ERROR: pos = %d\n", pos); 
        byebye(true);
      }
      else if (pos >= (xdim*ydim))
      {
        fprintf(stderr, "ERROR: pos = %d\n", pos); 
        byebye(true);
      }

      if (grid_minz[pos] > lasreader->point.get_Z())
      {
        grid_minz[pos] = lasreader->point.get_Z();
      }

      if (grid_maxz[pos] < lasreader->point.get_Z())
      {
        grid_maxz[pos] = lasreader->point.get_Z();
      }
    }

    lasreader->close();

    // prepare the quantized range value

    I32 R = I32_QUANTIZE(range / lasreader->header.z_scale_factor);

    // prepare range grid

    for (i = 0; i < xdim*ydim; i++)
    {
      if (grid_minz[i] < I32_MAX)
      {
        grid_minz[i] = grid_maxz[i] - grid_minz[i];
      }
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

    // prepare the LAS header for new file with the re-classified points

    strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
    lasreader->header.system_identifier[31] = '\0';
    char temp[64];
    sprintf(temp, "lascyient (version %d)", LAS_TOOLS_VERSION);
    strncpy(lasreader->header.generating_software, temp, 32);
    lasreader->header.generating_software[31] = '\0';

    // open the writer

    LASwriter* laswriter = laswriteopener.open(&lasreader->header);
    if (laswriter == 0)
    {
      fprintf(stderr, "ERROR: could not open laswriter\n");
      byebye(true, argc==1);
    }

    // reopen the reader

    if (!lasreadopener.reopen(lasreader))
    {
      fprintf(stderr, "ERROR: could not re-open lasreader\n");
      byebye(true);
    }

    I32 count = 0;

    // loop over the points

    while (lasreader->read_point())
    {
      xpos = I32_FLOOR((lasreader->point.get_x() - lasreader->header.min_x)/step);
      ypos = I32_FLOOR((lasreader->point.get_y() - lasreader->header.min_y)/step);

      pos = ypos*xdim + xpos;

      if (grid_minz[pos] < R)
      {
        lasreader->point.set_classification(8);
        count++;
      }

      laswriter->write_point(&lasreader->point);
      laswriter->update_inventory(&lasreader->point);
    }

    delete [] grid_minz;
    delete [] grid_maxz;

    // close the writer

    laswriter->update_header(&lasreader->header, TRUE);
    laswriter->close();
    if (verbose) fprintf(stderr,"classified %d points. processing file '%s' took %g sec.\n", count, lasreadopener.get_file_name(), taketime()-start_time); 
    delete laswriter;

    lasreader->close();
    delete lasreader;

    laswriteopener.set_file_name(0);
  }

  byebye(false, argc==1);

  return 0;
}
