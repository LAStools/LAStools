/*
===============================================================================

  FILE:  lasspatial.hpp
  
  CONTENTS:
  
    A base class for spatial management of LAS data allowing spatial indexing
    with quad trees, kd trees, tilings, or other spatial subdivisions.

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
  
    4 May 2011 -- created before meeting with Silke at the Marktplatz in OF
  
===============================================================================
*/
#ifndef LAS_SPATIAL_HPP
#define LAS_SPATIAL_HPP

#include "mydefs.hpp"

class ByteStreamIn;
class ByteStreamOut;

#define LAS_SPATIAL_QUAD_TREE 0

class LASspatial
{
public:
  virtual ~LASspatial() {};

  // read from file or write to file
  virtual bool read(ByteStreamIn* stream) = 0;
  virtual bool write(ByteStreamOut* stream) const = 0;

  // create or finalize the cell (in the spatial hierarchy) 
  virtual bool manage_cell(const U32 cell_index, const bool finalize=FALSE) = 0;

  // map points to cells
  virtual bool inside(const F64 x, const F64 y) const = 0;
  virtual U32 get_cell_index(const F64 x, const F64 y) const = 0;

  // map cells to coarser cells
  virtual bool coarsen(const I32 cell_index, I32* coarser_cell_index, U32* num_cell_indices, I32** cell_indices) const = 0;

  // describe cells
  virtual void get_cell_bounding_box(const I32 cell_index, F32* min, F32* max) const = 0;
  virtual void get_cell_bounding_box(const F64 x, const F64 y, F32* min, F32* max) const = 0;

  // decribe spatial extend
  virtual F64 get_min_x() const = 0;
  virtual F64 get_min_y() const = 0;
  virtual F64 get_max_x() const = 0;
  virtual F64 get_max_y() const = 0;

  // query spatial intersections
  virtual U32 intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y) = 0;
  virtual U32 intersect_tile(const F32 ll_x, const F32 ll_y, const F32 size) = 0;
  virtual U32 intersect_circle(const F64 center_x, const F64 center_y, const F64 radius) = 0;

  // iterate over cells
  virtual bool get_all_cells() = 0;
  virtual bool get_intersected_cells() = 0;
  virtual bool has_more_cells() = 0;

  I32 current_cell;
};

class LASspatialReadWrite
{
public:
  LASspatial* read(ByteStreamIn* stream) const;
  bool write(const LASspatial* spatial, ByteStreamOut* stream) const;
};

#endif
