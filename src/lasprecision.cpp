/*
===============================================================================

  FILE:  lasprecision.cpp
  
  CONTENTS:
  
    This tool computes statistics about the coordinates in a LAS/LAZ file that
    tell us whether the precision "advertised" in the header is really in the
    data. Often I find that the scaling factors in the header are miss-leading
    because they make it appear as if there was much more precision than there
    really is.

    The tool computes coordinate difference histograms that allow you to easily
    decide whether there is artificially high precision in the LAS/LAZ file. If
    you figured out the "correct" precision you can also resample the LAS/LAZ
    file to an appropriate level of precision. A big motivation to remove "fake"
    precision is that LAS files compress much better with laszip without this
    "fluff" in the low-order bits.

    For example you can find "fluff" in those examples:
     - Grass Lake Small.las
     - MARS_Sample_Filtered_LiDAR.las
     - Mount St Helens Oct 4 2004.las
     - IowaDNR-CloudPeakSoft-1.0-UTM15N.las
     - LAS12_Sample_withRGB_QT_Modeler.las
     - Lincoln.las
     - Palm Beach Pre Hurricane.las

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-17, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     1 May 2017 -- 3rd example for selective decompression for new LAS 1.4 points 
    30 November 2010 -- created spotting few paper cups at Starbuck's Offenbach
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "laswriter.hpp"
#include "geoprojectionconverter.hpp"

static void quicksort_for_shorts(short* a, int i, int j)
{
  int in_i = i;
  int in_j = j;
  short key = a[(i+j)/2];
  short w;
  do
  {
    while ( a[i] < key ) i++;
    while ( a[j] > key ) j--;
    if (i<j)
    {
      w = a[i];
      a[i] = a[j];
      a[j] = w;
    }
  } while (++i<=--j);
  if (i == j+3)
  {
    i--;
    j++;
  }
  if (j>in_i) quicksort_for_shorts(a, in_i, j);
  if (i<in_j) quicksort_for_shorts(a, i, in_j);
}

static void quicksort_for_ints(int* a, int i, int j)
{
  int in_i = i;
  int in_j = j;
  int key = a[(i+j)/2];
  int w;
  do
  {
    while ( a[i] < key ) i++;
    while ( a[j] > key ) j--;
    if (i<j)
    {
      w = a[i];
      a[i] = a[j];
      a[j] = w;
    }
  } while (++i<=--j);
  if (i == j+3)
  {
    i--;
    j++;
  }
  if (j>in_i) quicksort_for_ints(a, in_i, j);
  if (i<in_j) quicksort_for_ints(a, i, in_j);
}

static void quicksort_for_doubles(double* a, int i, int j)
{
  int in_i = i;
  int in_j = j;
  double key = a[(i+j)/2];
  double w;
  do
  {
    while ( a[i] < key ) i++;
    while ( a[j] > key ) j--;
    if (i<j)
    {
      w = a[i];
      a[i] = a[j];
      a[j] = w;
    }
  } while (++i<=--j);
  if (i == j+3)
  {
    i--;
    j++;
  }
  if (j>in_i) quicksort_for_doubles(a, in_i, j);
  if (i<in_j) quicksort_for_doubles(a, i, in_j);
}

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasprecision -i in.las\n");
  fprintf(stderr,"lasprecision -i in.las -number 1000000\n");
  fprintf(stderr,"lasprecision -i in.las -all -gps -lines 50\n");
  fprintf(stderr,"lasprecision -i in.las -no_x -no_y -no_z -rgb\n");
  fprintf(stderr,"lasprecision -i in.las -diff_diff\n");
  fprintf(stderr,"lasprecision -i in.las -o out.las -rescale 0.01 0.01 0.001 -reoffset 300000 2000000 0\n");
  fprintf(stderr,"lasprecision -i in.las -o out.las -rescale 0.333333333 0.333333333 0.01\n");
  fprintf(stderr,"lasprecision -h\n");
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
extern int lasprecision_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int lasprecision_multi_core(int argc, char *argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
#endif
  bool verbose = false;
  bool report_diff = true;
  bool report_diff_diff = false;
  bool report_x = true;
  bool report_y = true;
  bool report_z = true;
  bool report_gps = false;
  bool report_rgb = false;
  bool output = false;
  U32 report_lines = 20;
  U32 array_max = 5000000;
  bool projection_was_set = false;
  double start_time = 0;
  double full_start_time = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasprecision_gui(argc, argv, 0);
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
      if (strcmp(argv[i],"-o") == 0 || strcmp(argv[i],"-olas") == 0 || strcmp(argv[i],"-olaz") == 0 || strcmp(argv[i],"-obin") == 0 || strcmp(argv[i],"-otxt") == 0 || strcmp(argv[i],"-reoffset") == 0 || strcmp(argv[i],"-rescale") == 0)
      {
        output = true;
        break;
      }
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
    else if ((strcmp(argv[i],"-diff_diff") == 0) || (strcmp(argv[i],"-diff_diff_only") == 0))
    {
      report_diff_diff = true;
      report_diff = false;
    }
    else if (strcmp(argv[i],"-no_x") == 0)
    {
      report_x = false;
    }
    else if (strcmp(argv[i],"-no_y") == 0)
    {
      report_y = false;
    }
    else if (strcmp(argv[i],"-no_z") == 0)
    {
      report_z = false;
    }
    else if (strcmp(argv[i],"-gps") == 0)
    {
      report_gps = true;
    }
    else if (strcmp(argv[i],"-rgb") == 0)
    {
      report_rgb = true;
    }
    else if (strcmp(argv[i],"-number") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
        byebye(true);
      }
      i++;
      array_max = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-lines") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        byebye(true);
      }
      i++;
      report_lines = atoi(argv[i]);
    }
    else if (strcmp(argv[i],"-all") == 0)
    {
      array_max = U32_MAX;
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
    return lasprecision_gui(argc, argv, &lasreadopener);
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
      return lasprecision_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores);
    }
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    byebye(true, argc==1);
  }

  // maybe we do not need to read all layers (for compressed new LAS 1.4 point types only)

  if (!output)
  {
    U32 decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY;

    if (report_z) decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_Z;
    if (report_gps) decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME;
    if (report_rgb) decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_RGB;

    lasreadopener.set_decompress_selective(decompress_selective);
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

  // possibly loop over multiple input files

  while (lasreadopener.active())
  {
    if (verbose) full_start_time = start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

    // run presicion statistics across the first array_max points

    if (!output)
    {
      fprintf(stdout, "original scale factors: %g %g %g\n", lasreader->header.x_scale_factor, lasreader->header.y_scale_factor, lasreader->header.z_scale_factor);

      // create the arrays
      int* array_x = 0;
      int* array_y = 0;
      int* array_z = 0;
      if (report_x)
      {
        array_x = new int[array_max];
      }
      if (report_y)
      {
        array_y = new int[array_max];
      }
      if (report_z)
      {
        array_z = new int[array_max];
      }

      double* array_gps = 0;
      if (report_gps && lasreader->point.have_gps_time)
      {
        array_gps = new double[array_max];
      }

      short* array_r = 0;
      short* array_g = 0;
      short* array_b = 0;
      if (report_rgb && lasreader->point.have_rgb)
      {
        array_r = new short[array_max];
        array_g = new short[array_max];
        array_b = new short[array_max];
      }

      // do the first pass

      fprintf(stderr, "loading first %u of %u points\n", array_max, (U32)lasreader->npoints);

      // loop over points

      unsigned int array_count = 0;

      while ((lasreader->read_point()) && (array_count < array_max))
      {
        if (report_x)
        {
          array_x[array_count] = lasreader->point.get_X();
        }

        if (report_y)
        {
          array_y[array_count] = lasreader->point.get_Y();
        }

        if (report_z)
        {
          array_z[array_count] = lasreader->point.get_Z();
        }

        if (report_gps && lasreader->point.have_gps_time)
        {
          array_gps[array_count] = lasreader->point.gps_time;
        }

        if (report_rgb && lasreader->point.have_rgb)
        {
          array_r[array_count] = lasreader->point.rgb[0];
          array_g[array_count] = lasreader->point.rgb[1];
          array_b[array_count] = lasreader->point.rgb[2];
        }

        array_count++;
      }

      array_max = array_count;

      // sort values
  
      if (report_x)
      {
        quicksort_for_ints(array_x, 0, array_max-1);
      }

      if (report_y)
      {
        quicksort_for_ints(array_y, 0, array_max-1);
      }

      if (report_z)
      {
        quicksort_for_ints(array_z, 0, array_max-1);
      }
  
      if (report_gps && lasreader->point.have_gps_time)
      {
        quicksort_for_doubles(array_gps, 0, array_max-1);
      }

      if (report_rgb && lasreader->point.have_rgb)
      {
        quicksort_for_shorts(array_r, 0, array_max-1);
        quicksort_for_shorts(array_g, 0, array_max-1);
        quicksort_for_shorts(array_b, 0, array_max-1);
      }

      // create differences

      if (report_x)
      {
        for (array_count = 1; array_count < array_max; array_count++)
        {
          array_x[array_count-1] = array_x[array_count] - array_x[array_count-1];
        }
      }

      if (report_y)
      {
        for (array_count = 1; array_count < array_max; array_count++)
        {
          array_y[array_count-1] = array_y[array_count] - array_y[array_count-1];
        }
      }

      if (report_z)
      {
        for (array_count = 1; array_count < array_max; array_count++)
        {
          array_z[array_count-1] = array_z[array_count] - array_z[array_count-1];
        }
      }

      if (report_gps && lasreader->point.have_gps_time)
      {
        for (array_count = 1; array_count < array_max; array_count++)
        {
          array_gps[array_count-1] = array_gps[array_count] - array_gps[array_count-1];
        }
      }

      if (report_rgb && lasreader->point.have_rgb)
      {
        for (array_count = 1; array_count < array_max; array_count++)
        {
          array_r[array_count-1] = array_r[array_count] - array_r[array_count-1];
          array_g[array_count-1] = array_g[array_count] - array_g[array_count-1];
          array_b[array_count-1] = array_b[array_count] - array_b[array_count-1];
        }
      }

      // sort differences

      if (report_x)
      {
        quicksort_for_ints(array_x, 0, array_max-2);
      }

      if (report_y)
      {
        quicksort_for_ints(array_y, 0, array_max-2);
      }

      if (report_z)
      {
        quicksort_for_ints(array_z, 0, array_max-2);
      }
  
      if (report_gps && lasreader->point.have_gps_time)
      {
        quicksort_for_doubles(array_gps, 0, array_max-2);
      }

      if (report_rgb && lasreader->point.have_rgb)
      {
        quicksort_for_shorts(array_r, 0, array_max-2);
        quicksort_for_shorts(array_g, 0, array_max-2);
        quicksort_for_shorts(array_b, 0, array_max-2);
      }

      // compute difference of differences, sort them, output histogram

      // first for X & Y & Z

      unsigned int count_lines, array_last, array_first;
  
      if (report_x)
      {
        if (report_diff) fprintf(stdout, "X differences \n");
        for (count_lines = 0, array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_x[array_last] != array_x[array_count])
          {
            if (report_diff && (count_lines < report_lines)) {  count_lines++; fprintf(stdout, " %10d : %10d   %g\n", array_x[array_last], array_count - array_last, lasreader->header.x_scale_factor*array_x[array_last]); }
            array_x[array_first] = array_x[array_count] - array_x[array_last];
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "X differences of differences\n");
          quicksort_for_ints(array_x, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_x[array_last] != array_x[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_x[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }
      }

      if (report_y)
      {
        if (report_diff) fprintf(stdout, "Y differences \n");
        for (count_lines = 0, array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_y[array_last] != array_y[array_count])
          {
            if (report_diff && (count_lines < report_lines)) { count_lines++; fprintf(stdout, " %10d : %10d   %g\n", array_y[array_last], array_count - array_last, lasreader->header.y_scale_factor*array_y[array_last]); }
            array_y[array_first] = array_y[array_count] - array_y[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "Y differences of differences\n");
          quicksort_for_ints(array_y, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_y[array_last] != array_y[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_y[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }
      }

      if (report_z)
      {
        if (report_diff) fprintf(stdout, "Z differences \n");
        for (count_lines = 0, array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_z[array_last] != array_z[array_count])
          {
            if (report_diff && (count_lines < report_lines)) { count_lines++; fprintf(stdout, " %10d : %10d   %g\n", array_z[array_last], array_count - array_last, lasreader->header.z_scale_factor*array_z[array_last]); }
            array_z[array_first] = array_z[array_count] - array_z[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "Z differences of differences\n");
          quicksort_for_ints(array_z, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_z[array_last] != array_z[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_z[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }
      }

      // then for GPS

      if (report_gps && lasreader->point.have_gps_time)
      {
        if (report_diff) fprintf(stdout, "GPS time differences \n");
        for (array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_gps[array_last] != array_gps[array_count])
          {
            if (report_diff) fprintf(stdout, "  %.10g : %10d\n", array_gps[array_last], array_count - array_last);
            array_gps[array_first] = array_gps[array_count] - array_gps[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "GPS time  differences of differences\n");
          quicksort_for_doubles(array_gps, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_gps[array_last] != array_gps[array_count])
            {
              fprintf(stdout, "  %.10g : %10d\n", array_gps[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }
      }

      // then for R & G & B

      if (report_rgb && lasreader->point.have_rgb)
      {
        if (report_diff) fprintf(stdout, "R differences \n");
        for (array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_r[array_last] != array_r[array_count])
          {
            if (report_diff) fprintf(stdout, "  %10d : %10d\n", array_r[array_last], array_count - array_last);
            array_r[array_first] = array_r[array_count] - array_r[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "R differences of differences\n");
          quicksort_for_shorts(array_r, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_r[array_last] != array_r[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_r[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }

        if (report_diff) fprintf(stdout, "G differences \n");
        for (array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_g[array_last] != array_g[array_count])
          {
            if (report_diff) fprintf(stdout, "  %10d : %10d\n", array_g[array_last], array_count - array_last);
            array_g[array_first] = array_g[array_count] - array_g[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "G differences of differences\n");
          quicksort_for_shorts(array_g, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_g[array_last] != array_g[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_g[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }

        if (report_diff) fprintf(stdout, "B differences \n");
        for (array_first = 0, array_last = 0, array_count = 1; array_count < array_max; array_count++)
        {
          if (array_b[array_last] != array_b[array_count])
          {
            if (report_diff) fprintf(stdout, "  %10d : %10d\n", array_b[array_last], array_count - array_last);
            array_b[array_first] = array_b[array_count] - array_b[array_last]; 
            array_last = array_count;
            array_first++;
          }
        }
        if (report_diff_diff)
        {
          fprintf(stdout, "B differences of differences\n");
          quicksort_for_shorts(array_b, 0, array_first-1);
          for (array_last = 0, array_count = 1; array_count < array_first; array_count++)
          {
            if (array_b[array_last] != array_b[array_count])
            {
              fprintf(stdout, "  %10d : %10d\n", array_b[array_last], array_count - array_last);
              array_last = array_count;
            }
          }
        }
      }
      if (array_x) delete [] array_x;
      if (array_y) delete [] array_y;
      if (array_z) delete [] array_z;
      if (array_gps) delete [] array_gps;
      if (array_r) delete [] array_r;
      if (array_g) delete [] array_g;
      if (array_b) delete [] array_b;
    }
    else
    {
      // check output

      fprintf(stdout, "new scale factors: %g %g %g\n", lasreader->header.x_scale_factor,  lasreader->header.y_scale_factor,  lasreader->header.z_scale_factor);

    // check output

      if (!laswriteopener.active())
      {
        // create name from input name
        laswriteopener.make_file_name(lasreadopener.get_file_name());
      }

      // prepare the header for the surviving points

      strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);
      lasreader->header.system_identifier[31] = '\0';
      char temp[64];
      sprintf(temp, "lasprecision (%d)", LAS_TOOLS_VERSION);
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

      // open laswriter

      LASwriter* laswriter = laswriteopener.open(&lasreader->header);
      if (laswriter == 0)
      {
        fprintf(stderr, "ERROR: could not open laswriter\n");
        usage(argc==1);
      }

      // loop over points
      while (lasreader->read_point())
      {
        laswriter->write_point(&lasreader->point);
        laswriter->update_inventory(&lasreader->point);
      }

      laswriter->update_header(&lasreader->header, TRUE);
      laswriter->close();
      delete laswriter;

      laswriteopener.set_file_name(0);
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

  byebye(false, argc==1);

  return 0;
}
