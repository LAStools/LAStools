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

    (c) 2007-2012, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    29 April 2011 -- created after cable outage during the royal wedding (-:
  
===============================================================================
*/
#ifndef LAS_INDEX_HPP
#define LAS_INDEX_HPP

#include "lasdefinitions.hpp"

class LASspatial;
class LASinterval;
class LASreader;
class ByteStreamIn;
class ByteStreamOut;

class LASindex
{
public:
  LASindex();
  ~LASindex();

  // create spatial index
  void prepare(LASspatial* spatial, I32 threshold=1000);
  BOOL add(const LASpoint* point, const U32 index);
  void complete(U32 minimum_points=100000, I32 maximum_intervals=-1);

  // read from file or write to file
  BOOL read(const char* file_name);
  BOOL append(const char* file_name) const;
  BOOL write(const char* file_name) const;
  BOOL read(ByteStreamIn* stream);
  BOOL write(ByteStreamOut* stream) const;

  // intersect
  BOOL intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y);
  BOOL intersect_tile(const F32 ll_x, const F32 ll_y, const F32 size);
  BOOL intersect_circle(const F64 center_x, const F64 center_y, const F64 radius);

  // access the intersected intervals
  BOOL get_intervals();
  BOOL has_intervals();

  U32 start;
  U32 end;
  U32 full;
  U32 total;
  U32 cells;

  // read or seek next interval point
  BOOL read_next(LASreader* lasreader);
  BOOL seek_next(LASreader* lasreader);

  // for debugging
  void print(BOOL verbose);

  // for visualization
  LASspatial* get_spatial() const;
  LASinterval* get_interval() const;

private:
  BOOL merge_intervals();

  LASspatial* spatial;
  LASinterval* interval;
  BOOL have_interval;
};

#endif
