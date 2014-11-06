/*
===============================================================================

  FILE:  lasdefinitions.hpp
  
  CONTENTS:
  
    Contains the Header and Point classes for reading and writing LiDAR points
    in the LAS format

      Version 1.4,   Nov 14, 2011.
      Version 1.3,   Oct 24, 2010.
      Version 1.2, April 29, 2008.
      Version 1.1, March 07, 2005.
      Version 1.0,   May 09, 2003

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2014, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    9 December 2013 -- bug fix and improved writing of new LAS 1.4 point types
    21 December 2011 -- (limited) support for LAS 1.4 and attributed extra bytes 
    10 January 2011 -- licensing change for LGPL release and liblas integration
    16 December 2010 -- updated to support generic LASitem point formats
    3 December 2010 -- updated to (somewhat) support LAS format 1.3
    7 September 2008 -- updated to support LAS format 1.2 
    11 June 2007 -- number of return / scan direction bitfield order was wrong
    18 February 2007 -- created after repairing 2 vacuum cleaners in the garden
  
===============================================================================
*/
#ifndef LAS_DEFINITIONS_HPP
#define LAS_DEFINITIONS_HPP

#define LAS_TOOLS_VERSION 141021

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mydefs.hpp"
#include "laszip.hpp"

#define LAS_TOOLS_FORMAT_DEFAULT 0
#define LAS_TOOLS_FORMAT_LAS     1
#define LAS_TOOLS_FORMAT_LAZ     2
#define LAS_TOOLS_FORMAT_BIN     3
#define LAS_TOOLS_FORMAT_QFIT    4
#define LAS_TOOLS_FORMAT_VRML    5
#define LAS_TOOLS_FORMAT_TXT     6
#define LAS_TOOLS_FORMAT_SHP     7
#define LAS_TOOLS_FORMAT_ASC     8
#define LAS_TOOLS_FORMAT_BIL     9
#define LAS_TOOLS_FORMAT_FLT    10
#define LAS_TOOLS_FORMAT_DTM    11

class LASwavepacket
{
public:
  LASwavepacket() {zero();};
  void zero() {memset(data, 0, 29);};
  inline U8 getIndex() const {return data[0];};
  inline U64 getOffset() const {return ((U64*)&(data[1]))[0];};
  inline U32 getSize() const {return ((U32*)&(data[9]))[0];};
  inline F32 getLocation() const {return ((F32*)&(data[13]))[0];};
  inline F32 getXt() const {return ((F32*)&(data[17]))[0];};
  inline F32 getYt() const {return ((F32*)&(data[21]))[0];};
  inline F32 getZt() const {return ((F32*)&(data[25]))[0];};
  inline void setIndex(U8 index) {data[0] = index;};
  inline void setOffset(U64 offset) {((U64*)&(data[1]))[0] = offset;};
  inline void setSize(U32 size) {((U32*)&(data[9]))[0] = size;};
  inline void setLocation(F32 location) { ((F32*)&(data[13]))[0] = location;};
  inline void setXt(F32 xt) {((F32*)&(data[17]))[0] = xt;};
  inline void setYt(F32 yt) {((F32*)&(data[21]))[0] = yt;};
  inline void setZt(F32 zt) {((F32*)&(data[25]))[0] = zt;};
  inline void flipDirection() {((F32*)&(data[17]))[0] *= -1; ((F32*)&(data[21]))[0] *= -1; ((F32*)&(data[25]))[0] *= -1;};
private:
  U8 data[29];
};

class LASquantizer
{
public:
  F64 x_scale_factor;
  F64 y_scale_factor;
  F64 z_scale_factor;
  F64 x_offset;
  F64 y_offset;
  F64 z_offset;

  inline F64 get_x(const I32 X) const { return x_scale_factor*X+x_offset; };
  inline F64 get_y(const I32 Y) const { return y_scale_factor*Y+y_offset; };
  inline F64 get_z(const I32 Z) const { return z_scale_factor*Z+z_offset; };

  inline I32 get_X(const F64 x) const { if (x >= x_offset) return (I32)((x-x_offset)/x_scale_factor+0.5); else return (I32)((x-x_offset)/x_scale_factor-0.5); };
  inline I32 get_Y(const F64 y) const { if (y >= y_offset) return (I32)((y-y_offset)/y_scale_factor+0.5); else return (I32)((y-y_offset)/y_scale_factor-0.5); };
  inline I32 get_Z(const F64 z) const { if (z >= z_offset) return (I32)((z-z_offset)/z_scale_factor+0.5); else return (I32)((z-z_offset)/z_scale_factor-0.5); };

  LASquantizer()
  {
    x_scale_factor = 0.01;
    y_scale_factor = 0.01;
    z_scale_factor = 0.01;
    x_offset = 0.0;
    y_offset = 0.0;
    z_offset = 0.0;
  };

  LASquantizer & operator=(const LASquantizer & quantizer)
  {
    this->x_scale_factor = quantizer.x_scale_factor;
    this->y_scale_factor = quantizer.y_scale_factor;
    this->z_scale_factor = quantizer.z_scale_factor;
    this->x_offset = quantizer.x_offset;
    this->y_offset = quantizer.y_offset;
    this->z_offset = quantizer.z_offset;
    return *this;
  };
};

#define LAS_ATTRIBUTE_U8  0
#define LAS_ATTRIBUTE_I8  1
#define LAS_ATTRIBUTE_U16 2
#define LAS_ATTRIBUTE_I16 3
#define LAS_ATTRIBUTE_U32 4
#define LAS_ATTRIBUTE_I32 5
#define LAS_ATTRIBUTE_U64 6
#define LAS_ATTRIBUTE_I64 7
#define LAS_ATTRIBUTE_F32 8
#define LAS_ATTRIBUTE_F64 9

class LASattribute
{
public:
  U8 reserved[2];           // 2 bytes
  U8 data_type;             // 1 byte
  U8 options;               // 1 byte
  CHAR name[32];            // 32 bytes
  U8 unused[4];             // 4 bytes
  U64I64F64 no_data[3];     // 24 = 3*8 bytes
  U64I64F64 min[3];         // 24 = 3*8 bytes
  U64I64F64 max[3];         // 24 = 3*8 bytes
  F64 scale[3];             // 24 = 3*8 bytes
  F64 offset[3];            // 24 = 3*8 bytes
  CHAR description[32];     // 32 bytes

  LASattribute(U8 size)
  {
    if (size == 0) throw;
    memset(this, 0, sizeof(LASattribute));
    scale[0] = scale[1] = scale[2] = 1.0;
    this->options = size;
  };

  LASattribute(U32 type, const CHAR* name, const CHAR* description=0, U32 dim=1)
  {
    if (type > LAS_ATTRIBUTE_F64) throw;
    if ((dim < 1) || (dim > 3)) throw;
    if (name == 0) throw;
    memset(this, 0, sizeof(LASattribute));
    scale[0] = scale[1] = scale[2] = 1.0;
    this->data_type = (dim-1)*10+type+1;
    strncpy(this->name, name, 32);
    if (description) strncpy(this->description, description, 32);
  };

  inline bool set_no_data(U8 no_data, I32 dim=0) { if ((0 == get_type()) && (dim < get_dim())) { this->no_data[dim].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(I8 no_data, I32 dim=0) { if ((1 == get_type()) && (dim < get_dim())) { this->no_data[dim].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(U16 no_data, I32 dim=0) { if ((2 == get_type()) && (dim < get_dim())) { this->no_data[dim].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(I16 no_data, I32 dim=0) { if ((3 == get_type()) && (dim < get_dim())) { this->no_data[dim].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(U32 no_data, I32 dim=0) { if ((4 == get_type()) && (dim < get_dim())) { this->no_data[dim].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(I32 no_data, I32 dim=0) { if ((5 == get_type()) && (dim < get_dim())) { this->no_data[dim].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(U64 no_data, I32 dim=0) { if ((6 == get_type()) && (dim < get_dim())) { this->no_data[dim].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(I64 no_data, I32 dim=0) { if ((7 == get_type()) && (dim < get_dim())) { this->no_data[dim].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(F32 no_data, I32 dim=0) { if ((8 == get_type()) && (dim < get_dim())) { this->no_data[dim].f64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline bool set_no_data(F64 no_data, I32 dim=0) { if ((9 == get_type()) && (dim < get_dim())) { this->no_data[dim].f64 = no_data; options |= 0x01; return TRUE; } return FALSE; };

  inline void set_min(U8* min, I32 dim=0) { this->min[dim] = cast(min); options |= 0x02; };
  inline void update_min(U8* min, I32 dim=0) { this->min[dim] = smallest(cast(min), this->min[dim]); };
  inline bool set_min(U8 min, I32 dim=0) { if ((0 == get_type()) && (dim < get_dim())) { this->min[dim].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(I8 min, I32 dim=0) { if ((1 == get_type()) && (dim < get_dim())) { this->min[dim].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(U16 min, I32 dim=0) { if ((2 == get_type()) && (dim < get_dim())) { this->min[dim].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(I16 min, I32 dim=0) { if ((3 == get_type()) && (dim < get_dim())) { this->min[dim].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(U32 min, I32 dim=0) { if ((4 == get_type()) && (dim < get_dim())) { this->min[dim].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(I32 min, I32 dim=0) { if ((5 == get_type()) && (dim < get_dim())) { this->min[dim].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(U64 min, I32 dim=0) { if ((6 == get_type()) && (dim < get_dim())) { this->min[dim].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(I64 min, I32 dim=0) { if ((7 == get_type()) && (dim < get_dim())) { this->min[dim].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(F32 min, I32 dim=0) { if ((8 == get_type()) && (dim < get_dim())) { this->min[dim].f64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline bool set_min(F64 min, I32 dim=0) { if ((9 == get_type()) && (dim < get_dim())) { this->min[dim].f64 = min; options |= 0x02; return TRUE; } return FALSE; };

  inline void set_max(U8* max, I32 dim=0) { this->max[dim] = cast(max); options |= 0x04; };
  inline void update_max(U8* max, I32 dim=0) { this->max[dim] = biggest(cast(max), this->max[dim]); };
  inline bool set_max(U8 max, I32 dim=0) { if ((0 == get_type()) && (dim < get_dim())) { this->max[dim].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(I8 max, I32 dim=0) { if ((1 == get_type()) && (dim < get_dim())) { this->max[dim].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(U16 max, I32 dim=0) { if ((2 == get_type()) && (dim < get_dim())) { this->max[dim].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(I16 max, I32 dim=0) { if ((3 == get_type()) && (dim < get_dim())) { this->max[dim].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(U32 max, I32 dim=0) { if ((4 == get_type()) && (dim < get_dim())) { this->max[dim].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(I32 max, I32 dim=0) { if ((5 == get_type()) && (dim < get_dim())) { this->max[dim].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(U64 max, I32 dim=0) { if ((6 == get_type()) && (dim < get_dim())) { this->max[dim].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(I64 max, I32 dim=0) { if ((7 == get_type()) && (dim < get_dim())) { this->max[dim].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(F32 max, I32 dim=0) { if ((8 == get_type()) && (dim < get_dim())) { this->max[dim].f64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline bool set_max(F64 max, I32 dim=0) { if ((9 == get_type()) && (dim < get_dim())) { this->max[dim].f64 = max; options |= 0x04; return TRUE; } return FALSE; };

  inline bool set_scale(F64 scale, I32 dim=0) { if (data_type) { this->scale[dim] = scale; options |= 0x08; return TRUE; } return FALSE; };
  inline bool set_offset(F64 offset, I32 dim=0) { if (data_type) { this->offset[dim] = offset; options |= 0x10; return TRUE; } return FALSE; };

  inline bool has_no_data() const { return options & 0x01; };
  inline bool has_min() const { return options & 0x02; };
  inline bool has_max() const { return options & 0x04; };
  inline bool has_scale() const { return options & 0x08; };
  inline bool has_offset() const { return options & 0x10; };

  inline U32 get_size() const
  {
    if (data_type)
    {
      const U32 size_table[10] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8 };
      U32 type = get_type();
      U32 dim = get_dim();
      return size_table[type]*dim;
    }
    else
    {
      return options;
    }
  };

  inline F64 get_value_as_float(U8* value) const
  {
    F64 casted_value;
    I32 type = get_type();
    if (type == 0)
      casted_value = (F64)*((U8*)value);
    else if (type == 1)
      casted_value = (F64)*((I8*)value);
    else if (type == 2)
      casted_value = (F64)*((U16*)value);
    else if (type == 3)
      casted_value = (F64)*((I16*)value);
    else if (type == 4)
      casted_value = (F64)*((U32*)value);
    else if (type == 5)
      casted_value = (F64)*((I32*)value);
    else if (type == 6)
      casted_value = (F64)(I64)*((U64*)value);
    else if (type == 7)
      casted_value = (F64)*((I64*)value);
    else if (type == 8)
      casted_value = (F64)*((F32*)value);
    else
      casted_value = *((F64*)value);
    return offset[0]+scale[0]*casted_value;
  };

private:
  inline I32 get_type() const
  {
    return ((I32)data_type - 1)%10;
  };
  inline I32 get_dim() const
  {
    return 1 + ((I32)data_type - 1)/10;
  };
  inline U64I64F64 cast(U8* value) const
  {
    I32 type = get_type();
    U64I64F64 casted_value;
    if (type == 0)
      casted_value.u64 = *((U8*)value);
    else if (type == 1)
      casted_value.i64 = *((I8*)value);
    else if (type == 2)
      casted_value.u64 = *((U16*)value);
    else if (type == 3)
      casted_value.i64 = *((I16*)value);
    else if (type == 4)
      casted_value.u64 = *((U32*)value);
    else if (type == 5)
      casted_value.i64 = *((I32*)value);
    else if (type == 6)
      casted_value.u64 = *((U64*)value);
    else if (type == 7)
      casted_value.i64 = *((I64*)value);
    else if (type == 8)
      casted_value.f64 = *((F32*)value);
    else
      casted_value.f64 = *((F64*)value);
    return casted_value;
  };
  inline U64I64F64 smallest(U64I64F64 a, U64I64F64 b) const
  {
    I32 type = get_type();
    if (type >= 8) // float compare
    {
      if (a.f64 < b.f64) return a;
      else               return b;
    }
    if (type & 1) // int compare
    {
      if (a.i64 < b.i64) return a;
      else               return b;
    }
    if (a.u64 < b.u64) return a;
    else               return b;
  };
  inline U64I64F64 biggest(U64I64F64 a, U64I64F64 b) const
  {
    I32 type = get_type();
    if (type >= 8) // float compare
    {
      if (a.f64 > b.f64) return a;
      else               return b;
    }
    if (type & 1) // int compare
    {
      if (a.i64 > b.i64) return a;
      else               return b;
    }
    if (a.u64 > b.u64) return a;
    else               return b;
  };
};

class LASattributer
{
public:
  I32 number_attributes;
  LASattribute* attributes;
  I32* attribute_starts;
  I32* attribute_sizes;

  LASattributer()
  {
    number_attributes = 0;
    attributes = 0;
    attribute_starts = 0;
    attribute_sizes = 0;
  };

  ~LASattributer()
  {
    clean_attributes();
  };

  void clean_attributes()
  {
    if (number_attributes)
    {
      number_attributes = 0;
      free(attributes); attributes = 0;
      free(attribute_starts); attribute_starts = 0;
      free(attribute_sizes); attribute_sizes = 0;
    }
  };

  bool init_attributes(U32 number_attributes, LASattribute* attributes)
  {
    U32 i;
    clean_attributes();
    this->number_attributes = number_attributes;
    this->attributes = (LASattribute*)malloc(sizeof(LASattribute)*number_attributes);
    memcpy(this->attributes, attributes, sizeof(LASattribute)*number_attributes);
    attribute_starts = (I32*)malloc(sizeof(I32)*number_attributes);
    attribute_sizes = (I32*)malloc(sizeof(I32)*number_attributes);
    attribute_starts[0] = 0;
    attribute_sizes[0] = attributes[0].get_size();
    for (i = 1; i < number_attributes; i++)
    {
      attribute_starts[i] = attribute_starts[i-1] + attribute_sizes[i-1];
      attribute_sizes[i] = attributes[i].get_size();
    }
    return TRUE;
  };

  I32 add_attribute(const LASattribute attribute)
  {
    if (attribute.get_size())
    {
      if (attributes)
      {
        number_attributes++;
        attributes = (LASattribute*)realloc(attributes, sizeof(LASattribute)*number_attributes);
        attribute_starts = (I32*)realloc(attribute_starts, sizeof(I32)*number_attributes);
        attribute_sizes = (I32*)realloc(attribute_sizes, sizeof(I32)*number_attributes);
        attributes[number_attributes-1] = attribute;
        attribute_starts[number_attributes-1] = attribute_starts[number_attributes-2] + attribute_sizes[number_attributes-2];
        attribute_sizes[number_attributes-1] = attributes[number_attributes-1].get_size();
      }
      else
      {
        number_attributes = 1;
        attributes = (LASattribute*)malloc(sizeof(LASattribute));
        attribute_starts = (I32*)malloc(sizeof(I32));
        attribute_sizes = (I32*)malloc(sizeof(I32));
        attributes[0] = attribute;
        attribute_starts[0] = 0;
        attribute_sizes[0] = attributes[0].get_size();
      }
      return number_attributes-1;
    }
    return -1;
  };

  inline I16 get_attributes_size() const
  {
    return (attributes ? attribute_starts[number_attributes-1] + attribute_sizes[number_attributes-1] : 0);
  }

  I32 get_attribute_index(const CHAR* name) const
  {
    I32 i;
    for (i = 0; i < number_attributes; i++)
    {
      if (strcmp(attributes[i].name, name) == 0)
      {
        return i;
      }
    }
    return -1;
  }

  I32 get_attribute_start(const CHAR* name) const
  {
    I32 i;
    for (i = 0; i < number_attributes; i++)
    {
      if (strcmp(attributes[i].name, name) == 0)
      {
        return attribute_starts[i];
      }
    }
    return -1;
  }

  I32 get_attribute_start(I32 index) const
  {
    if (index < number_attributes)
    {
      return attribute_starts[index];
    }
    return -1;
  }

  I32 get_attribute_size(I32 index) const
  {
    if (index < number_attributes)
    {
      return attribute_sizes[index];
    }
    return -1;
  }

  bool remove_attribute(I32 index)
  {
    if (index < 0 || index >= number_attributes)
    {
      return FALSE;
    }
    for (index = index + 1; index < number_attributes; index++)
    {
      attributes[index-1] = attributes[index];
      if (index > 1)
      {
        attribute_starts[index-1] = attribute_starts[index-2] + attribute_sizes[index-2];
      }
      else
      {
        attribute_starts[index-1] = 0;
      }
      attribute_sizes[index-1] = attribute_sizes[index];
    }
    number_attributes--;
    if (number_attributes)
    {
      attributes = (LASattribute*)realloc(attributes, sizeof(LASattribute)*number_attributes);
      attribute_starts = (I32*)realloc(attribute_starts, sizeof(I32)*number_attributes);
      attribute_sizes = (I32*)realloc(attribute_sizes, sizeof(I32)*number_attributes);
    }
    else
    {
      free(attributes); attributes = 0;
      free(attribute_starts); attribute_starts = 0;
      free(attribute_sizes); attribute_sizes = 0;
    }
    return TRUE;
  }

  bool remove_attribute(const CHAR* name)
  {
    I32 index = get_attribute_index(name);
    if (index != -1)
    { 
      return remove_attribute(index);
    }
    return FALSE;
  }
};

class LASpoint
{
public:

// these fields contain the data that describe each point

  I32 X;
  I32 Y;
  I32 Z;
  U16 intensity;
  U8 return_number : 3;
  U8 number_of_returns : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification : 5;
  U8 synthetic_flag : 1;
  U8 keypoint_flag  : 1;
  U8 withheld_flag  : 1;
  I8 scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;

  // LAS 1.4 only
  I16 extended_scan_angle;
  U8 extended_point_type : 2;
  U8 extended_scanner_channel : 2;
  U8 extended_classification_flags : 4;
  U8 extended_classification;
  U8 extended_return_number : 4;
  U8 extended_number_of_returns : 4;

  // for 8 byte alignment of the GPS time
  U8 dummy[3];

  // LASlib only
  U32 deleted_flag;

  F64 gps_time;
  U16 rgb[4];
  LASwavepacket wavepacket;
  U8* extra_bytes;

// for converting between x,y,z integers and scaled/translated coordinates

  const LASquantizer* quantizer;
  F64 coordinates[3];

// for attributed access to the extra bytes

  const LASattributer* attributer;

// this field provides generic access to the point data

  U8** point;

// these fields describe the point format LAS specific

  bool have_gps_time;
  bool have_rgb;
  bool have_nir;
  bool have_wavepacket;
  I32 extra_bytes_number;
  U32 total_point_size;

// these fields describe the point format terms of generic items

  U16 num_items;
  LASitem* items;

// copy functions

  LASpoint(const LASpoint & other)
  {
    *this = other;
  }

  LASpoint & operator=(const LASpoint & other)
  {
    X = other.X;
    Y = other.Y;
    Z = other.Z;
    intensity = other.intensity;
    return_number = other.return_number;
    number_of_returns = other.number_of_returns;
    scan_direction_flag = other.scan_direction_flag;
    edge_of_flight_line = other.edge_of_flight_line;
    classification = other.classification;
    synthetic_flag = other.synthetic_flag;
    keypoint_flag = other.keypoint_flag;
    withheld_flag = other.withheld_flag;
    scan_angle_rank = other.scan_angle_rank;
    user_data = other.user_data;
    point_source_ID = other.point_source_ID;
    deleted_flag = other.deleted_flag;

    if (other.have_gps_time)
    {
      gps_time = other.gps_time;
    }
    if (other.have_rgb)
    {
      rgb[0] = other.rgb[0];
      rgb[1] = other.rgb[1];
      rgb[2] = other.rgb[2];
      if (other.have_nir)
      {
        rgb[3] = other.rgb[3];
      }
    }
    if (other.have_wavepacket)
    {
      wavepacket = other.wavepacket;
    }
    if (other.extra_bytes && extra_bytes)
    {
      memcpy(extra_bytes, other.extra_bytes, extra_bytes_number);
    }
    if (other.extended_point_type)
    {
      extended_classification = other.extended_classification;
      extended_classification_flags = other.extended_classification_flags;
      extended_number_of_returns = other.extended_number_of_returns;
      extended_return_number = other.extended_return_number;
      extended_scan_angle = other.extended_scan_angle;
      extended_scanner_channel = other.extended_scanner_channel;
    }
    else if (extended_point_type)
    {
      extended_classification = other.classification & 31;
      extended_classification_flags = other.classification >> 5;
      extended_number_of_returns = other.number_of_returns;
      extended_return_number = other.return_number;
      extended_scan_angle = I16_QUANTIZE(((F32)other.scan_angle_rank)/0.006);
      extended_scanner_channel = 0;
    }

    return *this;
  };

  void copy_to(U8* buffer) const
  {
    U32 i;
    U32 b = 0;
    for (i = 0; i < num_items; i++)
    {
      memcpy(&buffer[b], point[i], items[i].size);
      b += items[i].size;
    }
  };

  void copy_from(const U8* buffer)
  {
    U32 i;
    U32 b = 0;
    for (i = 0; i < num_items; i++)
    {
      memcpy(point[i], &buffer[b], items[i].size);
      b += items[i].size;
    }
  };

// these functions set the desired point format (and maybe add on attributes in extra bytes)

  bool init(const LASquantizer* quantizer, const U8 point_type, const U16 point_size, const LASattributer* attributer=0)
  {
    // clean the point

    clean();

    // switch over the point types we know

    if (!LASzip().setup(&num_items, &items, point_type, point_size, LASZIP_COMPRESSOR_NONE))
    {
      fprintf(stderr,"ERROR: unknown point type %d with point size %d\n", (I32)point_type, (I32)point_size);
      return FALSE;
    }

    // create point's item pointers

    point = new U8*[num_items];

    U16 i;
    for (i = 0; i < num_items; i++)
    {
      total_point_size += items[i].size;
      switch (items[i].type)
      {
      case LASitem::POINT14:
        have_gps_time = TRUE;
        extended_point_type = 1;
      case LASitem::POINT10:
        this->point[i] = (U8*)&(this->X);
        break;
      case LASitem::GPSTIME11:
        have_gps_time = TRUE;
        this->point[i] = (U8*)&(this->gps_time);
        break;
      case LASitem::RGBNIR14:
        have_nir = TRUE;
      case LASitem::RGB12:
        have_rgb = TRUE;
        this->point[i] = (U8*)(this->rgb);
        break;
      case LASitem::WAVEPACKET13:
        have_wavepacket = TRUE;
        this->point[i] = (U8*)&(this->wavepacket);
        break;
      case LASitem::BYTE:
        extra_bytes_number = items[i].size;
        extra_bytes = new U8[extra_bytes_number];
        this->point[i] = extra_bytes;
        break;
      default:
        return FALSE;
      }
    }
    this->quantizer = quantizer;
    this->attributer = attributer;
    return TRUE;
  };

  bool init(const LASquantizer* quantizer, const U32 num_items, const LASitem* items, const LASattributer* attributer=0)
  {
    U32 i,e;

    // clean the point

    clean();

    // create item description

    this->num_items = num_items;
    if (this->items) delete [] this->items;
    this->items = new LASitem[num_items];
    if (this->point) delete [] this->point;
    this->point = new U8*[num_items];

    for (i = 0, e = 0; i < num_items; i++)
    {
      this->items[i] = items[i];
      total_point_size += items[i].size;
      switch (items[i].type)
      {
      case LASitem::POINT14:
        have_gps_time = TRUE;
        extended_point_type = 1;
      case LASitem::POINT10:
        this->point[i] = (U8*)&(this->X);
        break;
      case LASitem::GPSTIME11:
        have_gps_time = TRUE;
        this->point[i] = (U8*)&(this->gps_time);
        break;
      case LASitem::RGBNIR14:
        have_nir = TRUE;
      case LASitem::RGB12:
        have_rgb = TRUE;
        this->point[i] = (U8*)(this->rgb);
        break;
      case LASitem::WAVEPACKET13:
        have_wavepacket = TRUE;
        this->point[i] = (U8*)&(this->wavepacket);
        break;
      case LASitem::BYTE:
        extra_bytes_number = items[i].size;
        extra_bytes = new U8[extra_bytes_number];
        this->point[i] = extra_bytes;
        break;
      default:
        return FALSE;
      }
    }
    this->quantizer = quantizer;
    this->attributer = attributer;
    return TRUE;
  };

  bool inside_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y) const
  {
    F64 xy;
    xy = get_x();
    if (xy < r_min_x || xy >= r_max_x) return FALSE;
    xy = get_y();
    if (xy < r_min_y || xy >= r_max_y) return FALSE;
    return TRUE;
  }

  bool inside_tile(const F32 ll_x, const F32 ll_y, const F32 ur_x, const F32 ur_y) const
  {
    F64 xy;
    xy = get_x();
    if (xy < ll_x || xy >= ur_x) return FALSE;
    xy = get_y();
    if (xy < ll_y || xy >= ur_y) return FALSE;
    return TRUE;
  }

  bool inside_circle(const F64 center_x, const F64 center_y, F64 squared_radius) const
  {
    F64 dx = center_x - get_x();
    F64 dy = center_y - get_y();
    return ((dx*dx+dy*dy) < squared_radius);
  }

  bool inside_box(const F64 min_x, const F64 min_y, const F64 min_z, const F64 max_x, const F64 max_y, const F64 max_z) const
  {
    F64 xyz;
    xyz = get_x();
    if (xyz < min_x || xyz >= max_x) return FALSE;
    xyz = get_y();
    if (xyz < min_y || xyz >= max_y) return FALSE;
    xyz = get_z();
    if (xyz < min_z || xyz >= max_z) return FALSE;
    return TRUE;
  }

  bool inside_bounding_box(const F64 min_x, const F64 min_y, const F64 min_z, const F64 max_x, const F64 max_y, const F64 max_z) const
  {
    F64 xyz;
    xyz = get_x();
    if (xyz < min_x || xyz > max_x) return FALSE;
    xyz = get_y();
    if (xyz < min_y || xyz > max_y) return FALSE;
    xyz = get_z();
    if (xyz < min_z || xyz > max_z) return FALSE;
    return TRUE;
  }

  bool is_zero() const
  {
    if (((U32*)&(this->X))[0] || ((U32*)&(this->X))[1] || ((U32*)&(this->X))[2] || ((U32*)&(this->X))[3] || ((U32*)&(this->X))[4])
    {
      return FALSE;
    }
    if (have_gps_time)
    {
      if (this->gps_time)
      {
        return FALSE;
      }
    }
    if (have_rgb)
    {
      if (this->rgb[0] || this->rgb[1] || this->rgb[2])
      {
        return FALSE;
      }
      if (have_nir)
      {
        if (this->rgb[3])
        {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  void zero()
  {
    X = 0;
    Y = 0;
    Z = 0;
    intensity = 0;
    return_number = 1;
    number_of_returns = 1;
    scan_direction_flag = 0;
    edge_of_flight_line = 0;
    classification = 0;
    synthetic_flag = 0;
    keypoint_flag = 0;
    withheld_flag = 0;
    scan_angle_rank = 0;
    user_data = 0;
    point_source_ID = 0;

    // LAS 1.4 only
    extended_scan_angle = 0;
    extended_scanner_channel = 0;
    extended_classification_flags = 0;
    extended_return_number = 1;
    extended_number_of_returns = 1;

    // LASlib only
    deleted_flag = 0;

    gps_time = 0.0;
    rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0;
    wavepacket.zero();
  };

  void clean()
  {
    zero();

    if (extra_bytes)
    {
      delete [] extra_bytes;
      extra_bytes = 0;
    };

    if (point) delete [] point;
    point = 0;

    have_gps_time = FALSE;
    have_rgb = FALSE;
    have_wavepacket = FALSE;
    have_nir = FALSE;
    extra_bytes_number = 0;
    total_point_size = 0;
    
    num_items = 0;
    if (items) delete [] items;
    items = 0;

    // LAS 1.4 only
    extended_point_type = 0;
  };

  LASpoint()
  {
    extra_bytes = 0;
    point = 0;
    items = 0;
    clean();
  };

  inline bool is_first() const { return get_return_number() == 1; };
  inline bool is_last() const { return get_return_number() >= get_number_of_returns(); };

  inline I32 get_X() const { return X; };
  inline I32 get_Y() const { return Y; };
  inline I32 get_Z() const { return Z; };
  inline U16 get_intensity() const { return intensity; };
  inline U8 get_return_number() const { return return_number; };
  inline U8 get_number_of_returns() const { return number_of_returns; };
  inline U8 get_scan_direction_flag() const { return scan_direction_flag; };
  inline U8 get_edge_of_flight_line() const { return edge_of_flight_line; };
  inline U8 get_classification() const { return classification; };
  inline U8 get_synthetic_flag() const { return synthetic_flag; };
  inline U8 get_keypoint_flag() const { return keypoint_flag; };
  inline U8 get_withheld_flag() const { return withheld_flag; };
  inline I8 get_scan_angle_rank() const { return scan_angle_rank; };
  inline U8 get_user_data() const { return user_data; };
  inline U16 get_point_source_ID() const { return point_source_ID; };
  inline U32 get_deleted_flag() const { return deleted_flag; };
  inline F64 get_gps_time() const { return gps_time; };
  inline const U16* get_rgb() const { return rgb; };

  inline void set_X(const I32 X) { this->X = X; };
  inline void set_Y(const I32 Y) { this->Y = Y; };
  inline void set_Z(const I32 Z) { this->Z = Z; };
  inline void set_intensity(const U16 intensity) { this->intensity = intensity; };
  inline void set_return_number(const U8 return_number) { this->return_number = (return_number > 7 ? 7 : return_number); };
  inline void set_number_of_returns(const U8 number_of_returns) { this->number_of_returns = (number_of_returns > 7 ? 7 : number_of_returns); };
  inline void set_scan_direction_flag(const U8 scan_direction_flag) { this->scan_direction_flag = scan_direction_flag; };
  inline void set_edge_of_flight_line(const U8 edge_of_flight_line) { this->edge_of_flight_line = edge_of_flight_line; };
  inline void set_classification(U8 classification) { this->classification = (classification & 31); };
  inline void set_synthetic_flag(U8 synthetic_flag) { this->synthetic_flag = synthetic_flag; };
  inline void set_keypoint_flag(U8 keypoint_flag) { this->keypoint_flag = keypoint_flag; };
  inline void set_withheld_flag(U8 withheld_flag) { this->withheld_flag = withheld_flag; };
  inline void set_user_data(U8 user_data) { this->user_data = user_data; };
  inline void set_point_source_ID(U16 point_source_ID) { this->point_source_ID = point_source_ID; };
  inline void set_deleted_flag(U8 deleted_flag) { this->deleted_flag = (U32)deleted_flag; };
  inline void set_gps_time(const F64 gps_time) { this->gps_time = gps_time; };
  inline void set_rgb(const U16* rgb) { memcpy(this->rgb, rgb, sizeof(this->rgb)); };

  inline F64 get_x() const { return quantizer->get_x(X); };
  inline F64 get_y() const { return quantizer->get_y(Y); };
  inline F64 get_z() const { return quantizer->get_z(Z); };

  inline void set_x(const F64 x) { this->X = quantizer->get_X(x); };
  inline void set_y(const F64 y) { this->Y = quantizer->get_Y(y); };
  inline void set_z(const F64 z) { this->Z = quantizer->get_Z(z); };

  inline void compute_coordinates()
  {
    coordinates[0] = get_x();
    coordinates[1] = get_y();
    coordinates[2] = get_z();
  };

  inline void compute_XYZ()
  {
    set_x(coordinates[0]);
    set_y(coordinates[1]);
    set_z(coordinates[2]);
  };

  inline void compute_XYZ(const LASquantizer* quantizer)
  {
    X = quantizer->get_X(coordinates[0]);
    Y = quantizer->get_Y(coordinates[1]);
    Z = quantizer->get_Z(coordinates[2]);
  };

  // generic functions for attributes in extra bytes

  inline bool has_attribute(I32 index) const
  {
    if (attributer)
    {
      if (index < attributer->number_attributes)
      {
        return TRUE;
      }
    }
    return FALSE;
  };

  inline bool get_attribute(I32 index, U8* data) const
  {
    if (has_attribute(index))
    {
      memcpy(data, extra_bytes + attributer->attribute_starts[index], attributer->attribute_sizes[index]);
      return TRUE;
    }
    return FALSE;
  };

  inline bool set_attribute(I32 index, const U8* data) 
  {
    if (has_attribute(index))
    {
      memcpy(extra_bytes + attributer->attribute_starts[index], data, attributer->attribute_sizes[index]);
      return TRUE;
    }
    return FALSE;
  };

  inline const CHAR* get_attribute_name(I32 index) const
  {
    if (has_attribute(index))
    {
      return attributer->attributes[index].name;
    }
    return 0;
  };

  inline F64 get_attribute_as_float(I32 index) const
  {
    if (has_attribute(index))
    {
      return attributer->attributes[index].get_value_as_float(extra_bytes + attributer->attribute_starts[index]);
    }
    return 0.0;
  };

  // typed and offset functions for attributes in extra bytes (more efficient)

  inline void get_attribute(I32 start, U8 &data) const { data = extra_bytes[start]; };
  inline void set_attribute(I32 start, U8 data) { extra_bytes[start] = data; };
  inline void get_attribute(I32 start, I8 &data) const { data = (I8)(extra_bytes[start]); };
  inline void set_attribute(I32 start, I8 data) { extra_bytes[start] = data; };
  inline void get_attribute(I32 start, U16 &data) const { data = *((U16*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U16 data) { *((U16*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I16 &data) const { data = *((I16*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I16 data) { *((I16*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, U32 &data) const { data = *((U32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U32 data) { *((U32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I32 &data) const { data = *((I32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I32 data) { *((I32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, U64 &data) const { data = *((U64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U64 data) { *((U64*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I64 &data) const { data = *((I64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I64 data) { *((I64*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, F32 &data) const { data = *((F32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, F32 data) { *((F32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, F64 &data) const { data = *((F64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, F64 data) { *((F64*)(extra_bytes + start)) = data; };

  ~LASpoint()
  {
    clean();
  };
};

class LASvlr
{
public:
  U16 reserved;
  CHAR user_id[16]; 
  U16 record_id;
  U16 record_length_after_header;
  CHAR description[32];
  U8* data;
  LASvlr() { memset(this, 0, sizeof(LASvlr)); };
};

class LASevlr
{
public:
  U16 reserved;
  CHAR user_id[16]; 
  U16 record_id;
  I64 record_length_after_header;
  CHAR description[32];
  U8* data;
  LASevlr() { memset(this, 0, sizeof(LASevlr)); };
};

class LASvlr_geo_keys
{
public:
  U16 key_directory_version;
  U16 key_revision;
  U16 minor_revision;
  U16 number_of_keys;
};

class LASvlr_key_entry
{
public:
  U16 key_id;
  U16 tiff_tag_location;
  U16 count;
  U16 value_offset;
};

class LASvlr_classification
{
public:
  U8 class_number;
  CHAR description[15];
};

class LASvlr_wave_packet_descr
{
public:
  LASvlr_wave_packet_descr() {clean();};
  void clean() {memset(data, 0, 26);};
  inline U8 getBitsPerSample() const {return data[0];};
  inline U8 getCompressionType() const {return data[1];};
  inline U32 getNumberOfSamples() const {return ((U32*)&(data[2]))[0];};
  inline U32 getTemporalSpacing() const {return ((U32*)&(data[6]))[0];};
  inline F64 getDigitizerGain() const {return ((F64*)&(data[10]))[0];};
  inline F64 getDigitizerOffset() const {return ((F64*)&(data[18]))[0];};
  inline void setBitsPerSample(U8 bps) {data[0] = bps;};
  inline void setCompressionType(U8 compression) {data[1] = compression;};
  inline void setNumberOfSamples(U32 samples) {((U32*)&(data[2]))[0] = samples;};
  inline void setTemporalSpacing(U32 spacing) {((U32*)&(data[6]))[0] = spacing;};
  inline void setDigitizerGain(F64 gain) {((F64*)&(data[10]))[0] = gain;};
  inline void setDigitizerOffset(F64 offset) {((F64*)&(data[18]))[0] = offset;};
private:
  U8 data[26];
};

class LASvlr_lastiling
{
public:
  U32 level;
  U32 level_index;
  U32 implicit_levels : 30;
  U32 buffer : 1;
  U32 reversible : 1;
  F32 min_x;
  F32 max_x;
  F32 min_y;
  F32 max_y;
};

class LASvlr_lasoriginal
{
public:
  I64 number_of_point_records;
  I64 number_of_points_by_return[15];
  F64 max_x;
  F64 min_x;
  F64 max_y;
  F64 min_y;
  F64 max_z;
  F64 min_z;
};

class LASheader : public LASquantizer, public LASattributer
{
public:
  CHAR file_signature[4];
  U16 file_source_ID;
  U16 global_encoding;
  U32 project_ID_GUID_data_1;
  U16 project_ID_GUID_data_2;
  U16 project_ID_GUID_data_3;
  U8 project_ID_GUID_data_4[8];
  U8 version_major;
  U8 version_minor;
  CHAR system_identifier[32];
  CHAR generating_software[32];
  U16 file_creation_day;
  U16 file_creation_year;
  U16 header_size;
  U32 offset_to_point_data;
  U32 number_of_variable_length_records;
  U8 point_data_format;
  U16 point_data_record_length;
  U32 number_of_point_records;
  U32 number_of_points_by_return[5];
  F64 max_x;
  F64 min_x;
  F64 max_y;
  F64 min_y;
  F64 max_z;
  F64 min_z;

  // LAS 1.3 only
  U64 start_of_waveform_data_packet_record;

  // LAS 1.4 only
  U64 start_of_first_extended_variable_length_record;
  U32 number_of_extended_variable_length_records;
  U64 extended_number_of_point_records;
  U64 extended_number_of_points_by_return[15];

  U32 user_data_in_header_size;
  U8* user_data_in_header;

  LASvlr* vlrs;
  LASevlr* evlrs;
  LASvlr_geo_keys* vlr_geo_keys;
  LASvlr_key_entry* vlr_geo_key_entries;
  F64* vlr_geo_double_params;
  CHAR* vlr_geo_ascii_params;
  CHAR* vlr_geo_wkt_ogc_math;
  CHAR* vlr_geo_wkt_ogc_cs;
  LASvlr_classification* vlr_classification;
  LASvlr_wave_packet_descr** vlr_wave_packet_descr;

  LASzip* laszip;
  LASvlr_lastiling* vlr_lastiling;
  LASvlr_lasoriginal* vlr_lasoriginal;

  U32 user_data_after_header_size;
  U8* user_data_after_header;

  LASheader()
  {
    clean_las_header();
  };

  // set bounding box

  void set_bounding_box(F64 min_x, F64 min_y, F64 min_z, F64 max_x, F64 max_y, F64 max_z, bool auto_scale=TRUE, bool auto_offset=TRUE)
  {
    if (auto_scale)
    {
      if (-360 < min_x  && -360 < min_y && max_x < 360 && max_y < 360)
      {
        x_scale_factor = 0.0000001;
        y_scale_factor = 0.0000001;
      }
      else
      {
        x_scale_factor = 0.01;
        y_scale_factor = 0.01;
      }
      z_scale_factor = 0.01;
    }
    if (auto_offset)
    {
      if (-360 < min_x  && -360 < min_y && max_x < 360 && max_y < 360)
      {
        x_offset = 0;
        y_offset = 0;
        z_offset = 0;
      }
      else
      {
        x_offset = ((I32)((min_x + max_x)/200000))*100000;
        y_offset = ((I32)((min_y + max_y)/200000))*100000;
        z_offset = ((I32)((min_z + max_z)/200000))*100000;
      }
    }
    this->min_x = x_offset + x_scale_factor*I32_QUANTIZE((min_x-x_offset)/x_scale_factor);
    this->min_y = y_offset + y_scale_factor*I32_QUANTIZE((min_y-y_offset)/y_scale_factor);
    this->min_z = z_offset + z_scale_factor*I32_QUANTIZE((min_z-z_offset)/z_scale_factor);
    this->max_x = x_offset + x_scale_factor*I32_QUANTIZE((max_x-x_offset)/x_scale_factor);
    this->max_y = y_offset + y_scale_factor*I32_QUANTIZE((max_y-y_offset)/y_scale_factor);
    this->max_z = z_offset + z_scale_factor*I32_QUANTIZE((max_z-z_offset)/z_scale_factor);
  };

  // clean functions

  void clean_las_header()
  {
    memset((void*)this, 0, sizeof(LASheader));
    file_signature[0] = 'L'; file_signature[1] = 'A'; file_signature[2] = 'S'; file_signature[3] = 'F';
    version_major = 1;
    version_minor = 2;
    header_size = 227;
    offset_to_point_data = 227;
    point_data_record_length = 20;
    x_scale_factor = 0.01;
    y_scale_factor = 0.01;
    z_scale_factor = 0.01;
  };

  void clean_user_data_in_header()
  {
    if (user_data_in_header)
    {
      header_size -= user_data_in_header_size;
      offset_to_point_data -= user_data_in_header_size;
      delete [] user_data_in_header;
      user_data_in_header = 0;
      user_data_in_header_size = 0;
    }
  };

  void clean_vlrs()
  {
    if (vlrs)
    {
      U32 i;
      for (i = 0; i < number_of_variable_length_records; i++)
      {
        offset_to_point_data -= (54 + vlrs[i].record_length_after_header);
        if (vlrs[i].data && (vlrs[i].data != (U8*)attributes))
        {
          delete [] vlrs[i].data;
        }
      }
      free(vlrs);
      vlrs = 0;
      vlr_geo_keys = 0;
      vlr_geo_key_entries = 0;
      vlr_geo_double_params = 0;
      vlr_geo_ascii_params = 0;
      vlr_geo_wkt_ogc_math = 0;
      vlr_geo_wkt_ogc_cs = 0;
      vlr_classification = 0;
      if (vlr_wave_packet_descr) delete [] vlr_wave_packet_descr;
      vlr_wave_packet_descr = 0;
      number_of_variable_length_records = 0;
    }
  };

  void clean_evlrs()
  {
    if (evlrs)
    {
      U32 i;
      for (i = 0; i < number_of_extended_variable_length_records; i++)
      {
        if (evlrs[i].data && (evlrs[i].data != (U8*)attributes))
        {
          delete [] evlrs[i].data;
        }
      }
      free(evlrs);
      evlrs = 0;
      start_of_first_extended_variable_length_record = 0;
      number_of_extended_variable_length_records = 0;
    }
  };

  void clean_laszip()
  {
    if (laszip)
    {
      delete laszip;
    }
    laszip = 0;
  };

  void clean_lastiling()
  {
    if (vlr_lastiling)
    {
      delete vlr_lastiling;
    }
    vlr_lastiling = 0;
  };

  void clean_lasoriginal()
  {
    if (vlr_lasoriginal)
    {
      delete vlr_lasoriginal;
    }
    vlr_lasoriginal = 0;
  };

  void clean_user_data_after_header()
  {
    if (user_data_after_header)
    {
      offset_to_point_data -= user_data_after_header_size;
      delete [] user_data_after_header;
      user_data_after_header = 0;
      user_data_after_header_size = 0;
    }
  };

  void clean()
  {
    clean_user_data_in_header();
    clean_vlrs();
    clean_evlrs();
    clean_laszip();
    clean_lastiling();
    clean_lasoriginal();
    clean_user_data_after_header();
    clean_attributes();
    clean_las_header();
  };

  void unlink()
  {
    user_data_in_header_size = 0;
    user_data_in_header = 0;
    vlrs = 0;
    number_of_variable_length_records = 0;
    evlrs = 0;
    start_of_first_extended_variable_length_record = 0;
    number_of_extended_variable_length_records = 0;
    laszip = 0;
    vlr_lastiling = 0;
    vlr_lasoriginal = 0;
    user_data_after_header_size = 0;
    user_data_after_header = 0;
    number_attributes = 0;
    offset_to_point_data = header_size;
  }

  LASheader & operator=(const LASquantizer & quantizer)
  {
    this->x_scale_factor = quantizer.x_scale_factor;
    this->y_scale_factor = quantizer.y_scale_factor;
    this->z_scale_factor = quantizer.z_scale_factor;
    this->x_offset = quantizer.x_offset;
    this->y_offset = quantizer.y_offset;
    this->z_offset = quantizer.z_offset;
    return *this;
  };

  bool check() const
  {
    if (strncmp(file_signature, "LASF", 4) != 0)
    {
      fprintf(stderr,"ERROR: wrong file signature '%s'\n", file_signature);
      return FALSE;
    }
    if ((version_major != 1) || (version_minor > 4))
    {
      fprintf(stderr,"WARNING: unknown version %d.%d (should be 1.0 or 1.1 or 1.2 or 1.3 or 1.4)\n", version_major, version_minor);
    }
    if (header_size < 227)
    {
      fprintf(stderr,"ERROR: header size is %d but should be at least 227\n", header_size);
      return FALSE;
    }
    if (offset_to_point_data < header_size)
    {
      fprintf(stderr,"ERROR: offset to point data %d is smaller than header size %d\n", offset_to_point_data, header_size);
      return FALSE;
    }
    if (x_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: x scale factor is zero.\n");
    }
    if (y_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: y scale factor is zero.\n");
    }
    if (z_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: z scale factor is zero.\n");
    }
    if (max_x < min_x || max_y < min_y || max_z < min_z)
    {
      fprintf(stderr,"WARNING: invalid bounding box [ %g %g %g / %g %g %g ]\n", min_x, min_y, min_z, max_x, max_y, max_z);
    }
    return TRUE;
  };

  bool is_lonlat() const
  {
    if ((-360.0 <= min_x) && (-90.0 <= min_y) && (max_x <= 360.0) && (max_y <= 90.0))
    {
      return TRUE;
    }
    return FALSE;
  }

  // note that data needs to be allocated with new [] and not malloc and that LASheader
  // will become the owner over this and manage its deallocation 
  void add_vlr(const CHAR* user_id, const U16 record_id, const U16 record_length_after_header, U8* data, const bool keep_description=FALSE, const CHAR* description=0)
  {
    U32 i = 0;
    bool found_description = FALSE;
    if (vlrs)
    {
      for (i = 0; i < number_of_variable_length_records; i++)
      {
        if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
        {
          if (vlrs[i].record_length_after_header)
          {
            offset_to_point_data -= vlrs[i].record_length_after_header;
            delete [] vlrs[i].data;
            vlrs[i].data = 0;
          }
          found_description = TRUE;
          break;
        }
      }
      if (i == number_of_variable_length_records)
      {
        number_of_variable_length_records++;
        offset_to_point_data += 54;
        vlrs = (LASvlr*)realloc(vlrs, sizeof(LASvlr)*number_of_variable_length_records);
      }
    }
    else
    {
      number_of_variable_length_records = 1;
      offset_to_point_data += 54;
      vlrs = (LASvlr*)malloc(sizeof(LASvlr)*number_of_variable_length_records);
    }
    vlrs[i].reserved = 0xAABB;
    strncpy(vlrs[i].user_id, user_id, 16);
    vlrs[i].record_id = record_id;
    vlrs[i].record_length_after_header = record_length_after_header;
    if (keep_description && found_description)
    {
      // do nothing
    }
    else if (description)
    {
      sprintf(vlrs[i].description, "%31s", description);
    }
    else
    {
      sprintf(vlrs[i].description, "by LAStools of rapidlasso GmbH");
    }
    if (record_length_after_header)
    {
      offset_to_point_data += record_length_after_header;
      vlrs[i].data = data;
    }
    else
    {
      vlrs[i].data = 0;
    }
  };

  const LASvlr* get_vlr(const CHAR* user_id, U16 record_id) const
  {
    U32 i = 0;
    for (i = 0; i < number_of_variable_length_records; i++)
    {
      if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
      {
        return &(vlrs[i]);
      }
    }
    return 0;
  };

  bool remove_vlr(U32 i)
  {
    if (vlrs)
    {
      if (i < number_of_variable_length_records)
      {
        offset_to_point_data -= (54 + vlrs[i].record_length_after_header);
        if (vlrs[i].record_length_after_header)
        {
          delete [] vlrs[i].data;
        }
        number_of_variable_length_records--;
        if (number_of_variable_length_records)
        {
          vlrs[i] = vlrs[number_of_variable_length_records];
          vlrs = (LASvlr*)realloc(vlrs, sizeof(LASvlr)*number_of_variable_length_records);
        }
        else
        {
          free(vlrs);
          vlrs = 0;
        }
      }
      return TRUE;
    }
    return FALSE;
  };

  bool remove_vlr(const CHAR* user_id, U16 record_id)
  {
    U32 i;
    for (i = 0; i < number_of_variable_length_records; i++)
    {
      if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
      {
        return remove_vlr(i);
      }
    }
    return FALSE;
  };

  void set_lastiling(U32 level, U32 level_index, U32 implicit_levels, bool buffer, bool reversible, F32 min_x, F32 max_x, F32 min_y, F32 max_y)
  {
    clean_lastiling();
    vlr_lastiling = new LASvlr_lastiling();
    vlr_lastiling->level = level;
    vlr_lastiling->level_index = level_index;
    vlr_lastiling->implicit_levels = implicit_levels;
    vlr_lastiling->buffer = buffer;
    vlr_lastiling->reversible = reversible;
    vlr_lastiling->min_x = min_x;
    vlr_lastiling->max_x = max_x;
    vlr_lastiling->min_y = min_y;
    vlr_lastiling->max_y = max_y;
  };

  void set_lasoriginal()
  {
    clean_lasoriginal();
    vlr_lasoriginal = new LASvlr_lasoriginal();
    if (version_minor >= 4)
    {
      vlr_lasoriginal->number_of_point_records = extended_number_of_point_records;
      vlr_lasoriginal->number_of_points_by_return[0] = extended_number_of_points_by_return[0];
      vlr_lasoriginal->number_of_points_by_return[1] = extended_number_of_points_by_return[1];
      vlr_lasoriginal->number_of_points_by_return[2] = extended_number_of_points_by_return[2];
      vlr_lasoriginal->number_of_points_by_return[3] = extended_number_of_points_by_return[3];
      vlr_lasoriginal->number_of_points_by_return[4] = extended_number_of_points_by_return[4];
      vlr_lasoriginal->number_of_points_by_return[5] = extended_number_of_points_by_return[5];
      vlr_lasoriginal->number_of_points_by_return[6] = extended_number_of_points_by_return[6];
      vlr_lasoriginal->number_of_points_by_return[7] = extended_number_of_points_by_return[7];
      vlr_lasoriginal->number_of_points_by_return[8] = extended_number_of_points_by_return[8];
      vlr_lasoriginal->number_of_points_by_return[9] = extended_number_of_points_by_return[9];
      vlr_lasoriginal->number_of_points_by_return[10] = extended_number_of_points_by_return[10];
      vlr_lasoriginal->number_of_points_by_return[11] = extended_number_of_points_by_return[11];
      vlr_lasoriginal->number_of_points_by_return[12] = extended_number_of_points_by_return[12];
      vlr_lasoriginal->number_of_points_by_return[13] = extended_number_of_points_by_return[13];
      vlr_lasoriginal->number_of_points_by_return[14] = extended_number_of_points_by_return[14];
    }
    else
    {
      vlr_lasoriginal->number_of_point_records = number_of_point_records;
      vlr_lasoriginal->number_of_points_by_return[0] = number_of_points_by_return[0];
      vlr_lasoriginal->number_of_points_by_return[1] = number_of_points_by_return[1];
      vlr_lasoriginal->number_of_points_by_return[2] = number_of_points_by_return[2];
      vlr_lasoriginal->number_of_points_by_return[3] = number_of_points_by_return[3];
      vlr_lasoriginal->number_of_points_by_return[4] = number_of_points_by_return[4];
    }
    vlr_lasoriginal->max_x = max_x;
    vlr_lasoriginal->min_x = min_x;
    vlr_lasoriginal->max_y = max_y;
    vlr_lasoriginal->min_y = min_y;
    vlr_lasoriginal->max_z = max_z;
    vlr_lasoriginal->min_z = min_z;
  }

  bool restore_lasoriginal()
  {
    if (vlr_lasoriginal)
    {
      if (version_minor >= 4)
      {
        extended_number_of_point_records = vlr_lasoriginal->number_of_point_records;
        extended_number_of_points_by_return[0] = vlr_lasoriginal->number_of_points_by_return[0];
        extended_number_of_points_by_return[1] = vlr_lasoriginal->number_of_points_by_return[1];
        extended_number_of_points_by_return[2] = vlr_lasoriginal->number_of_points_by_return[2];
        extended_number_of_points_by_return[3] = vlr_lasoriginal->number_of_points_by_return[3];
        extended_number_of_points_by_return[4] = vlr_lasoriginal->number_of_points_by_return[4];
        extended_number_of_points_by_return[5] = vlr_lasoriginal->number_of_points_by_return[5];
        extended_number_of_points_by_return[6] = vlr_lasoriginal->number_of_points_by_return[6];
        extended_number_of_points_by_return[7] = vlr_lasoriginal->number_of_points_by_return[7];
        extended_number_of_points_by_return[8] = vlr_lasoriginal->number_of_points_by_return[8];
        extended_number_of_points_by_return[9] = vlr_lasoriginal->number_of_points_by_return[9];
        extended_number_of_points_by_return[10] = vlr_lasoriginal->number_of_points_by_return[10];
        extended_number_of_points_by_return[11] = vlr_lasoriginal->number_of_points_by_return[11];
        extended_number_of_points_by_return[12] = vlr_lasoriginal->number_of_points_by_return[12];
        extended_number_of_points_by_return[13] = vlr_lasoriginal->number_of_points_by_return[13];
        extended_number_of_points_by_return[14] = vlr_lasoriginal->number_of_points_by_return[14];
      }
      else
      {
        number_of_point_records = (U32)vlr_lasoriginal->number_of_point_records;
        number_of_points_by_return[0] = (U32)vlr_lasoriginal->number_of_points_by_return[0];
        number_of_points_by_return[1] = (U32)vlr_lasoriginal->number_of_points_by_return[1];
        number_of_points_by_return[2] = (U32)vlr_lasoriginal->number_of_points_by_return[2];
        number_of_points_by_return[3] = (U32)vlr_lasoriginal->number_of_points_by_return[3];
        number_of_points_by_return[4] = (U32)vlr_lasoriginal->number_of_points_by_return[4];
      }
      max_x = vlr_lasoriginal->max_x;
      min_x = vlr_lasoriginal->min_x;
      max_y = vlr_lasoriginal->max_y;
      min_y = vlr_lasoriginal->min_y;
      max_z = vlr_lasoriginal->max_z;
      min_z = vlr_lasoriginal->min_z;
      delete vlr_lasoriginal;
      vlr_lasoriginal = 0;
      return TRUE;
    }
    return FALSE;
  }

  void set_geo_keys(const I32 number_of_keys, const LASvlr_key_entry* geo_keys)
  {
    vlr_geo_keys = new LASvlr_geo_keys[number_of_keys+1];
    vlr_geo_keys->key_directory_version = 1;
    vlr_geo_keys->key_revision = 1;
    vlr_geo_keys->minor_revision = 0;
    vlr_geo_keys->number_of_keys = number_of_keys;
    vlr_geo_key_entries = (LASvlr_key_entry*)&vlr_geo_keys[1];
    memcpy(vlr_geo_key_entries, geo_keys, sizeof(LASvlr_key_entry)*number_of_keys);
    add_vlr("LASF_Projection", 34735, sizeof(LASvlr_geo_keys)*(number_of_keys+1), (U8*)vlr_geo_keys);
  }

  void set_geo_double_params(const I32 num_geo_double_params, const F64* geo_double_params)
  {
    vlr_geo_double_params = new F64[num_geo_double_params];
    memcpy(vlr_geo_double_params, geo_double_params, sizeof(F64)*num_geo_double_params);
    add_vlr("LASF_Projection", 34736, sizeof(F64)*num_geo_double_params, (U8*)vlr_geo_double_params);
  }

  void del_geo_double_params()
  {
    if (vlr_geo_double_params)
    {
      remove_vlr("LASF_Projection", 34736);
      vlr_geo_double_params = 0;
    }
  }

  void set_geo_ascii_params(const I32 num_geo_ascii_params, const CHAR* geo_ascii_params)
  {
    vlr_geo_ascii_params = new CHAR[num_geo_ascii_params];
    memcpy(vlr_geo_ascii_params, geo_ascii_params, sizeof(CHAR)*num_geo_ascii_params);
    add_vlr("LASF_Projection", 34737, sizeof(CHAR)*num_geo_ascii_params, (U8*)vlr_geo_ascii_params);
  }

  void del_geo_ascii_params()
  {
    if (vlr_geo_ascii_params)
    {
      remove_vlr("LASF_Projection", 34737);
      vlr_geo_ascii_params = 0;
    }
  }

  void set_geo_wkt_ogc_math(const I32 num_geo_wkt_ogc_math, const CHAR* geo_wkt_ogc_math)
  {
    vlr_geo_wkt_ogc_math = new CHAR[num_geo_wkt_ogc_math];
    memcpy(vlr_geo_wkt_ogc_math, geo_wkt_ogc_math, sizeof(CHAR)*num_geo_wkt_ogc_math);
    add_vlr("LASF_Projection", 2111, sizeof(CHAR)*num_geo_wkt_ogc_math, (U8*)vlr_geo_wkt_ogc_math);
  }

  void del_geo_wkt_ogc_math()
  {
    if (vlr_geo_wkt_ogc_math)
    {
      remove_vlr("LASF_Projection", 2111);
      vlr_geo_wkt_ogc_math = 0;
    }
  }

  void set_geo_wkt_ogc_cs(const I32 num_geo_wkt_ogc_cs, const CHAR* geo_wkt_ogc_cs)
  {
    vlr_geo_wkt_ogc_cs = new CHAR[num_geo_wkt_ogc_cs];
    memcpy(vlr_geo_wkt_ogc_cs, geo_wkt_ogc_cs, sizeof(CHAR)*num_geo_wkt_ogc_cs);
    add_vlr("LASF_Projection", 2112, sizeof(CHAR)*num_geo_wkt_ogc_cs, (U8*)vlr_geo_wkt_ogc_cs);
  }

  void del_geo_wkt_ogc_cs()
  {
    if (vlr_geo_wkt_ogc_cs)
    {
      remove_vlr("LASF_Projection", 2112);
      vlr_geo_wkt_ogc_cs = 0;
    }
  }

  void update_extra_bytes_vlr(const bool keep_description=FALSE)
  {
    if (number_attributes)
    {
      U16 record_length_after_header = sizeof(LASattribute)*number_attributes;
      U8* data = new U8[record_length_after_header];
      memcpy(data, attributes, record_length_after_header);
      add_vlr("LASF_Spec", 4, record_length_after_header, data, keep_description);
    }
    else
    {
      remove_vlr("LASF_Spec", 4);
    }
  }

  ~LASheader()
  {
    clean();
  };
};

#endif
