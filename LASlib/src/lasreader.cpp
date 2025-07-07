/*
===============================================================================

    FILE:  lasreader.cpp

    CONTENTS:

        see corresponding header file

    PROGRAMMERS:

        info@rapidlasso.de  -  https://rapidlasso.de

    COPYRIGHT:

        (c) 2005-2019, rapidlasso GmbH - fast tools to catch reality

        This is free software; you can redistribute and/or modify it under the
        terms of the GNU Lesser General Licence as published by the Free Software
        Foundation. See the LICENSE.txt file for more information.

        This software is distributed WITHOUT ANY WARRANTY and without even the
        implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    CHANGE HISTORY:

        see corresponding header file

===============================================================================
*/
#include "lasreader.hpp"

#include "lascopc.hpp"
#include "lasfilter.hpp"
#include "lasindex.hpp"
#include "laskdtree.hpp"
#include "lasmessage.hpp"
#include "lasreader_asc.hpp"
#include "lasreader_bil.hpp"
#include "lasreader_bin.hpp"
#include "lasreader_dtm.hpp"
#include "lasreader_las.hpp"
#include "lasreader_ply.hpp"
#include "lasreader_qfit.hpp"
#include "lasreader_shp.hpp"
#include "lasreader_txt.hpp"
#include "lasreaderbuffered.hpp"
#include "lasreadermerged.hpp"
#include "lasreaderpipeon.hpp"
#include "lasreaderstored.hpp"
#include "lastransform.hpp"

#include <stdlib.h>
#include <string.h>
#include <string>
#include <filesystem>

LASreader::LASreader(LASreadOpener* opener) {
  this->opener = opener;
  if (!opener) LASMessage(LAS_VERBOSE, "reader opened without opener");
  npoints = 0;
  p_idx = 0;
  p_cnt = 0;
  read_simple = &LASreader::read_point_default;
  read_complex = 0;
  index = 0;
  copc_index = 0;
  copc_stream_order = 0;
  copc_resolution = 0;
  copc_depth = I32_MAX;
  filter = 0;
  transform = 0;
  ignore = 0;
  inside = 0;
  t_ll_x = 0;
  t_ll_y = 0;
  t_size = 0;
  t_ur_x = 0;
  t_ur_y = 0;
  c_center_x = 0;
  c_center_y = 0;
  c_radius = 0;
  c_radius_squared = 0;
  r_min_x = 0;
  r_min_y = 0;
  r_max_x = 0;
  r_max_y = 0;
  orig_min_x = 0;
  orig_min_y = 0;
  orig_max_x = 0;
  orig_max_y = 0;
  inside_depth = 0;
  transform_matrix = {};
}

LASreader::~LASreader() {
  if (index) delete index;
  if (copc_index) delete copc_index;
  if (transform) transform->check_for_overflow();
}

void LASreader::dealloc() {
  delete this;
}

void LASreader::set_index(LASindex* index) {
  if (this->index) delete this->index;
  this->index = index;
}

void LASreader::set_copcindex(COPCindex* copc_index) {
  if (this->copc_index) delete this->copc_index;
  this->copc_index = copc_index;
}

void LASreader::set_filter(LASfilter* filter) {
  this->filter = filter;
  if (filter && transform) {
    read_simple = &LASreader::read_point_filtered_and_transformed;
  } else if (filter) {
    read_simple = &LASreader::read_point_filtered;
  } else if (transform) {
    read_simple = &LASreader::read_point_transformed;
  } else {
    read_simple = &LASreader::read_point_default;
  }
  read_complex = &LASreader::read_point_default;
}

void LASreader::set_transform(LAStransform* transform) {
  this->transform = transform;
  if (filter && transform) {
    read_simple = &LASreader::read_point_filtered_and_transformed;
  } else if (filter) {
    read_simple = &LASreader::read_point_filtered;
  } else if (transform) {
    read_simple = &LASreader::read_point_transformed;
  } else {
    read_simple = &LASreader::read_point_default;
  }
  read_complex = &LASreader::read_point_default;
}

void LASreader::set_ignore(LASignore* ignore) {
  this->ignore = ignore;
}

BOOL LASreader::inside_none() {
  if (filter || transform) {
    read_complex = &LASreader::read_point_default;
  } else {
    read_simple = &LASreader::read_point_default;
  }
  if (inside) {
    header.min_x = orig_min_x;
    header.min_y = orig_min_y;
    header.max_x = orig_max_x;
    header.max_y = orig_max_y;
    inside = 0;
  }
  return TRUE;
}

BOOL LASreader::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size) {
  inside = 1;
  t_ll_x = ll_x;
  t_ll_y = ll_y;
  t_size = size;
  t_ur_x = ll_x + size;
  t_ur_y = ll_y + size;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = ll_x;
  header.min_y = ll_y;
  header.max_x = ll_x + size;
  header.max_y = ll_y + size;
  header.max_x -= header.x_scale_factor;
  header.max_y -= header.y_scale_factor;
  if (((orig_min_x > header.max_x) || (orig_min_y > header.max_y) || (orig_max_x < header.min_x) || (orig_max_y < header.min_y))) {
    if (filter || transform) {
      read_complex = &LASreader::read_point_none;
    } else {
      read_simple = &LASreader::read_point_none;
    }
  } else if (filter || transform) {
    if (index) {
      if (index) index->intersect_tile(ll_x, ll_y, size);
      read_complex = &LASreader::read_point_inside_tile_indexed;
    } else {
      read_complex = &LASreader::read_point_inside_tile;
    }
  } else {
    if (index) {
      if (index) index->intersect_tile(ll_x, ll_y, size);
      read_simple = &LASreader::read_point_inside_tile_indexed;
    } else {
      read_simple = &LASreader::read_point_inside_tile;
    }
  }
  return TRUE;
}

BOOL LASreader::inside_circle(const F64 center_x, const F64 center_y, const F64 radius) {
  inside = 2;
  c_center_x = center_x;
  c_center_y = center_y;
  c_radius = radius;
  c_radius_squared = radius * radius;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = center_x - radius;
  header.min_y = center_y - radius;
  header.max_x = center_x + radius;
  header.max_y = center_y + radius;
  if (((orig_min_x > header.max_x) || (orig_min_y > header.max_y) || (orig_max_x < header.min_x) || (orig_max_y < header.min_y))) {
    if (filter || transform) {
      read_complex = &LASreader::read_point_none;
    } else {
      read_simple = &LASreader::read_point_none;
    }
  } else if (filter || transform) {
    if (index) {
      if (index) index->intersect_circle(center_x, center_y, radius);
      read_complex = &LASreader::read_point_inside_circle_indexed;
    } else if (copc_index) {
      copc_index->intersect_circle(center_x, center_y, radius);
      read_complex = &LASreader::read_point_inside_circle_copc_indexed;
    } else {
      read_complex = &LASreader::read_point_inside_circle;
    }
  } else {
    if (index) {
      if (index) index->intersect_circle(center_x, center_y, radius);
      read_simple = &LASreader::read_point_inside_circle_indexed;
    } else if (copc_index) {
      copc_index->intersect_circle(center_x, center_y, radius);
      read_simple = &LASreader::read_point_inside_circle_copc_indexed;
    } else {
      read_simple = &LASreader::read_point_inside_circle;
    }
  }
  return TRUE;
}

BOOL LASreader::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y) {
  inside = 3;
  r_min_x = min_x;
  r_min_y = min_y;
  r_max_x = max_x;
  r_max_y = max_y;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = min_x;
  header.min_y = min_y;
  header.max_x = max_x;
  header.max_y = max_y;
  if (((orig_min_x > max_x) || (orig_min_y > max_y) || (orig_max_x < min_x) || (orig_max_y < min_y))) {
    if (filter || transform) {
      read_complex = &LASreader::read_point_none;
    } else {
      read_simple = &LASreader::read_point_none;
    }
  } else if (filter || transform) {
    if (index) {
      index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_complex = &LASreader::read_point_inside_rectangle_indexed;
    } else if (copc_index) {
      copc_index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_complex = &LASreader::read_point_inside_rectangle_copc_indexed;
    } else {
      read_complex = &LASreader::read_point_inside_rectangle;
    }
  } else {
    if (index) {
      index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_simple = &LASreader::read_point_inside_rectangle_indexed;
    } else if (copc_index) {
      copc_index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_simple = &LASreader::read_point_inside_rectangle_copc_indexed;
    } else {
      read_simple = &LASreader::read_point_inside_rectangle;
    }
  }
  return TRUE;
}

BOOL LASreader::inside_copc_depth(const U8 mode, const I32 depth, const F32 resolution) {
  if (!header.vlr_copc_info) return FALSE;

  inside_depth = mode;
  copc_depth = depth;
  copc_resolution = resolution;

  if (mode == 0)
    return FALSE;
  else if (mode == 1)
    copc_index->set_depth_limit(depth);
  else if (mode == 2)
    copc_index->set_resolution(resolution);
  else
    return FALSE;

  // If inside we are already using read_point_inside_[rectangle|circle]_copc_indexed
  // We do not overwrite read_[simple|complex] with a non spatial aware reader.
  if (inside) return TRUE;

  if (filter || transform)
    read_complex = &LASreader::read_point_inside_depth_copc_indexed;
  else
    read_simple = &LASreader::read_point_inside_depth_copc_indexed;

  return TRUE;
}

BOOL LASreader::read_point_inside_tile() {
  while (read_point_default()) {
    if (point.inside_tile(t_ll_x, t_ll_y, t_ur_x, t_ur_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_tile_indexed() {
  while (index->seek_next((LASreader*)this)) {
    if (read_point_default() && point.inside_tile(t_ll_x, t_ll_y, t_ur_x, t_ur_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_circle() {
  while (read_point_default()) {
    if (point.inside_circle(c_center_x, c_center_y, c_radius_squared)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_circle_indexed() {
  while (index->seek_next((LASreader*)this)) {
    if (read_point_default() && point.inside_circle(c_center_x, c_center_y, c_radius_squared)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_circle_copc_indexed() {
  while (copc_index->seek_next((LASreader*)this)) {
    if (read_point_default() && point.inside_circle(c_center_x, c_center_y, c_radius_squared)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_rectangle() {
  while (read_point_default()) {
    if (point.inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_rectangle_indexed() {
  while (index->seek_next((LASreader*)this)) {
    if (read_point_default() && point.inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_rectangle_copc_indexed() {
  bool inrect;
  while (copc_index->seek_next((LASreader*)this)) {
    inrect = point.get_x() >= r_min_x && point.get_x() <= r_max_x && point.get_y() >= r_min_y && point.get_y() <= r_max_y;

    if (read_point_default() && point.inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_depth_copc_indexed() {
  while (copc_index->seek_next((LASreader*)this)) {
    if (read_point_default()) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL LASreader::read_point_none() {
  return FALSE;
}

BOOL LASreader::read_point_filtered() {
  while ((this->*read_complex)()) {
    if (!filter->filter(&point)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_transformed() {
  if ((this->*read_complex)()) {
    transform->transform(&point);
    return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_filtered_and_transformed() {
  if (read_point_filtered()) {
    transform->transform(&point);
    return TRUE;
  }
  return FALSE;
}

BOOL LASreadOpener::is_piped() const {
  return (!file_names && use_stdin);
}

BOOL LASreadOpener::is_inside() const {
  return (inside_tile != 0 || inside_circle != 0 || inside_rectangle != 0);
}

I32 LASreadOpener::unparse(CHAR* string) const {
  I32 n = 0;
  if (inside_tile) {
    n = sprintf(string, "-inside_tile %g %g %g ", inside_tile[0], inside_tile[1], inside_tile[2]);
  } else if (inside_circle) {
    n = sprintf(string, "-inside_circle %lf %lf %lf ", inside_circle[0], inside_circle[1], inside_circle[2]);
  } else if (inside_rectangle) {
    n = sprintf(string, "-inside_rectangle %lf %lf %lf %lf ", inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
  }
  if (stored) {
    n += sprintf(string + n, "-stored ");
  }
  if (merged) {
    n += sprintf(string + n, "-merged ");
  }
  if (files_are_flightlines) {
    if (files_are_flightlines == 1) {
      n += sprintf(string + n, "-faf ");
    } else {
      n += sprintf(string + n, "-faf %d ", files_are_flightlines);
    }
  }
  if (apply_file_source_ID) {
    n += sprintf(string + n, "-apply_file_source_ID ");
  }
  if (scale_factor) {
    if (scale_factor[2] == 0.0) {
      if ((scale_factor[0] != 0.0) && (scale_factor[1] != 0.0)) {
        n += sprintf(string + n, "-rescale_xy %lf %lf ", scale_factor[0], scale_factor[1]);
      }
    } else {
      if ((scale_factor[0] == 0.0) && (scale_factor[1] == 0.0)) {
        n += sprintf(string + n, "-rescale_z %lf ", scale_factor[2]);
      } else {
        n += sprintf(string + n, "-rescale %lf %lf %lf ", scale_factor[0], scale_factor[1], scale_factor[2]);
      }
    }
  }
  if (offset) {
    n += sprintf(string + n, "-reoffset %lf %lf %lf ", offset[0], offset[1], offset[2]);
  } else if (auto_reoffset) {
    n += sprintf(string + n, "-auto_reoffset ");
  } else if (offset_adjust) {
    n += sprintf(string + n, "-offset_adjust ");
  }
  if (populate_header) {
    n += sprintf(string + n, "-populate ");
  }
  if (io_ibuffer_size != LAS_TOOLS_IO_IBUFFER_SIZE) {
    n += sprintf(string + n, "-io_ibuffer %u ", io_ibuffer_size);
  }
  if (!temp_file_base.empty()) {
    n += sprintf(string + n, "-temp_files \"%s\" ", temp_file_base.c_str());
  }
  return n;
}

BOOL LASreadOpener::is_buffered() const {
  return ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0)));
}

BOOL LASreadOpener::is_header_populated() const {
  return (populate_header || (file_name && IsLasLazFile(std::string(file_name))));
}

void LASreadOpener::reset() {
  file_name_current = 0;
  file_name = 0;
}

// setup to use z from attribute by default for certain tools
void LASreadOpener::z_from_attribute_bydefault() {
  if (!z_from_attribute) {
    z_from_attribute = true;
    z_from_attribute_try = true;
  }
}

LASreader* LASreadOpener::open(const CHAR* other_file_name, BOOL reset_after_other) {
  inside_depth_opener = 0;
  if (filter) filter->reset();
  if (transform) transform->reset();

  if (file_names || other_file_name) {
    use_stdin = FALSE;
    if (file_name_current == file_name_number && other_file_name == 0) return 0;
    if ((other_file_name == 0) && ((file_name_number > 1) || (file_names_ID)) && merged) {
      LASreaderMerged* lasreadermerged = new LASreaderMerged(this);
      lasreadermerged->set_scale_factor(scale_factor);
      lasreadermerged->set_offset(offset);
      lasreadermerged->set_parse_string(parse_string);
      lasreadermerged->set_skip_lines(skip_lines);
      lasreadermerged->set_populate_header(populate_header);
      lasreadermerged->set_keep_lastiling(keep_lastiling);
      lasreadermerged->set_translate_intensity(translate_intensity);
      lasreadermerged->set_scale_intensity(scale_intensity);
      lasreadermerged->set_translate_scan_angle(translate_scan_angle);
      lasreadermerged->set_scale_scan_angle(scale_scan_angle);
      lasreadermerged->set_io_ibuffer_size(io_ibuffer_size);
      lasreadermerged->set_copc_stream_order(copc_stream_order);
      if (file_names_ID) {
        for (file_name_current = 0; file_name_current < file_name_number; file_name_current++)
          lasreadermerged->add_file_name(file_names[file_name_current], file_names_ID[file_name_current]);
      } else {
        for (file_name_current = 0; file_name_current < file_name_number; file_name_current++)
          lasreadermerged->add_file_name(file_names[file_name_current]);
      }
      if (!lasreadermerged->open()) {
        laserror("cannot open lasreadermerged with %d file names", file_name_number);
        delete lasreadermerged;
        return 0;
      }
      if (files_are_flightlines) lasreadermerged->set_files_are_flightlines(files_are_flightlines);
      if (apply_file_source_ID) lasreadermerged->set_apply_file_source_ID(TRUE);
      if (filter) lasreadermerged->set_filter(filter);
      if (transform) lasreadermerged->set_transform(transform);
      if (ignore) lasreadermerged->set_ignore(ignore);
      if (inside_tile) lasreadermerged->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreadermerged->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreadermerged->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      if (inside_depth_opener) lasreadermerged->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
      LASreader* lasreader = 0;
      if (stored) {
        LASreaderStored* lasreaderstored = new LASreaderStored(this);
        if (!lasreaderstored->open(lasreadermerged)) {
          laserror("could not open lasreaderstored with lasreadermerged");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      } else {
        lasreader = lasreadermerged;
      }
      if (pipe_on) {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
        if (!lasreaderpipeon->open(lasreader)) {
          laserror("cannot open lasreaderpipeon with lasreadermerged");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      } else {
        return lasreader;
      }
    } else if ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0))) {
      U32 i;
      LASreaderBuffered* lasreaderbuffered = new LASreaderBuffered(this);
      lasreaderbuffered->set_buffer_size(buffer_size);
      lasreaderbuffered->set_scale_factor(scale_factor);
      lasreaderbuffered->set_offset(offset);
      lasreaderbuffered->set_parse_string(parse_string);
      lasreaderbuffered->set_skip_lines(skip_lines);
      lasreaderbuffered->set_populate_header(populate_header);
      lasreaderbuffered->set_translate_intensity(translate_intensity);
      lasreaderbuffered->set_scale_intensity(scale_intensity);
      lasreaderbuffered->set_translate_scan_angle(translate_scan_angle);
      lasreaderbuffered->set_scale_scan_angle(scale_scan_angle);
      if (other_file_name)  // special case when other file name was specified
      {
        file_name = other_file_name;
        lasreaderbuffered->set_file_name(other_file_name);
        if (reset_after_other) {
          file_name_current = 0;
        }
        for (i = 0; i < file_name_number; i++) {
          if (strcmp(other_file_name, file_names[i])) {
            lasreaderbuffered->add_neighbor_file_name(file_names[i]);
          }
        }
        if (neighbor_file_name_number) {
          for (i = 0; i < neighbor_file_name_number; i++) {
            if (strcmp(other_file_name, neighbor_file_names[i])) {
              lasreaderbuffered->add_neighbor_file_name(neighbor_file_names[i]);
            }
          }
        }
      } else  // usual case for processing on-the-fly buffered files
      {
        file_name = file_names[file_name_current];
        lasreaderbuffered->set_file_name(file_name);
        if (kdtree_rectangles) {
          if (!kdtree_rectangles->was_built()) {
            kdtree_rectangles->build();
          }
          kdtree_rectangles->overlap(
              file_names_min_x[file_name_current] - buffer_size, file_names_min_y[file_name_current] - buffer_size,
              file_names_max_x[file_name_current] + buffer_size, file_names_max_y[file_name_current] + buffer_size);
          if (kdtree_rectangles->has_overlaps()) {
            U32 index;
            while (kdtree_rectangles->get_overlap(index)) {
              if (file_name != file_names[index]) {
                lasreaderbuffered->add_neighbor_file_name(file_names[index]);
              }
            }
          }
        } else {
          for (i = 0; i < file_name_number; i++) {
            if (file_name != file_names[i]) {
              lasreaderbuffered->add_neighbor_file_name(file_names[i]);
            }
          }
        }
        if (neighbor_file_name_number) {
          if (neighbor_kdtree_rectangles) {
            if (!neighbor_kdtree_rectangles->was_built()) {
              neighbor_kdtree_rectangles->build();
            }
            neighbor_kdtree_rectangles->overlap(
                file_names_min_x[file_name_current] - buffer_size, file_names_min_y[file_name_current] - buffer_size,
                file_names_max_x[file_name_current] + buffer_size, file_names_max_y[file_name_current] + buffer_size);
            if (neighbor_kdtree_rectangles->has_overlaps()) {
              U32 index;
              while (neighbor_kdtree_rectangles->get_overlap(index)) {
                if (strcmp(file_name, neighbor_file_names[index])) {
                  lasreaderbuffered->add_neighbor_file_name(neighbor_file_names[index]);
                }
              }
            }
          } else {
            for (i = 0; i < neighbor_file_name_number; i++) {
              if (strcmp(file_name, neighbor_file_names[i])) {
                lasreaderbuffered->add_neighbor_file_name(neighbor_file_names[i]);
              }
            }
          }
        }
        file_name_current++;
      }
      if (filter) lasreaderbuffered->set_filter(filter);
      if (transform) lasreaderbuffered->set_transform(transform);
      if (ignore) lasreaderbuffered->set_ignore(ignore);
      if (!lasreaderbuffered->open()) {
        laserror("cannot open lasreaderbuffered with %d file names", file_name_number + neighbor_file_name_number);
        delete lasreaderbuffered;
        return 0;
      }
      if (inside_tile) lasreaderbuffered->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreaderbuffered->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreaderbuffered->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored) {
        LASreaderStored* lasreaderstored = new LASreaderStored(this);
        if (!lasreaderstored->open(lasreaderbuffered)) {
          laserror("could not open lasreaderstored with lasreaderbuffered");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      } else {
        lasreader = lasreaderbuffered;
      }
      if (pipe_on) {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
        if (!lasreaderpipeon->open(lasreader)) {
          laserror("cannot open lasreaderpipeon with lasreaderbuffered");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      } else {
        return lasreader;
      }
    } else {
      if (other_file_name) {
        file_name = other_file_name;
        if (reset_after_other) {
          file_name_current = 0;
        }
      } else {
        file_name = file_names[file_name_current];
        file_name_current++;
      }
      if (files_are_flightlines) {
        transform->setPointSource(file_name_current + files_are_flightlines + files_are_flightlines_index);
      }
      if (IsLasLazFile(file_name)) {
        LASreaderLAS* lasreaderlas = nullptr;
        if (scale_factor == 0 && offset == 0) {
          if (auto_reoffset || (offset_adjust && !transform))
            lasreaderlas = new LASreaderLASreoffset(this);
          else
            lasreaderlas = new LASreaderLAS(this);
        } else if (scale_factor != 0 && offset == 0) {
          if (auto_reoffset || (offset_adjust && !transform))
            lasreaderlas = new LASreaderLASrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2]);
          else if (!offset_adjust)
            lasreaderlas = new LASreaderLASrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
          else
            lasreaderlas = new LASreaderLAS(this);
        } else if (scale_factor == 0 && offset != 0)
          lasreaderlas = new LASreaderLASreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderlas = new LASreaderLASrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);

        lasreaderlas->set_keep_copc(keep_copc);
        if (lasreaderlas->open(file_name, io_ibuffer_size, FALSE, decompress_selective)) {
          LASMessage(LAS_VERY_VERBOSE, "open file '%s'", file_name);
        } else {
          laserror("cannot open lasreaderlas with file name '%s'", file_name);
          delete lasreaderlas;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderlas->set_index(index);
        else {
          delete index;
          index = 0;
        }

        // Creation of the COPC index
        if (lasreaderlas->header.vlr_copc_entries) {
          if (index) {
            LASMessage(LAS_WARNING, "both LAX file and COPC spatial indexing registered. COPC has the precedence.");
            lasreaderlas->set_index(0);
          }

          COPCindex* copc_index = new COPCindex(lasreaderlas->header);
          if (copc_stream_order == 0)
            copc_index->set_stream_ordered_by_chunk();
          else if (copc_stream_order == 1)
            copc_index->set_stream_ordered_spatially();
          else if (copc_stream_order == 2)
            copc_index->set_stream_ordered_by_depth();
          lasreaderlas->set_copcindex(copc_index);

          // If no user-defined query we force a query anyway to never read a copc file in order but
          // instead we enforce the use of the index to read in a spatially coherent order.
          if (!inside_circle && !inside_rectangle && !inside_depth_opener) set_max_depth(I32_MAX);
        }
        if (files_are_flightlines) {
          lasreaderlas->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        } else if (apply_file_source_ID) {
          transform->setPointSource(lasreaderlas->header.file_source_ID);
        }
        if (filter) lasreaderlas->set_filter(filter);
        if (transform) lasreaderlas->set_transform(transform);
        if (ignore) lasreaderlas->set_ignore(ignore);
        if (inside_rectangle)
          lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile)
          lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else if (inside_circle)
          lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_depth_opener) {
          if (!lasreaderlas->get_copcindex()) {
            laserror("queries with a depth limit are restrited to COPC files.");
            delete lasreaderlas;
            return 0;
          }

          lasreaderlas->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
        }

        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderlas);

        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderlas)) {
            laserror("could not open lasreaderstored with lasreaderlas");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderlas;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderlas");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".bin")) {
        LASreaderBIN* lasreaderbin;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreaderbin = new LASreaderBINreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreaderbin = new LASreaderBIN(this);
        } else if (offset == 0 && scale_factor == 0)
          lasreaderbin = new LASreaderBIN(this);
        else if (scale_factor != 0 && offset == 0)
          lasreaderbin = new LASreaderBINrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderbin = new LASreaderBINreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderbin = new LASreaderBINrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderbin->open(file_name)) {
          laserror("cannot open lasreaderbin with file name '%s'", file_name);
          delete lasreaderbin;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderbin->set_index(index);
        else
          delete index;
        if (files_are_flightlines) lasreaderbin->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderbin->set_filter(filter);
        if (transform) lasreaderbin->set_transform(transform);
        if (ignore) lasreaderbin->set_ignore(ignore);
        if (inside_tile) lasreaderbin->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderbin->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderbin->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderbin);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderbin)) {
            laserror("could not open lasreaderstored with lasreaderbin");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderbin;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderbin");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".shp")) {
        LASreaderSHP* lasreadershp;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreadershp = new LASreaderSHPreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreadershp = new LASreaderSHP(this);
        } else if (offset == 0 && (scale_factor == 0 || (offset_adjust && transform)))
          lasreadershp = new LASreaderSHP(this);
        else if (scale_factor != 0 && offset == 0 && !offset_adjust)
          lasreadershp = new LASreaderSHPrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0 && (offset_adjust && !transform))
          lasreadershp = new LASreaderSHPreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreadershp = new LASreaderSHPrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreadershp->open(file_name)) {
          laserror("cannot open lasreadershp with file name '%s'", file_name);
          delete lasreadershp;
          return 0;
        }
        if (files_are_flightlines) lasreadershp->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreadershp->set_filter(filter);
        if (transform) lasreadershp->set_transform(transform);
        if (ignore) lasreadershp->set_ignore(ignore);
        if (inside_tile) lasreadershp->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreadershp->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreadershp->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreadershp);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreadershp)) {
            laserror("could not open lasreaderstored with lasreadershp");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreadershp;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreadershp");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".asc")) {
        LASreaderASC* lasreaderasc;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreaderasc = new LASreaderASCreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreaderasc = new LASreaderASC(this);
        } else if (scale_factor == 0 && offset == 0)
          lasreaderasc = new LASreaderASC(this);
        else if (scale_factor != 0 && offset == 0)
          lasreaderasc = new LASreaderASCrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderasc = new LASreaderASCreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderasc = new LASreaderASCrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderasc->open(file_name, comma_not_point)) {
          laserror("cannot open lasreaderasc with file name '%s'", file_name);
          delete lasreaderasc;
          return 0;
        }
        if (files_are_flightlines) lasreaderasc->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderasc->set_filter(filter);
        if (transform) lasreaderasc->set_transform(transform);
        if (ignore) lasreaderasc->set_ignore(ignore);
        if (inside_tile) lasreaderasc->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderasc->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderasc->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderasc);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderasc)) {
            laserror("could not open lasreaderstored with lasreaderasc");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderasc;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderasc");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".bil")) {
        LASreaderBIL* lasreaderbil;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreaderbil = new LASreaderBILreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreaderbil = new LASreaderBIL(this);
        } else if (scale_factor == 0 && offset == 0)
          lasreaderbil = new LASreaderBIL(this);
        else if (scale_factor != 0 && offset == 0)
          lasreaderbil = new LASreaderBILrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderbil = new LASreaderBILreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderbil = new LASreaderBILrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderbil->open(file_name)) {
          laserror("cannot open lasreaderbil with file name '%s'", file_name);
          delete lasreaderbil;
          return 0;
        }
        if (files_are_flightlines) lasreaderbil->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderbil->set_filter(filter);
        if (transform) lasreaderbil->set_transform(transform);
        if (ignore) lasreaderbil->set_ignore(ignore);
        if (inside_tile) lasreaderbil->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderbil->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderbil->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderbil);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderbil)) {
            laserror("could not open lasreaderstored with lasreaderbil");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderbil;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderbil");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".dtm")) {
        LASreaderDTM* lasreaderdtm;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreaderdtm = new LASreaderDTMreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreaderdtm = new LASreaderDTM(this);
        } else if (scale_factor == 0 && offset == 0)
          lasreaderdtm = new LASreaderDTM(this);
        else if (scale_factor != 0 && offset == 0)
          lasreaderdtm = new LASreaderDTMrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderdtm = new LASreaderDTMreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderdtm = new LASreaderDTMrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderdtm->open(file_name)) {
          laserror("cannot open lasreaderdtm with file name '%s'", file_name);
          delete lasreaderdtm;
          return 0;
        }
        if (files_are_flightlines) lasreaderdtm->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderdtm->set_filter(filter);
        if (transform) lasreaderdtm->set_transform(transform);
        if (ignore) lasreaderdtm->set_ignore(ignore);
        if (inside_tile) lasreaderdtm->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderdtm->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderdtm->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderdtm);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderdtm)) {
            laserror("could not open lasreaderstored with lasreaderdtm");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderdtm;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderdtm");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".ply")) {
        LASreaderPLY* lasreaderply = new LASreaderPLY(this);
        if (translate_intensity != 0.0f) lasreaderply->set_translate_intensity(translate_intensity);
        if (scale_intensity != 1.0f) lasreaderply->set_scale_intensity(scale_intensity);
        lasreaderply->set_scale_factor(scale_factor);
        lasreaderply->set_offset(offset);
        if (!lasreaderply->open(file_name, point_type, populate_header)) {
          laserror("cannot open lasreaderply with file name '%s'", file_name);
          delete lasreaderply;
          return 0;
        }
        if (files_are_flightlines) lasreaderply->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderply->set_filter(filter);
        if (transform) lasreaderply->set_transform(transform);
        if (ignore) lasreaderply->set_ignore(ignore);
        if (inside_tile) lasreaderply->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderply->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderply->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderply);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderply)) {
            laserror("could not open lasreaderstored with lasreaderply");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderply;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderply");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else if (HasFileExt(std::string(file_name), ".qi")) {
        LASreaderQFIT* lasreaderqfit;
        if (offset_adjust) {
          if (offset != 0 && !transform)
            lasreaderqfit = new LASreaderQFITreoffset(this, offset[0], offset[1], offset[2]);
          else
            lasreaderqfit = new LASreaderQFIT(this);
        } else if (scale_factor == 0 && offset == 0)
          lasreaderqfit = new LASreaderQFIT(this);
        else if (scale_factor != 0 && offset == 0)
          lasreaderqfit = new LASreaderQFITrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderqfit = new LASreaderQFITreoffset(this, offset[0], offset[1], offset[2]);
        else
          lasreaderqfit = new LASreaderQFITrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderqfit->open(file_name)) {
          laserror("cannot open lasreaderqfit with file name '%s'", file_name);
          delete lasreaderqfit;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderqfit->set_index(index);
        else
          delete index;
        if (files_are_flightlines) lasreaderqfit->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderqfit->set_filter(filter);
        if (transform) lasreaderqfit->set_transform(transform);
        if (ignore) lasreaderqfit->set_ignore(ignore);
        if (inside_tile) lasreaderqfit->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderqfit->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderqfit->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreaderqfit);
        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreaderqfit)) {
            laserror("could not open lasreaderstored with lasreaderqfit");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreaderqfit;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreaderqfit");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      } else {
        LASreaderTXT* lasreadertxt = new LASreaderTXT(this);
        if (ipts)
          lasreadertxt->set_pts(TRUE);
        else {
          if (iptx) lasreadertxt->set_ptx(TRUE);
          if (iptx_transform) lasreadertxt->set_ptx_transform(TRUE);
        }
        if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
        if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
        if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
        if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
        lasreadertxt->set_scale_factor(scale_factor);
        lasreadertxt->set_offset(offset);
        if (number_attributes) {
          for (I32 i = 0; i < number_attributes; i++) {
            lasreadertxt->add_attribute(
                attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i],
                attribute_pre_scales[i], attribute_pre_offsets[i], attribute_no_datas[i]);
          }
        }
        if (!lasreadertxt->open(file_name, point_type, parse_string, skip_lines, populate_header)) {
          laserror("cannot open lasreadertxt with file name '%s'", file_name);
          delete lasreadertxt;
          return 0;
        }
        if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreadertxt->set_filter(filter);
        // we may have a ptx_transform defined in the reader params: add them to the transformations
        if (lasreadertxt->iptx_transform) {
          if (transform == 0) transform = new LAStransform();
          transform->add_operation(new LASoperationTransformMatrix(lasreadertxt->transform_matrix));
        }
        if (transform) lasreadertxt->set_transform(transform);
        if (ignore) lasreadertxt->set_ignore(ignore);
        if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        if (offset_adjust && transform && offset == 0) adjust_offset_when_transformation(lasreadertxt);

        LASreader* lasreader = 0;
        if (stored) {
          LASreaderStored* lasreaderstored = new LASreaderStored(this);
          if (!lasreaderstored->open(lasreadertxt)) {
            laserror("could not open lasreaderstored with lasreadertxt");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        } else {
          lasreader = lasreadertxt;
        }
        if (pipe_on) {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
          if (!lasreaderpipeon->open(lasreader)) {
            laserror("cannot open lasreaderpipeon with lasreadertxt");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        } else {
          return lasreader;
        }
      }
    }
  } else if (use_stdin) {
    use_stdin = FALSE;
    populate_header = TRUE;
    if (itxt) {
      LASreaderTXT* lasreadertxt = new LASreaderTXT(this);
      if (ipts)
        lasreadertxt->set_pts(TRUE);
      else {
        if (iptx) lasreadertxt->set_ptx(TRUE);
        if (iptx_transform) lasreadertxt->set_ptx_transform(TRUE);
      }
      if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
      if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
      if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
      if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
      lasreadertxt->set_scale_factor(scale_factor);
      lasreadertxt->set_offset(offset);
      if (number_attributes) {
        for (I32 i = 0; i < number_attributes; i++) {
          lasreadertxt->add_attribute(
              attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i],
              attribute_pre_scales[i], attribute_pre_offsets[i], attribute_no_datas[i]);
        }
      }
      if (!lasreadertxt->open(stdin, 0, point_type, parse_string, skip_lines, FALSE)) {
        laserror("cannot open lasreadertxt with file name '%s'", file_name);
        delete lasreadertxt;
        return 0;
      }
      if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
      if (filter) lasreadertxt->set_filter(filter);
      if (transform) lasreadertxt->set_transform(transform);
      if (ignore) lasreadertxt->set_ignore(ignore);
      if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored) {
        LASreaderStored* lasreaderstored = new LASreaderStored(this);
        if (!lasreaderstored->open(lasreadertxt)) {
          laserror("could not open lasreaderstored with lasreadertxt");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      } else {
        lasreader = lasreadertxt;
      }
      if (pipe_on) {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
        if (!lasreaderpipeon->open(lasreader)) {
          laserror("cannot open lasreaderpipeon with lasreadertxt");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      } else {
        return lasreader;
      }
    } else {
      LASreaderLAS* lasreaderlas;
      if (scale_factor == 0 && offset == 0)
        lasreaderlas = new LASreaderLAS(this);
      else if (scale_factor != 0 && offset == 0)
        lasreaderlas = new LASreaderLASrescale(this, scale_factor[0], scale_factor[1], scale_factor[2]);
      else if (scale_factor == 0 && offset != 0)
        lasreaderlas = new LASreaderLASreoffset(this, offset[0], offset[1], offset[2]);
      else
        lasreaderlas = new LASreaderLASrescalereoffset(this, scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
      if (!lasreaderlas->open(stdin)) {
        laserror("cannot open lasreaderlas from stdin ");
        delete lasreaderlas;
        return 0;
      }
      if (filter) lasreaderlas->set_filter(filter);
      if (transform) lasreaderlas->set_transform(transform);
      if (ignore) lasreaderlas->set_ignore(ignore);
      if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      if (inside_depth_opener) lasreaderlas->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
      LASreader* lasreader = 0;
      if (stored) {
        LASreaderStored* lasreaderstored = new LASreaderStored(this);
        if (!lasreaderstored->open(lasreaderlas)) {
          laserror("could not open lasreaderstored with lasreaderlas");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      } else {
        lasreader = lasreaderlas;
      }
      if (pipe_on) {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn(this);
        if (!lasreaderpipeon->open(lasreader)) {
          laserror("cannot open lasreaderpipeon with lasreaderlas from stdin");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      } else {
        return lasreader;
      }
    }
  } else {
    return 0;
  }
}

BOOL LASreadOpener::reopen(LASreader* lasreader, BOOL remain_buffered) {
  if (lasreader == 0) {
    LASMessage(LAS_WARNING, "pointer to LASreader is NULL");
    return FALSE;
  }

  // make sure the LASreader was closed

  lasreader->close();

  if (filter) filter->reset();
  if (transform) transform->reset();

  if (pipe_on) {
    LASreaderPipeOn* lasreaderpipeon = (LASreaderPipeOn*)lasreader;
    lasreaderpipeon->p_idx = 0;
    lasreaderpipeon->p_cnt = 0;
    lasreader = lasreaderpipeon->get_lasreader();
  }

  if (stored) {
    LASreaderStored* lasreaderstored = (LASreaderStored*)lasreader;
    if (!lasreaderstored->reopen()) {
      LASMessage(LAS_WARNING, "could not reopen lasreaderstored for stored input");
      return FALSE;
    }
    return TRUE;
  } else if (file_names) {
    if ((file_name_number > 1) && merged) {
      LASreaderMerged* lasreadermerged = (LASreaderMerged*)lasreader;
      if (!lasreadermerged->reopen()) {
        LASMessage(LAS_WARNING, "cannot reopen lasreadermerged");
        return FALSE;
      }
      if (inside_rectangle || inside_tile || inside_circle) {
        lasreadermerged->inside_none();
        if (inside_rectangle)
          lasreadermerged->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile)
          lasreadermerged->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else
          lasreadermerged->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      }
      if (inside_depth_opener) lasreadermerged->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
      return TRUE;
    } else if ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0))) {
      LASreaderBuffered* lasreaderbuffered = (LASreaderBuffered*)lasreader;
      if (!lasreaderbuffered->reopen()) {
        laserror("cannot reopen lasreaderbuffered");
        return FALSE;
      }
      if (inside_rectangle || inside_tile || inside_circle) {
        lasreaderbuffered->inside_none();
        if (inside_rectangle)
          lasreaderbuffered->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile)
          lasreaderbuffered->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else
          lasreaderbuffered->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      }
      if (inside_depth_opener) lasreaderbuffered->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
      if (!remain_buffered) lasreaderbuffered->remove_buffer();
      return TRUE;
    } else {
      if (!file_name) return FALSE;
      if (IsLasLazFile(std::string(file_name))) {
        LASreaderLAS* lasreaderlas = (LASreaderLAS*)lasreader;
        if (!lasreaderlas->open(file_name, io_ibuffer_size, FALSE, decompress_selective)) {
          laserror("cannot reopen lasreaderlas with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderlas->inside_none();
          if (inside_rectangle)
            lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        if (inside_depth_opener) lasreaderlas->inside_copc_depth(inside_depth_opener, copc_depth, copc_resolution);
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".bin")) {
        LASreaderBIN* lasreaderbin = (LASreaderBIN*)lasreader;
        if (!lasreaderbin->open(file_name)) {
          laserror("cannot reopen lasreaderbin with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderbin->inside_none();
          if (inside_rectangle)
            lasreaderbin->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderbin->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderbin->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".shp")) {
        LASreaderSHP* lasreadershp = (LASreaderSHP*)lasreader;
        if (!lasreadershp->reopen(file_name)) {
          laserror("cannot reopen lasreadershp with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreadershp->inside_none();
          if (inside_rectangle)
            lasreadershp->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreadershp->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreadershp->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".qi")) {
        LASreaderQFIT* lasreaderqfit = (LASreaderQFIT*)lasreader;
        if (!lasreaderqfit->reopen(file_name)) {
          laserror("cannot reopen lasreaderqfit with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderqfit->inside_none();
          if (inside_rectangle)
            lasreaderqfit->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderqfit->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderqfit->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".asc")) {
        LASreaderASC* lasreaderasc = (LASreaderASC*)lasreader;
        if (!lasreaderasc->reopen(file_name)) {
          laserror("cannot reopen lasreaderasc with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderasc->inside_none();
          if (inside_rectangle)
            lasreaderasc->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderasc->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderasc->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".bil")) {
        LASreaderBIL* lasreaderbil = (LASreaderBIL*)lasreader;
        if (!lasreaderbil->reopen(file_name)) {
          laserror("cannot reopen lasreaderbil with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderbil->inside_none();
          if (inside_rectangle)
            lasreaderbil->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderbil->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderbil->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else if (HasFileExt(std::string(file_name), ".dtm")) {
        LASreaderDTM* lasreaderdtm = (LASreaderDTM*)lasreader;
        if (!lasreaderdtm->reopen(file_name)) {
          laserror("cannot reopen lasreaderdtm with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreaderdtm->inside_none();
          if (inside_rectangle)
            lasreaderdtm->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreaderdtm->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreaderdtm->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      } else {
        LASreaderTXT* lasreadertxt = (LASreaderTXT*)lasreader;
        if (!lasreadertxt->reopen(file_name)) {
          laserror("cannot reopen lasreadertxt with file name '%s'", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle) {
          lasreadertxt->inside_none();
          if (inside_rectangle)
            lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile)
            lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else
            lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
    }
  } else {
    laserror("no lasreader input specified");
    return FALSE;
  }
}

LASwaveform13reader* LASreadOpener::open_waveform13(const LASheader* lasheader) {
  if (lasheader->point_data_format < 4) return 0;
  if ((lasheader->point_data_format > 5) && (lasheader->point_data_format < 9)) return 0;
  if (lasheader->vlr_wave_packet_descr == 0) return 0;
  if (get_file_name() == 0) return 0;
  LASwaveform13reader* waveform13reader = new LASwaveform13reader();
  if ((lasheader->global_encoding & 2) && (lasheader->start_of_waveform_data_packet_record > lasheader->offset_to_point_data)) {
    if (waveform13reader->open(get_file_name(), lasheader->start_of_waveform_data_packet_record, lasheader->vlr_wave_packet_descr)) {
      return waveform13reader;
    }
  } else {
    if (waveform13reader->open(get_file_name(), 0, lasheader->vlr_wave_packet_descr)) {
      return waveform13reader;
    }
  }
  delete waveform13reader;
  return 0;
}

void LASreadOpener::usage() const {
  // pre-formated multiline message
  LASMessage(
      LAS_INFO,
      "Supported LAS Inputs\n"
      "  -i lidar.las\n"
      "  -i lidar.laz\n"
      "  -i lidar1.las lidar2.las lidar3.las -merged\n"
      "  -i *.las - merged\n"
      "  -i flight0??.laz flight1??.laz\n"
      "  -i terrasolid.bin\n"
      "  -i esri.shp\n"
      "  -i lidar.txt -iparse xyzti -iskip 2 (on-the-fly from ASCII)\n"
      "  -i lidar.txt -iparse xyzi -itranslate_intensity 1024\n"
      "  -lof file_list.txt\n"
      "  -stdin (pipe from stdin)\n"
      "  -rescale 0.01 0.01 0.001\n"
      "  -rescale_xy 0.01 0.01\n"
      "  -rescale_z 0.01\n"
      "  -reoffset 600000 4000000 0\n"
      "Fast AOI Queries for LAS/LAZ with spatial indexing LAX files\n"
      "  -inside min_x min_y max_x max_y\n"
      "  -inside_tile ll_x ll_y size\n"
      "  -inside_circle center_x center_y radius\n"
      "Fast AOI Queries for LAZ 1.4 with spatial indexing COPC VLR\n"
      "  -inside min_x min_y max_x max_y\n"
      "  -inside_circle center_x center_y radius\n"
      "  -max_depth 3\n"
      "  -resolution 0.1");
}

void LASreadOpener::parse(int argc, char* argv[], BOOL parse_ignore, BOOL suppress_ignore) {
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '\0') {
      continue;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
      return;
    } else if (strcmp(argv[i], "-hh") == 0) {
      LASfilter().usage();
      LAStransform().usage();
      LASignore().usage();
      usage();
      return;
    } else if (strncmp(argv[i], "-i", 2) == 0) {
      if (strcmp(argv[i], "-i") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs at least 1 argument: file_name or wild_card", argv[i]);
        }
        *argv[i] = '\0';
        i += 1;
        do {
          add_file_name(argv[i], unique);
          *argv[i] = '\0';
          i += 1;
        } while ((i < argc) && (*argv[i] != '-') && (*argv[i] != '\0'));
        i -= 1;
      } else if (strncmp(argv[i], "-ignore_", 8) == 0) {
        if (parse_ignore) {
          if (ignore == 0) {
            ignore = new LASignore();
          }
          if (!ignore->parse(i, argc, argv)) {
            delete ignore;
            ignore = 0;
            return;
          }
        } else if (!suppress_ignore) {
          LASMessage(LAS_WARNING, "this tool does not process '-ignore_xxxx' options");
        }
      } else if (strncmp(argv[i], "-inside", 7) == 0) {
        if (strcmp(argv[i], "-inside_tile") == 0) {
          if ((i + 3) >= argc) {
            laserror("'%s' needs 3 arguments: ll_x ll_y size", argv[i]);
          }
          F32 ll_x;
          if (sscanf(argv[i + 1], "%f", &ll_x) != 1) {
            laserror("'%s' needs 3 arguments: ll_x ll_y size, but '%s' is not a valid ll_x.", argv[i], argv[i + 1]);
          }
          F32 ll_y;
          if (sscanf(argv[i + 2], "%f", &ll_y) != 1) {
            laserror("'%s' needs 3 arguments: ll_x ll_y size, but '%s' is not a valid ll_y.", argv[i], argv[i + 2]);
          }
          F32 size;
          if (sscanf(argv[i + 3], "%f", &size) != 1) {
            laserror("'%s' needs 3 arguments: ll_x ll_y size, but '%s' is not a valid size.", argv[i], argv[i + 3]);
          }
          if (size <= 0.0f) {
            laserror("'%s' needs 3 arguments: ll_x ll_y size, but %f is not valid a size.", argv[i], size);
          }
          set_inside_tile(ll_x, ll_y, size);
          *argv[i] = '\0';
          *argv[i + 1] = '\0';
          *argv[i + 2] = '\0';
          *argv[i + 3] = '\0';
          i += 3;
        } else if (strcmp(argv[i], "-inside_circle") == 0) {
          if ((i + 3) >= argc) {
            laserror("'%s' needs 3 arguments: center_x center_y radius", argv[i]);
          }
          F64 center_x;
          if (sscanf(argv[i + 1], "%lf", &center_x) != 1) {
            laserror("'%s' needs 3 arguments: center_x center_y radius, but '%s' is not a valid center_x.", argv[i], argv[i + 1]);
          }
          F64 center_y;
          if (sscanf(argv[i + 2], "%lf", &center_y) != 1) {
            laserror("'%s' needs 3 arguments: center_x center_y radius, but '%s' is not a valid center_y.", argv[i], argv[i + 2]);
          }
          F64 radius;
          if (sscanf(argv[i + 3], "%lf", &radius) != 1) {
            laserror("'%s' needs 3 arguments: center_x center_y radius, but '%s' is not a valid radius.", argv[i], argv[i + 3]);
          }
          if (radius <= 0.0f) {
            laserror("'%s' needs 3 arguments: center_x center_y radius, but %lf is not valid a radius.", argv[i], radius);
          }
          set_inside_circle(center_x, center_y, radius);
          *argv[i] = '\0';
          *argv[i + 1] = '\0';
          *argv[i + 2] = '\0';
          *argv[i + 3] = '\0';
          i += 3;
        } else if (strcmp(argv[i], "-inside") == 0 || strcmp(argv[i], "-inside_rectangle") == 0) {
          if ((i + 4) >= argc) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y", argv[i]);
          }
          F64 min_x;
          if (sscanf(argv[i + 1], "%lf", &min_x) != 1) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but '%s' is not a valid min_x.", argv[i], argv[i + 1]);
          }
          F64 min_y;
          if (sscanf(argv[i + 2], "%lf", &min_y) != 1) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but '%s' is not a valid min_y.", argv[i], argv[i + 2]);
          }
          F64 max_x;
          if (sscanf(argv[i + 3], "%lf", &max_x) != 1) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but '%s' is not a valid max_x.", argv[i], argv[i + 3]);
          }
          F64 max_y;
          if (sscanf(argv[i + 4], "%lf", &max_y) != 1) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but '%s' is not a valid max_y.", argv[i], argv[i + 4]);
          }
          if (min_x >= max_x) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but %lf / %lf are not a valid min_x / max_x pair.", argv[i], min_x, max_x);
          }
          if (min_y >= max_y) {
            laserror("'%s' needs 4 arguments: min_x min_y max_x max_y, but %lf / %lf are not a valid min_y / max_y pair.", argv[i], min_y, max_y);
          }
          set_inside_rectangle(min_x, min_y, max_x, max_y);
          *argv[i] = '\0';
          *argv[i + 1] = '\0';
          *argv[i + 2] = '\0';
          *argv[i + 3] = '\0';
          *argv[i + 4] = '\0';
          i += 4;
        } else {
          laserror("unknown '-inside' option '%s'", argv[i]);
        }
      } else if (strcmp(argv[i], "-iadd_extra") == 0 || strcmp(argv[i], "-iadd_attribute") == 0) {
        if ((i + 3) >= argc) {
          laserror("'%s' needs 3 arguments: data_type name description", argv[i]);
        }
        if (((i + 4) < argc) && (atof(argv[i + 4]) != 0.0)) {
          if (((i + 5) < argc) && ((atof(argv[i + 5]) != 0.0) || (strcmp(argv[i + 5], "0") == 0) || (strcmp(argv[i + 5], "0.0") == 0))) {
            if (((i + 6) < argc) && (atof(argv[i + 6]) != 0.0)) {
              if (((i + 7) < argc) && ((atof(argv[i + 7]) != 0.0) || (strcmp(argv[i + 7], "0") == 0) || (strcmp(argv[i + 7], "0.0") == 0))) {
                if (((i + 8) < argc) && ((atof(argv[i + 8]) != 0.0) || (strcmp(argv[i + 8], "0") == 0) || (strcmp(argv[i + 8], "0.0") == 0))) {
                  add_attribute(
                      atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]), atof(argv[i + 6]), atof(argv[i + 7]),
                      atof(argv[i + 8]));
                  *argv[i] = '\0';
                  *argv[i + 1] = '\0';
                  *argv[i + 2] = '\0';
                  *argv[i + 3] = '\0';
                  *argv[i + 4] = '\0';
                  *argv[i + 5] = '\0';
                  *argv[i + 6] = '\0';
                  *argv[i + 7] = '\0';
                  *argv[i + 8] = '\0';
                  i += 8;
                } else {
                  add_attribute(
                      atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]), atof(argv[i + 6]), atof(argv[i + 7]));
                  *argv[i] = '\0';
                  *argv[i + 1] = '\0';
                  *argv[i + 2] = '\0';
                  *argv[i + 3] = '\0';
                  *argv[i + 4] = '\0';
                  *argv[i + 5] = '\0';
                  *argv[i + 6] = '\0';
                  *argv[i + 7] = '\0';
                  i += 7;
                }
              } else {
                add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]), atof(argv[i + 6]));
                *argv[i] = '\0';
                *argv[i + 1] = '\0';
                *argv[i + 2] = '\0';
                *argv[i + 3] = '\0';
                *argv[i + 4] = '\0';
                *argv[i + 5] = '\0';
                *argv[i + 6] = '\0';
                i += 6;
              }
            } else {
              add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]));
              *argv[i] = '\0';
              *argv[i + 1] = '\0';
              *argv[i + 2] = '\0';
              *argv[i + 3] = '\0';
              *argv[i + 4] = '\0';
              *argv[i + 5] = '\0';
              i += 5;
            }
          } else {
            add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]));
            *argv[i] = '\0';
            *argv[i + 1] = '\0';
            *argv[i + 2] = '\0';
            *argv[i + 3] = '\0';
            *argv[i + 4] = '\0';
            i += 4;
          }
        } else {
          add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3]);
          *argv[i] = '\0';
          *argv[i + 1] = '\0';
          *argv[i + 2] = '\0';
          *argv[i + 3] = '\0';
          i += 3;
        }
      } else if (strcmp(argv[i], "-iparse") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: string", argv[i]);
        }
        set_parse_string(argv[i + 1]);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-iskip") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: number_of_lines", argv[i]);
        }
        U32 number_of_lines;
        if (sscanf(argv[i + 1], "%u", &number_of_lines) != 1) {
          laserror("'%s' needs 1 argument: number_of_lines but '%s' is not a valid number.", argv[i], argv[i + 1]);
        }
        if (number_of_lines == 0) {
          laserror("'%s' needs 1 argument: number_of_lines but %u is not valid.", argv[i], number_of_lines);
        }
        set_skip_lines(number_of_lines);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-io_ibuffer") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: size", argv[i]);
        }
        U32 buffer_size;
        if (sscanf(argv[i + 1], "%u", &buffer_size) != 1) {
          laserror("'%s' needs 1 argument: size but '%s' is not a valid size.", argv[i], argv[i + 1]);
        }
        if (buffer_size == 0) {
          laserror("'%s' needs 1 argument: size but %u is not valid.", argv[i], buffer_size);
        }
        set_io_ibuffer_size(buffer_size);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-itranslate_intensity") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: translation", argv[i]);
        }
        F32 translation;
        if (sscanf(argv[i + 1], "%f", &translation) != 1) {
          laserror("'%s' needs 1 argument: translation but '%s' is not valid.", argv[i], argv[i + 1]);
        }
        if (translation == 0.0f) {
          laserror("'%s' needs 1 argument: translation but %f is not valid.", argv[i], translation);
        }
        set_translate_intensity(translation);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-iscale_intensity") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: scale", argv[i]);
        }
        F32 scale;
        if (sscanf(argv[i + 1], "%f", &scale) != 1) {
          laserror("'%s' needs 1 argument: scale but '%s' is not valid.", argv[i], argv[i + 1]);
        }
        if (scale == 0.0f) {
          laserror("'%s' needs 1 argument: scale but %f is not valid.", argv[i], scale);
        }
        set_scale_intensity(scale);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-itranslate_scan_angle") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: translation", argv[i]);
        }
        F32 translation;
        if (sscanf(argv[i + 1], "%f", &translation) != 1) {
          laserror("'%s' needs 1 argument: translation but '%s' is not valid.", argv[i], argv[i + 1]);
        }
        if (translation == 0.0f) {
          laserror("'%s' needs 1 argument: translation but %f is not valid.", argv[i], translation);
        }
        set_translate_scan_angle(translation);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-iscale_scan_angle") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: scale", argv[i]);
        }
        F32 scale;
        if (sscanf(argv[i + 1], "%f", &scale) != 1) {
          laserror("'%s' needs 1 argument: scale but '%s' is not valid.", argv[i], argv[i + 1]);
        }
        if (scale == 0.0f) {
          laserror("'%s' needs 1 argument: scale but %f is not valid.", argv[i], scale);
        }
        set_scale_scan_angle(scale);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-ipts") == 0) {
        itxt = TRUE;
        ipts = TRUE;
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-iptx") == 0) {
        itxt = TRUE;
        iptx = TRUE;
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-iptx_transform") == 0) {
        itxt = TRUE;
        iptx_transform = TRUE;
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-itxt") == 0) {
        itxt = TRUE;
        *argv[i] = '\0';
      }
    } else if (strncmp(argv[i], "-r", 2) == 0) {
      if (strcmp(argv[i], "-rescale") == 0) {
        if ((i + 3) >= argc) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z", argv[i]);
        }
        F64 scale_factor[3];
        if (sscanf(argv[i + 1], "%lf", &(scale_factor[0])) != 1) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but '%s' is not a valid rescale_x", argv[i], argv[i + 1]);
        }
        if (scale_factor[0] == 0.0) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but %lf is not a valid rescale_x", argv[i], scale_factor[0]);
        }
        if (sscanf(argv[i + 2], "%lf", &(scale_factor[1])) != 1) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but '%s' is not a valid rescale_y", argv[i], argv[i + 2]);
        }
        if (scale_factor[1] == 0.0) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but %lf is not a valid rescale_y", argv[i], scale_factor[1]);
        }
        if (sscanf(argv[i + 3], "%lf", &(scale_factor[2])) != 1) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but '%s' is not a valid rescale_z", argv[i], argv[i + 3]);
        }
        if (scale_factor[2] == 0.0) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y rescale_z, but %lf is not a valid rescale_z", argv[i], scale_factor[2]);
        }
        set_scale_factor(scale_factor);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        *argv[i + 2] = '\0';
        *argv[i + 3] = '\0';
        i += 3;
      } else if (strcmp(argv[i], "-rescale_xy") == 0) {
        if ((i + 2) >= argc) {
          laserror("'%s' needs 2 argument: rescale_x rescale_y", argv[i]);
        }
        F64 scale_factor[3];
        if (sscanf(argv[i + 1], "%lf", &(scale_factor[0])) != 1) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y, but '%s' is not a valid rescale_x", argv[i], argv[i + 1]);
        }
        if (scale_factor[0] == 0.0) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y, but %lf is not a valid rescale_x", argv[i], scale_factor[0]);
        }
        if (sscanf(argv[i + 2], "%lf", &(scale_factor[1])) != 1) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y, but '%s' is not a valid rescale_y", argv[i], argv[i + 2]);
        }
        if (scale_factor[1] == 0.0) {
          laserror("'%s' needs 3 arguments: rescale_x rescale_y, but %lf is not a valid rescale_y", argv[i], scale_factor[1]);
        }
        scale_factor[2] = 0.0;
        set_scale_factor(scale_factor);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        *argv[i + 2] = '\0';
        i += 2;
      } else if (strcmp(argv[i], "-rescale_z") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: rescale_z", argv[i]);
        }
        F64 scale_factor[3];
        scale_factor[0] = 0.0;
        scale_factor[1] = 0.0;
        if (sscanf(argv[i + 1], "%lf", &(scale_factor[2])) != 1) {
          laserror("'%s' needs 1 argument: rescale_z, but '%s' is not a valid rescale_z", argv[i], argv[i + 1]);
        }
        if (scale_factor[2] == 0.0) {
          laserror("'%s' needs 1 argument: rescale_z, but %lf is not a valid rescale_z", argv[i], scale_factor[2]);
        }
        set_scale_factor(scale_factor);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      } else if (strcmp(argv[i], "-reoffset") == 0) {
        if ((i + 3) >= argc) {
          laserror("'%s' needs 3 arguments: reoffset_x, reoffset_y, reoffset_z", argv[i]);
        }
        F64 offset[3];
        if (sscanf(argv[i + 1], "%lf", &(offset[0])) != 1) {
          laserror("'%s' needs 3 arguments: reoffset_x, reoffset_y, reoffset_z, but '%s' is not a valid reoffset_x", argv[i], argv[i + 1]);
        }
        if (sscanf(argv[i + 2], "%lf", &(offset[1])) != 1) {
          laserror("'%s' needs 3 arguments: reoffset_x, reoffset_y, reoffset_z, but '%s' is not a valid reoffset_y", argv[i], argv[i + 2]);
        }
        if (sscanf(argv[i + 3], "%lf", &(offset[2])) != 1) {
          laserror("'%s' needs 3 arguments: reoffset_x, reoffset_y, reoffset_z, but '%s' is not a valid reoffset_z", argv[i], argv[i + 3]);
        }
        set_offset(offset);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        *argv[i + 2] = '\0';
        *argv[i + 3] = '\0';
        i += 3;
      } else if (strcmp(argv[i], "-resolution") == 0)  // copc
      {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: resolution", argv[i]);
        }
        F32 resolution;
        if (sscanf(argv[i + 1], "%f", &(resolution)) != 1) {
          laserror("'%s' needs 1 argument: resolution, but '%s' is not a valid resolution", argv[i], argv[i + 1]);
        }
        if (resolution <= 0) {
          laserror("'%s' needs 1 argument: resolution, but %lf is not a valid resolution", argv[i], resolution);
        }

        set_resolution(resolution);
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      }
    } else if (strcmp(argv[i], "-unique") == 0) {
      unique = TRUE;
      *argv[i] = '\0';
    } else if (strcmp(argv[i], "-comma_not_point") == 0) {
      comma_not_point = TRUE;
      *argv[i] = '\0';
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      if (strcmp(argv[i], "-stdin") == 0) {
        use_stdin = TRUE;
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-stored") == 0) {
        set_stored(TRUE);
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-stream_order_spatial") == 0)  // COPC only
      {
        set_copc_stream_ordered_spatially();
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-stream_order_normal") == 0)  // COPC only
      {
        set_copc_stream_ordered_by_chunk();
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-stream_order_level") == 0)  // COPC only
      {
        set_copc_stream_ordered_by_level();
        *argv[i] = '\0';
      }
    } else if (strcmp(argv[i], "-lof") == 0) {
      if ((i + 1) >= argc) {
        laserror("'%s' needs 1 argument: list_of_files", argv[i]);
      }
      if (!add_list_of_files(argv[i + 1], unique)) {
        laserror("cannot load list of files '%s'", argv[i + 1]);
      }
      *argv[i] = '\0';
      *argv[i + 1] = '\0';
      i += 1;
    } else if (strncmp(argv[i], "-a", 2) == 0) {
      if (strcmp(argv[i], "-auto_reoffset") == 0) {
        set_auto_reoffset(TRUE);
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-apply_file_source_ID") == 0) {
        set_apply_file_source_ID(TRUE);
        *argv[i] = '\0';
      }
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      if (strcmp(argv[i], "-files_are_flightlines") == 0 || strcmp(argv[i], "-faf") == 0) {
        if (((i + 1) < argc) && ('1' <= argv[i + 1][0]) && (argv[i + 1][0] <= '9')) {
          set_files_are_flightlines(atoi(argv[i + 1]));
          *argv[i] = '\0';
          *argv[i + 1] = '\0';
          i += 1;
        } else {
          set_files_are_flightlines(1);
          *argv[i] = '\0';
        }
      } else if (strcmp(argv[i], "-faf_index") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs 1 argument: index", argv[i]);
        }
        set_files_are_flightlines_index(atoi(argv[i + 1]));
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      }
    } else if (strcmp(argv[i], "-offset_adjust") == 0) {
      set_offset_adjust(TRUE);
      *argv[i] = '\0';
    } else if (strcmp(argv[i], "-merged") == 0) {
      set_merged(TRUE);
      *argv[i] = '\0';
    } else if (strcmp(argv[i], "-buffered") == 0) {
      if ((i + 1) >= argc) {
        laserror("'%s' needs 1 argument: buffer_size", argv[i]);
      }
      F32 buffer_size;
      if (sscanf(argv[i + 1], "%f", &buffer_size) != 1) {
        laserror("'%s' needs 1 argument: buffer_size. but '%s' is not a valid buffer_size", argv[i], argv[i + 1]);
      }
      if (buffer_size <= 0.0f) {
        laserror("'%s' needs 1 argument: buffer_size, but %f is not valid", argv[i], buffer_size);
      }
      set_buffer_size(buffer_size);
      *argv[i] = '\0';
      *argv[i + 1] = '\0';
      i += 1;
    } else if (strcmp(argv[i], "-temp_files") == 0) {
      if ((i + 1) >= argc) {
        laserror("'%s' needs 1 argument: base name", argv[i]);
      }
      temp_file_base = std::string(argv[i + 1]);
      *argv[i] = '\0';
      *argv[i + 1] = '\0';
      i += 1;
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      if (strcmp(argv[i], "-neighbors") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs at least 1 argument: file_name or wild_card", argv[i]);
        }
        *argv[i] = '\0';
        i += 1;
        do {
          add_neighbor_file_name(argv[i]);
          *argv[i] = '\0';
          i += 1;
        } while ((i < argc) && (*argv[i] != '-') && (*argv[i] != '\0'));
        i -= 1;
      } else if (strcmp(argv[i], "-neighbors_lof") == 0) {
        if ((i + 1) >= argc) {
          laserror("'%s' needs at least 1 argument: file_name", argv[i]);
        }
        if (!add_neighbor_list_of_files(argv[i + 1], unique)) {
          laserror("cannot load neighbor list of files '%s'", argv[i + 1]);
        }
        *argv[i] = '\0';
        *argv[i + 1] = '\0';
        i += 1;
      }
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      if (strcmp(argv[i], "-pipe_on") == 0) {
        set_pipe_on(TRUE);
        *argv[i] = '\0';
      } else if (strcmp(argv[i], "-populate") == 0) {
        set_populate_header(TRUE);
        *argv[i] = '\0';
      }
    } else if (strcmp(argv[i], "-do_not_populate") == 0) {
      set_populate_header(FALSE);
      *argv[i] = '\0';
    } else if (strcmp(argv[i], "-max_depth") == 0)  // copc
    {
      if ((i + 1) >= argc) {
        laserror("'%s' needs 1 argument: depth", argv[i]);
      }
      I32 depth;
      if (sscanf(argv[i + 1], "%d", &(depth)) != 1) {
        laserror("'%s' needs 1 argument: depth, but '%s' is not a valid depth", argv[i], argv[i + 1]);
      }
      if (depth < 0) {
        laserror("'%s' needs 1 argument: depth, but %d is not a valid depth", argv[i], depth);
      }
      set_max_depth(depth);
      *argv[i] = '\0';
      *argv[i + 1] = '\0';
      i += 1;
    } else if (strcmp(argv[i], "-z_from_attribute") == 0) {
      z_from_attribute = true;
      *argv[i] = '\0';
      if (((i + 1) < argc) && (argv[i + 1][0] != '-')) {
        i++;
        z_from_attribute_idx = atoi(argv[i]);
        *argv[i] = '\0';
      }
      i += 1;
    }
  }

  // check that there are only buffered neighbors for single files

  if (neighbor_file_name_number) {
    if (file_name_number > 1) {
      laserror("neighbors only supported for one buffered input file, not for %d", file_name_number);
    }
    if (buffer_size == 0.0f) {
      laserror("neighbors only make sense when used with '-buffered 50' or similar");
    }
  }

  if (filter)
    filter->clean();
  else
    filter = new LASfilter();
  if (!filter->parse(argc, argv)) {
    delete filter;
    filter = 0;
    return;
  }
  if (!filter->active()) {
    delete filter;
    filter = 0;
  }

  if (transform)
    transform->clean();
  else
    transform = new LAStransform();

  if (!transform->parse(argc, argv)) {
    delete transform;
    transform = 0;
    return;
  }
  if (!transform->active()) {
    if (transform->filtered()) {
      LASMessage(LAS_WARNING, "no LAStransform specified. '-filtered_transform' has no effect.");
    }
    delete transform;
    transform = 0;
  } else if (transform->filtered()) {
    if (filter == 0) {
      LASMessage(LAS_WARNING, "no LASfilter specified. '-filtered_transform' has no effect.");
    } else {
      transform->setFilter(filter);
      filter = 0;
    }
  }

  if (files_are_flightlines || apply_file_source_ID) {
    if (transform == 0) transform = new LAStransform();
    transform->setPointSource(0);
  }

  return;
}

U32 LASreadOpener::get_file_name_number() const {
  return file_name_number;
}

U32 LASreadOpener::get_file_name_current() const {
  return file_name_current;
}

#ifdef LASTOOLS_FULL
std::string LASreadOpener::get_file_name_opt_only(bool name_only) {
  if (name_only)
    return std::string(get_file_name_only());
  else {
    // get FULL filename
    std::filesystem::path ffn = get_file_name();  
    return std::filesystem::absolute(ffn).string();
  }
}
#endif

const CHAR* LASreadOpener::get_file_name() const {
  if (file_name) return file_name;
  if (file_name_number) return file_names[0];
  return 0;
}

const CHAR* LASreadOpener::get_file_name_only() const {
  const CHAR* file_name_only = 0;
  const CHAR* file_name_curr = get_file_name();
  if (file_name_curr) {
    I32 len = (I32)strlen(file_name_curr);
    while ((len > 0) && (file_name_curr[len] != '\\') && (file_name_curr[len] != '/') && (file_name_curr[len] != ':')) len--;
    if (len) {
      file_name_only = file_name_curr + len + 1;
    } else {
      file_name_only = file_name_curr;
    }
  }
  return file_name_only;
}

const CHAR* LASreadOpener::get_file_extension_only() const {
  const CHAR* file_extension_only = 0;
  const CHAR* file_name_curr = get_file_name();

  if (file_name_curr) {
    I32 len = (I32)strlen(file_name_curr);
    while ((len > 0) && (file_name_curr[len] != '.')) len--;
    if (len) {
      file_extension_only = file_name_curr + len + 1;
    }
  }

  return file_extension_only;
}

const CHAR* LASreadOpener::get_file_name(U32 number) const {
  return file_names[number];
}

const CHAR* LASreadOpener::get_file_name_only(U32 number) const {
  const CHAR* file_name_only = 0;
  const CHAR* file_name_curr = get_file_name(number);

  if (file_name_curr) {
    I32 len = (I32)strlen(file_name_curr);
    while ((len > 0) && (file_name_curr[len] != '\\') && (file_name_curr[len] != '/') && (file_name_curr[len] != ':')) len--;
    if (len) {
      file_name_only = file_name_curr + len + 1;
    } else {
      file_name_only = file_name_curr;
    }
  }

  return file_name_only;
}

const CHAR* LASreadOpener::get_file_extension_only(U32 number) const {
  const CHAR* file_extension_only = 0;
  const CHAR* file_name_curr = get_file_name(number);

  if (file_name_curr) {
    I32 len = (I32)strlen(file_name_curr);
    while ((len > 0) && (file_name_curr[len] != '.')) len--;
    if (len) {
      file_extension_only = file_name_curr + len + 1;
    }
  }

  return file_extension_only;
}

I32 LASreadOpener::get_file_format(U32 number) const {
  if (IsLasLazFile(std::string(file_names[number]))) {
    return LAS_TOOLS_FORMAT_LAS;
  } else if (HasFileExt(std::string(file_names[number]), ".bin")) {
    return LAS_TOOLS_FORMAT_BIN;
  } else if (HasFileExt(std::string(file_names[number]), ".shp")) {
    return LAS_TOOLS_FORMAT_SHP;
  } else if (HasFileExt(std::string(file_names[number]), ".qi")) {
    return LAS_TOOLS_FORMAT_QFIT;
  } else if (HasFileExt(std::string(file_names[number]), ".asc")) {
    return LAS_TOOLS_FORMAT_ASC;
  } else if (HasFileExt(std::string(file_names[number]), ".bil")) {
    return LAS_TOOLS_FORMAT_BIL;
  } else if (HasFileExt(std::string(file_names[number]), ".dtm")) {
    return LAS_TOOLS_FORMAT_DTM;
  } else {
    return LAS_TOOLS_FORMAT_TXT;
  }
}

CHAR* LASreadOpener::get_file_name_base() const {
  CHAR* file_name_base = 0;

  if (file_name) {
    file_name_base = LASCopyString(file_name);
    // remove extension
    I32 len = (I32)strlen(file_name_base);
    while ((len > 0) && (file_name_base[len] != '\\') && (file_name_base[len] != '/') && (file_name_base[len] != ':')) len--;
    file_name_base[len] = '\0';
  }

  return file_name_base;
}

CHAR* LASreadOpener::get_file_name_base(U32 number) const {
  CHAR* file_name_base = 0;

  if (get_file_name(number)) {
    file_name_base = LASCopyString(get_file_name(number));
    // remove extension
    I32 len = (I32)strlen(file_name_base);
    while ((len > 0) && (file_name_base[len] != '\\') && (file_name_base[len] != '/') && (file_name_base[len] != ':')) len--;
    file_name_base[len] = '\0';
  }

  return file_name_base;
}

void LASreadOpener::set_merged(const BOOL merged) {
  this->merged = merged;
}

void LASreadOpener::set_stored(const BOOL stored) {
  this->stored = stored;
}

void LASreadOpener::set_buffer_size(const F32 buffer_size) {
  this->buffer_size = buffer_size;
}

F32 LASreadOpener::get_buffer_size() const {
  return buffer_size;
}

void LASreadOpener::set_filter(LASfilter* filter) {
  this->filter = filter;
}

void LASreadOpener::set_transform(LAStransform* transform) {
  this->transform = transform;
}

void LASreadOpener::set_auto_reoffset(const BOOL auto_reoffset) {
  this->auto_reoffset = auto_reoffset;
}

/// Call up after transformation object has been set in LASreaderOpener (if it is a transformation)!
void LASreadOpener::adjust_offset_when_transformation(LASreader* lasreader, BOOL set_header_direct /*=TRUE*/) {
  if (transform && offset_adjust && lasreader && offset == 0) {
    transform->adjust_offset(lasreader, scale_factor);
  }
}

void LASreadOpener::set_files_are_flightlines(const I32 files_are_flightlines) {
  this->files_are_flightlines = files_are_flightlines;
  if (files_are_flightlines > (I32)(U16_MAX)) {
    LASMessage(LAS_WARNING, "files_are_flightlines start value %d is too large", files_are_flightlines);
  } else if ((files_are_flightlines + files_are_flightlines_index) > (I32)(U16_MAX)) {
    LASMessage(LAS_WARNING, "files_are_flightlines start value %d plus index %d is too large", files_are_flightlines, files_are_flightlines_index);
  }
}

void LASreadOpener::set_files_are_flightlines_index(const I32 files_are_flightlines_index) {
  this->files_are_flightlines_index = files_are_flightlines_index - 1;
  if (files_are_flightlines_index > (I32)(U16_MAX)) {
    LASMessage(LAS_WARNING, "files_are_flightlines_index index value %d is too large", files_are_flightlines_index);
  } else if ((files_are_flightlines + files_are_flightlines_index) > (I32)(U16_MAX)) {
    LASMessage(LAS_WARNING, "files_are_flightlines start value %d plus index %d is too large", files_are_flightlines, files_are_flightlines_index);
  }
}

void LASreadOpener::set_apply_file_source_ID(const BOOL apply_file_source_ID) {
  this->apply_file_source_ID = apply_file_source_ID;
}

void LASreadOpener::set_io_ibuffer_size(const U32 buffer_size) {
  this->io_ibuffer_size = buffer_size;
}

void LASreadOpener::set_file_name(const CHAR* file_name, BOOL unique) {
  add_file_name(file_name, unique);
}

#ifdef _WIN32
#include <windows.h>
BOOL LASreadOpener::add_file_name(const CHAR* file_name, BOOL unique) {
  BOOL r = FALSE;
  HANDLE h;
  WIN32_FIND_DATA info;
  h = FindFirstFile(file_name, &info);
  if (h != INVALID_HANDLE_VALUE) {
    // find the path
    I32 len = (I32)strlen(file_name);
    while (len && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;
    if (len) {
      len++;
      CHAR full_file_name[512];
      strncpy_las(full_file_name, sizeof(full_file_name), file_name, len);
      size_t remaining_size = (sizeof(full_file_name) > len) ? (sizeof(full_file_name) - len) : 0;
      do {
        snprintf(&full_file_name[len], remaining_size, "%s", info.cFileName);
        if (add_file_name_single(full_file_name, unique)) r = TRUE;
      } while (FindNextFile(h, &info));
    } else {
      do {
        if (add_file_name_single(info.cFileName, unique)) r = TRUE;
      } while (FindNextFile(h, &info));
    }
    FindClose(h);
  }
  return r;
}
#endif

#ifdef _WIN32
BOOL LASreadOpener::add_file_name_single(const CHAR* file_name, BOOL unique)
#else
BOOL LASreadOpener::add_file_name(const CHAR* file_name, BOOL unique)
#endif
{
  if (unique) {
    U32 i;
    for (i = 0; i < file_name_number; i++) {
      if (strcmp(file_names[i], file_name) == 0) {
        return FALSE;
      }
    }
  }
  if (file_name_number == file_name_allocated) {
    if (file_names) {
      file_name_allocated *= 2;
      file_names = (CHAR**)realloc_las(file_names, sizeof(CHAR*) * file_name_allocated);
    } else {
      file_name_allocated = 16;
      file_names = (CHAR**)malloc(sizeof(CHAR*) * file_name_allocated);
    }
    if (file_names == 0) {
      laserror("alloc for file_names pointer array failed at %d", file_name_allocated);
    }
  }
  if (file_names != nullptr) file_names[file_name_number] = LASCopyString(file_name);
  file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::add_file_name(const CHAR* file_name, U32 ID, BOOL unique) {
  if (unique) {
    U32 i;
    for (i = 0; i < file_name_number; i++) {
      if (strcmp(file_names[i], file_name) == 0) {
        return FALSE;
      }
    }
  }
  if (file_name_number == file_name_allocated) {
    if (file_names) {
      file_name_allocated *= 2;
      file_names = (CHAR**)realloc_las(file_names, sizeof(CHAR*) * file_name_allocated);
      file_names_ID = (U32*)realloc_las(file_names_ID, sizeof(U32) * file_name_allocated);
    } else {
      file_name_allocated = 16;
      file_names = (CHAR**)malloc(sizeof(CHAR*) * file_name_allocated);
      file_names_ID = (U32*)malloc(sizeof(U32) * file_name_allocated);
    }
    if (file_names == 0) {
      laserror("alloc for file_names pointer array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_ID == 0) {
      laserror("alloc for file_names_ID array failed at %d", file_name_allocated);
      return FALSE;
    }
  }
  file_names[file_name_number] = LASCopyString(file_name);
  file_names_ID[file_name_number] = ID;
  file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::add_file_name(const CHAR* file_name, U32 ID, I64 npoints, F64 min_x, F64 min_y, F64 max_x, F64 max_y, BOOL unique) {
  if (unique) {
    U32 i;
    for (i = 0; i < file_name_number; i++) {
      if (strcmp(file_names[i], file_name) == 0) {
        return FALSE;
      }
    }
  }
  if (file_name_number == file_name_allocated) {
    if (file_names) {
      file_name_allocated *= 2;
      file_names = (CHAR**)realloc_las(file_names, sizeof(CHAR*) * file_name_allocated);
      file_names_ID = (U32*)realloc_las(file_names_ID, sizeof(U32) * file_name_allocated);
      file_names_npoints = (I64*)realloc_las(file_names_npoints, sizeof(I64) * file_name_allocated);
      file_names_min_x = (F64*)realloc_las(file_names_min_x, sizeof(F64) * file_name_allocated);
      file_names_min_y = (F64*)realloc_las(file_names_min_y, sizeof(F64) * file_name_allocated);
      file_names_max_x = (F64*)realloc_las(file_names_max_x, sizeof(F64) * file_name_allocated);
      file_names_max_y = (F64*)realloc_las(file_names_max_y, sizeof(F64) * file_name_allocated);
    } else {
      file_name_allocated = 16;
      file_names = (CHAR**)malloc(sizeof(CHAR*) * file_name_allocated);
      file_names_ID = (U32*)malloc(sizeof(U32) * file_name_allocated);
      file_names_npoints = (I64*)malloc(sizeof(I64) * file_name_allocated);
      file_names_min_x = (F64*)malloc(sizeof(F64) * file_name_allocated);
      file_names_min_y = (F64*)malloc(sizeof(F64) * file_name_allocated);
      file_names_max_x = (F64*)malloc(sizeof(F64) * file_name_allocated);
      file_names_max_y = (F64*)malloc(sizeof(F64) * file_name_allocated);
      if (kdtree_rectangles == 0) {
        kdtree_rectangles = new LASkdtreeRectangles();
        if (kdtree_rectangles == 0) {
          laserror("alloc for LASkdtreeRectangles failed");
          return FALSE;
        }
      }
      kdtree_rectangles->init();
    }
    if (file_names == 0) {
      laserror("alloc for file_names pointer array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_ID == 0) {
      laserror("alloc for file_names_ID array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_npoints == 0) {
      laserror("alloc for file_names_npoints array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_min_x == 0) {
      laserror("alloc for file_names_min_x array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_min_y == 0) {
      laserror("alloc for file_names_min_y array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_max_x == 0) {
      laserror("alloc for file_names_max_x array failed at %d", file_name_allocated);
      return FALSE;
    }
    if (file_names_max_y == 0) {
      laserror("alloc for file_names_max_y array failed at %d", file_name_allocated);
      return FALSE;
    }
  }
  file_names[file_name_number] = LASCopyString(file_name);
  file_names_ID[file_name_number] = ID;
  file_names_npoints[file_name_number] = npoints;
  file_names_min_x[file_name_number] = min_x;
  file_names_min_y[file_name_number] = min_y;
  file_names_max_x[file_name_number] = max_x;
  file_names_max_y[file_name_number] = max_y;
  kdtree_rectangles->add(min_x, min_y, max_x, max_y);
  file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::add_list_of_files(const CHAR* list_of_files, BOOL unique) {
  FILE* file = LASfopen(list_of_files, "r");
  if (file == 0) {
    laserror("cannot open '%s'", list_of_files);
    return FALSE;
  }
  CHAR line[2048];
  CHAR name[2048];
  U32 ID;
  I64 npoints;
  F64 min_x;
  F64 min_y;
  F64 max_x;
  F64 max_y;
  int num;
  while (fgets(line, 2048, file)) {
    // find end of line
    I32 len = (I32)strlen(line) - 1;
    // remove extra white spaces and line return at the end
    while ((len > 0) && ((line[len] == '\n') || (line[len] == ' ') || (line[len] == '\t') || (line[len] == '\012'))) {
      line[len] = '\0';
      len--;
    }
    // try to parse number of points and xy bounds
    num = sscanf_las(line, "%u,%lld,%lf,%lf,%lf,%lf,", &ID, &npoints, &min_x, &min_y, &max_x, &max_y);

    if (num == 6) {
      // skip ID, number of points, and xy bounds
      num = 0;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      // remove extra white spaces at the beginning
      while ((num < len) && ((line[num] == ' ') || (line[num] == '\t'))) num++;
      add_file_name(&line[num], ID, npoints, min_x, min_y, max_x, max_y, unique);
    } else {
      // try to parse number and file name
      num = sscanf_las(line, "%u,%s", &ID, name);
      if (num == 2) {
        // skip ID
        num = 0;
        while ((num < len) && (line[num] != ',')) num++;
        num++;
        // remove extra white spaces at the beginning
        while ((num < len) && ((line[num] == ' ') || (line[num] == '\t'))) num++;
        add_file_name(&line[num], ID, unique);
      } else {
        add_file_name(line, unique);
      }
    }
  }
  fclose(file);
  return TRUE;
}

BOOL LASreadOpener::add_neighbor_file_name(const CHAR* neighbor_file_name, I64 npoints, F64 min_x, F64 min_y, F64 max_x, F64 max_y, BOOL unique) {
  if (unique) {
    U32 i;
    for (i = 0; i < neighbor_file_name_number; i++) {
      if (strcmp(neighbor_file_names[i], neighbor_file_name) == 0) {
        return FALSE;
      }
    }
  }
  if (neighbor_file_name_number == neighbor_file_name_allocated) {
    if (neighbor_file_names) {
      neighbor_file_name_allocated *= 2;
      neighbor_file_names = (CHAR**)realloc_las(neighbor_file_names, sizeof(CHAR*) * neighbor_file_name_allocated);
      neighbor_file_names_npoints = (I64*)realloc_las(neighbor_file_names_npoints, sizeof(I64) * neighbor_file_name_allocated);
      neighbor_file_names_min_x = (F64*)realloc_las(neighbor_file_names_min_x, sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_min_y = (F64*)realloc_las(neighbor_file_names_min_y, sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_max_x = (F64*)realloc_las(neighbor_file_names_max_x, sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_max_y = (F64*)realloc_las(neighbor_file_names_max_y, sizeof(F64) * neighbor_file_name_allocated);
    } else {
      neighbor_file_name_allocated = 16;
      neighbor_file_names = (CHAR**)malloc(sizeof(CHAR*) * neighbor_file_name_allocated);
      neighbor_file_names_npoints = (I64*)malloc(sizeof(I64) * neighbor_file_name_allocated);
      neighbor_file_names_min_x = (F64*)malloc(sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_min_y = (F64*)malloc(sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_max_x = (F64*)malloc(sizeof(F64) * neighbor_file_name_allocated);
      neighbor_file_names_max_y = (F64*)malloc(sizeof(F64) * neighbor_file_name_allocated);
      if (neighbor_kdtree_rectangles == 0) {
        neighbor_kdtree_rectangles = new LASkdtreeRectangles();
        if (neighbor_kdtree_rectangles == 0) {
          laserror("alloc for neighbor LASkdtreeRectangles failed");
          return FALSE;
        }
      }
      kdtree_rectangles->init();
    }
    if (neighbor_file_names == 0) {
      laserror("alloc for neighbor_file_names pointer array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
    if (neighbor_file_names_npoints == 0) {
      laserror("alloc for neighbor_file_names_npoints pointer array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
    if (neighbor_file_names_min_x == 0) {
      laserror("alloc for neighbor_file_names_min_x array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
    if (neighbor_file_names_min_y == 0) {
      laserror("alloc for neighbor_file_names_min_y array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
    if (neighbor_file_names_max_x == 0) {
      laserror("alloc for neighbor_file_names_max_x array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
    if (neighbor_file_names_max_y == 0) {
      laserror("alloc for neighbor_file_names_max_y array failed at %d", neighbor_file_name_allocated);
      return FALSE;
    }
  }
  neighbor_file_names[neighbor_file_name_number] = LASCopyString(neighbor_file_name);
  neighbor_file_names_npoints[neighbor_file_name_number] = npoints;
  neighbor_file_names_min_x[neighbor_file_name_number] = min_x;
  neighbor_file_names_min_y[neighbor_file_name_number] = min_y;
  neighbor_file_names_max_x[neighbor_file_name_number] = max_x;
  neighbor_file_names_max_y[neighbor_file_name_number] = max_y;
  neighbor_kdtree_rectangles->add(min_x, min_y, max_x, max_y);
  neighbor_file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::add_neighbor_list_of_files(const CHAR* neighbor_list_of_files, BOOL unique) {
  FILE* file = LASfopen(neighbor_list_of_files, "r");
  if (file == 0) {
    laserror("cannot open '%s'", neighbor_list_of_files);
    return FALSE;
  }
  CHAR line[2048];
  U32 ID;
  I64 npoints;
  F64 min_x;
  F64 min_y;
  F64 max_x;
  F64 max_y;
  int num;
  while (fgets(line, 2048, file)) {
    // find end of line
    I32 len = (I32)strlen(line) - 1;
    // remove extra white spaces and line return at the end
    while ((len > 0) && ((line[len] == '\n') || (line[len] == ' ') || (line[len] == '\t') || (line[len] == '\012'))) {
      line[len] = '\0';
      len--;
    }
    // try to parse number of points and xy bounds
    num = sscanf_las(line, "%u,%lld,%lf,%lf,%lf,%lf,", &ID, &npoints, &min_x, &min_y, &max_x, &max_y);
    if (num == 6) {
      // skip number of points and xy bounds
      num = 0;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      while ((num < len) && (line[num] != ',')) num++;
      num++;
      // remove extra white spaces at the beginning
      while ((num < len) && ((line[num] == ' ') || (line[num] == '\t'))) num++;
      add_neighbor_file_name(&line[num], npoints, min_x, min_y, max_x, max_y, unique);
    } else {
#ifdef _WIN32
      add_neighbor_file_name_single(line);
#else
      add_neighbor_file_name(line);
#endif
    }
  }
  fclose(file);
  return TRUE;
}

void LASreadOpener::delete_file_name(U32 file_name_id) {
  if (file_name_id < file_name_number) {
    U32 i;
    if (file_names[file_name_id] != NULL) {
      free(file_names[file_name_id]);
      file_names[file_name_id] = NULL;
    }
    for (i = file_name_id + 1; i < file_name_number; i++) {
      file_names[i - 1] = file_names[i];
    }
  }
  file_names[file_name_number - 1] = NULL;
  file_name_number--;
}

BOOL LASreadOpener::set_file_name_current(U32 file_name_id) {
  if (file_name_id < file_name_number) {
    file_name_current = file_name_id;
    file_name = file_names[file_name_current];
    return TRUE;
  }
  return FALSE;
}

#ifdef _WIN32
#include <windows.h>
BOOL LASreadOpener::add_neighbor_file_name(const CHAR* neighbor_file_name, BOOL unique) {
  BOOL r = FALSE;
  HANDLE h;
  WIN32_FIND_DATA info;
  h = FindFirstFile(neighbor_file_name, &info);
  if (h != INVALID_HANDLE_VALUE) {
    // find the path
    I32 len = (I32)strlen(neighbor_file_name);
    while (len && (neighbor_file_name[len] != '\\') && (neighbor_file_name[len] != '/') && (neighbor_file_name[len] != ':')) len--;
    if (len) {
      len++;
      CHAR full_neighbor_file_name[512];
      strncpy_las(full_neighbor_file_name, sizeof(full_neighbor_file_name), neighbor_file_name, len);
      size_t remaining_size = (sizeof(full_neighbor_file_name) > len) ? (sizeof(full_neighbor_file_name) - len) : 0;
      do {
        snprintf(&full_neighbor_file_name[len], remaining_size, "%s", info.cFileName);
        if (add_neighbor_file_name_single(full_neighbor_file_name, unique)) r = TRUE;
      } while (FindNextFile(h, &info));
    } else {
      do {
        if (add_neighbor_file_name_single(info.cFileName, unique)) r = TRUE;
      } while (FindNextFile(h, &info));
    }
    FindClose(h);
  }
  return r;
}
#endif

#ifdef _WIN32
BOOL LASreadOpener::add_neighbor_file_name_single(const CHAR* neighbor_file_name, BOOL unique)
#else
BOOL LASreadOpener::add_neighbor_file_name(const CHAR* neighbor_file_name, BOOL unique)
#endif
{
  if (unique) {
    U32 i;
    for (i = 0; i < neighbor_file_name_number; i++) {
      if (strcmp(neighbor_file_names[i], neighbor_file_name) == 0) {
        return FALSE;
      }
    }
  }
  if (neighbor_file_name_number == neighbor_file_name_allocated) {
    if (neighbor_file_names) {
      neighbor_file_name_allocated *= 2;
      neighbor_file_names = (CHAR**)realloc_las(neighbor_file_names, sizeof(CHAR*) * neighbor_file_name_allocated);
    } else {
      neighbor_file_name_allocated = 16;
      neighbor_file_names = (CHAR**)malloc(sizeof(CHAR*) * neighbor_file_name_allocated);
    }
    if (neighbor_file_names == 0) {
      laserror("alloc for neighbor_file_names pointer array failed at %d", neighbor_file_name_allocated);
    }
  }
  if (neighbor_file_names != nullptr) neighbor_file_names[neighbor_file_name_number] = LASCopyString(neighbor_file_name);
  neighbor_file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::set_point_type(U8 point_type) {
  if (point_type > 10) {
    return FALSE;
  }
  this->point_type = point_type;
  return TRUE;
}

void LASreadOpener::set_parse_string(const CHAR* parse_string) {
  if (this->parse_string) free(this->parse_string);
  if (parse_string) {
    this->parse_string = LASCopyString(parse_string);
  } else {
    this->parse_string = 0;
  }
}

const CHAR* LASreadOpener::get_parse_string() const {
  return parse_string;
}

void LASreadOpener::set_scale_factor(const F64* scale_factor) {
  if (scale_factor) {
    if (this->scale_factor == 0) this->scale_factor = new F64[3];
    this->scale_factor[0] = scale_factor[0];
    this->scale_factor[1] = scale_factor[1];
    this->scale_factor[2] = scale_factor[2];
  } else if (this->scale_factor) {
    delete[] this->scale_factor;
    this->scale_factor = 0;
  }
}

void LASreadOpener::set_offset(const F64* offset) {
  if (offset) {
    if (this->offset == 0) this->offset = new F64[3];
    this->offset[0] = offset[0];
    this->offset[1] = offset[1];
    this->offset[2] = offset[2];
  } else if (this->offset) {
    delete[] this->offset;
    this->offset = 0;
  }
}

void LASreadOpener::set_translate_intensity(const F32 translation) {
  this->translate_intensity = translation;
}

void LASreadOpener::set_scale_intensity(const F32 scale) {
  this->scale_intensity = scale;
}

void LASreadOpener::set_translate_scan_angle(const F32 translation) {
  this->translate_scan_angle = translation;
}

void LASreadOpener::set_scale_scan_angle(const F32 scale) {
  this->scale_scan_angle = scale;
}

void LASreadOpener::add_attribute(
    I32 data_type, const CHAR* name, const CHAR* description, F64 scale, F64 offset, F64 pre_scale, F64 pre_offset, F64 no_data) {
  if ((data_type < 1) || (data_type > 10)) {
    LASMessage(LAS_WARNING, "attribute data type %d not supported. ignoring attribute '%s'.", data_type, name);
    return;
  }
  attribute_data_types[number_attributes] = data_type;
  attribute_names[number_attributes] = (name ? LASCopyString(name) : 0);
  attribute_descriptions[number_attributes] = (description ? LASCopyString(description) : 0);
  attribute_scales[number_attributes] = scale;
  attribute_offsets[number_attributes] = offset;
  attribute_pre_scales[number_attributes] = pre_scale;
  attribute_pre_offsets[number_attributes] = pre_offset;
  attribute_no_datas[number_attributes] = no_data;
  number_attributes++;
}

void LASreadOpener::set_skip_lines(const U32 number_of_lines) {
  this->skip_lines = number_of_lines;
}

void LASreadOpener::set_populate_header(BOOL populate_header) {
  this->populate_header = populate_header;
}

void LASreadOpener::set_keep_lastiling(BOOL keep_lastiling) {
  this->keep_lastiling = keep_lastiling;
}

void LASreadOpener::set_keep_copc(BOOL keep_copc) {
  this->keep_copc = keep_copc;
}

void LASreadOpener::set_pipe_on(BOOL pipe_on) {
  this->pipe_on = pipe_on;
}

void LASreadOpener::set_decompress_selective(U32 decompress_selective) {
  this->decompress_selective = decompress_selective;
  if (filter) {
    this->decompress_selective |= filter->get_decompress_selective();
  }
  if (transform) {
    this->decompress_selective |= transform->get_decompress_selective();
  }
  if (ignore) {
    this->decompress_selective |= ignore->get_decompress_selective();
  }
}

void LASreadOpener::set_inside_tile(const F32 ll_x, const F32 ll_y, const F32 size) {
  if (inside_tile == 0) inside_tile = new F32[3];
  inside_tile[0] = ll_x;
  inside_tile[1] = ll_y;
  inside_tile[2] = size;
}

void LASreadOpener::set_inside_circle(const F64 center_x, const F64 center_y, const F64 radius) {
  if (inside_circle == 0) inside_circle = new F64[3];
  inside_circle[0] = center_x;
  inside_circle[1] = center_y;
  inside_circle[2] = radius;
}

void LASreadOpener::set_inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y) {
  if (inside_rectangle == 0) inside_rectangle = new F64[4];
  inside_rectangle[0] = min_x;
  inside_rectangle[1] = min_y;
  inside_rectangle[2] = max_x;
  inside_rectangle[3] = max_y;
}

void LASreadOpener::set_max_depth(const I32 max_depth) {
  inside_depth_opener = 1;
  copc_depth = max_depth;
}

void LASreadOpener::set_resolution(const F32 resolution) {
  inside_depth_opener = 2;
  copc_resolution = resolution;
}

BOOL LASreadOpener::active() const {
  return ((file_name_current < file_name_number) || use_stdin);
}

LASreadOpener::LASreadOpener() {
  io_ibuffer_size = LAS_TOOLS_IO_IBUFFER_SIZE;
  file_name = 0;
  file_names = 0;
  file_names_ID = 0;
  file_names_npoints = 0;
  file_names_min_x = 0;
  file_names_min_y = 0;
  file_names_max_x = 0;
  file_names_max_y = 0;
  kdtree_rectangles = 0;
  neighbor_file_names = 0;
  neighbor_file_names_npoints = 0;
  neighbor_file_names_min_x = 0;
  neighbor_file_names_min_y = 0;
  neighbor_file_names_max_x = 0;
  neighbor_file_names_max_y = 0;
  neighbor_kdtree_rectangles = 0;
  merged = FALSE;
  stored = FALSE;
  use_stdin = FALSE;
  comma_not_point = FALSE;
  scale_factor = 0;
  offset = 0;
  buffer_size = 0.0f;
  auto_reoffset = FALSE;
  offset_adjust = FALSE;
  files_are_flightlines = 0;
  files_are_flightlines_index = -1;
  apply_file_source_ID = FALSE;
  itxt = FALSE;
  ipts = FALSE;
  iptx = FALSE;
  iptx_transform = FALSE;
  translate_intensity = 0.0f;
  scale_intensity = 1.0f;
  translate_scan_angle = 0.0f;
  scale_scan_angle = 1.0f;
  number_attributes = 0;
  for (I32 i = 0; i < 32; i++) {
    attribute_data_types[i] = 0;
    attribute_names[i] = 0;
    attribute_descriptions[i] = 0;
    attribute_scales[i] = 1.0;
    attribute_offsets[i] = 0.0;
    attribute_pre_scales[i] = 1.0;
    attribute_pre_offsets[i] = 0.0;
    attribute_no_datas[i] = F64_MAX;
  }
  point_type = 0;
  parse_string = 0;
  skip_lines = 0;
  populate_header = FALSE;
  keep_lastiling = FALSE;
  keep_copc = FALSE;
  pipe_on = FALSE;
  unique = FALSE;
  file_name_number = 0;
  file_name_allocated = 0;
  file_name_current = 0;
  neighbor_file_name_number = 0;
  neighbor_file_name_allocated = 0;
  decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_ALL;
  inside_tile = 0;
  inside_circle = 0;
  inside_rectangle = 0;
  filter = 0;
  transform = 0;
  ignore = 0;
#if defined(_WIN32)
  temp_file_base = "";
#else
  temp_file_base = ".";
#endif
  // COPC
  inside_depth_opener = 0;
  copc_stream_order = 1;
  copc_resolution = 0;
  copc_depth = -1;
}

LASreadOpener::~LASreadOpener() {
  if (file_names) {
    U32 i;
    for (i = 0; i < file_name_number; i++) free(file_names[i]);
    free(file_names);
    if (file_names_ID) {
      free(file_names_ID);
      if (file_names_npoints) {
        free(file_names_npoints);
        free(file_names_min_x);
        free(file_names_min_y);
        free(file_names_max_x);
        free(file_names_max_y);
      }
    }
  }
  if (kdtree_rectangles) delete kdtree_rectangles;
  if (neighbor_file_names) {
    U32 i;
    for (i = 0; i < neighbor_file_name_number; i++) free(neighbor_file_names[i]);
    free(neighbor_file_names);
    if (neighbor_file_names_npoints) {
      free(neighbor_file_names_npoints);
      free(neighbor_file_names_min_x);
      free(neighbor_file_names_min_y);
      free(neighbor_file_names_max_x);
      free(neighbor_file_names_max_y);
    }
  }
  if (parse_string) free(parse_string);
  if (scale_factor) delete[] scale_factor;
  if (offset) delete[] offset;
  if (inside_tile) delete[] inside_tile;
  if (inside_circle) delete[] inside_circle;
  if (inside_rectangle) delete[] inside_rectangle;
  if (filter) delete filter;
  if (transform) delete transform;
  if (ignore) delete ignore;
}
