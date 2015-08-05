/*
===============================================================================

  FILE:  lasmerge.cpp
  
  CONTENTS:
  
    This tool merges multiple LAS file into a single LAS file and outputs it in
    the LAS format. As input the user either provides multiple LAS file names or
    a text file containing a list of LAS file names.

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
  
    20 August 2014 -- new option '-keep_lastiling' to preserve the LAStiling VLR
    20 August 2014 -- copy VLRs from empty (zero points) LAS/LAZ files to others
     5 August 2011 -- possible to add/change projection info in command line
    13 May 2011 -- moved indexing, filtering, transforming into LASreader
    21 January 2011 -- added LASreadOpener and reading of multiple LAS files 
     7 January 2011 -- added the LASfilter to drop or keep points 
    12 March 2009 -- updated to ask for input if started without arguments 
    07 November 2007 -- created after email from luis.viveros@digimapas.cl
  
===============================================================================
*/

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "geoprojectionconverter.hpp"

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasmerge -i *.las -o out.las\n");
  fprintf(stderr,"lasmerge -lof lasfiles.txt -o out.las\n");
  fprintf(stderr,"lasmerge -i *.las -o out0000.laz -split 1000000000\n");
  fprintf(stderr,"lasmerge -i file1.las file2.las file3.las -o out.las\n");
  fprintf(stderr,"lasmerge -i file1.las file2.las -reoffset 600000 4000000 0 -olas > out.las\n");
  fprintf(stderr,"lasmerge -lof lasfiles.txt -rescale 0.01 0.01 0.01 -verbose -o out.las\n");
  fprintf(stderr,"lasmerge -h\n");
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

#ifdef COMPILE_WITH_GUI
extern int lasmerge_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
  bool verbose = false;
  bool keep_lastiling = false;
  U32 chopchop = 0;
  bool projection_was_set = false;
  double start_time = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasmerge_gui(argc, argv, 0);
#else
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter input file 1: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.add_file_name(file_name);
    fprintf(stderr,"enter input file 2: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.add_file_name(file_name);
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
    else if (strcmp(argv[i],"-split") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        byebye(true);
      }
      i++;
      chopchop = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-keep_lastiling") == 0)
    {
      keep_lastiling = true;
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
    return lasmerge_gui(argc, argv, &lasreadopener);
  }
#endif

  // read all the input files merged

  lasreadopener.set_merged(TRUE);

  // maybe we want to keep the lastiling 

  if (keep_lastiling)
  {
    lasreadopener.set_keep_lastiling(TRUE);
  }

  // we need to precompute the bounding box

  lasreadopener.set_populate_header(TRUE);

  // check input and output

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  if (!laswriteopener.active())
  {
    fprintf(stderr, "ERROR: no output specified\n");
    byebye(true, argc==1);
  }

  // make sure we do not corrupt the input file

  if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
  {
    fprintf(stderr, "ERROR: input and output file name are identical\n");
    usage(true);
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

  if (verbose) start_time = taketime();

  LASreader* lasreader = lasreadopener.open();
  if (lasreader == 0)
  {
    fprintf(stderr, "ERROR: could not open lasreader\n");
    byebye(true, argc==1);
  }

#ifdef _WIN32
  if (verbose) { fprintf(stderr,"merging headers took %g sec. there are %I64d points in total.\n", taketime()-start_time, lasreader->npoints); start_time = taketime(); }
#else
  if (verbose) { fprintf(stderr,"merging headers took %g sec. there are %lld points in total.\n", taketime()-start_time, lasreader->npoints); start_time = taketime(); }
#endif

  // prepare the header for the surviving points

  strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
  lasreader->header.system_identifier[31] = '\0';
  char temp[64];
  sprintf(temp, "lasmerge (version %d)", LAS_TOOLS_VERSION);
  strncpy(lasreader->header.generating_software, temp, 32);
  lasreader->header.generating_software[31] = '\0';

  if (projection_was_set)
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
  }

  if (chopchop)
  {
    I32 file_number = 0;
    LASwriter* laswriter = 0;
    // loop over the points
    while (lasreader->read_point())
    {
      if (laswriter == 0)
      {
        // open the next writer
        laswriteopener.make_file_name(0, file_number);
        file_number++;
        laswriter = laswriteopener.open(&lasreader->header);
      }
      laswriter->write_point(&lasreader->point);
      laswriter->update_inventory(&lasreader->point);
      if (laswriter->p_count == chopchop)
      {
        // close the current writer
        laswriter->update_header(&lasreader->header, TRUE);
        laswriter->close();
        if (verbose) { fprintf(stderr,"splitting file '%s' took %g sec.\n", laswriteopener.get_file_name(), taketime()-start_time); start_time = taketime(); }
        delete laswriter;
        laswriter = 0;
      }
    }
    if (laswriter && laswriter->p_count)
    {
      // close the current writer
      laswriter->update_header(&lasreader->header, TRUE);
      laswriter->close();
      if (verbose) { fprintf(stderr,"splitting file '%s' took %g sec.\n", laswriteopener.get_file_name(), taketime()-start_time); start_time = taketime(); }
      delete laswriter;
      laswriter = 0;
    }
  }
  else
  {
    // open the writer
    LASwriter* laswriter = laswriteopener.open(&lasreader->header);
    if (laswriter == 0)
    {
      fprintf(stderr, "ERROR: could not open laswriter\n");
      byebye(true, argc==1);
    }
    // loop over the points
    while (lasreader->read_point())
    {
      laswriter->write_point(&lasreader->point);
      laswriter->update_inventory(&lasreader->point);
    }
    // close the writer
    laswriter->update_header(&lasreader->header, TRUE);
    laswriter->close();
    if (verbose) fprintf(stderr,"merging files took %g sec.\n", taketime()-start_time); 
    delete laswriter;
  }

  lasreader->close();
  delete lasreader;

  byebye(false, argc==1);

  return 0;
}
