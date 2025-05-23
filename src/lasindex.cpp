/*
===============================================================================

  FILE:  lasindex.cpp

  CONTENTS:

    This tool creates a *.lax file for a given *.las or *.laz file that contains
    a spatial indexing. When this LAX file is present it will be used to speed up
    access to the relevant areas of the LAS/LAZ file whenever a spatial queries
    of the type

      -inside_tile ll_x ll_y size
      -inside_circle center_x center_y radius
      -inside_rectangle min_x min_y max_x max_y  (or simply -inside)

     appears in the command line of any LAStools invocation. This acceleration is
     also available to users of the LASlib API. The LASreader class has three new
     functions called

     BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
     BOOL inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
     BOOL inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);

     if any of these functions is called the LASreader will only return the points
     that fall inside the specified region and use - when available - the spatial
     indexing information in the LAX file created by lasindex.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2011-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:


    22 March 2022 -- Add -o parameter for user defined output file
     1 May 2017 -- 2nd example for selective decompression for new LAS 1.4 points
    17 May 2011 -- enabling batch processing with wildcards or multiple file names
    29 April 2011 -- created after cable outage during the royal wedding (-:

===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"
#include "lasmessage.hpp"
#include "lastool.hpp"

class LasTool_lasindex : public LasTool
{
private:
public:
  void usage() override
  {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "lasindex lidar.las\n");
    fprintf(stderr, "lasindex *.las\n");
    fprintf(stderr, "lasindex flight1*.las flight2*.las -verbose\n");
    fprintf(stderr, "lasindex lidar.las -tile_size 2 -maximum -50\n");
    fprintf(stderr, "lasindex -h\n");
  };
};

static F64 taketime()
{
  return (F64)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_GUI
extern int lasindex_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern void lasindex_multi_core(int argc, char *argv[], LASreadOpener* lasreadopener, int cores, BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  LasTool_lasindex lastool;
  lastool.init(argc, argv, "lasindex");
  int i;
  F32 tile_size = 0.0f;
  U32 threshold = 1000;
  U32 minimum_points = 100000;
  I32 maximum_intervals = -20;
  BOOL meta = FALSE;
  BOOL dont_reindex = FALSE;
  BOOL append = FALSE;
  F64 start_time = 0.0;
  F64 total_start_time = 0.0;
  char *output_file = NULL;

  LASreadOpener lasreadopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasindex_gui(argc, argv, 0);
#else
    wait_on_exit();
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
#endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
    }
    lasreadopener.parse(argc, argv);
  }

  auto arg_local = [&](int& i) -> bool {
    if (strcmp(argv[i],"-tile_size") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: size", argv[i]);
      }
      i++;
      tile_size = (F32)atof(argv[i]);
    }
    else if (strcmp(argv[i],"-maximum") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: number", argv[i]);
      }
      i++;
      maximum_intervals = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-minimum") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: number", argv[i]);
      }
      i++;
      minimum_points = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-threshold") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: value", argv[i]);
      }
      i++;
      threshold = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-meta") == 0)
    {
      meta = TRUE;
    }
    else if (strcmp(argv[i],"-dont_reindex") == 0)
    {
      dont_reindex = TRUE;
    }
    else if (strcmp(argv[i],"-append") == 0)
    {
      append = TRUE;
    }
    else if (strcmp(argv[i],"-o") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: file output name", argv[i]);
      }
      output_file = argv[i+1];
      i++;
    }
    else
    {
      return false;
    }
    return true;
  };

  lastool.parse(arg_local);

  if (lasreadopener.is_merged())
  {
    laserror("on-the-fly merged input files merged not supported by lasindex");
  }

#ifdef COMPILE_WITH_GUI
  if (lastool.gui)
  {
    return lasindex_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (lastool.cores > 1)
  {
    if (lasreadopener.get_use_stdin())
    {
      LASMessage(LAS_WARNING, "using stdin. ignoring '-cores %d' ...", lastool.cores);
    }
    else if (lasreadopener.get_file_name_number() < 2)
    {
      LASMessage(LAS_WARNING, "only %u input files. ignoring '-cores %d' ...", lasreadopener.get_file_name_number(), lastool.cores);
    }
    else
    {
      lasindex_multi_core(argc, argv, &lasreadopener, lastool.cores, lastool.cpu64);
    }
  }
  if (lastool.cpu64)
  {
    lasindex_multi_core(argc, argv, &lasreadopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    laserror("no input specified");
  }

  // only decompress the one layer we need (for new LAS 1.4 point types only)

  lasreadopener.set_decompress_selective(LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY);

/*
  // lasquadtree test

  LASquadtree quadtree;
  quadtree.setup(0, 99, 0, 99, 10);
  quadtree.intersect_rectangle(10, 10, 20, 20);
  quadtree.get_intersected_cells();
  while (quadtree.has_intersected_cells())
  {
    F32 min[2],max[2];
    quadtree.get_cell_bounding_box(quadtree.intersected_cell, min, max);
    fprintf(stderr," checking tile %d with %g/%g %g/%g\n", quadtree.intersected_cell, min[0], min[1], max[0], max[1]);
  }
  quadtree.intersect_tile(10, 10, 10);
  quadtree.get_intersected_cells();
  while (quadtree.has_intersected_cells())
  {
    F32 min[2],max[2];
    quadtree.get_cell_bounding_box(quadtree.intersected_cell, min, max);
    fprintf(stderr," checking tile %d with %g/%g %g/%g\n", quadtree.intersected_cell, min[0], min[1], max[0], max[1]);
  }
  fprintf(stderr,"intersect circle\n");
  quadtree.intersect_circle(10, 10, 10);
  quadtree.get_intersected_cells();
  while (quadtree.has_intersected_cells())
  {
    F32 min[2],max[2];
    quadtree.get_cell_bounding_box(quadtree.intersected_cell, min, max);
    fprintf(stderr," checking tile %d with %g/%g %g/%g\n", quadtree.intersected_cell, min[0], min[1], max[0], max[1]);
  }
*/

  // possibly loop over multiple input files

  if (lasreadopener.get_file_name_number() > 1)
  {
    total_start_time = taketime();
  }

  U32 skipped = 0;
  U32 indexed = 0;

  FILE* file_meta = 0;

  if (meta)
  {
    CHAR* meta_file_name_base = lasreadopener.get_file_name_base(0);
    I32 len = 0;
    if (meta_file_name_base)
    {
      len = (I32)strlen(meta_file_name_base);
    }
    CHAR* meta_file_name = (CHAR*)malloc(len + 9);
    if (len)
    {
      snprintf(meta_file_name, len + 9, "%s\\lax.txt", meta_file_name_base);
    }
    else
    {
      snprintf(meta_file_name, 9, "lax.txt");
    }
    free(meta_file_name_base);

    file_meta = LASfopen(meta_file_name, "w");

    if (file_meta == 0)
    {
      LASMessage(LAS_WARNING, "cannot open '%s'. skipping creation of meta index ...", meta_file_name);
    }
    free(meta_file_name);
  }

  while (lasreadopener.active())
  {
    start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      laserror("could not open lasreader");
    }

    if (file_meta)
    {
      fprintf(file_meta, "%u,%lld,%lf,%lf,%lf,%lf,%s\012", lasreadopener.get_file_name_current()-1, lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.max_x, lasreader->header.max_y, lasreadopener.get_file_name());
    }

    if (dont_reindex)
    {
      if (lasreader->get_index())
      {
        LASMessage(LAS_INFO, "skipping already indexed file '%s'", lasreadopener.get_file_name());
        delete lasreader;
        skipped++;
        continue;
      }
    }

    // setup the quadtree

    LASquadtree* lasquadtree = new LASquadtree;
    if (tile_size == 0.0f)
    {
      F32 t;
      if (((lasreader->header.max_x - lasreader->header.min_x) < 1000) && ((lasreader->header.max_y - lasreader->header.min_y) < 1000))
      {
        t = 10.0f;
      }
      else if (((lasreader->header.max_x - lasreader->header.min_x) < 10000) && ((lasreader->header.max_y - lasreader->header.min_y) < 10000))
      {
        t = 100.0f;
      }
      else if (((lasreader->header.max_x - lasreader->header.min_x) < 100000) && ((lasreader->header.max_y - lasreader->header.min_y) < 100000))
      {
        t = 1000.0f;
      }
      else if (((lasreader->header.max_x - lasreader->header.min_x) < 1000000) && ((lasreader->header.max_y - lasreader->header.min_y) < 1000000))
      {
        t = 10000.0f;
      }
      else
      {
        t = 100000.0f;
      }
      LASMessage(LAS_VERBOSE, "no tile size specified. setting it to %g ...", t);
      lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, t);
    }
    else
    {
      lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);
    }

    // create index and add points

    LASindex lasindex;
    lasindex.prepare(lasquadtree, threshold);
    while (lasreader->read_point()) lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(lasreader->p_idx-1));

    // delete the reader

    lasreader->close();
    delete lasreader;

    // adaptive coarsening

    lasindex.complete(minimum_points, maximum_intervals);

    // write to file

    if (append)
    {
      if (output_file != NULL)
      {
        lasindex.append(output_file);
      }
      else
      {
        lasindex.append(lasreadopener.get_file_name());
      }
    }
    else
    {
      if (output_file != NULL)
      {
        lasindex.write(output_file);
      }
      else
      {
        lasindex.write(lasreadopener.get_file_name());
      }
    }
    indexed++;
    LASMessage(LAS_INFO, "done with '%s'. took %g sec.", lasreadopener.get_file_name(), taketime()-start_time);
  }

  if (file_meta)
  {
    fclose(file_meta);
    file_meta = 0;
  }

  if (lasreadopener.get_file_name_number() > 1)
  {
    if (dont_reindex)
    {
      LASMessage(LAS_INFO, "done with %u files. skipped %u. indexed %u. total time %g sec.", lasreadopener.get_file_name_number(), skipped, indexed, taketime()-total_start_time);
    }
    else
    {
      LASMessage(LAS_INFO, "done with %u files. total time %g sec.", lasreadopener.get_file_name_number(), taketime()-total_start_time);
    }
  }
  byebye();
  return 0;
}
