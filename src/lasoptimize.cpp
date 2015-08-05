/*
===============================================================================

  FILE:  lasoptimize.cpp
  
  CONTENTS:
  
    This tool optimizes LIDAR data in LAS or LAZ format through compression,
    indexing, and (optional) rearrangement of points and storing them to our
    losslessly compressed LAZ format.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-14, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 March 2014 -- created during OSGeo Code Sprint in Vienna next to Hobu
  
===============================================================================
*/

#include <time.h>
#include <stdlib.h>

#include <map>
using namespace std;

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "bytestreamin.hpp"
#include "geoprojectionconverter.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasoptimize lidar.las\n");
  fprintf(stderr,"lasoptimize *.las -rearrange\n");
  fprintf(stderr,"lasoptimize *.las -rearrange -average 10000 -minimum 10000\n");
  fprintf(stderr,"lasoptimize -h\n");
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
extern int lasoptimize_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int lasoptimize_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  BOOL gui = FALSE;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
#endif
  BOOL verbose = FALSE;
  bool report_file_size = false;
  bool projection_was_set = false;
  bool format_not_specified = false;
  BOOL append = FALSE;
  BOOL rearrange = FALSE;
  F32 bucket_size = -1.0f;
  U32 tile_size = 100;
  U32 threshold = 1000;
  I32 average = 20000;
  U32 minimum_points = 100000;
  I32 maximum_intervals = -20;
  double start_time = 0.0;
  double total_start_time = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasoptimize_gui(argc, argv, 0);
#else
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
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
      verbose = TRUE;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by martin@rapidlasso.com) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-gui") == 0)
    {
#ifdef COMPILE_WITH_GUI
      gui = TRUE;
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
    else if (strcmp(argv[i],"-append") == 0)
    {
      append = TRUE;
    }
    else if (strcmp(argv[i],"-rearrange") == 0)
    {
      rearrange = TRUE;
    }
    else if (strcmp(argv[i],"-tile_size") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        usage(true);
      }
      i++;
      tile_size = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-average") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        usage(true);
      }
      i++;
      average = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-maximum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      i++;
      maximum_intervals = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-minimum") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      i++;
      minimum_points = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-threshold") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
        usage(true);
      }
      i++;
      threshold = atoi(argv[i]);
    }
    else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage(true);
    }
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return lasoptimize_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (cores > 1)
  {
    if (lasreadopener.get_file_name_number() < 2)
    {
      fprintf(stderr,"WARNING: only %u input files. ignoring '-cores %d' ...\n", lasreadopener.get_file_name_number(), cores);
    }
    else if (lasreadopener.is_merged())
    {
      fprintf(stderr,"WARNING: input files merged on-the-fly. ignoring '-cores %d' ...\n", cores);
    }
    else
    {
      return lasoptimize_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores);
    }
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(true, argc==1);
  }

  // make sure the header bounding box is populated

  lasreadopener.set_populate_header(TRUE);

  // check output

  if (laswriteopener.is_piped())
  {
    fprintf(stderr,"ERROR: piped output not supported\n");
    usage(true, argc==1);
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

  // check if the output format was *not* specified in the command line

  format_not_specified = !laswriteopener.format_was_specified();

  if (verbose) total_start_time = taketime();

  // loop over multiple input files

  while (lasreadopener.active())
  {
    if (verbose) start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();

    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      usage(true, argc==1);
    }

    // create output file name if no output was specified 
    if (!laswriteopener.active())
    {
      if (lasreadopener.get_file_name() == 0)
      {
        fprintf(stderr,"ERROR: no output specified\n");
        usage(true, argc==1);
      }
      laswriteopener.set_force(TRUE);
      if (format_not_specified)
      {
        if (lasreader->get_format() == LAS_TOOLS_FORMAT_LAZ)
        {
          laswriteopener.set_format(LAS_TOOLS_FORMAT_LAS);
        }
        else
        {
          laswriteopener.set_format(LAS_TOOLS_FORMAT_LAZ);
        }
      }
      laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
    }

    // maybe set projection

    if (projection_was_set)
    {
      lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
      if (geo_double_params)
      {
        lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
      }
      else
      {
        lasreader->header.del_geo_double_params();
      }
      lasreader->header.del_geo_ascii_params();
    }

    // special check for LAS 1.3+ files that contain internal waveform data ... it will not be carried on

    if ((lasreader->header.version_major == 1) && (lasreader->header.version_minor >= 3))
    {
      if (lasreader->header.global_encoding & 2) // if bit # 1 is set we have internal waveform data
      {
        lasreader->header.global_encoding &= ~((U16)2); // remove internal bit
      }
      if (lasreader->header.global_encoding & 4) // if bit # 2 is set we have external waveform data
      {
        if (rearrange)
        {
          lasreader->header.global_encoding &= ~((U16)4); // remove external bit
        }
      }
      if (lasreader->header.start_of_waveform_data_packet_record) // always zero the offset
      {
        lasreader->header.start_of_waveform_data_packet_record = 0;
      }
    }

    I64 bytes_written = 0;

    // should we also rearrange the data

    if (rearrange)
    {
      if (lasreader->npoints > U32_MAX)
      {
        fprintf(stderr, "ERROR: cannot handle that many points yet.\n");
        fprintf(stderr, "       contact martin.isenburg@rapidlasso.com for help what options to use.\n");
        byebye(true, argc==1);
      }

      U32 npoints = (U32)lasreader->npoints;

      U32 point_size = lasreader->point.total_point_size;
      U8* point_buffer = new U8[point_size*npoints];
      if (point_buffer == 0)
      {
        fprintf(stderr,"ERROR: allocating point_buffer for %u points of size %d failed\n", npoints, point_size);
        fprintf(stderr,"       write 'martin.isenburg@rapidlasso.com' to request out-of-core implementation\n");
        byebye(true, argc==1);
      }

/*
      if (lasreader->header.vlr_lastiling && lasreader->header.vlr_lastiling->level)
      {
        if (verbose) fprintf(stderr, "input is part of a tiling. it is a tile at level %d with level index %d.\n", lasreader->header.vlr_lastiling->level, lasreader->header.vlr_lastiling->level_index);

        if (bucket_size != -1)
        {
          if (!destroy_tiling && !just_reorder)
          {
            fprintf(stderr,"WARNING: the input LAS/LAZ file is a tile of a larger tiling so specifying\n");
            fprintf(stderr,"         a bucket size will either destroy the tiling or just reorder the\n");
            fprintf(stderr,"         points without the benefit of sub-tile finalization. Please force\n");
            fprintf(stderr,"         either option with '-destroy_tiling' or '-just_reorder' or simply\n");
            fprintf(stderr,"         do not specify a bucket size.\n");
            byebye(true, argc==1);
          }
          if (destroy_tiling)
          {
            if (verbose) fprintf(stderr, "the existing tiling is being destroyed.\n");
            lasreader->header.clean_lastiling();
          }
        }
      }
      else if (levels != -1)
      {
        fprintf(stderr,"WARNING: the input LAS/LAZ file is a not part of a tiling so specifying\n");
        fprintf(stderr,"         a sub-tile finalization to a certain level does not make sense.\n");
        fprintf(stderr,"         ignoring the argument '-levels %d'\n", levels);
        levels = -1;
      }
*/

      LASquadtree* lasquadtree = new LASquadtree();
/*
    if (lasreader->header.vlr_lastiling && (lasreader->header.vlr_lastiling->level != 0) && (lasreader->header.vlr_lastiling->buffer == 0) && (bucket_size == -1))
    {
      if (levels == -1)
      {
        if (verbose) fprintf(stderr, "no level for subtiling selected. setting it to 7 ...\n");
        lasquadtree->subtiling_setup(lasreader->header.vlr_lastiling->min_x, lasreader->header.vlr_lastiling->max_x, lasreader->header.vlr_lastiling->min_y, lasreader->header.vlr_lastiling->max_y, lasreader->header.vlr_lastiling->level, lasreader->header.vlr_lastiling->level_index, 7);
      }
      else
      {
        lasquadtree->subtiling_setup(lasreader->header.vlr_lastiling->min_x, lasreader->header.vlr_lastiling->max_x, lasreader->header.vlr_lastiling->min_y, lasreader->header.vlr_lastiling->max_y, lasreader->header.vlr_lastiling->level, lasreader->header.vlr_lastiling->level_index, levels);
      }
    }
    else
    {
    }
*/
      if (bucket_size == -1)
      {
        F32 b;
        if (((lasreader->header.max_x - lasreader->header.min_x) < 1000) && ((lasreader->header.max_y - lasreader->header.min_y) < 1000))
        {
          b = 1.0f;
        }
        else if (((lasreader->header.max_x - lasreader->header.min_x) < 10000) && ((lasreader->header.max_x - lasreader->header.min_x) < 10000))
        {
          b = 10.0f;
        }
        else if (((lasreader->header.max_x - lasreader->header.min_x) < 100000) && ((lasreader->header.max_x - lasreader->header.min_x) < 100000))
        {
          b = 100.0f;
        }
        else if (((lasreader->header.max_x - lasreader->header.min_x) < 1000000) && ((lasreader->header.max_x - lasreader->header.min_x) < 1000000))
        {
          b = 1000.0f;
        }
        else
        {
          b = 10000.0f;
        }
        if (verbose) fprintf(stderr,"no bucket size specified. setting it to %g ...\n", b);
        lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, b);
      }
      else
      {
        lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, bucket_size);
      }

      // create point counter (and output position) array

      U32 max_level_index = lasquadtree->get_max_level_index();
      U32* finalization_counters = new U32[max_level_index];
      if (finalization_counters == 0)
      {
        fprintf(stderr,"ERROR: allocating finalization_counters array of size %d\n", max_level_index);
        fprintf(stderr,"       write 'martin.isenburg@rapidlasso.com' to request out-of-core implementation\n");
        byebye(true, argc==1);
      }
      U32 u;
      for (u = 0; u < max_level_index; u++)
      {
        finalization_counters[u] = 0;
      }

      // read the points and populate finalization_counters array

      if (verbose) fprintf(stderr, "first pass: reading %u points and populating finalization counters.\n", npoints);

      F64 x,y;
      U32 level_index;
      U32 counter = 0;
      U32 occupied = 0;
      U32 level_index_min = U32_MAX;
      U32 level_index_max = U32_MIN;

      while (lasreader->read_point())
      {
        x = lasreader->point.get_x();
        y = lasreader->point.get_y();
        // make sure
        if (!lasquadtree->inside(x, y))
        {
          fprintf(stderr,"ERROR: x and y are not inside of lasquadtree\n");
          byebye(true, argc==1);
        }
        // get level index
        level_index = lasquadtree->get_level_index(x, y);
        if (level_index < level_index_min) level_index_min = level_index;
        if (level_index > level_index_max) level_index_max = level_index;
        // count number of occupied finalization cells
        if (finalization_counters[level_index] == 0) occupied++;
        // increment counter
        finalization_counters[level_index]++;
        counter++;
      }
      lasreader->close();

      float a = ((float)counter)/((float)occupied);

      if (verbose) { fprintf(stderr,"took %g sec. points per cell average is %.1f\n", taketime()-start_time, a); start_time = taketime(); }

      // if average points per cell requested then coarsen finalization grid accordingly 
      if (average != -1)
      {
        if (verbose && a < average) fprintf(stderr,"lowering the level to get an points per cell average of over %d\n", average);

        while (a < average)
        {
          occupied = 0;
          lasquadtree->levels--;
          max_level_index = max_level_index/4;
          for (u = 0; u < max_level_index; u++)
          {
            finalization_counters[u] = finalization_counters[4*u+0] + finalization_counters[4*u+1] + finalization_counters[4*u+2] + finalization_counters[4*u+3];
            if (finalization_counters[u]) occupied++;
          }
          a = ((float)counter)/((float)occupied);
          if (verbose) fprintf(stderr," ... lowering to %d gets an points per cell average of %.1f\n", lasquadtree->levels, a);
        }
      }

      // loop over once point counter array and compute output positions

      finalization_counters[max_level_index-1] = counter - finalization_counters[max_level_index-1];
      for (u = max_level_index-2; u > 0; u--)
      {
        finalization_counters[u] = finalization_counters[u+1] - finalization_counters[u];
      }
      finalization_counters[0] = finalization_counters[1] - finalization_counters[0];

      // reopen lasreader

      if (!lasreadopener.reopen(lasreader))
      {
        fprintf(stderr, "ERROR: could not re-open lasreader\n");
        byebye(true, argc==1);
      }

      // read the points again and copy them to the right location

      if (verbose) fprintf(stderr, "second pass: reading the points into the right location.\n");

      while (lasreader->read_point())
      {
        x = lasreader->point.get_x();
        y = lasreader->point.get_y();
        // get level index
        level_index = lasquadtree->get_level_index(x, y);
        // get location
        u = finalization_counters[level_index];
        // increment location for next point
        finalization_counters[level_index]++;
        // copy point to location
        lasreader->point.copy_to(&point_buffer[u*point_size]);
      }
      lasreader->close();

      if (verbose) { fprintf(stderr,"took %g sec.\n", taketime()-start_time); start_time = taketime(); }

      // setup tiling information in header

/*
    if (lasreader->header.vlr_lastiling && (lasreader->header.vlr_lastiling->level != 0))
    {
      // keep tiling ...
      if ((lasreader->header.vlr_lastiling->buffer != 0) || (bucket_size != -1))
      {
        // ... but no implicit finalization levels
        lasreader->header.vlr_lastiling->implicit_levels = 0;
        if (verbose) fprintf(stderr, "kept %s tiling without implicit subtiling ...\n", ((lasreader->header.vlr_lastiling->buffer != 0) ? "buffered" : "existing"));
      }
      else
      {
        // ... with new implicit finalization levels
        lasreader->header.vlr_lastiling->implicit_levels = lasquadtree.levels;
        if (verbose) fprintf(stderr, "kept existing tiling and added implicit subtiling with %d levels ...\n", lasquadtree->levels);
      }
    }
    else
    {
    }
*/
      // no tiling but implicit finalization levels
      lasreader->header.set_lastiling(0, 0, lasquadtree->levels, FALSE, FALSE, lasquadtree->min_x, lasquadtree->max_x, lasquadtree->min_y, lasquadtree->max_y);
      if (verbose) fprintf(stderr, "created implicit subtiling (i.e. finalization) with %d levels ...\n", lasquadtree->levels);

      // check output

      if (!laswriteopener.active())
      {
        laswriteopener.make_file_name(lasreadopener.get_file_name());
      }

      // prepare the header for the sorted points

      strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
      lasreader->header.system_identifier[31] = '\0';
      char temp[64];
      sprintf(temp, "lasoptimize (%d)", LAS_TOOLS_VERSION);
      strncpy(lasreader->header.generating_software, temp, 32);
      lasreader->header.generating_software[31] = '\0';

      if (projection_was_set)
      {
        lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
        if (geo_double_params)
        {
          lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
        }
        else
        {
          lasreader->header.del_geo_double_params();
        }
        lasreader->header.del_geo_ascii_params();
      }

      // maybe- use a smaller chunk size

//      laswriteopener.set_chunk_size(25000);

      // open laswriter

      LASwriter* laswriter = laswriteopener.open(&lasreader->header);

      if (laswriter == 0)
      {
        fprintf(stderr, "ERROR: could not open laswriter\n");
        byebye(true, argc==1);
      }

      // create lax index
      LASindex lasindex;
      lasindex.prepare(lasquadtree, threshold);

      // loop over points and output them

      if (verbose) fprintf(stderr, "third pass: writing the %d sorted points.\n", counter);

      lasreader->point.copy_from(&point_buffer[0]);
      level_index = lasquadtree->get_level_index(lasreader->point.get_x(), lasreader->point.get_y());
      for (u = 0; u < counter; u++)
      {
        lasreader->point.copy_from(&point_buffer[u*point_size]);
        if (level_index != lasquadtree->get_level_index(lasreader->point.get_x(), lasreader->point.get_y()))
        {
          level_index = lasquadtree->get_level_index(lasreader->point.get_x(), lasreader->point.get_y());
        }
        laswriter->write_point(&lasreader->point);
        lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(laswriter->p_count));
      }

      // flush the writer
      bytes_written = laswriter->close();
      delete laswriter;

      // adaptive coarsening
      lasindex.complete(minimum_points, maximum_intervals);

      // append lax to file
      lasindex.append(laswriteopener.get_file_name());

      delete [] point_buffer;
      delete [] finalization_counters;
    }
    else
    {
      // setup the quadtree
      LASquadtree* lasquadtree = new LASquadtree();
      lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, tile_size);

      // create lax index
      LASindex lasindex;
      lasindex.prepare(lasquadtree, threshold);

      // open laswriter

      LASwriter* laswriter = laswriteopener.open(&lasreader->header);

      if (laswriter == 0)
      {
        fprintf(stderr, "ERROR: could not open laswriter\n");
        usage(true, argc==1);
      }

    // compress points and add to index
      while (lasreader->read_point())
      {
        lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(laswriter->p_count));
        laswriter->write_point(&lasreader->point);
      }

      // flush the writer
      bytes_written = laswriter->close();
      delete laswriter;

      // adaptive coarsening
      lasindex.complete(minimum_points, maximum_intervals);

      // append lax to file
      lasindex.append(laswriteopener.get_file_name());
    }

#ifdef _WIN32
    if (verbose) fprintf(stderr,"%g secs to write %I64d bytes for '%s' with %I64d points of type %d\n", taketime()-start_time, bytes_written, laswriteopener.get_file_name(), lasreader->p_count, lasreader->header.point_data_format);
#else
    if (verbose) fprintf(stderr,"%g secs to write %lld bytes for '%s' with %lld points of type %d\n", taketime()-start_time, bytes_written, laswriteopener.get_file_name(), lasreader->p_count, lasreader->header.point_data_format);
#endif

    laswriteopener.set_file_name(0);
    if (format_not_specified)
    {
      laswriteopener.set_format((const CHAR*)NULL);
    }

    lasreader->close();
    delete lasreader;
  }

  if (projection_was_set)
  {
    free(geo_keys);
    if (geo_double_params)
    {
      free(geo_double_params);
    }
  }

  if (verbose && lasreadopener.get_file_name_number() > 1) fprintf(stderr,"needed %g sec for %u files\n", taketime()-total_start_time, lasreadopener.get_file_name_number());

  byebye(false, argc==1);

  return 0;
}
