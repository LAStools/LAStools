/*
===============================================================================

  FILE:  lasreaderbuffered.hpp
  
  CONTENTS:
  
    Reads LIDAR points from the LAS format and add those points from provided
    neighboring files that fall within the bounding box of this file extended
    by the user-specified buffer.

    First the buffer points are read from all the neighboring files so that
    the header can be properly populated. By default they are stored in main
    memory so they do not have to be read twice from disk.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    17 July 2012 -- created after converting the LASzip paper from LaTeX to Word
  
===============================================================================
*/
#ifndef LAS_READER_BUFFERED_HPP
#define LAS_READER_BUFFERED_HPP

#include "lasreader.hpp"

class LASreaderBuffered : public LASreader
{
public:

  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  void set_translate_intensity(F32 translate_intensity);
  void set_scale_intensity(F32 scale_intensity);
  void set_translate_scan_angle(F32 translate_scan_angle);
  void set_scale_scan_angle(F32 scale_scan_angle);
  void set_parse_string(const CHAR* parse_string);
  void set_skip_lines(I32 skip_lines);
  void set_populate_header(bool populate_header);

  bool set_file_name(const CHAR* file_name);
  bool add_neighbor_file_name(const CHAR* file_name);
  void set_buffer_size(const F32 buffer_size);

  bool remove_buffer();

  bool open();
  bool reopen();

  void set_filter(LASfilter* filter);
  void set_transform(LAStransform* transform);

  bool inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
  bool inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
  bool inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);

  I32 get_format() const;

  bool seek(const I64 p_index){ return FALSE; };

  ByteStreamIn* get_stream() const { return 0; };
  void close(bool close_stream=TRUE);

  LASreaderBuffered();
  ~LASreaderBuffered();

protected:
  bool read_point_default();

private:
  void clean();

  void clean_buffer();
  bool copy_point_to_buffer();
  bool copy_point_from_buffer();
  U32 get_number_buffered_points() const;

  const U32 points_per_buffer;
  U8** buffers;
  U8* current_buffer;
  U32 size_of_buffers_array;
  U32 number_of_buffers;
  U32 buffered_points;
  U32 point_count;

  LASreadOpener lasreadopener;
  LASreadOpener lasreadopener_neighbors;
  LASreader* lasreader;
  F32 buffer_size;
  bool point_type_change;
  bool point_size_change;
  bool rescale;
  bool reoffset;
  F64* scale_factor;
  F64* offset;
};

#endif
