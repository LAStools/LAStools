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
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2011-2019, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
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

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasindex lidar.las\n");
  fprintf(stderr,"lasindex *.las\n");
  fprintf(stderr,"lasindex flight1*.las flight2*.las -verbose\n");
  fprintf(stderr,"lasindex lidar.las -tile_size 2 -maximum -50\n");
  fprintf(stderr,"lasindex -h\n");
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

static F64 taketime()
{
  return (F64)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_GUI
extern int lasindex_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int lasindex_multi_core(int argc, char *argv[], LASreadOpener* lasreadopener, int cores, BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
  BOOL cpu64 = FALSE;
#endif
  BOOL quiet = FALSE;
  BOOL verbose = FALSE;
  BOOL very_verbose = FALSE;
  F32 tile_size = 0.0f;
  U32 threshold = 1000;
  U32 minimum_points = 100000;
  I32 maximum_intervals = -20;
  BOOL meta = FALSE;
  BOOL dont_reindex = FALSE;
  BOOL append = FALSE;
  F64 start_time = 0.0;
  F64 total_start_time = 0.0;

  LASreadOpener lasreadopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasindex_gui(argc, argv, 0);
#else
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
      if (argv[i][0] == '–') argv[i][0] = '-';
    }
    if (!lasreadopener.parse(argc, argv)) byebye(true);
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
      verbose = TRUE;
    }
    else if (strcmp(argv[i],"-vv") == 0 || strcmp(argv[i],"-very_verbose") == 0)
    {
      verbose = TRUE;
      very_verbose = TRUE;
    }
    else if (strcmp(argv[i],"-quiet") == 0)
    {
      quiet = TRUE;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-fail") == 0)
    {
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
    else if (strcmp(argv[i],"-cpu64") == 0)
    {
#ifdef COMPILE_WITH_MULTI_CORE
      cpu64 = TRUE;
#else
      fprintf(stderr, "WARNING: not compiled with 64 bit support. ignoring '-cpu64' ...\n");
#endif
      argv[i][0] = '\0';
    }
    else if (strcmp(argv[i],"-tile_size") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        byebye(true);
      }
      i++;
      tile_size = (F32)atof(argv[i]);
    }
    else if (strcmp(argv[i],"-maximum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        byebye(true);
      }
      i++;
      maximum_intervals = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-minimum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        byebye(true);
      }
      i++;
      minimum_points = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-threshold") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
        byebye(true);
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
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      byebye(true);
    }
  }

  if (lasreadopener.is_merged())
  {
    fprintf(stderr,"ERROR: on-the-fly merged input files merged not supported by lasindex\n");
    byebye(true);
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return lasindex_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (cores > 1)
  {
    if (lasreadopener.get_file_name_number() < 2)
    {
      fprintf(stderr,"WARNING: only %u input files. ignoring '-cores %d' ...\n", lasreadopener.get_file_name_number(), cores);
    }
    else
    {
      return lasindex_multi_core(argc, argv, &lasreadopener, cores, cpu64);
    }
  }
  if (cpu64)
  {
    return lasindex_multi_core(argc, argv, &lasreadopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    byebye(true, argc==1);
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

  if (verbose && lasreadopener.get_file_name_number() > 1)
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
      sprintf(meta_file_name, "%s\\lax.txt", meta_file_name_base); 
    }
    else
    {
      sprintf(meta_file_name, "lax.txt"); 
    }
    free(meta_file_name_base);

    file_meta = fopen(meta_file_name, "w");

    if (file_meta == 0)
    {
      fprintf(stderr,"WARNING: cannot open '%s'. skipping creation of meta index ...\n", meta_file_name);
    }
    free(meta_file_name);
  }

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

    if (file_meta)
    {
#ifdef _WIN32
      fprintf(file_meta, "%u,%I64d,%lf,%lf,%lf,%lf,%s\012", lasreadopener.get_file_name_current()-1, lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.max_x, lasreader->header.max_y, lasreadopener.get_file_name());
#else
      fprintf(file_meta, "%u,%lld,%lf,%lf,%lf,%lf,%s\012", lasreadopener.get_file_name_current()-1, lasreader->npoints, lasreader->header.min_x, lasreader->header.min_y, lasreader->header.max_x, lasreader->header.max_y, lasreadopener.get_file_name());
#endif
    }

    if (dont_reindex)
    {
      if (lasreader->get_index())
      {
        if (!quiet) fprintf(stderr, "skipping already indexed file '%s'\n", lasreadopener.get_file_name());
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
      if (verbose) fprintf(stderr,"no tile size specified. setting it to %g ...\n", t);
      lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, t);
    }
    else
    {
      lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);
    }

    // create index and add points

    LASindex lasindex;
    lasindex.prepare(lasquadtree, threshold);
    while (lasreader->read_point()) lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(lasreader->p_count-1));
  
    // delete the reader
    
    lasreader->close();
    delete lasreader;

    // adaptive coarsening

    lasindex.complete(minimum_points, maximum_intervals, very_verbose);

    // write to file

    if (append)
    {
      lasindex.append(lasreadopener.get_file_name());
    }
    else
    {
      lasindex.write(lasreadopener.get_file_name());
    }

    indexed++;

    if (!quiet) fprintf(stderr,"done with '%s'. took %g sec.\n", lasreadopener.get_file_name(), taketime()-start_time);
  }

  if (file_meta)
  {
    fclose(file_meta);
    file_meta = 0;
  }
  
  if (!quiet && lasreadopener.get_file_name_number() > 1)
  {
    if (dont_reindex)
    {
      fprintf(stderr,"done with %u files. skipped %u. indexed %u. total time %g sec.\n", lasreadopener.get_file_name_number(), skipped, indexed, taketime()-total_start_time);
    }
    else
    {
      fprintf(stderr,"done with %u files. total time %g sec.\n", lasreadopener.get_file_name_number(), taketime()-total_start_time);
    }
  }

  byebye(false, argc==1);

  return 0;
}
