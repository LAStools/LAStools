/*
===============================================================================

  FILE:  lasindex.hpp
  
  CONTENTS:
  
    This class can create a spatial indexing, store a spatial indexing, write
    a spatial indexing to file, read a spatial indexing from file, and - most
    importantly - it can be used together with a lasreader for efficient access
    to a particular spatial region of a LAS file or a LAZ file.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     2 April 2015 -- add seek_next(LASreadPoint* reader, I64 &p_count) for DLL
     2 April 2015 -- delete read_next(LASreader* lasreader) that was not used
    31 March 2015 -- remove unused LASquadtree inheritance of abstract LASspatial 
    29 April 2011 -- created after cable outage during the royal wedding (-:
  
===============================================================================
*/
#ifndef LAS_INDEX_HPP
#define LAS_INDEX_HPP

#include "mydefs.hpp"

class LASquadtree;
class LASinterval;
#ifdef LASZIPDLL_EXPORTS
class LASreadPoint;
#else
class LASreader;
#endif
class ByteStreamIn;
class ByteStreamOut;

class LASindex
{
public:
  LASindex();
  ~LASindex();

  // create spatial index
  void prepare(LASquadtree* spatial, I32 threshold=1000);
  bool add(const F64 x, const F64 y, const U32 index);
  void complete(U32 minimum_points=100000, I32 maximum_intervals=-1, const bool verbose=TRUE);

  // read from file or write to file
  bool read(const char* file_name);
  bool append(const char* file_name) const;
  bool write(const char* file_name) const;
  bool read(ByteStreamIn* stream);
  bool write(ByteStreamOut* stream) const;

  // intersect
  bool intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y);
  bool intersect_tile(const F32 ll_x, const F32 ll_y, const F32 size);
  bool intersect_circle(const F64 center_x, const F64 center_y, const F64 radius);

  // access the intersected intervals
  bool get_intervals();
  bool has_intervals();

  U32 start;
  U32 end;
  U32 full;
  U32 total;
  U32 cells;

  // seek to next interval
#ifdef LASZIPDLL_EXPORTS
  bool seek_next(LASreadPoint* reader, I64 &p_count);
#else
  bool seek_next(LASreader* lasreader);
#endif

  // for debugging
  void print(bool verbose);

  // for visualization
  LASquadtree* get_spatial() const;
  LASinterval* get_interval() const;

private:
  bool merge_intervals();

  LASquadtree* spatial;
  LASinterval* interval;
  bool have_interval;
};

#endif
