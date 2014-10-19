/*
===============================================================================

  FILE:  laszip_dll.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/

#define LASZIP_DYN_LINK
#define LASZIP_SOURCE

#include "laszip_dll.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "laszip.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamin_file.hpp"
#include "laswritepoint.hpp"
#include "lasreadpoint.hpp"

typedef struct laszip_dll {
  laszip_header_struct header;
  I64 p_count;
  I64 npoints;
  laszip_point_struct point;
  U8** point_items;
  FILE* file;
  ByteStreamIn* streamin;
  LASreadPoint* reader;
  ByteStreamOut* streamout;
  LASwritePoint* writer;
  CHAR error[1024]; 
  CHAR warning[1024]; 
} laszip_dll_struct;

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_version(
    laszip_U8*                         version_major
    , laszip_U8*                       version_minor
    , laszip_U16*                      version_revision
    , laszip_U32*                      version_build
)
{
  try
  {
    *version_major = LASZIP_VERSION_MAJOR;
    *version_minor = LASZIP_VERSION_MINOR;
    *version_revision = LASZIP_VERSION_REVISION;
    *version_build = LASZIP_VERSION_BUILD_DATE;
  }
  catch (...)
  {
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_error(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    error
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    *error = laszip_dll->error;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_error");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_warning(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    warning
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    *warning = laszip_dll->warning;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_warning");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_create(
    laszip_POINTER*                    pointer
)
{
  if (pointer == 0) return 1;

  try
  {
    laszip_dll_struct* laszip_dll = new laszip_dll_struct;
    if (laszip_dll == 0)
    {
      return 1;
    }

    // zero everything

    memset(laszip_dll, 0, sizeof(laszip_dll_struct));
    
    // create the default

    laszip_clean(laszip_dll);

    *pointer = laszip_dll;
  }
  catch (...)
  {
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_clean(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot clean while reader is open.");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot clean while writer is open.");
      return 1;
    }

    // dealloc everything alloc in the header

    if (laszip_dll->header.user_data_in_header)
    {
      delete [] laszip_dll->header.user_data_in_header;
    }

    if (laszip_dll->header.vlrs)
    {
      U32 i;
      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        if (laszip_dll->header.vlrs[i].data)
        {
          delete [] laszip_dll->header.vlrs[i].data;
        }
      }
      free(laszip_dll->header.vlrs);
    }

    if (laszip_dll->header.user_data_after_header)
    {
      delete [] laszip_dll->header.user_data_after_header;
    }

    // dealloc everything alloc in the point

    if (laszip_dll->point.extra_bytes)
    {
      delete [] laszip_dll->point.extra_bytes;
    }

    // zero everything

    memset(laszip_dll, 0, sizeof(laszip_dll_struct));
    
    // create default header

    sprintf(laszip_dll->header.generating_software, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    laszip_dll->header.version_major = 1;
    laszip_dll->header.version_minor = 2;
    laszip_dll->header.header_size = 227;
    laszip_dll->header.offset_to_point_data = 227;
    laszip_dll->header.point_data_format = 1;
    laszip_dll->header.point_data_record_length = 28;
    laszip_dll->header.x_scale_factor = 0.01;
    laszip_dll->header.y_scale_factor = 0.01;
    laszip_dll->header.z_scale_factor = 0.01;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_clean");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_destroy(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  int err = 0;

  try
  {
    err = laszip_clean(laszip_dll);
    delete laszip_dll;
  }
  catch (...)
  {
    return 1;
  }

  return err;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_header_pointer(
    laszip_POINTER                     pointer
    , laszip_header_struct**           header_pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (header_pointer == 0)
    {
      sprintf(laszip_dll->error, "laszip_header_struct header_pointer is zero");
      return 1;
    }

    *header_pointer = &laszip_dll->header;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_header_pointer");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_point_pointer(
    laszip_POINTER                     pointer
    , laszip_point_struct**            point_pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (point_pointer == 0)
    {
      sprintf(laszip_dll->error, "laszip_point_struct point_pointer is zero");
      return 1;
    }

    *point_pointer = &laszip_dll->point;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_point_pointer");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_point_count(
    laszip_POINTER                     pointer
    , laszip_I64*                      count
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (count == 0)
    {
      sprintf(laszip_dll->error, "laszip_I64 count pointer is zero");
      return 1;
    }

    if ((laszip_dll->reader == 0) && (laszip_dll->writer == 0))
    {
      sprintf(laszip_dll->error, "getting count before reader or writer was opened");
      return 1;
    }

    *count = laszip_dll->p_count;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_point_count");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_header(
    laszip_POINTER                     pointer
    , const laszip_header_struct*      header
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (header == 0)
    {
      sprintf(laszip_dll->error, "laszip_header_struct pointer is zero");
      return 1;
    }
    
    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set header after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot set header after writer was opened");
      return 1;
    }

    U32 i;

    laszip_dll->header.file_source_ID = header->file_source_ID;
    laszip_dll->header.global_encoding = header->global_encoding;
    laszip_dll->header.project_ID_GUID_data_1 = header->project_ID_GUID_data_1;
    laszip_dll->header.project_ID_GUID_data_2 = header->project_ID_GUID_data_2;
    laszip_dll->header.project_ID_GUID_data_3 = header->project_ID_GUID_data_3;
    memcpy(laszip_dll->header.project_ID_GUID_data_4, header->project_ID_GUID_data_4, 8);
    laszip_dll->header.version_major = header->version_major;
    laszip_dll->header.version_minor = header->version_minor;
    memcpy(laszip_dll->header.system_identifier, header->system_identifier, 32);
    memcpy(laszip_dll->header.generating_software, header->generating_software, 32);
    laszip_dll->header.file_creation_day = header->file_creation_day;
    laszip_dll->header.file_creation_year = header->file_creation_year;
    laszip_dll->header.header_size = header->header_size;
    laszip_dll->header.offset_to_point_data = header->offset_to_point_data;
    laszip_dll->header.number_of_variable_length_records = header->number_of_variable_length_records;
    laszip_dll->header.point_data_format = header->point_data_format;
    laszip_dll->header.point_data_record_length = header->point_data_record_length;
    laszip_dll->header.number_of_point_records = header->number_of_point_records;
    for (i = 0; i < 5; i++) laszip_dll->header.number_of_points_by_return[i] = header->number_of_points_by_return[i];
    laszip_dll->header.x_scale_factor = header->x_scale_factor;
    laszip_dll->header.y_scale_factor = header->y_scale_factor;
    laszip_dll->header.z_scale_factor = header->z_scale_factor;
    laszip_dll->header.x_offset = header->x_offset;
    laszip_dll->header.y_offset = header->y_offset;
    laszip_dll->header.z_offset = header->z_offset;
    laszip_dll->header.max_x = header->max_x;
    laszip_dll->header.min_x = header->min_x;
    laszip_dll->header.max_y = header->max_y;
    laszip_dll->header.min_y = header->min_y;
    laszip_dll->header.max_z = header->max_z;
    laszip_dll->header.min_z = header->min_z;

    if (laszip_dll->header.version_minor >= 3)
    {
      laszip_dll->header.start_of_waveform_data_packet_record = header->start_of_first_extended_variable_length_record;
    }

    if (laszip_dll->header.version_minor >= 4)
    {
      laszip_dll->header.start_of_first_extended_variable_length_record = header->start_of_first_extended_variable_length_record;
      laszip_dll->header.number_of_extended_variable_length_records = header->number_of_extended_variable_length_records;
      laszip_dll->header.extended_number_of_point_records = header->extended_number_of_point_records;
      for (i = 0; i < 15; i++) laszip_dll->header.extended_number_of_points_by_return[i] = header->extended_number_of_points_by_return[i];
    }

    laszip_dll->header.user_data_in_header_size = header->user_data_in_header_size;
    if (laszip_dll->header.user_data_in_header)
    {
      delete [] laszip_dll->header.user_data_in_header;
      laszip_dll->header.user_data_in_header = 0;
    }
    if (header->user_data_in_header_size)
    {
      laszip_dll->header.user_data_in_header = new U8[header->user_data_in_header_size];
      memcpy(laszip_dll->header.user_data_in_header, header->user_data_in_header, header->user_data_in_header_size);
    }

    if (laszip_dll->header.vlrs)
    {
      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        if (laszip_dll->header.vlrs[i].data)
        {
          delete [] laszip_dll->header.vlrs[i].data;
        }
      }
      free(laszip_dll->header.vlrs);
      laszip_dll->header.vlrs = 0;
    }
    if (header->number_of_variable_length_records)
    {
      laszip_dll->header.vlrs = (laszip_vlr*)malloc(sizeof(laszip_vlr)*header->number_of_variable_length_records);
      for (i = 0; i < header->number_of_variable_length_records; i++)
      {
        laszip_dll->header.vlrs[i].reserved = header->vlrs[i].reserved;
        memcpy(laszip_dll->header.vlrs[i].user_id, header->vlrs[i].user_id, 16);
        laszip_dll->header.vlrs[i].record_id = header->vlrs[i].record_id;
        laszip_dll->header.vlrs[i].record_length_after_header = header->vlrs[i].record_length_after_header;
        memcpy(laszip_dll->header.vlrs[i].description, header->vlrs[i].description, 32);
        if (header->vlrs[i].record_length_after_header)
        {
          laszip_dll->header.vlrs[i].data = new U8[header->vlrs[i].record_length_after_header];
          memcpy(laszip_dll->header.vlrs[i].data, header->vlrs[i].data, header->vlrs[i].record_length_after_header);
        }
        else
        {
          laszip_dll->header.vlrs[i].data = 0;
        }
      }
      memcpy(laszip_dll->header.user_data_in_header, header->user_data_in_header, header->user_data_in_header_size);
    }

    laszip_dll->header.user_data_after_header_size = header->user_data_after_header_size;
    if (laszip_dll->header.user_data_after_header)
    {
      delete [] laszip_dll->header.user_data_after_header;
      laszip_dll->header.user_data_after_header = 0;
    }
    if (header->user_data_after_header_size)
    {
      laszip_dll->header.user_data_after_header = new U8[header->user_data_after_header_size];
      memcpy(laszip_dll->header.user_data_after_header, header->user_data_after_header, header->user_data_after_header_size);
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_header");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_check_for_integer_overflow(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    // get a pointer to the header

    laszip_header_struct* header = &(laszip_dll->header);
    
    // quantize and dequantize the bounding box with current scale_factor and offset

    I32 quant_min_x = I32_QUANTIZE((header->min_x-header->x_offset)/header->x_scale_factor);
    I32 quant_max_x = I32_QUANTIZE((header->max_x-header->x_offset)/header->x_scale_factor);
    I32 quant_min_y = I32_QUANTIZE((header->min_y-header->y_offset)/header->y_scale_factor);
    I32 quant_max_y = I32_QUANTIZE((header->max_y-header->y_offset)/header->y_scale_factor);
    I32 quant_min_z = I32_QUANTIZE((header->min_z-header->z_offset)/header->z_scale_factor);
    I32 quant_max_z = I32_QUANTIZE((header->max_z-header->z_offset)/header->z_scale_factor);

    F64 dequant_min_x = header->x_scale_factor*quant_min_x+header->x_offset;
    F64 dequant_max_x = header->x_scale_factor*quant_max_x+header->x_offset;
    F64 dequant_min_y = header->y_scale_factor*quant_min_y+header->y_offset;
    F64 dequant_max_y = header->y_scale_factor*quant_max_y+header->y_offset;
    F64 dequant_min_z = header->z_scale_factor*quant_min_z+header->z_offset;
    F64 dequant_max_z = header->z_scale_factor*quant_max_z+header->z_offset;

    // make sure that there is not sign flip (a 32-bit integer overflow) for the bounding box

    if ((header->min_x > 0) != (dequant_min_x > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for min_x from %g to %g. set scale factor for x coarser than %g\n", header->min_x, dequant_min_x, header->x_scale_factor);
      return 1;
    }
    if ((header->max_x > 0) != (dequant_max_x > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for max_x from %g to %g. set scale factor for x coarser than %g\n", header->max_x, dequant_max_x, header->x_scale_factor);
      return 1;
    }
    if ((header->min_y > 0) != (dequant_min_y > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for min_y from %g to %g. set scale factor for y coarser than %g\n", header->min_y, dequant_min_y, header->y_scale_factor);
      return 1;
    }
    if ((header->max_y > 0) != (dequant_max_y > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for max_y from %g to %g. set scale factor for y coarser than %g\n", header->max_y, dequant_max_y, header->y_scale_factor);
      return 1;
    }
    if ((header->min_z > 0) != (dequant_min_z > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for min_z from %g to %g. set scale factor for z coarser than %g\n", header->min_z, dequant_min_z, header->z_scale_factor);
      return 1;
    }
    if ((header->max_z > 0) != (dequant_max_z > 0))
    {
      sprintf(laszip_dll->error, "quantization sign flip for max_z from %g to %g. set scale factor for z coarser than %g\n", header->max_z, dequant_max_z, header->z_scale_factor);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_auto_offset");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_auto_offset(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot auto offset after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot auto offset after writer was opened");
      return 1;
    }

    // get a pointer to the header

    laszip_header_struct* header = &(laszip_dll->header);

    // check scale factor

    F64 x_scale_factor = header->x_scale_factor;
    F64 y_scale_factor = header->y_scale_factor;
    F64 z_scale_factor = header->z_scale_factor;

    if ((x_scale_factor <= 0) || !F64_IS_FINITE(x_scale_factor))
    {
      sprintf(laszip_dll->error, "invalid x scale_factor %g in header", header->x_scale_factor);
      return 1;
    }

    if ((y_scale_factor <= 0) || !F64_IS_FINITE(y_scale_factor))
    {
      sprintf(laszip_dll->error, "invalid y scale_factor %g in header", header->y_scale_factor);
      return 1;
    }

    if ((z_scale_factor <= 0) || !F64_IS_FINITE(z_scale_factor))
    {
      sprintf(laszip_dll->error, "invalid z scale_factor %g in header", header->z_scale_factor);
      return 1;
    }

    F64 center_bb_x = (header->min_x + header->max_x) / 2;
    F64 center_bb_y = (header->min_y + header->max_y) / 2;
    F64 center_bb_z = (header->min_z + header->max_z) / 2;

    if (!F64_IS_FINITE(center_bb_x))
    {
      sprintf(laszip_dll->error, "invalid x coordinate at center of bounding box (min: %g max: %g)", header->min_x, header->max_x);
      return 1;
    }

    if (!F64_IS_FINITE(center_bb_y))
    {
      sprintf(laszip_dll->error, "invalid y coordinate at center of  bounding box (min: %g max: %g)", header->min_y, header->max_y);
      return 1;
    }

    if (!F64_IS_FINITE(center_bb_z))
    {
      sprintf(laszip_dll->error, "invalid z coordinate at center of  bounding box (min: %g max: %g)", header->min_z, header->max_z);
      return 1;
    }

    F64 x_offset = header->x_offset;
    F64 y_offset = header->y_offset;
    F64 z_offset = header->z_offset;

    header->x_offset = (I64_FLOOR(center_bb_x/x_scale_factor/10000000))*10000000*x_scale_factor;
    header->y_offset = (I64_FLOOR(center_bb_y/y_scale_factor/10000000))*10000000*y_scale_factor;
    header->z_offset = (I64_FLOOR(center_bb_z/z_scale_factor/10000000))*10000000*z_scale_factor;

    if (laszip_check_for_integer_overflow(pointer))
    {
      header->x_offset = x_offset;
      header->y_offset = y_offset;
      header->z_offset = z_offset;
      return 1;
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_auto_offset");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_point(
    laszip_POINTER                     pointer
    , const laszip_point_struct*       point
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (point == 0)
    {
      sprintf(laszip_dll->error, "laszip_point_struct pointer is zero");
      return 1;
    }
    
    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set point for reader");
      return 1;
    }

    memcpy(&laszip_dll->point, point, ((U8*)&(laszip_dll->point.extra_bytes)) - ((U8*)&(laszip_dll->point.X)));

    if (laszip_dll->point.extra_bytes)
    {
      if (point->extra_bytes)
      {
        if (laszip_dll->point.num_extra_bytes == point->num_extra_bytes)
        {
          memcpy(laszip_dll->point.extra_bytes, point->extra_bytes, laszip_dll->point.num_extra_bytes);
        }
        else
        {
          sprintf(laszip_dll->error, "target point has %d extra bytes but source point has %d", laszip_dll->point.num_extra_bytes, point->num_extra_bytes);
          return 1;
        }
      }
      else
      {
        sprintf(laszip_dll->error, "target point has extra bytes but source point does not");
        return 1;
      }
    }
    else
    {
      if (point->extra_bytes)
      {
        sprintf(laszip_dll->error, "source point has extra bytes but target point does not");
        return 1;
      }
    } 
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_point");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_coordinates(
    laszip_POINTER                     pointer
    , const laszip_F64*                coordinates
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (coordinates == 0)
    {
      sprintf(laszip_dll->error, "laszip_F64 coordinates pointer is zero");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set coordinates for reader");
      return 1;
    }

    // get a pointer to the header

    laszip_header_struct* header = &(laszip_dll->header);

    // get a pointer to the point

    laszip_point_struct* point = &(laszip_dll->point);

    // set the coordinates

    point->X = I32_QUANTIZE((coordinates[0]-header->x_offset)/header->x_scale_factor);
    point->Y = I32_QUANTIZE((coordinates[1]-header->y_offset)/header->y_scale_factor);
    point->Z = I32_QUANTIZE((coordinates[2]-header->z_offset)/header->z_scale_factor);
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_coordinates");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_get_coordinates(
    laszip_POINTER                     pointer
    , laszip_F64*                      coordinates
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (coordinates == 0)
    {
      sprintf(laszip_dll->error, "laszip_F64 coordinates pointer is zero");
      return 1;
    }

    // get a pointer to the header

    laszip_header_struct* header = &(laszip_dll->header);

    // get a pointer to the point

    laszip_point_struct* point = &(laszip_dll->point);

    // get the coordinates

    coordinates[0] = header->x_scale_factor*point->X+header->x_offset;
    coordinates[1] = header->y_scale_factor*point->Y+header->y_offset;
    coordinates[2] = header->z_scale_factor*point->Z+header->z_offset;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_get_coordinates");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_geokeys(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_geokey_struct*      key_entries
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(laszip_dll->error, "number of key_entries is zero");
      return 1;
    }

    if (key_entries == 0)
    {
      sprintf(laszip_dll->error, "key_entries pointer is zero");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set geokeys after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot set geokeys after writer was opened");
      return 1;
    }

    // create the geokey directory

    laszip_geokey_struct* key_entries_plus_one = new laszip_geokey_struct[number+1];
    if (key_entries_plus_one == 0) return FALSE;
    key_entries_plus_one[0].key_id = 1;            // aka key_directory_version
    key_entries_plus_one[0].tiff_tag_location = 1; // aka key_revision
    key_entries_plus_one[0].count = 0;             // aka minor_revision
    key_entries_plus_one[0].value_offset = number; // aka number_of_keys
    memcpy(key_entries_plus_one + 1, key_entries, sizeof(laszip_geokey_struct)*number);

    // fill a VLR

    laszip_vlr_struct vlr;
    memset(&vlr, 0, sizeof(laszip_vlr_struct));
    vlr.reserved = 0xAABB;
    strncpy(vlr.user_id, "LASF_Projection", 16);
    vlr.record_id = 34735;
    vlr.record_length_after_header = 8 + number*8;
    sprintf(vlr.description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    vlr.data = (U8*)key_entries_plus_one;

    // add the VLR

    if (laszip_add_vlr(pointer, &vlr))
    {
      sprintf(laszip_dll->error, "setting %u geodouble_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_geokey_entries");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_geodouble_params(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_F64*                geodouble_params
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(laszip_dll->error, "number of geodouble_params is zero");
      return 1;
    }

    if (geodouble_params == 0)
    {
      sprintf(laszip_dll->error, "geodouble_params pointer is zero");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set geodouble_params after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot set geodouble_params after writer was opened");
      return 1;
    }

    // fill a VLR

    laszip_vlr_struct vlr;
    memset(&vlr, 0, sizeof(laszip_vlr_struct));
    vlr.reserved = 0xAABB;
    strncpy(vlr.user_id, "LASF_Projection", 16);
    vlr.record_id = 34736;
    vlr.record_length_after_header = number*8;
    sprintf(vlr.description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    vlr.data = (U8*)geodouble_params;

    // add the VLR

    if (laszip_add_vlr(pointer, &vlr))
    {
      sprintf(laszip_dll->error, "setting %u geodouble_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_geodouble_params");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_set_geoascii_params(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_CHAR*               geoascii_params
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(laszip_dll->error, "number of geoascii_params is zero");
      return 1;
    }

    if (geoascii_params == 0)
    {
      sprintf(laszip_dll->error, "geoascii_params pointer is zero");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot set geoascii_params after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot set geoascii_params after writer was opened");
      return 1;
    }

    // fill a VLR

    laszip_vlr_struct vlr;
    memset(&vlr, 0, sizeof(laszip_vlr_struct));
    vlr.reserved = 0xAABB;
    strncpy(vlr.user_id, "LASF_Projection", 16);
    vlr.record_id = 34737;
    vlr.record_length_after_header = number;
    sprintf(vlr.description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    vlr.data = (U8*)geoascii_params;

    // add the VLR

    if (laszip_add_vlr(pointer, &vlr))
    {
      sprintf(laszip_dll->error, "setting %u geoascii_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_set_geoascii_params");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_add_vlr(
    laszip_POINTER                     pointer
    , const laszip_vlr_struct*         vlr
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (vlr == 0)
    {
      sprintf(laszip_dll->error, "laszip_vlr_struct vlr pointer is zero");
      return 1;
    }

    if ((vlr->record_length_after_header > 0) && (vlr->data == 0))
    {
      sprintf(laszip_dll->error, "VLR has record_length_after_header of %u but VLR data pointer is zero", (U32)vlr->record_length_after_header);
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "cannot add vlr after reader was opened");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "cannot add vlr after writer was opened");
      return 1;
    }


    U32 i = 0;

    if (laszip_dll->header.vlrs)
    {
      // overwrite existing VLR ?

      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        if ((strncmp(laszip_dll->header.vlrs[i].user_id, vlr->user_id, 16) == 0) && (laszip_dll->header.vlrs[i].record_id == vlr->record_id))
        {
          if (laszip_dll->header.vlrs[i].record_length_after_header)
          {
            laszip_dll->header.offset_to_point_data -= laszip_dll->header.vlrs[i].record_length_after_header;
            delete [] laszip_dll->header.vlrs[i].data;
          }
          break;
        }
      }

      // create new VLR

      if (i == laszip_dll->header.number_of_variable_length_records)
      {
        laszip_dll->header.number_of_variable_length_records++;
        laszip_dll->header.offset_to_point_data += 54;
        laszip_dll->header.vlrs = (laszip_vlr_struct*)realloc(laszip_dll->header.vlrs, sizeof(laszip_vlr_struct)*laszip_dll->header.number_of_variable_length_records);
        if (laszip_dll->header.vlrs == 0)
        {
          return FALSE;
        }
      }
    }
    else
    {
      laszip_dll->header.number_of_variable_length_records = 1;
      laszip_dll->header.offset_to_point_data += 54;
      laszip_dll->header.vlrs = (laszip_vlr_struct*)malloc(sizeof(laszip_vlr_struct));
      if (laszip_dll->header.vlrs == 0)
      {
        return FALSE;
      }
    }

    // zero the VLR

    memset(&(laszip_dll->header.vlrs[i]), 0, sizeof(laszip_vlr_struct));

    // copy the VLR

    laszip_dll->header.vlrs[i].reserved = vlr->reserved;
    strncpy(laszip_dll->header.vlrs[i].user_id, vlr->user_id, 16);
    laszip_dll->header.vlrs[i].record_id = vlr->record_id;
    laszip_dll->header.vlrs[i].record_length_after_header = vlr->record_length_after_header;
    strncpy(laszip_dll->header.vlrs[i].description, vlr->description, 32);
    if (vlr->record_length_after_header)
    {
      laszip_dll->header.offset_to_point_data += vlr->record_length_after_header;
      laszip_dll->header.vlrs[i].data = new U8[vlr->record_length_after_header];
      memcpy(laszip_dll->header.vlrs[i].data, vlr->data, vlr->record_length_after_header);
    }
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_add_vlr");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_open_writer(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL                      compress
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (file_name == 0)
    {
      sprintf(laszip_dll->error, "laszip_CHAR file_name pointer is zero");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "reader is already open");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "writer is already open");
      return 1;
    }

    // check header and prepare point

    U32 vlrs_size = 0;

    if (laszip_dll->header.version_major != 1) 
    {
      sprintf(laszip_dll->error, "unknown LAS version %d.%d", (I32)laszip_dll->header.version_major, (I32)laszip_dll->header.version_minor);
      return 1;
    }

    if (compress && (laszip_dll->header.point_data_format > 5)) 
    {
      sprintf(laszip_dll->error, "compressor does not yet support point data format %d", (I32)laszip_dll->header.point_data_format);
      return 1;
    }

    if (laszip_dll->header.number_of_variable_length_records)
    {
      if (laszip_dll->header.vlrs == 0)
      {
        sprintf(laszip_dll->error, "number_of_variable_length_records is %u but vlrs pointer is zero", laszip_dll->header.number_of_variable_length_records);
        return 1;
      }

      U32 i;
 
      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        vlrs_size += 54;
        if (laszip_dll->header.vlrs[i].record_length_after_header)
        {
          if (laszip_dll->header.vlrs == 0)
          {
            sprintf(laszip_dll->error, "vlrs[%u].record_length_after_header is %u but vlrs[%u].data pointer is zero", i, laszip_dll->header.vlrs[i].record_length_after_header, i);
            return 1;
          }
          vlrs_size += laszip_dll->header.vlrs[i].record_length_after_header;
        }
      }
    }

    if ((vlrs_size + laszip_dll->header.header_size + laszip_dll->header.user_data_after_header_size) != laszip_dll->header.offset_to_point_data)
    {
      sprintf(laszip_dll->error,"header_size (%u) plus vlrs_size (%u) plus user_data_after_header_size (%u) does not equal offset_to_point_data (%u)", (U32)laszip_dll->header.header_size, vlrs_size, laszip_dll->header.user_data_after_header_size, laszip_dll->header.offset_to_point_data);
      return 1;
    }

    LASzip* laszip = new LASzip;

    if (laszip == 0)
    {
      sprintf(laszip_dll->error, "could not alloc LASzip");
      return 1;
    }

    if (!laszip->setup(laszip_dll->header.point_data_format, laszip_dll->header.point_data_record_length, LASZIP_COMPRESSOR_NONE))
    {
      sprintf(laszip_dll->error, "invalid combination of point_data_format %d and point_data_record_length %d", (I32)laszip_dll->header.point_data_format, (I32)laszip_dll->header.point_data_record_length);
      return 1;
    }

    // create point's item pointers

    laszip_dll->point_items = new U8*[laszip->num_items];

    if (laszip_dll->point_items == 0)
    {
      sprintf(laszip_dll->error, "could not alloc point_items");
      return 1;
    }

    U32 i;
    for (i = 0; i < laszip->num_items; i++)
    {
      switch (laszip->items[i].type)
      {
      case LASitem::POINT14:
      case LASitem::POINT10:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.X);
        break;
      case LASitem::GPSTIME11:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.gps_time);
        break;
      case LASitem::RGBNIR14:
      case LASitem::RGB12:
        laszip_dll->point_items[i] = (U8*)laszip_dll->point.rgb;
        break;
      case LASitem::WAVEPACKET13:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.wave_packet);
        break;
      case LASitem::BYTE:
        laszip_dll->point.num_extra_bytes = laszip->items[i].size;
        if (laszip_dll->point.extra_bytes) delete [] laszip_dll->point.extra_bytes;
        laszip_dll->point.extra_bytes = new U8[laszip_dll->point.num_extra_bytes];
        laszip_dll->point_items[i] = laszip_dll->point.extra_bytes;
        break;
      default:
        sprintf(laszip_dll->error, "unknown LASitem type %d", (I32)laszip->items[i].type);
        return 1;
      }
    }

    U32 laszip_vrl_payload_size = 0;

    if (compress)
    {
      if (!laszip->setup(laszip_dll->header.point_data_format, laszip_dll->header.point_data_record_length, LASZIP_COMPRESSOR_DEFAULT))
      {
        sprintf(laszip_dll->error, "cannot compress point_data_format %d with point_data_record_length %d", (I32)laszip_dll->header.point_data_format, (I32)laszip_dll->header.point_data_record_length);
        return 1;
      }
      laszip->request_version(2);
      laszip_vrl_payload_size = 34 + 6*laszip->num_items;
    }
    else
    {
      laszip->request_version(0);
    }

    // open the file

    laszip_dll->file = fopen(file_name, "wb");

    if (laszip_dll->file == 0)
    {
      sprintf(laszip_dll->error, "cannot open file '%s'", file_name);
      return 1;
    }

    if (IS_LITTLE_ENDIAN())
      laszip_dll->streamout = new ByteStreamOutFileLE(laszip_dll->file);
    else
      laszip_dll->streamout = new ByteStreamOutFileBE(laszip_dll->file);

    if (laszip_dll->streamout == 0)
    {
      sprintf(laszip_dll->error, "could not alloc ByteStreamOutFile");
      return 1;
    }

    // write the header variable after variable

    try { laszip_dll->streamout->putBytes((U8*)"LASF", 4); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.file_signature");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.file_source_ID)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.file_source_ID");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.global_encoding)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.global_encoding");
      return 1;
    }
    try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_1)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.project_ID_GUID_data_1");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_2)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.project_ID_GUID_data_2");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_3)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.project_ID_GUID_data_3");
      return 1;
    }
    try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.project_ID_GUID_data_4, 8); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.project_ID_GUID_data_4");
      return 1;
    }
    try { laszip_dll->streamout->putBytes((U8*)&(laszip_dll->header.version_major), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.version_major");
      return 1;
    }
    try { laszip_dll->streamout->putBytes((U8*)&(laszip_dll->header.version_minor), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.version_minor");
      return 1;
    }
    try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.system_identifier, 32); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.system_identifier");
      return 1;
    }
    sprintf(laszip_dll->header.generating_software, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.generating_software, 32); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.generating_software");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.file_creation_day)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.file_creation_day");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.file_creation_year)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.file_creation_year");
      return 1;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.header_size)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.header_size");
      return 1;
    }
    if (compress)
    {
      laszip_dll->header.offset_to_point_data += (54 + laszip_vrl_payload_size);
    }
    try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.offset_to_point_data)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.offset_to_point_data");
      return 1;
    }
    if (compress)
    {
      laszip_dll->header.offset_to_point_data -= (54 + laszip_vrl_payload_size);
      laszip_dll->header.number_of_variable_length_records += 1;
    }
    try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.number_of_variable_length_records)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.number_of_variable_length_records");
      return 1;
    }
    if (compress)
    {
      laszip_dll->header.number_of_variable_length_records -= 1;
      laszip_dll->header.point_data_format |= 128;
    }
    try { laszip_dll->streamout->putBytes((U8*)&(laszip_dll->header.point_data_format), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.point_data_format");
      return 1;
    }
    if (compress)
    {
      laszip_dll->header.point_data_format &= 127;
    }
    try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.point_data_record_length)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.point_data_record_length");
      return 1;
    }
    try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.number_of_point_records)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.number_of_point_records");
      return 1;
    }
    for (i = 0; i < 5; i++)
    {
      try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.number_of_points_by_return[i])); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.number_of_points_by_return %d", i);
        return 1;
      }
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.x_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.x_scale_factor");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.y_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.y_scale_factor");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.z_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.z_scale_factor");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.x_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.x_offset");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.y_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.y_offset");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.z_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.z_offset");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.max_x)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.max_x");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.min_x)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.min_x");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.max_y)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.max_y");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.min_y)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.min_y");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.max_z)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.max_z");
      return 1;
    }
    try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.min_z)); } catch(...)
    {
      sprintf(laszip_dll->error, "writing header.min_z");
      return 1;
    }

    // special handling for LAS 1.3
    if ((laszip_dll->header.version_major == 1) && (laszip_dll->header.version_minor >= 3))
    {
      if (laszip_dll->header.header_size < 235)
      {
        sprintf(laszip_dll->error, "for LAS 1.%d header_size should at least be 235 but it is only %d", laszip_dll->header.version_minor, laszip_dll->header.header_size);
        return 1;
      }
      else
      {
        try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.start_of_waveform_data_packet_record)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.start_of_waveform_data_packet_record");
          return 1;
        }
        laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 235;
      }
    }
    else
    {
      laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 227;
    }

    // special handling for LAS 1.4
    if ((laszip_dll->header.version_major == 1) && (laszip_dll->header.version_minor >= 4))
    {
      if (laszip_dll->header.header_size < 375)
      {
        sprintf(laszip_dll->error, "for LAS 1.%d header_size should at least be 375 but it is only %d", laszip_dll->header.version_minor, laszip_dll->header.header_size);
        return 1;
      }
      else
      {
        try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.start_of_first_extended_variable_length_record)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.start_of_first_extended_variable_length_record");
          return 1;
        }
        try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip_dll->header.number_of_extended_variable_length_records)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.number_of_extended_variable_length_records");
          return 1;
        }
        try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.extended_number_of_point_records)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.extended_number_of_point_records");
          return 1;
        }
        for (i = 0; i < 15; i++)
        {
          try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip_dll->header.extended_number_of_points_by_return[i])); } catch(...)
          {
            sprintf(laszip_dll->error, "writing header.extended_number_of_points_by_return[%d]", i);
            return 1;
          }
        }
        laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 375;
      }
    }

    // write any number of user-defined bytes that might have been added to the header
    if (laszip_dll->header.user_data_in_header_size)
    {
      try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.user_data_in_header, laszip_dll->header.user_data_in_header_size); } catch(...)
      {
        sprintf(laszip_dll->error, "writing %d bytes of data into header.user_data_in_header", laszip_dll->header.user_data_in_header_size);
        return 1;
      }
    }

    // write variable length records into the header

    if (laszip_dll->header.number_of_variable_length_records)
    {
      U32 i;
 
      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        // write variable length records variable after variable (to avoid alignment issues)

        try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.vlrs[i].reserved)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.vlrs[%d].reserved", i);
          return 1;
        }

        try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.vlrs[i].user_id, 16); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.vlrs[%d].user_id", i);
          return 1;
        }
        try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.vlrs[i].record_id)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.vlrs[%d].record_id", i);
          return 1;
        }
        try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip_dll->header.vlrs[i].record_length_after_header)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.vlrs[%d].record_length_after_header", i);
          return 1;
        }
        try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.vlrs[i].description, 32); } catch(...)
        {
          sprintf(laszip_dll->error, "writing header.vlrs[%d].description", i);
          return 1;
        }

        // write data following the header of the variable length record

        if (laszip_dll->header.vlrs[i].record_length_after_header)
        {
          try { laszip_dll->streamout->putBytes(laszip_dll->header.vlrs[i].data, laszip_dll->header.vlrs[i].record_length_after_header); } catch(...)
          {
            sprintf(laszip_dll->error, "writing %d bytes of data into header.vlrs[%d].data", laszip_dll->header.vlrs[i].record_length_after_header, i);
            return 1;
          }
        }
      }
    }

    if (compress)
    {
      // write the LASzip VLR header

      U16 reserved = 0xAABB;
      try { laszip_dll->streamout->put16bitsLE((U8*)&reserved); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.vlrs[%d].reserved", i);
        return 1;
      }
      U8 user_id[16] = "laszip encoded\0";
      try { laszip_dll->streamout->putBytes((U8*)user_id, 16); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.vlrs[%d].user_id", i);
        return 1;
      }
      U16 record_id = 22204;
      try { laszip_dll->streamout->put16bitsLE((U8*)&record_id); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.vlrs[%d].record_id", i);
        return 1;
      }
      U16 record_length_after_header = laszip_vrl_payload_size;
      try { laszip_dll->streamout->put16bitsLE((U8*)&record_length_after_header); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.vlrs[%d].record_length_after_header", i);
        return 1;
      }
      CHAR description[32];
      memset(description, 0, 32);
      sprintf(description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
      try { laszip_dll->streamout->putBytes((U8*)description, 32); } catch(...)
      {
        sprintf(laszip_dll->error, "writing header.vlrs[%d].description", i);
        return 1;
      }

      // write the LASzip VLR payload

      //     U16  compressor                2 bytes 
      //     U32  coder                     2 bytes 
      //     U8   version_major             1 byte 
      //     U8   version_minor             1 byte
      //     U16  version_revision          2 bytes
      //     U32  options                   4 bytes 
      //     I32  chunk_size                4 bytes
      //     I64  number_of_special_evlrs   8 bytes
      //     I64  offset_to_special_evlrs   8 bytes
      //     U16  num_items                 2 bytes
      //        U16 type                2 bytes * num_items
      //        U16 size                2 bytes * num_items
      //        U16 version             2 bytes * num_items
      // which totals 34+6*num_items

      try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->compressor)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing compressor %d", (I32)laszip->compressor);
        return 1;
      }
      try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->coder)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing coder %d", (I32)laszip->coder);
        return 1;
      }
      try { laszip_dll->streamout->putBytes((U8*)&(laszip->version_major), 1); } catch(...)
      {
        sprintf(laszip_dll->error, "writing version_major %d", (I32)laszip->version_major);
        return 1;
      }
      try { laszip_dll->streamout->putBytes((U8*)&(laszip->version_minor), 1); } catch(...)
      {
        sprintf(laszip_dll->error, "writing version_minor %d", (I32)laszip->version_minor);
        return 1;
      }
      try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->version_revision)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing version_revision %d", (I32)laszip->version_revision);
        return 1;
      }
      try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip->options)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing options %u", laszip->options);
        return 1;
      }
      try { laszip_dll->streamout->put32bitsLE((U8*)&(laszip->chunk_size)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing chunk_size %u", laszip->chunk_size);
        return 1;
      }
      try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip->number_of_special_evlrs)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing number_of_special_evlrs %d", (I32)laszip->number_of_special_evlrs);
        return 1;
      }
      try { laszip_dll->streamout->put64bitsLE((U8*)&(laszip->offset_to_special_evlrs)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing offset_to_special_evlrs %d", (I32)laszip->offset_to_special_evlrs);
        return 1;
      }
      try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->num_items)); } catch(...)
      {
        sprintf(laszip_dll->error, "writing num_items %d", (I32)laszip->num_items);
        return 1;
      }

      U32 j;
      for (j = 0; j < laszip->num_items; j++)
      {
        U16 type = (U16)(laszip->items[j].type);
        try { laszip_dll->streamout->put16bitsLE((U8*)&type); } catch(...)
        {
          sprintf(laszip_dll->error, "writing type %d of item %d", (I32)laszip->items[j].type, j);
          return 1;
        }
        try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->items[j].size)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing size %d of item %d", (I32)laszip->items[j].size, j);
          return 1;
        }
        try { laszip_dll->streamout->put16bitsLE((U8*)&(laszip->items[j].version)); } catch(...)
        {
          sprintf(laszip_dll->error, "writing version %d of item %d", (I32)laszip->items[j].version, j);
          return 1;
        }
      }
    }


    // write any number of user-defined bytes that might have been added after the header

    if (laszip_dll->header.user_data_after_header_size)
    {
      try { laszip_dll->streamout->putBytes((U8*)laszip_dll->header.user_data_after_header, laszip_dll->header.user_data_after_header_size); } catch(...)
      {
        sprintf(laszip_dll->error, "writing %u bytes of data into header.user_data_after_header", laszip_dll->header.user_data_after_header_size);
        return 1;
      }
    }

    // create the point writer

    laszip_dll->writer = new LASwritePoint();
    if (laszip_dll->writer == 0)
    {
      sprintf(laszip_dll->error, "could not alloc LASwritePoint");
      return 1;
    }

    if (!laszip_dll->writer->setup(laszip->num_items, laszip->items, laszip))
    {
      sprintf(laszip_dll->error, "setup of LASwritePoint failed");
      return 1;
    }

    if (!laszip_dll->writer->init(laszip_dll->streamout))
    {
      sprintf(laszip_dll->error, "init of LASwritePoint failed");
      return 1;
    }

    delete laszip;

    // set the point number and point count

    laszip_dll->npoints = laszip_dll->header.number_of_point_records;
    laszip_dll->p_count = 0;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_open_writer '%s'", file_name);
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_write_point(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (laszip_dll->writer == 0)
    {
      sprintf(laszip_dll->error, "writing points before writer was opened");
      return 1;
    }

    // write the point
    if (!laszip_dll->writer->write(laszip_dll->point_items))
    {
#ifdef _WIN32
      sprintf(laszip_dll->error, "writing point with index %I64d of %I64d total points", laszip_dll->p_count, laszip_dll->npoints);
#else
      sprintf(laszip_dll->error, "writing point with index %lld of %lld total points", laszip_dll->p_count, laszip_dll->npoints);
#endif
      return 1;
    }

    laszip_dll->p_count++;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_write_point");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_close_writer(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (laszip_dll->writer == 0)
    {
      sprintf(laszip_dll->error, "closing writer before it was opened");
      return 1;
    }

    if (!laszip_dll->writer->done())
    {
      sprintf(laszip_dll->error, "done of LASwritePoint failed");
      return 1;
    }

    delete laszip_dll->writer;
    laszip_dll->writer = 0;

    delete [] laszip_dll->point_items;
    laszip_dll->point_items = 0;

    delete laszip_dll->streamout;
    laszip_dll->streamout = 0;

    fclose(laszip_dll->file);
    laszip_dll->file = 0;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_writer_close");
    return 1;
  }
   
  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_open_reader(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL*                     is_compressed
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (file_name == 0)
    {
      sprintf(laszip_dll->error, "file_name pointer is zero");
      return 1;
    }

    if (is_compressed == 0)
    {
      sprintf(laszip_dll->error, "is_compressed pointer is zero");
      return 1;
    }

    if (laszip_dll->writer)
    {
      sprintf(laszip_dll->error, "writer is already open");
      return 1;
    }

    if (laszip_dll->reader)
    {
      sprintf(laszip_dll->error, "reader is already open");
      return 1;
    }

    // open the file

    laszip_dll->file = fopen(file_name, "rb");

    if (laszip_dll->file == 0)
    {
      sprintf(laszip_dll->error, "cannot open file '%s'", file_name);
      return 1;
    }

    if (IS_LITTLE_ENDIAN())
      laszip_dll->streamin = new ByteStreamInFileLE(laszip_dll->file);
    else
      laszip_dll->streamin = new ByteStreamInFileBE(laszip_dll->file);

    if (laszip_dll->streamin == 0)
    {
      sprintf(laszip_dll->error, "could not alloc ByteStreamInFile");
      return 1;
    }

    // read the header variable after variable

    U32 i;

    CHAR file_signature[5];
    try { laszip_dll->streamin->getBytes((U8*)file_signature, 4); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.file_signature");
      return 1;
    }
    if (strncmp(file_signature, "LASF", 4) != 0)
    {
      sprintf(laszip_dll->error, "wrong file_signature. not a LAS/LAZ file.");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.file_source_ID)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.file_source_ID");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.global_encoding)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.global_encoding");
      return 1;
    }
    try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_1)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.project_ID_GUID_data_1");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_2)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.project_ID_GUID_data_2");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.project_ID_GUID_data_3)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.project_ID_GUID_data_3");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.project_ID_GUID_data_4, 8); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.project_ID_GUID_data_4");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)&(laszip_dll->header.version_major), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.version_major");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)&(laszip_dll->header.version_minor), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.version_minor");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.system_identifier, 32); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.system_identifier");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.generating_software, 32); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.generating_software");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.file_creation_day)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.file_creation_day");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.file_creation_year)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.file_creation_year");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.header_size)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.header_size");
      return 1;
    }
    try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.offset_to_point_data)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.offset_to_point_data");
      return 1;
    }
    try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.number_of_variable_length_records)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.number_of_variable_length_records");
      return 1;
    }
    try { laszip_dll->streamin->getBytes((U8*)&(laszip_dll->header.point_data_format), 1); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.point_data_format");
      return 1;
    }
    try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.point_data_record_length)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.point_data_record_length");
      return 1;
    }
    try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.number_of_point_records)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.number_of_point_records");
      return 1;
    }
    for (i = 0; i < 5; i++)
    {
      try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.number_of_points_by_return[i])); } catch(...)
      {
        sprintf(laszip_dll->error, "reading header.number_of_points_by_return %d", i);
        return 1;
      }
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.x_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.x_scale_factor");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.y_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.y_scale_factor");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.z_scale_factor)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.z_scale_factor");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.x_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.x_offset");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.y_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.y_offset");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.z_offset)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.z_offset");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.max_x)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.max_x");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.min_x)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.min_x");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.max_y)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.max_y");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.min_y)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.min_y");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.max_z)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.max_z");
      return 1;
    }
    try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.min_z)); } catch(...)
    {
      sprintf(laszip_dll->error, "reading header.min_z");
      return 1;
    }

    // special handling for LAS 1.3
    if ((laszip_dll->header.version_major == 1) && (laszip_dll->header.version_minor >= 3))
    {
      if (laszip_dll->header.header_size < 235)
      {
        sprintf(laszip_dll->error, "for LAS 1.%d header_size should at least be 235 but it is only %d", laszip_dll->header.version_minor, laszip_dll->header.header_size);
        return 1;
      }
      else
      {
        try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.start_of_waveform_data_packet_record)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.start_of_waveform_data_packet_record");
          return 1;
        }
        laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 235;
      }
    }
    else
    {
      laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 227;
    }

    // special handling for LAS 1.4
    if ((laszip_dll->header.version_major == 1) && (laszip_dll->header.version_minor >= 4))
    {
      if (laszip_dll->header.header_size < 375)
      {
        sprintf(laszip_dll->error, "for LAS 1.%d header_size should at least be 375 but it is only %d", laszip_dll->header.version_minor, laszip_dll->header.header_size);
        return 1;
      }
      else
      {
        try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.start_of_first_extended_variable_length_record)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.start_of_first_extended_variable_length_record");
          return 1;
        }
        try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip_dll->header.number_of_extended_variable_length_records)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.number_of_extended_variable_length_records");
          return 1;
        }
        try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.extended_number_of_point_records)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.extended_number_of_point_records");
          return 1;
        }
        for (i = 0; i < 15; i++)
        {
          try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip_dll->header.extended_number_of_points_by_return[i])); } catch(...)
          {
            sprintf(laszip_dll->error, "reading header.extended_number_of_points_by_return[%d]", i);
            return 1;
          }
        }
        laszip_dll->header.user_data_in_header_size = laszip_dll->header.header_size - 375;
      }
    }

    // load any number of user-defined bytes that might have been added to the header
    if (laszip_dll->header.user_data_in_header_size)
    {
      if (laszip_dll->header.user_data_in_header)
      {
        delete [] laszip_dll->header.user_data_in_header;
      }
      laszip_dll->header.user_data_in_header = new U8[laszip_dll->header.user_data_in_header_size];

      try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.user_data_in_header, laszip_dll->header.user_data_in_header_size); } catch(...)
      {
        sprintf(laszip_dll->error, "reading %u bytes of data into header.user_data_in_header", laszip_dll->header.user_data_in_header_size);
        return 1;
      }
    }

    // read variable length records into the header

    U32 vlrs_size = 0;
    LASzip* laszip = 0;

    if (laszip_dll->header.number_of_variable_length_records)
    {
      U32 i;

      laszip_dll->header.vlrs = (laszip_vlr*)malloc(sizeof(laszip_vlr)*laszip_dll->header.number_of_variable_length_records);
 
      for (i = 0; i < laszip_dll->header.number_of_variable_length_records; i++)
      {
        // make sure there are enough bytes left to read a variable length record before the point block starts

        if (((int)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size) < 54)
        {
          sprintf(laszip_dll->warning, "only %d bytes until point block after reading %d of %d vlrs. skipping remaining vlrs ...", (int)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size, i, laszip_dll->header.number_of_variable_length_records);
          laszip_dll->header.number_of_variable_length_records = i;
          break;
        }

        // read variable length records variable after variable (to avoid alignment issues)

        try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.vlrs[i].reserved)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.vlrs[%u].reserved", i);
          return 1;
        }

        try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.vlrs[i].user_id, 16); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.vlrs[%u].user_id", i);
          return 1;
        }
        try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.vlrs[i].record_id)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.vlrs[%u].record_id", i);
          return 1;
        }
        try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip_dll->header.vlrs[i].record_length_after_header)); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.vlrs[%u].record_length_after_header", i);
          return 1;
        }
        try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.vlrs[i].description, 32); } catch(...)
        {
          sprintf(laszip_dll->error, "reading header.vlrs[%u].description", i);
          return 1;
        }

        // keep track on the number of bytes we have read so far

        vlrs_size += 54;

        // check variable length record contents

        if (laszip_dll->header.vlrs[i].reserved != 0xAABB)
        {
          sprintf(laszip_dll->warning,"wrong header.vlrs[%d].reserved: %d != 0xAABB", i, laszip_dll->header.vlrs[i].reserved);
        }

        // make sure there are enough bytes left to read the data of the variable length record before the point block starts

        if (((int)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size) < laszip_dll->header.vlrs[i].record_length_after_header)
        {
          sprintf(laszip_dll->warning, "only %d bytes until point block when trying to read %d bytes into header.vlrs[%d].data", (int)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size, laszip_dll->header.vlrs[i].record_length_after_header, i);
          laszip_dll->header.vlrs[i].record_length_after_header = (int)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size;
        }

        // load data following the header of the variable length record

        if (laszip_dll->header.vlrs[i].record_length_after_header)
        {
          if (strcmp(laszip_dll->header.vlrs[i].user_id, "laszip encoded") == 0)
          {
            if (laszip)
            {
              delete laszip;
            }

            laszip = new LASzip();

            if (laszip == 0)
            {
              sprintf(laszip_dll->error, "could not alloc LASzip");
              return 1;
            }

            // read the LASzip VLR payload

            //     U16  compressor                2 bytes 
            //     U32  coder                     2 bytes 
            //     U8   version_major             1 byte 
            //     U8   version_minor             1 byte
            //     U16  version_revision          2 bytes
            //     U32  options                   4 bytes 
            //     I32  chunk_size                4 bytes
            //     I64  number_of_special_evlrs   8 bytes
            //     I64  offset_to_special_evlrs   8 bytes
            //     U16  num_items                 2 bytes
            //        U16 type                2 bytes * num_items
            //        U16 size                2 bytes * num_items
            //        U16 version             2 bytes * num_items
            // which totals 34+6*num_items

            try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->compressor)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading compressor %d", (I32)laszip->compressor);
              return 1;
            }
            try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->coder)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading coder %d", (I32)laszip->coder);
              return 1;
            }
            try { laszip_dll->streamin->getBytes((U8*)&(laszip->version_major), 1); } catch(...)
            {
              sprintf(laszip_dll->error, "reading version_major %d", (I32)laszip->version_major);
              return 1;
            }
            try { laszip_dll->streamin->getBytes((U8*)&(laszip->version_minor), 1); } catch(...)
            {
              sprintf(laszip_dll->error, "reading version_minor %d", (I32)laszip->version_minor);
              return 1;
            }
            try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->version_revision)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading version_revision %d", (I32)laszip->version_revision);
              return 1;
            }
            try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip->options)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading options %u", laszip->options);
              return 1;
            }
            try { laszip_dll->streamin->get32bitsLE((U8*)&(laszip->chunk_size)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading chunk_size %u", laszip->chunk_size);
              return 1;
            }
            try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip->number_of_special_evlrs)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading number_of_special_evlrs %d", (I32)laszip->number_of_special_evlrs);
              return 1;
            }
            try { laszip_dll->streamin->get64bitsLE((U8*)&(laszip->offset_to_special_evlrs)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading offset_to_special_evlrs %d", (I32)laszip->offset_to_special_evlrs);
              return 1;
            }
            try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->num_items)); } catch(...)
            {
              sprintf(laszip_dll->error, "reading num_items %d", (I32)laszip->num_items);
              return 1;
            }
            laszip->items = new LASitem[laszip->num_items];
            U32 j;
            for (j = 0; j < laszip->num_items; j++)
            {
              U16 type;
              try { laszip_dll->streamin->get16bitsLE((U8*)&type); } catch(...)
              {
                sprintf(laszip_dll->error, "reading type of item %u", j);
                return 1;
              }
              laszip->items[j].type = (LASitem::Type)type;
              try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->items[j].size)); } catch(...)
              {
                sprintf(laszip_dll->error, "reading size of item %u", j);
                return 1;
              }
              try { laszip_dll->streamin->get16bitsLE((U8*)&(laszip->items[j].version)); } catch(...)
              {
                sprintf(laszip_dll->error, "reading version of item %u", j);
                return 1;
              }
            }
          }
          else
          {
            laszip_dll->header.vlrs[i].data = new U8[laszip_dll->header.vlrs[i].record_length_after_header];

            try { laszip_dll->streamin->getBytes(laszip_dll->header.vlrs[i].data, laszip_dll->header.vlrs[i].record_length_after_header); } catch(...)
            {
              sprintf(laszip_dll->error, "reading %d bytes of data into header.vlrs[%u].data", (I32)laszip_dll->header.vlrs[i].record_length_after_header, i);
              return 1;
            }
          }
        }
        else
        {
          laszip_dll->header.vlrs[i].data = 0;
        }

        // keep track on the number of bytes we have read so far

        vlrs_size += laszip_dll->header.vlrs[i].record_length_after_header;

        // special handling for LASzip VLR

        if (strcmp(laszip_dll->header.vlrs[i].user_id, "laszip encoded") == 0)
        {
          // we take our the VLR for LASzip away
          laszip_dll->header.offset_to_point_data -= (54+laszip_dll->header.vlrs[i].record_length_after_header);
          vlrs_size -= (54+laszip_dll->header.vlrs[i].record_length_after_header);
          i--;
          laszip_dll->header.number_of_variable_length_records--;
        }
      }
    }

    // load any number of user-defined bytes that might have been added after the header

    laszip_dll->header.user_data_after_header_size = (I32)laszip_dll->header.offset_to_point_data - vlrs_size - laszip_dll->header.header_size;
    if (laszip_dll->header.user_data_after_header_size)
    {
      if (laszip_dll->header.user_data_after_header)
      {
        delete [] laszip_dll->header.user_data_after_header;
      }
      laszip_dll->header.user_data_after_header = new U8[laszip_dll->header.user_data_after_header_size];

      try { laszip_dll->streamin->getBytes((U8*)laszip_dll->header.user_data_after_header, laszip_dll->header.user_data_after_header_size); } catch(...)
      {
        sprintf(laszip_dll->error, "reading %u bytes of data into header.user_data_after_header", laszip_dll->header.user_data_after_header_size);
        return 1;
      }
    }

    // remove extra bits in point data type

    if ((laszip_dll->header.point_data_format & 128) || (laszip_dll->header.point_data_format & 64)) 
    {
      if (!laszip)
      {
        sprintf(laszip_dll->error, "this file was compressed with an experimental version of LASzip. contact 'martin.isenburg@rapidlasso.com' for assistance");
        return 1;
      }
      laszip_dll->header.point_data_format &= 127;
    }

    // check if file is compressed

    if (laszip)
    {
      // yes. check the compressor state
      *is_compressed = 1;
      if (!laszip->check())
      {
        sprintf(laszip_dll->error, "%s upgrade to the latest release of LAStools (with LASzip) or contact 'martin.isenburg@rapidlasso.com' for assistance", laszip->get_error());
        return 1;
      }      
    }
    else
    {
      // no. setup an un-compressed read
      *is_compressed = 0;
      laszip = new LASzip;
      if (laszip == 0)
      {
        sprintf(laszip_dll->error, "could not alloc LASzip");
        return 1;
      }
      if (!laszip->setup(laszip_dll->header.point_data_format, laszip_dll->header.point_data_record_length, LASZIP_COMPRESSOR_NONE))
      {
        sprintf(laszip_dll->error, "invalid combination of point_data_format %d and point_data_record_length %d", (I32)laszip_dll->header.point_data_format, (I32)laszip_dll->header.point_data_record_length);
        return 1;
      }
    }

    // create point's item pointers

    laszip_dll->point_items = new U8*[laszip->num_items];

    if (laszip_dll->point_items == 0)
    {
      sprintf(laszip_dll->error, "could not alloc point_items");
      return 1;
    }

    for (i = 0; i < laszip->num_items; i++)
    {
      switch (laszip->items[i].type)
      {
      case LASitem::POINT14:
      case LASitem::POINT10:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.X);
        break;
      case LASitem::GPSTIME11:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.gps_time);
        break;
      case LASitem::RGBNIR14:
      case LASitem::RGB12:
        laszip_dll->point_items[i] = (U8*)laszip_dll->point.rgb;
        break;
      case LASitem::WAVEPACKET13:
        laszip_dll->point_items[i] = (U8*)&(laszip_dll->point.wave_packet);
        break;
      case LASitem::BYTE:
        laszip_dll->point.num_extra_bytes = laszip->items[i].size;
        if (laszip_dll->point.extra_bytes) delete [] laszip_dll->point.extra_bytes;
        laszip_dll->point.extra_bytes = new U8[laszip_dll->point.num_extra_bytes];
        laszip_dll->point_items[i] = laszip_dll->point.extra_bytes;
        break;
      default:
        sprintf(laszip_dll->error, "unknown LASitem type %d", (I32)laszip->items[i].type);
        return 1;
      }
    }

    // create the point reader

    laszip_dll->reader = new LASreadPoint();
    if (laszip_dll->reader == 0)
    {
      sprintf(laszip_dll->error, "could not alloc LASreadPoint");
      return 1;
    }

    if (!laszip_dll->reader->setup(laszip->num_items, laszip->items, laszip))
    {
      sprintf(laszip_dll->error, "setup of LASreadPoint failed");
      return 1;
    }

    if (!laszip_dll->reader->init(laszip_dll->streamin))
    {
      sprintf(laszip_dll->error, "init of LASreadPoint failed");
      return 1;
    }

    delete laszip;

    // set the point number and point count

    laszip_dll->npoints = laszip_dll->header.number_of_point_records;
    laszip_dll->p_count = 0;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_open_reader");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_seek_point(
    laszip_POINTER                     pointer
    , laszip_I64                       index
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    // seek to the point
    if (!laszip_dll->reader->seek((U32)laszip_dll->p_count, (U32)index))
    {
#ifdef _WIN32
      sprintf(laszip_dll->error, "seeking from index %I64d to index %I64d for file with %I64d points", laszip_dll->p_count, index, laszip_dll->npoints);
#else
      sprintf(laszip_dll->error, "seeking from index %lld to index %lld for file with %lld points", laszip_dll->p_count, index, laszip_dll->npoints);
#endif
      return 1;
    }
    laszip_dll->p_count = index;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_seek_point");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_read_point(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    // read the point
    if (!laszip_dll->reader->read(laszip_dll->point_items))
    {
#ifdef _WIN32
      sprintf(laszip_dll->error, "reading point with index %I64d of %I64d total points", laszip_dll->p_count, laszip_dll->npoints);
#else
      sprintf(laszip_dll->error, "reading point with index %lld of %lld total points", laszip_dll->p_count, laszip_dll->npoints);
#endif
      return 1;
    }
    
    laszip_dll->p_count++;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_read_point");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
LASZIP_API laszip_I32
laszip_close_reader(
    laszip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  laszip_dll_struct* laszip_dll = (laszip_dll_struct*)pointer;

  try
  {
    if (laszip_dll->reader == 0)
    {
      sprintf(laszip_dll->error, "closing reader before it was opened");
      return 1;
    }

    if (!laszip_dll->reader->done())
    {
      sprintf(laszip_dll->error, "done of LASreadPoint failed");
      return 1;
    }

    delete laszip_dll->reader;
    laszip_dll->reader = 0;

    delete [] laszip_dll->point_items;
    laszip_dll->point_items = 0;

    delete laszip_dll->streamin;
    laszip_dll->streamin = 0;

    fclose(laszip_dll->file);
    laszip_dll->file = 0;
  }
  catch (...)
  {
    sprintf(laszip_dll->error, "internal error in laszip_close_reader");
    return 1;
  }

  laszip_dll->error[0] = '\0';
  return 0;
}
