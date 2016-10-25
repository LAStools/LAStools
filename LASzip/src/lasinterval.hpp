/*
===============================================================================

  FILE:  lasinterval.hpp
  
  CONTENTS:
  
    Used by lasindex to manage intervals of consecutive LiDAR points that are
    read sequentially.

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
  
    29 April 2011 -- created after cable outage during the royal wedding (-:
  
===============================================================================
*/
#ifndef LAS_INTERVAL_HPP
#define LAS_INTERVAL_HPP

#include "mydefs.hpp"

class ByteStreamIn;
class ByteStreamOut;

class LASintervalCell
{
public:
  U32 start;
  U32 end;
  LASintervalCell* next;
  LASintervalCell();
  LASintervalCell(const U32 p_index);
  LASintervalCell(const LASintervalCell* cell);
};

class LASintervalStartCell : public LASintervalCell
{
public:
  U32 full;
  U32 total;
  LASintervalCell* last;
  LASintervalStartCell();
  LASintervalStartCell(const U32 p_index);
  bool add(const U32 p_index, const U32 threshold=1000);
};

class LASinterval
{
public:
  LASinterval(const U32 threshold=1000);
  ~LASinterval();

  // add points and create cells with intervals
  bool add(const U32 p_index, const I32 c_index);

  // get total number of cells
  U32 get_number_cells() const;

  // get total number of intervals
  U32 get_number_intervals() const;

  // merge cells (and their intervals) into one cell
  bool merge_cells(const U32 num_indices, const I32* indices, const I32 new_index);

  // merge adjacent intervals with small gaps in cells to reduce total interval number to maximum
  void merge_intervals(U32 maximum, const bool verbose=TRUE);

  // read from file or write to file
  bool read(ByteStreamIn* stream);
  bool write(ByteStreamOut* stream) const;

  // get one cell after the other
  void get_cells();
  bool has_cells();

  // get a particular cell
  bool get_cell(const I32 c_index);

  // add cell's intervals to those that will be merged 
  bool add_current_cell_to_merge_cell_set();
  bool add_cell_to_merge_cell_set(const I32 c_index, const bool erase=FALSE);
  bool merge(const bool erase=FALSE);
  void clear_merge_cell_set();
  bool get_merged_cell();

  // iterate intervals of current cell (or over merged intervals)
  bool has_intervals();

  I32 index;
  U32 start;
  U32 end;
  U32 full;
  U32 total;

private:
  void* cells;
  void* cells_to_merge;
  U32 threshold;
  U32 number_intervals;
  I32 last_index;
  LASintervalStartCell* last_cell;
  LASintervalCell* current_cell;
  LASintervalStartCell* merged_cells;
  bool merged_cells_temporary;
};

#endif
