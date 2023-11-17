/*
 ===============================================================================

 FILE:  lascopcindex.cpp

 CONTENTS:

 This tool creates a COPC *.laz file version 1.4 format 6, 7 or 8 for a given set of *.las or *.laz
 files. A COPC file is a LAZ 1.4 file that stores point data organized in a clustered octree. It
 contains a VLR that describe the octree organization of data that are stored in LAZ 1.4 chunks
 (https://copc.io/). The input files are merged and the points are sorted.

 When COPC index is present it will be used to speed up access to the relevant areas of the LAZ file
 whenever a spatial queries or depth queries of the type

 -inside_tile ll_x ll_y size
 -inside_circle center_x center_y radius
 -inside_rectangle min_x min_y max_x max_y  (or simply -inside)
 -max_depth d
 -resolution r

 appears in the command line of any LAStools invocation. This acceleration is
 also available to users of the LASlib API. The LASreader class has four new
 functions called

 BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
 BOOL inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
 BOOL inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);
 BOOL inside_copc_depth(const U8 mode, const I32 depth, const F32 resolution);

 if any of these functions is called the LASreader will only return the points
 that fall inside the specified region or depth and use - when available - the spatial
 indexing information in the COPC EVLR.

 PROGRAMMERS:

 Jean-Romain Roussel

 COPYRIGHT:

 (c) 2023, rapidlasso GmbH - fast tools to catch reality

 This is free software; you can redistribute and/or modify it under the
 terms of the GNU Lesser General Licence as published by the Free Software
 Foundation. See the LICENSE.txt file for more information.

 This software is distributed WITHOUT ANY WARRANTY and without even the
 implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 CHANGE HISTORY:

 24 May 2023 -- created after planting vegetable in the garden

 ===============================================================================
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "lasreadpoint.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lascopc.hpp"
#include "lasprogress.hpp"
#include "geoprojectionconverter.hpp"

#ifdef _MSC_VER 
#define strcasecmp _stricmp
#endif

// MSVC does not like std::max({a, b, c}) nor std::max() and wants max(). g++ want std::max not max().
#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) MIN2(MIN2(a, b), (c))
#define MAX3(a, b, c) MAX2(MAX2(a, b), (c))

// Default: Linux 1024, Windows 512 may be increased to 2048 with MSVC, MacOS ??? (minus 3 for stdin, stderr, stdout)
#ifdef _WIN32
I32 MAX_FOPEN = 500;
#else
I32 MAX_FOPEN = 1000; 
#endif

static void usage(bool error = false, bool wait = false)
{
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "lascopcindex in.las\n");
  fprintf(stderr, "lascopcindex -i *.laz\n");
  fprintf(stderr, "lascopcindex -i *.laz -first_only\n");
  fprintf(stderr, "lascopcindex -merged -i *.las -drop_return 4 5\n");
  fprintf(stderr, "lascopcindex -merged -lof file_list.txt -o out.copc.laz\n");
  fprintf(stderr, "lascopcindex -merged -i *.las -o out.copc.laz -progress\n");
  fprintf(stderr, "lascopcindex -merged -i *.las -o out.copc.laz -max_depth 5\n");
  fprintf(stderr, "lascopcindex -merged -i *.las -o out.copc.laz -root_light\n");
  fprintf(stderr, "lascopcindex tls.laz -tls\n");
  fprintf(stderr, "lascopcindex -merged -i *.las -o out.copc.laz -ondisk -verbose\n");
  fprintf(stderr, "lascopcindex -h\n");
  if (wait)
  {
    fprintf(stderr, "<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static void byebye(bool error = false, bool wait = false)
{
  if (wait)
  {
    fprintf(stderr, "<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static U64 taketime()
{
  return (U64)time(NULL); // using time instead of clock to get wall clock
}

static inline F64 get_gps_time(const U8* buf) { return *((F64*)&buf[22]); };
static inline U8 get_scanner_channel(const U8* buf) { return (buf[15] >> 4) & 0x03; };
static inline U8 get_return_number(const U8* buf) { return buf[14] & 0x0F; };
static int compare_buffers(const void *a, const void *b)
{
  if (get_gps_time((U8*)a) < get_gps_time((U8*)b)) return -1;
  if (get_gps_time((U8*)a) > get_gps_time((U8*)b)) return 1;
  if (get_scanner_channel((U8*)a) < get_scanner_channel((U8*)b)) return -1;
  if (get_scanner_channel((U8*)a) > get_scanner_channel((U8*)b)) return 1;
  if (get_return_number((U8*)a) < get_return_number((U8*)b)) return -1;
  return 1;
}

struct LASfinalizer
{
  F64 xmin;
  F64 ymin;
  F64 zmin;
  F64 xmax;
  F64 ymax;
  F64 zmax;
  F64 xres;
  F64 yres;
  F64 zres;
  I32 ncols;
  I32 nrows;
  I32 nlays;
  I64 npoints;
  bool finalized;
  U32* grid;

  LASfinalizer(const LASheader *header, const I32 division)
  {
    xmin = header->min_x;
    xmax = header->max_x;
    ymin = header->min_y;
    ymax = header->max_y;
    zmin = header->min_z;
    zmax = header->max_z;

    F64 size = MAX3(xmax - xmin, ymax - ymin, zmax - zmin);
    F64 grid_spacing = size / division;

    ncols = I32_CEIL((xmax - xmin) / grid_spacing);
    nrows = I32_CEIL((ymax - ymin) / grid_spacing);
    nlays = I32_CEIL((zmax - zmin) / grid_spacing);

    xres = (xmax - xmin) / ncols;
    yres = (ymax - ymin) / nrows;
    zres = (zmax - zmin) / nlays;

    npoints = 0;
    finalized = false;
    grid = new U32[ncols * nrows * nlays]();
    memset((void*)grid, 0, ncols * nrows * nlays * sizeof(U32));
  };

  ~LASfinalizer() { delete[] grid; };

  bool add(const LASpoint *point)
  {
    I32 cell = cell_from_xyz(point->get_x(), point->get_y(), point->get_z());
    grid[cell]++;
    npoints++;
    return true;
  };

  bool remove(const LASpoint *point)
  {
    finalized = false;
    I32 cell = cell_from_xyz(point->get_x(), point->get_y(), point->get_z());
    if (grid[cell] == 0) throw std::runtime_error("internal error in the finalizer. Please report");
    grid[cell]--;
    npoints--;
    finalized = grid[cell] == 0;
    return true;
  };

  I32 cell_from_xyz(const F64 x, const F64 y, const F64 z)
  {
    I32 col = I32_FLOOR((x - xmin) / xres);
    I32 row = I32_FLOOR((ymax - y) / yres);
    I32 lay = I32_FLOOR((z - zmin) / zres);

    // Can happen with an invalid header.
    if (x <= xmin) col = 0;
    if (y >= ymax) row = 0;
    if (z <= zmin) lay = 0;
    if (x >= xmax) col = ncols - 1;
    if (y <= ymin) row = nrows - 1;
    if (z >= zmax) lay = nlays - 1;

    return lay * nrows * ncols + row * ncols + col;
  };

  bool is_finalized(const F64 xmin, const F64 ymin, const F64 zmin, const F64 xmax, const F64 ymax, const F64 zmax)
  {
    U32 count = 0;

    I32 startx = I32_FLOOR((xmin - this->xmin) / xres);
    I32 starty = I32_FLOOR((this->ymax - ymax) / yres);
    I32 startz = I32_FLOOR((zmin - this->zmin) / zres);
    I32 endx   = I32_CEIL((xmax - this->xmin) / xres);
    I32 endy   = I32_CEIL((this->ymax - ymin) / yres);
    I32 endz   = I32_CEIL((zmax - this->zmin) / zres);

    startx = MAX2(startx, 0);
    starty = MAX2(starty, 0);
    startz = MAX2(startz, 0);
    endx   = MIN2(endx, ncols - 1);
    endy   = MIN2(endy, nrows - 1);
    endz   = MIN2(endz, nlays - 1);

    I32 cell;
    for (I32 col = startx; col <= endx; col++)
    {
      for (I32 row = starty; row <= endy; row++)
      {
        for (I32 lay = startz; lay <= endz; lay++)
        {
          cell = lay * nrows * ncols + row * ncols + col;
          count += grid[cell];
        }
      }
    }

    return count == 0;
  };
};

struct VoxelRecord
{
  U16 bufid; // The ID of the buffer in which the point was inserted (used for optimization).
  I32 posid; // The position of the point in the array
  VoxelRecord() { bufid = 0; posid = 0; };
  VoxelRecord(U16 buf, I32 pos) { bufid = buf; posid = pos; };
};

struct Octant
{
  Octant(){};
  ~Octant(){};

  void sort() 
  { 
    load();
    qsort((void *)point_buffer, point_count, point_size, compare_buffers); 
  };
  I32 npoints() const { return point_count; };

  virtual void load() { return; };
  virtual void open() { return; };
  virtual void close(bool force) { return; };
  virtual void reactivate() { return; }
  virtual void desactivate() { return; }
  virtual void clean() = 0;
  virtual void swap(LASpoint *laspoint, const I32 pos) = 0;
  virtual void insert(const U8*buffer, const I32 cell, const U16 chunk) = 0;
  virtual void insert(const LASpoint *point, const I32 cell, const U16 chunk) = 0;

  U8* point_buffer;
  I32 point_count;
  I32 point_size;
  I32 point_capacity;
  std::unordered_map<I32, VoxelRecord> occupancy;
};

struct OctantInMemory : public Octant
{
  OctantInMemory(const U32 size)
  {
    point_size = size;
    point_count = 0;
    point_capacity = 25000;
    point_buffer = (U8*)malloc(point_capacity * point_size);
    occupancy.reserve(point_capacity);
  };

  // No copy constructor. We don't want any copy of dynamically allocated U8* point_buffer. 
  // Desallocation is performed manually with clean() at appropriate places.

  void insert(const U8* buffer, const I32 cell, const U16 chunk)
  {
    if (point_count == point_capacity)
    {
      point_capacity *= 2;
      point_buffer = (U8*)realloc(point_buffer, point_capacity * point_size);
    }

    memcpy(point_buffer + point_count * point_size, buffer, point_size);

    // cell = -1 means that recording the location of the point is useless (save memory)
    if (cell >= 0) occupancy.insert({cell, VoxelRecord(chunk, point_count)});

    point_count++;
  };

  void insert(const LASpoint *laspoint, const I32 cell, const U16 chunk)
  {
    if (point_count == point_capacity)
    {
      point_capacity *= 2;
      point_buffer = (U8*)realloc(point_buffer, point_capacity * point_size);
    }

    laspoint->copy_to(point_buffer + point_count * point_size);

    // cell = -1 means that recording the location of the point is useless (save memory)
    if (cell >= 0) occupancy.insert({cell, VoxelRecord(chunk, point_count)});

    point_count++;
  };

  void swap(LASpoint *laspoint, const I32 pos)
  {
    U8* tmp = (U8*)malloc(point_size);
    laspoint->copy_to(tmp);
    laspoint->copy_from(point_buffer + pos * point_size);
    memcpy(point_buffer + pos * point_size, tmp, point_size);
    free(tmp);
  };

  void clean()
  {
    if (point_buffer) free(point_buffer);
  };
};

struct OctantOnDisk : public Octant
{
  FILE* fp;
  char* filename_points;
  char* filename_octant;
  static I32 num_connexions;
  bool active;

  OctantOnDisk(const EPTkey& key, const char* dir, const U32 size)
  {
    active = true;

    point_size = size;
    point_count = 0;
    point_capacity = 0;
    point_buffer = 0;

    fp = 0;
    char suffix[32];

    filename_points = (char*)malloc((strlen(dir)+32)*sizeof(char));
    strcpy(filename_points, dir);
    sprintf(suffix, "points-%d-%d-%d-%d.bin", key.x, key.y, key.z, key.d);
    strcat(filename_points, suffix);

    filename_octant = (char*)malloc((strlen(dir)+32)*sizeof(char));
    strcpy(filename_octant, dir);
    sprintf(suffix, "octant-%d-%d-%d-%d.bin", key.x, key.y, key.z, key.d);
    strcat(filename_octant, suffix);
    
    open("w+b");
    close();

    occupancy.reserve(25000);
  };

  // No copy constructor. We don't want any copy of dynamically allocated U8* point_buffer. 
  // Desallocation is performed manually with clean() at appropriate places

  void insert(const U8* buffer, const I32 cell, const U16 chunk)
  {
    reactivate("r+b");
  
    fwrite(buffer, point_size, 1, fp);
 
    // cell = -1 means that recording the location of the point is useless (save memory)
    if (cell >= 0)
      occupancy.insert({cell, VoxelRecord(chunk, point_count)});

    point_count++;

    close();
  };

  void insert(const LASpoint* laspoint, const I32 cell, const U16 chunk)
  {
    reactivate("r+b");

    U8* buffer = (U8*)malloc(point_size);
    laspoint->copy_to(buffer);
    fwrite(buffer, point_size, 1, fp);
    free(buffer);

    // cell = -1 means that recording the location of the point is useless (save memory)
    if (cell >= 0)
      occupancy.insert({cell, VoxelRecord(chunk, point_count)});

    point_count++;

    close();
  };

  void swap(LASpoint* laspoint, const I32 pos)
  {
    reactivate("r+b");

    U8* buffer1 = (U8*)malloc(point_size);
    U8* buffer2 = (U8*)malloc(point_size);
    fseek(fp, pos*point_size, SEEK_SET);
    fread(buffer1, point_size, 1, fp);
    laspoint->copy_to(buffer2);
    laspoint->copy_from(buffer1);
    fseek(fp, pos*point_size, SEEK_SET);
    fwrite(buffer2, point_size, 1, fp);
    fseek(fp, 0, SEEK_END);
    free(buffer1);
    free(buffer2);

    close();
  };

  void reactivate(const char* mode) 
  {
    if (!active)
    {
      // read occupancy map from disk
      I32 cell;
      U16 buffid;
      I32 posid;
      FILE *f = fopen(filename_octant, "rb");
      if (f == 0)
      {
        fprintf(stderr, "ERROR: cannot open file '%s': %s\n", filename_octant, strerror(errno));
        throw std::runtime_error("Unexpected I/O error.");
      }
      while (fread(&cell, sizeof(I32), 1, f))
      {
        fread(&buffid, sizeof(U16), 1, f);
        fread(&posid, sizeof(I32), 1, f);
        occupancy[cell] = VoxelRecord(buffid, posid);
      }
      fclose(f);
      remove(filename_octant);
    } 
    
    open(mode);
    active = true;
  };

  void desactivate() 
  { 
    if (active)
    {
      // write occupancy map on disk
      FILE *f = fopen(filename_octant, "wb");
      if (f == 0)
      {
        fprintf(stderr, "ERROR: cannot open file '%s': %s\n", filename_octant, strerror(errno));
        throw std::runtime_error("Unexpected I/O error.");
      }
      for (const auto & e: occupancy)
      {
        fwrite(&e.first, sizeof(I32), 1, f);
        fwrite(&e.second.bufid, sizeof(U16), 1, f);
        fwrite(&e.second.posid, sizeof(I32), 1, f);
      }
      fclose(f);

      // clear occupancy grid
      occupancy.clear();
      occupancy = std::unordered_map<I32, VoxelRecord>();

      // close the file that stores the points
      close(true); 

      active = false;
    }
  }

  void load()
  {
    if (point_buffer) throw std::runtime_error("ERROR: internal error load() has been called twice. Please report.");

    reactivate("r+b");

    point_buffer = (U8*)malloc(point_count * point_size);
    fseek(fp, 0, SEEK_SET);
    fread(point_buffer, point_size, point_count, fp);

    close();
  };

  void clean()
  {
    close(true);

    if (filename_points)
    {
      remove(filename_points);
      free(filename_points);
    }

    if (filename_octant)
    {
      free(filename_octant);
    }

    if (point_buffer) 
    {
      free(point_buffer);
    }
  };

  void open(const char* mode)
  {
    if (fp == 0)
    {
      fp = fopen(filename_points, mode);
      if (fp == 0)
      {
        fprintf(stderr, "ERROR: cannot open file '%s': %s\n", filename_points, strerror(errno));
        throw std::runtime_error("Unexpected I/O error.");
      }
      fseek(fp, 0, SEEK_END);
      num_connexions++;
    }
  }

  void close(bool force = false)
  {
    if (fp) 
    {
      if (force || num_connexions >= MAX_FOPEN)
      {
        fclose(fp);
        fp = 0;
        num_connexions--;
      }
    }
  }
};

I32 OctantOnDisk::num_connexions = 0;

typedef std::unordered_map<EPTkey, std::unique_ptr<Octant>, EPTKeyHasher> Registry;

int main(int argc, char *argv[])
{
  /*#ifdef COMPILE_WITH_GUI
    bool gui = false;
  #endif*/

  // Program options
  BOOL verbose = FALSE;
  BOOL very_verbose = FALSE;
  BOOL progress = FALSE;
  BOOL shuffle = TRUE;
  BOOL swap = TRUE;
  BOOL sort = TRUE;
  BOOL ondisk = FALSE;
  BOOL unordered = FALSE;
  BOOL units = FALSE;
  U32  root_grid_size = 256;

  // Internal variables
  I32 i = 0;
  U32 seed = 0;
  I64 num_points = 0;
  I32 max_depth = -1;
  BOOL error = FALSE;
  U32 max_points_per_octant = 100000; // not absolute, only used to estimate the depth.
  I32 min_points_per_octant = 100;    // not absolute, use to (maybe) remove too small chunks
  F32 occupancy_resolution = 50;
  F32 proba_swap_event = 0.95F;
  I32 num_points_buffer = 1000000; // Approx 40 MB
  CHAR* tmpdir = 0;
  I32 max_files_opened = (I32)(0.5*MAX_FOPEN);
  const I32 limit_depth = 10;
  const std::array<EPTkey, 8> unordered_keys = EPTkey::root().get_children();

  LASreadOpener lasreadopener;
  lasreadopener.set_populate_header(true);

  LASwriteOpener laswriteopener;
  laswriteopener.set_format(LAS_TOOLS_FORMAT_LAZ);
  laswriteopener.set_chunk_size(0);

  GeoProjectionConverter geoprojectionconverter;

  if (argc == 1)
  {
    /*#ifdef COMPILE_WITH_GUI
        return lascopcindex_gui(argc, argv, 0);
    #else*/
    fprintf(stderr, "%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr, "enter input file: ");
    fgets(file_name, 256, stdin);
    file_name[strlen(file_name) - 1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr, "enter output file: ");
    fgets(file_name, 256, stdin);
    file_name[strlen(file_name) - 1] = '\0';
    laswriteopener.set_file_name(file_name);
    // #endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == 0x96)
        argv[i][0] = '-';
    }
    //if (!geoprojectionconverter.parse(argc, argv)) byebye(true);
    if (!lasreadopener.parse(argc, argv)) byebye(true);
    if (!laswriteopener.parse(argc, argv)) byebye(true);
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
    {
      fprintf(stderr, "LAStools (by info@rapidlasso.de) version %d\n", LAS_TOOLS_VERSION);
      usage();
    }
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0)
    {
      verbose = TRUE;
    }
    else if (strcmp(argv[i], "-vv") == 0 || strcmp(argv[i], "-very_verbose") == 0)
    {
      verbose = TRUE;
      very_verbose = TRUE;
    }
    else if (strcmp(argv[i], "-version") == 0)
    {
      fprintf(stderr, "LAStools (by info@rapidlasso.de) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i], "-fail") == 0)
    {
    }
    else if (strcmp(argv[i], "-gui") == 0)
    {
      /*#ifdef COMPILE_WITH_GUI
            gui = true;
      #else*/
      fprintf(stderr, "WARNING: not compiled with GUI support. ignoring '-gui' ...\n");
      // #endif
    }
    else if (strcmp(argv[i], "-cores") == 0)
    {
      fprintf(stderr, "WARNING: not compiled with multi-core batching. ignoring '-cores' ...\n");
      i++;
    }
    else if (strcmp(argv[i], "-cpu64") == 0)
    {
      fprintf(stderr, "WARNING: not compiled with 64 bit support. ignoring '-cpu64' ...\n");
      argv[i][0] = '\0';
    }
    else if (strcmp(argv[i], "-progress") == 0)
    {
      progress = TRUE;
    }
    else if (strcmp(argv[i], "-noswap") == 0)
    {
      swap = FALSE;
    }
    else if (strcmp(argv[i], "-noshuffle") == 0)
    {
      shuffle = FALSE;
    }
    else if (strcmp(argv[i], "-nosort") == 0)
    {
      sort = FALSE;
    }
    else if (strcmp(argv[i], "-root_light") == 0)
    {
      root_grid_size = 128;
    }
    else if (strcmp(argv[i], "-root_medium") == 0)
    {
      root_grid_size = 256;
      max_points_per_octant *= 2;
    }
    else if (strcmp(argv[i], "-root_dense") == 0)
    {
      root_grid_size = 512;
      max_points_per_octant *= 4;
    }
    else if (strcmp(argv[i], "-tls") == 0)
    {
      root_grid_size = 128;
      max_points_per_octant = 1000000;
      unordered = TRUE;

#ifdef _MSC_VER
      _setmaxstdio(2000);
      MAX_FOPEN = _getmaxstdio();
      max_files_opened = (I32)(0.5*MAX_FOPEN);
#endif
    }
    else if (strcmp(argv[i], "-unordered") == 0)
    {
      unordered = TRUE;
    }
    else if (strcmp(argv[i], "-ondisk") == 0)
    {
      ondisk = TRUE;

#ifdef _MSC_VER
      if (_setmaxstdio(MAX_FOPEN) == -1)
      {
        fprintf(stderr, "WARNING: the operating system cannot open %d files and is limited to %d\n", MAX_FOPEN, _getmaxstdio());
        byebye(true);
      }
      MAX_FOPEN = _getmaxstdio();
      max_files_opened = (I32)(0.5*MAX_FOPEN);
#endif
    }
    else if (strcmp(argv[i], "-m") == 0)
    {
      units = TRUE;
      occupancy_resolution = 50;
    }
    else if (strcmp(argv[i], "-ft") == 0)
    {
      units = TRUE;
      occupancy_resolution = 160;
    }
    else if (strcmp(argv[i], "-depth") == 0)
    {
      if ((i + 1) >= argc)
      {
        fprintf(stderr, "ERROR: '%s' needs 1 argument: depth\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i + 1], "%d", &max_depth) != 1)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
        usage(true);
      }
      if (max_depth > limit_depth || max_depth < 0)
        max_depth = -1;
      i += 1;
    }
    else if (strcmp(argv[i], "-max_files") == 0) // For debugging
    {
      if ((i + 1) >= argc)
      {
        fprintf(stderr, "ERROR: '%s' needs 1 argument: num\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i + 1], "%d", &MAX_FOPEN) != 1)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
        usage(true);
      }
      max_files_opened = (I32)(0.5*MAX_FOPEN);
      i += 1;
    }
    else if (strcmp(argv[i], "-seed") == 0)
    {
      if ((i + 1) >= argc)
      {
        fprintf(stderr, "ERROR: '%s' needs 1 argument: seed\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i + 1], "%u", &seed) != 1)
      {
        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
        usage(true);
      }
      i += 1;
    }
    else if (strcmp(argv[i], "-tmpdir") == 0)
    {
      if ((i + 1) >= argc)
      {
        fprintf(stderr, "ERROR: '%s' needs at least 1 argument: directory\n", argv[i]);
        return FALSE;
      }
      i += 1;
      tmpdir = LASCopyString(argv[i]);
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

  if (verbose) progress = FALSE;

  /*#ifdef COMPILE_WITH_GUI
    if (gui)
    {
      return lasopcindex_gui(argc, argv, &lasreadopener, &laswriteopener);
    }
  #endif*/

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input specified\n");
    usage(true, argc == 1);
  }

  if (lasreadopener.get_file_name_number() > 1 && unordered)
  {
    fprintf(stderr, "Memory optimization for spatially unordered files is supported only for a single file.\n");
    usage(true, argc == 1);
  }

  if (lasreadopener.is_stored())
  {
    fprintf(stderr, "LASreaderStored is not supported for lascopcindex.\n");
    usage(true, argc == 1);
  }

  if (lasreadopener.is_piped())
  {
    fprintf(stderr, "LASreaderPipon is not supported for lascopcindex.\n");
    usage(true, argc == 1);
  }

  while (lasreadopener.active())
  {
    num_points = 0;
    LASreader* lasreader = lasreadopener.open();

    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      usage(true, argc == 1);
    }

    if (!lasreader->point.have_gps_time)
    {
      fprintf(stderr, "WARNING: building a COPC file without gpstime.\n");
    }

    // create output file name if needed

    if (laswriteopener.get_file_name() == 0)
    {
      if (lasreadopener.get_file_name() == 0)
      {
        fprintf(stderr, "ERROR: no output file specified\n");
        byebye(true, argc==1);
      }
      laswriteopener.set_appendix(".copc");
      laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
    }

    if (!laswriteopener.active())
    {
      fprintf(stderr, "ERROR: no output specified\n");
      usage(true, argc == 1);
    }

    // check and fix correctness of file extension
    
    if (laswriteopener.get_file_name())
    {
      CHAR* file_name =  LASCopyString(laswriteopener.get_file_name());
      I32 len = (I32)strlen(file_name);
      while ((len >= 0) && (file_name[len] != '.')) len--;
      if ((strncmp(file_name + len, ".las", 4) == 0) || (strncmp(file_name + len, ".LAS", 4) == 0))
      {
        fprintf(stderr, "WARNING: output file has wrong extension. COPC files must be LAZ files. Output was renamed automatically.\n");
        len = (I32)strlen(file_name);
        if (file_name[len-1] == 'S') file_name[len-1] = 'Z'; else file_name[len-1] = 'z';
        laswriteopener.set_file_name(file_name);
      }
      free(file_name);
    }

    // replace the extension with ".copc.laz" (not mandatory by the standard but nicer).

    if (laswriteopener.get_file_name())
    {
      if (!strstr(laswriteopener.get_file_name(), ".copc.laz"))
      {
        const char* file_name_base = laswriteopener.get_file_name_base();
        char* file_name;
        const char* extension = ".copc.laz";
        file_name = (char*)malloc((strlen(file_name_base)+10)*sizeof(char));
        strcpy(file_name, file_name_base);
        strcat(file_name, extension);
        laswriteopener.set_file_name(file_name);
        fprintf(stderr, "WARNING: output was renamed '%s'\n", laswriteopener.get_file_name_only());
        free(file_name);
      }
    }

    // make sure we do not corrupt the input file

    if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
    {
      fprintf(stderr, "ERROR: input and output file name are identical: '%s'\n", lasreadopener.get_file_name());
      usage(true);
    }

    LASheader* lasheader = &lasreader->header;

    // check if we have long/lat coordinates. Abort if long/lat detected
    if (!units)
    {
      F64 delta_x = lasheader->max_x - lasheader->min_x;
      F64 delta_y = lasheader->max_y - lasheader->min_y;
      F64 delta_z = lasheader->max_z - lasheader->min_z;

      BOOL weird_z = 1000*delta_x < delta_z || 1000*delta_y < delta_z; // we have z and xy in different units
      BOOL accurate_coordinates = lasheader->x_scale_factor < 0.0001 || lasheader->y_scale_factor < 0.0001;
      BOOL longlat_extent = lasheader->max_x <= 90 && lasheader->min_x >= -90 && lasheader->max_y <= 180 && lasheader->min_y >= -180; // could be TLS centered on (0,0)
    
      if (weird_z && longlat_extent && accurate_coordinates)
      {
        fprintf(stderr, "ERROR: long/lat coordinates detected. COPC indexing supports only projected coordinates. If this is a false positive please use -m or -ft to provide the coordinates units.\n");
        delete lasreader;
        usage(true, argc == 1);
      }
    }

    if (unordered || ondisk) num_points_buffer *= 2; // reduce swap events

    if (verbose && unordered) fprintf(stderr, "Memory optimization for spatially unordered file: enabled\n\n");
    if (verbose && ondisk)    fprintf(stderr, "Processing points on disk: enabled\n\n");

    srand(seed);

    try
    {
      // =============================================================================================
      // PASS 1: Finalize the point cloud
      // =============================================================================================

      if (verbose || progress) fprintf(stderr, "Pass 1/2: finalizing the point cloud\n");

      U64 t0 = taketime();

      I32 tmp_max_depth = max_depth;
      if (tmp_max_depth < 0)
      {
        tmp_max_depth = EPToctree::compute_max_depth(*lasheader, max_points_per_octant);
        tmp_max_depth = (tmp_max_depth > limit_depth) ? limit_depth : tmp_max_depth;
      }

      LASprogress progressbar(lasreader);
      progressbar.set_display(verbose || progress);

      LASfinalizer lasfinalizer(lasheader, 2 << tmp_max_depth);
      LASinventory *lasinventory = new LASinventory;
      LASoccupancyGrid *lasoccupancygrid = new LASoccupancyGrid(occupancy_resolution);

      if (verbose)
      {
        fprintf(stderr, "Finalizer subdivided the coverage in %dx%dx%d = %d regions\n", lasfinalizer.ncols, lasfinalizer.nrows, lasfinalizer.nlays, lasfinalizer.ncols*lasfinalizer.nrows*lasfinalizer.nlays);
        fprintf(stderr, "Finalizer resolutions = %.1lf %.1lf %.1lf\n", lasfinalizer.xres, lasfinalizer.yres, lasfinalizer.zres);
      }

      F64 gpstime_minimum = F64_MAX;
      F64 gpstime_maximum = F64_MIN;
      while (lasreader->read_point())
      {
        if (gpstime_minimum > lasreader->point.get_gps_time()) gpstime_minimum = lasreader->point.get_gps_time();
        if (gpstime_maximum < lasreader->point.get_gps_time()) gpstime_maximum = lasreader->point.get_gps_time();
        lasfinalizer.add(&lasreader->point);
        lasoccupancygrid->add(&lasreader->point);
        lasinventory->add(&lasreader->point);
        num_points++;

        progressbar.update(lasreader);
        progressbar.print();
      }

      progressbar.done();

      // The octree must be built after the first pass in case a filter command changes the bounding box
      lasinventory->update_header(&lasreader->header);
      EPToctree octree(lasreader->header);
      octree.set_gridsize(root_grid_size);

      if (max_depth < 0)
      {
        max_depth = EPToctree::compute_max_depth(*lasheader, max_points_per_octant);
        max_depth = (max_depth > limit_depth) ? limit_depth : max_depth;
      }

      // Estimate the binomial probabilities for point swapping. See algorithm implementation details
      F64 area = occupancy_resolution * occupancy_resolution * lasoccupancygrid->get_num_occupied();
      F64 density = num_points / area;
      F64 voxel_sizes = octree.get_size() / octree.get_gridsize();
      F64 swap_probabilities[limit_depth + 1];
      for (i = 0; i <= limit_depth; i++)
      {
        I32 expected_num_point = (I32)(voxel_sizes * voxel_sizes * density);
        swap_probabilities[i] = (expected_num_point >= 5) ? 1 - std::pow(1 - proba_swap_event, 1 / (F32)expected_num_point) : 0;
        voxel_sizes /= 2;
      }

      delete lasoccupancygrid;
      delete lasinventory;

      U64 t1 = taketime();
      if (verbose)
      {
        fprintf(stderr, "Area covered: %.0lf\n", area);
        fprintf(stderr, "Number of points: %llu\n", num_points);
        fprintf(stderr, "Density of points: %.1lf\n", density);
        fprintf(stderr, "Maximum depth of the octree: %d\n", max_depth);
        if (swap) 
        { 
          fprintf(stderr, "Swap probabilities per level: "); for (i = 0; i <= max_depth; i++) fprintf(stderr, "%.4lf ", swap_probabilities[i]); fprintf(stderr, "\n"); 
        }
        fprintf(stderr, "Pass 1 took %u sec.\n", (U32)(t1 - t0));
      }

      // =============================================================================================
      // PREPARATION TO FILE CONVERSION AND WRITER (see las2las)
      // =============================================================================================

      lasreadopener.reopen(lasreader);

      // Target PDRF if conversion required
      U8 target_point_data_format = 6;
      if (lasreader->point.have_rgb) target_point_data_format = 7;
      if (lasreader->point.have_nir) target_point_data_format = 8;

      // Upgrade to LAS 1.4
      if (lasreader->header.version_minor < 3)
      {
        lasreader->header.header_size += (8 + 140);
        lasreader->header.offset_to_point_data += (8 + 140);
        lasreader->header.start_of_waveform_data_packet_record = 0;
      }
      else if (lasreader->header.version_minor == 3)
      {
        lasreader->header.header_size += 140;
        lasreader->header.offset_to_point_data += 140;
      }

      if (lasreader->header.version_minor < 4)
      {
        lasreader->header.version_minor = 4;
        lasreader->header.extended_number_of_point_records = lasreader->header.number_of_point_records;
        lasreader->header.number_of_point_records = 0;
        for (i = 0; i < 5; i++)
        {
          lasreader->header.extended_number_of_points_by_return[i] = lasreader->header.number_of_points_by_return[i];
          lasreader->header.number_of_points_by_return[i] = 0;
        }
      }

      // Upgrade to point format > 5
      if (lasreader->header.point_data_format < 6 || lasreader->header.point_data_format > 8)
      {
        // were there extra bytes before
        I32 num_extra_bytes = 0;
        switch (lasreader->header.point_data_format)
        {
          case 0: num_extra_bytes = lasreader->header.point_data_record_length - 20; break;
          case 1: num_extra_bytes = lasreader->header.point_data_record_length - 28; break;
          case 2: num_extra_bytes = lasreader->header.point_data_record_length - 26; break;
          case 3: num_extra_bytes = lasreader->header.point_data_record_length - 34; break;
          case 4: num_extra_bytes = lasreader->header.point_data_record_length - 57; break;
          case 5: num_extra_bytes = lasreader->header.point_data_record_length - 63; break;
          case 6: num_extra_bytes = lasreader->header.point_data_record_length - 30; break;
          case 7: num_extra_bytes = lasreader->header.point_data_record_length - 36; break;
          case 8: num_extra_bytes = lasreader->header.point_data_record_length - 38; break;
          case 9: num_extra_bytes = lasreader->header.point_data_record_length - 59; break;
          case 10: num_extra_bytes = lasreader->header.point_data_record_length - 67; break;
        }
        if (num_extra_bytes < 0)
        {
          fprintf(stderr, "ERROR: point record length has %d fewer bytes than needed\n", num_extra_bytes);
          byebye(true);
        }

        lasreader->header.clean_laszip();
        lasreader->header.point_data_format = (U8)target_point_data_format;
        switch (lasreader->header.point_data_format)
        {
          case 6: lasreader->header.point_data_record_length = 30 + num_extra_bytes; break;
          case 7: lasreader->header.point_data_record_length = 36 + num_extra_bytes; break;
          case 8: lasreader->header.point_data_record_length = 38 + num_extra_bytes; break;
        }
      }

      // convert CRS from GeoTIF to OGC WKT
      if (lasreader->header.vlr_geo_keys)
      {
        char* ogc_wkt = 0;
        I32 len = 0;
        geoprojectionconverter.set_projection_from_geo_keys(lasheader->vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasheader->vlr_geo_key_entries, lasheader->vlr_geo_ascii_params, lasheader->vlr_geo_double_params);
        if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt))
        {
          fprintf(stderr, "WARNING: cannot convert CRS from GeoTIFF to OGC WKT.\n");
        }

        if (ogc_wkt)
        {
          lasheader->set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
          lasheader->remove_vlr("LASF_Projection", 34735);
          lasheader->del_geo_ascii_params();
          lasheader->del_geo_double_params();
          lasheader->set_geo_ogc_wkt(len, ogc_wkt);
          if (verbose) fprintf(stderr, "CRS converted from GeoTIF to OGC WKT string: %s\n", ogc_wkt);
          free(ogc_wkt);
        }
      }

      // For format conversion
      LASpoint *laspoint = new LASpoint;
      laspoint->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);

      // Creation of the COPC info VLR
      LASvlr_copc_info *info = new LASvlr_copc_info[1];
      info->center_x = octree.get_center_x();
      info->center_y = octree.get_center_y();
      info->center_z = octree.get_center_z();
      info->gpstime_maximum = gpstime_maximum;
      info->gpstime_minimum = gpstime_minimum;
      info->halfsize = octree.get_halfsize();
      info->spacing = (octree.get_halfsize() * 2) / octree.get_gridsize();
      info->root_hier_offset = 0; // delayed write when closing the writer
      info->root_hier_size = 0;   // delayed write when closing the writer
      memset(info->reserved, 0, 11 * sizeof(U64));

      // COPC info *MUST* be the first VLR. We must realloc manually if any VLRs exist
      if (lasreader->header.number_of_variable_length_records == 0)
      {
        lasreader->header.add_vlr("copc", 1, sizeof(LASvlr_copc_info), (U8*)info, FALSE, "copc info");
      }
      else
      {
        lasreader->header.number_of_variable_length_records++;
        lasreader->header.offset_to_point_data += 54;
        lasreader->header.vlrs = (LASvlr *)realloc(lasreader->header.vlrs, lasreader->header.number_of_variable_length_records * sizeof(LASvlr));
        for (U32 i = lasreader->header.number_of_variable_length_records - 1; i > 0; i--) lasreader->header.vlrs[i] = lasreader->header.vlrs[i - 1];
        memset((void*)&(lasreader->header.vlrs[0]), 0, sizeof(LASvlr));
        lasreader->header.vlrs[0].reserved = 0;
        strncpy(lasreader->header.vlrs[0].user_id, "copc", sizeof(lasreader->header.vlrs[0].user_id));
        lasreader->header.vlrs[0].record_id = 1;
        lasreader->header.vlrs[0].record_length_after_header = sizeof(LASvlr_copc_info);
        lasreader->header.offset_to_point_data += lasreader->header.vlrs[0].record_length_after_header;
        sprintf(lasreader->header.vlrs[0].description, "%.31s", "copc info");
        lasreader->header.vlrs[0].data = (U8*)info;
      }

      strncpy(lasreader->header.system_identifier, "LAStools (c) by rapidlasso GmbH", 32);

      // Placeholder to write a proper header. The actual content is resolved when closing the writer.
      lasreader->header.add_evlr("copc", 1000, 1, new U8[1], FALSE, "EPT hierarchy");

      // Create the COPC file
      LASwriter* laswriter = laswriteopener.open(&lasreader->header);
      
      if (laswriter == 0)
      {
        fprintf(stderr, "ERROR: could not open laswriter\n");
        usage(true, argc == 1);
      }

      // =============================================================================================
      // PASS 2: This pass reads the points in a buffer of size n = num_points_buffer and shuffles the buffer.
      // Then it builds the octree and writes LAZ chunks as soon as a region of the finalizer is finalized
      // to free up memory.
      // =============================================================================================

      if (verbose || progress) fprintf(stderr, "\nPass 2/2: building and writing the COPC file\n");
      U64 t4 = taketime();

      // Counters
      U8  id_unordered_key = 0;
      U16 id_buffer = 0;
      I64 num_points_read = 0;
    
      // Buffer of points
      U32 elem_size = laspoint->total_point_size;
      I32 buffer_size = 0;
      U8* buffer = (U8*)malloc(num_points_buffer * elem_size);
      U8* temp = (U8*)malloc(elem_size);

      // EPT hierarchy
      std::vector<LASvlr_copc_entry> entries;

      // For -unordered optimization
      EPTkey current_unordered_key = unordered_keys[0];
      bool skip = false;

      // The octree is an std::unordered_map
      Registry registry;
      Registry::iterator it;

      // Setup progress bar (*3 because updated at 3 strategic locations)
      progressbar.set_total((U64)num_points*3);
      progressbar.set_display(progress);

      // tmpdir
      if (ondisk && tmpdir == 0) tmpdir = LASCopyString(laswriteopener.get_file_name_base());

      while (lasreader->read_point())
      {
        num_points_read++;
        bool end_of_stream = num_points_read == num_points;

        // Optimization for non-spatially coherent files (typically TLS). We perform 8 reads, skipping
        // points that are not in the current region of interest.
        if (unordered)
        {
          skip = octree.get_key(&lasreader->point, 1) != current_unordered_key;
          if (end_of_stream && id_unordered_key < (unordered_keys.size() - 1))
          {
            num_points_read = 0;
            lasreader->seek(0);
            current_unordered_key = unordered_keys[++id_unordered_key];
          }
        }

        if (!skip)
        {
          *laspoint = lasreader->point; // Conversion to target format
          laspoint->copy_to(buffer + buffer_size * elem_size);
          buffer_size++;
          progressbar++;
          progressbar.print();
        }

        // The buffer is full. Process the points.
        if (buffer_size == num_points_buffer || end_of_stream)
        {
          // First, we shuffle the points
          if (shuffle)
          {
            for (i = 0; i < buffer_size; i++)
            {
              I32 j = rand() % buffer_size;
              U8* block1 = buffer + i * elem_size;
              U8* block2 = buffer + j * elem_size;
              memcpy(temp, block1, elem_size);
              memcpy(block1, block2, elem_size);
              memcpy(block2, temp, elem_size);
            }
          }

          // We put the incoming points (coming in a random order) in the octree
          for (i = 0; i < buffer_size; i++)
          {
            laspoint->copy_from(buffer + i * elem_size);
            lasfinalizer.remove(laspoint);

            // Search a place to insert the point
            I32 lvl = 0;
            I32 cell = 0;
            EPTkey key;
            bool accepted = false;
            while (!accepted)
            {
              key = octree.get_key(laspoint, lvl);

              if (lvl == max_depth)
                cell = -1; // Do not build an occupancy grid for last level. Point must be inserted anyway.
              else
                cell = octree.get_cell(laspoint, key);

              it = registry.find(key);
              if (it == registry.end())
              {
                if (ondisk)
                {
                  it = registry.insert({key, std::make_unique<OctantOnDisk>(key, tmpdir, elem_size)}).first;

                  // If too many files are opened we desactivate the octants. They will be auto-reactivated when needed.
                  // Innactive octants are automatically closed until they become active again. More than 500
                  // files opened can arise for very large point-clouds but most are likely to be inactive.
                  if (OctantOnDisk::num_connexions > max_files_opened)
                  {
                    if (verbose) fprintf(stderr, "File connexions limit reached (%d). Closing all files temporarily.\n", OctantOnDisk::num_connexions);
                    for (auto &e : registry) e.second->desactivate();
                  }
                }
                else
                {
                  it = registry.insert({key, std::make_unique<OctantInMemory>(elem_size)}).first;
                }

                if (very_verbose) fprintf(stderr, "[%.0lf%%] Creation of octant %d-%d-%d-%d\n", progressbar.get_progress(), key.d, key.x, key.y, key.z);
              }

              auto it2 = it->second->occupancy.find(cell);
              accepted = (it2 == it->second->occupancy.end()) || (lvl == max_depth);

              if (swap && !accepted)
              {
                // bufid != id_buffer: save the heavy cost (on disk) of swapping.
                // No need to swap two points from the same buffer: they are already shuffled.
                if (it2->second.bufid != id_buffer && (((F32)rand()/(F32)RAND_MAX)) < swap_probabilities[lvl])
                {
                  it->second->swap(laspoint, it2->second.posid);
                  it2->second.bufid = id_buffer;
                }
              }

              lvl++;
            }

            // Insert the point
            it->second->insert(laspoint, cell, id_buffer);

            // Check if we finalized a cell of the finalizer.
            // We can potentially write some chunks in the .copc.laz and free up memory
            if (lasfinalizer.finalized)
            {
              // Loop through all octants to find the ones that are finalized (could be optimized).
              for (it = registry.begin(); it != registry.end();)
              {
                // Bounding box of the octant
                F64 res = octree.get_size() / (1 << it->first.d);
                F64 minx = res * it->first.x + octree.get_xmin();
                F64 miny = res * it->first.y + octree.get_ymin();
                F64 minz = res * it->first.z + octree.get_zmin();
                F64 maxx = minx + res;
                F64 maxy = miny + res;
                F64 maxz = minz + res;

                // If the octant is not finalized we can't do anything yet
                if (!lasfinalizer.is_finalized(minx, miny, minz, maxx, maxy, maxz))
                {
                  it++;
                  continue;
                }

                // Check if the chunk is not too small. Otherwise, redistribute the points in the parent octant.
                // There is no guarantee that parents still exist. They may have already been written and freed.
                // (Requiring that chunks have more than min_points_per_octant is not a strong requirement,
                // but producing a LAZ chunks with only 2 or 3 points is suboptimal).
                if (it->second->npoints() <= min_points_per_octant)
                {
                  bool moved = false;
                  key = it->first;
                  while (key != EPTkey::root() && moved == false)
                  {
                    key = key.get_parent();
                    auto it2 = registry.find(key);
                    if (it2 != registry.end())
                    {
                      if (very_verbose) fprintf(stderr, "[%.0lf%%] Moving %d points from %d-%d-%d-%d to %d-%d-%d-%d\n", progressbar.get_progress(), it->second->npoints(), it->first.d, it->first.x, it->first.y, it->first.z, it2->first.d, it2->first.x, it2->first.y, it2->first.z);

                      it->second->load();
                      for (I32 k = 0; k < it->second->npoints(); k++)
                        it2->second->insert(it->second->point_buffer + k * elem_size, -1, id_buffer);

                      it->second->clean();

                      // The octant must be inserted in the list because it may have childs
                      if (it->first.d < max_depth)
                      {
                        LASvlr_copc_entry entry;
                        entry.key.depth = it->first.d;
                        entry.key.x = it->first.x;
                        entry.key.y = it->first.y;
                        entry.key.z = it->first.z;
                        entry.point_count = 0;
                        entry.offset = 0;
                        entry.byte_size = 0;
                        entries.push_back(entry);
                      }

                      it = registry.erase(it);
                      moved = true;
                    }
                  }

                  // Points were moved in another octant and the octant was deleted: we do not write this octant
                  if (moved) continue;
                }

                // The octant is finalized: we can write the chunk and free up the memory
                LASvlr_copc_entry entry;
                entry.key.depth = it->first.d;
                entry.key.x = it->first.x;
                entry.key.y = it->first.y;
                entry.key.z = it->first.z;
                entry.point_count = it->second->npoints();
                entry.offset = laswriter->tell();

                // The points *MUST* be sorted (to optimize compression)
                if (sort) it->second->sort();

                // Write the chunk
                for (I32 k = 0; k < it->second->npoints(); k++)
                {
                  laspoint->copy_from(it->second->point_buffer + k * elem_size);
                  laswriter->write_point(laspoint);
                  laswriter->update_inventory(laspoint);

                  progressbar++;
                  progressbar.print();
                }
                laswriter->chunk();

                if (very_verbose) fprintf(stderr, "[%.0lf%%] Octant %d-%d-%d-%d written in COPC file\n", progressbar.get_progress(), it->first.d, it->first.x, it->first.y, it->first.z);

                // Record the VLR entry
                entry.byte_size = (I32)(laswriter->tell() - entry.offset);
                entries.push_back(entry);

                // We will never see this octant again. Goodbye.
                it->second->clean();
                it = registry.erase(it);
              }
            }

            progressbar++;
            progressbar.print();
          }

          id_buffer++;
          buffer_size = 0;

          if (verbose)
          { 
            F32 million = (F32)((U64)num_points_buffer*id_buffer/1000000.0);
            fprintf(stderr, "[%.0lf%%] Processed %.1f million points | LAZ chunks written: %u", progressbar.get_progress(), million, (U32)entries.size() );
            if (ondisk) fprintf(stderr, " | Files opened: %d/%d", OctantOnDisk::num_connexions, (I32)registry.size());
            fprintf(stderr, "\n");
          }

          if (ondisk)
          {
            // If too many files are opened we desactivate the octants. They will be auto-reactivated when needed.
            // Innactive octants are automatically closed until they become active again. More than 500
            // files opened can arise for very large point-clouds but most are likely to be inactive.
            if (OctantOnDisk::num_connexions > max_files_opened)
            {
              if (verbose) fprintf(stderr, "File connexions limit reached (%d). Closing all files temporarily.\n", OctantOnDisk::num_connexions);
              for (auto &e : registry) e.second->desactivate();
            }
          }
        }
      }

      progressbar.done();

      // Construct the EPT hierarchy eVLR
      LASvlr_copc_entry* hierarchy = new LASvlr_copc_entry[entries.size()];
      std::copy(entries.begin(), entries.end(), hierarchy);
      lasreader->header.remove_evlr("copc", 1000);
      lasreader->header.add_evlr("copc", 1000, entries.size() * sizeof(LASvlr_copc_entry), (U8*)hierarchy, FALSE, "EPT hierarchy");
      laswriter->update_header(&lasreader->header, TRUE, TRUE); // Propagate the updated EPT hierarchy and restores the pointer
      laswriter->close();                                       // Write the hierarchy eVLR and updates the copc info.
      lasreader->close();

      if (laswriter->npoints != num_points)
        fprintf(stderr, "ERROR: Different number of points in input and output. Something went wrong. Please report this error.\n");

      delete lasreader;
      delete laswriter;
      delete laspoint;
      free(buffer);
      free(temp);
      free(tmpdir);

      U64 t5 = taketime();
      if (verbose)
      {
        U32 num_chunks = (U32)entries.size();
        U32 num_chunks_few_points = 0;
        I32 highest_num_points = 0;
        I32 lowest_num_points = I32_MAX;
        for (const auto &chunk : entries)
        {
          if (chunk.point_count > highest_num_points) highest_num_points = chunk.point_count;
          if (chunk.point_count < lowest_num_points) lowest_num_points = chunk.point_count;
          if (chunk.point_count <= (I32)min_points_per_octant) num_chunks_few_points++;
        }

        fprintf(stderr, "Number of chunks: %u\n", num_chunks);
        fprintf(stderr, "Highest number of points in a chunk: %u\n", highest_num_points);
        fprintf(stderr, "Lowest number of points in a chunk: %u\n", lowest_num_points);
        fprintf(stderr, "Number of chunks with less than %u points: %u\n", min_points_per_octant, num_chunks_few_points);
        fprintf(stderr, "Pass 2 took %u sec.\n\n", (U32)(t5 - t4));

        fprintf(stderr, "Total time: %u sec.\n", (U32)(t5 - t0));
      }

      laswriteopener.set_file_name(0);
    }
    catch(std::exception& e)
    {
      fprintf(stderr, "ERROR: %s\n", e.what());
      error = true;
    }
    catch (...)
    {
      fprintf(stderr, "ERROR processing file '%s'. maybe file is corrupt?\n", lasreadopener.get_file_name());
      error = true;

      laswriteopener.set_file_name(0);
    }
  }

  byebye(error, argc == 1);

  return 0;
}
