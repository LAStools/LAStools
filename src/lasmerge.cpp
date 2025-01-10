/*
===============================================================================

  FILE:  lasmerge.cpp

  CONTENTS:

    This tool merges multiple LAS file into a single LAS file and outputs it in
    the LAS format. As input the user either provides multiple LAS file names or
    a text file containing a list of LAS file names.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-12, rapidlasso GmbH - fast tools to catch reality

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
#include "lastool.hpp"

class LasTool_lasmerge : public LasTool
{
private:
public:
  void usage() override
  {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "lasmerge -i *.las -o out.las\n");
    fprintf(stderr, "lasmerge -lof lasfiles.txt -o out.las\n");
    fprintf(stderr, "lasmerge -i *.las -o out0000.laz -split 1000000000\n");
    fprintf(stderr, "lasmerge -i file1.las file2.las file3.las -o out.las\n");
    fprintf(stderr, "lasmerge -i file1.las file2.las -reoffset 600000 4000000 0 -olas > out.las\n");
    fprintf(stderr, "lasmerge -lof lasfiles.txt -rescale 0.01 0.01 0.01 -verbose -o out.las\n");
    fprintf(stderr, "lasmerge -h\n");
  };
};

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_GUI
extern int lasmerge_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern void lasmerge_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  LasTool_lasmerge lastool;
  lastool.init(argc, argv, "lasmerge");
  int i;
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
    wait_on_exit = true;
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
      if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
    }
    geoprojectionconverter.parse(argc, argv);
    lasreadopener.parse(argc, argv);
    laswriteopener.parse(argc, argv);
  }

  auto arg_local = [&](int& i) -> bool {
    if (strcmp(argv[i],"-split") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: size", argv[i]);
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
      return false;
    }
    return true;
  };

  lastool.parse(arg_local);

#ifdef COMPILE_WITH_GUI
  if (lastool.gui)
  {
    return lasmerge_gui(argc, argv, &lasreadopener);
  }
#endif

  // read all the input files merged
  lasreadopener.set_merged(TRUE);

#ifdef COMPILE_WITH_MULTI_CORE
  if (lastool.cpu64)
  {
    lasmerge_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, TRUE);
  }
#endif

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
    laserror("no input specified");
  }

  if (!laswriteopener.active())
  {
    laserror("no output specified");
  }

  // make sure we do not corrupt the input file

  if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
  {
    laserror("input and output file name are identical");
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

  start_time = taketime();

  LASreader* lasreader = lasreadopener.open();
  if (lasreader == nullptr)
  {
    laserror("could not open lasreader");
  }
#pragma warning(push)
#pragma warning(disable : 6011)
  LASMessage(LAS_VERBOSE, "merging headers took %g sec. there are %lld points in total.", taketime()-start_time, lasreader->npoints);
#pragma warning(push)
  start_time = taketime();

  // prepare the header for the surviving points

  strncpy_las(lasreader->header.system_identifier, sizeof(lasreader->header.system_identifier), "LAStools (c) by rapidlasso GmbH", 32);
  lasreader->header.system_identifier[31] = '\0';
  char temp[64];
  snprintf(temp, sizeof(temp), "lasmerge%s (version %d)", (IS64?"64":""), LAS_TOOLS_VERSION);
  memset(lasreader->header.generating_software, 0, 32);
  strncpy_las(lasreader->header.generating_software, sizeof(lasreader->header.generating_software), temp, 32);
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
        if (laswriter == 0)
        {
          laserror("could not open laswriter");
        }
      }
      laswriter->write_point(&lasreader->point);
      laswriter->update_inventory(&lasreader->point);
      if (laswriter->p_count == chopchop)
      {
        // close the current writer
        laswriter->update_header(&lasreader->header, TRUE);
        laswriter->close();
        LASMessage(LAS_VERBOSE, "splitting file '%s' took %g sec.",       laswriteopener.get_file_name(), taketime()-start_time);
        start_time = taketime();
        delete laswriter;
        laswriter = 0;
      }
    }
    if (laswriter && laswriter->p_count)
    {
      // close the current writer
      laswriter->update_header(&lasreader->header, TRUE);
      laswriter->close();
      LASMessage(LAS_VERBOSE, "splitting file '%s' took %g sec.", laswriteopener.get_file_name(), taketime()-start_time);
      start_time = taketime(); 
      delete laswriter;
      laswriter = 0;
    }
  }
  else
  {
    if (lasreader->npoints > U32_MAX)
    {
      if (lasreader->header.version_minor < 4)
      {
        laserror("cannot merge %lld points into single LAS 1.%d file. maximum is %u", lasreader->npoints, lasreader->header.version_minor, U32_MAX);
      }
    }
    // open the writer
    LASwriter* laswriter = laswriteopener.open(&lasreader->header);
    if (laswriter == 0)
    {
      laserror("could not open laswriter");
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
    LASMessage(LAS_VERBOSE, "merging files took %g sec.", taketime()-start_time); 
    delete laswriter;
  }
  lasreader->close();
  delete lasreader;
  byebye();
  return 0;
}
