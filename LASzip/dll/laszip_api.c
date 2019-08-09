/*
===============================================================================

  FILE:  laszip_api.c

  CONTENTS:

    A simple set of linkable function signatures for the DLL of LASzip

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see header file

===============================================================================
*/

#include "laszip_api.h"

// DLL function definitions

#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
/*---------------- DLL functions to manage the LASzip DLL -------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_version_def)
(
    laszip_U8*                         version_major
    , laszip_U8*                       version_minor
    , laszip_U16*                      version_revision
    , laszip_U32*                      version_build
);
laszip_get_version_def laszip_get_version_ptr = 0;
LASZIP_API laszip_I32
laszip_get_version
(
    laszip_U8*                         version_major
    , laszip_U8*                       version_minor
    , laszip_U16*                      version_revision
    , laszip_U32*                      version_build
)
{
  if (laszip_get_version_ptr)
  {
    return (*laszip_get_version_ptr)(version_major, version_minor, version_revision, version_build);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_create_def)
(
    laszip_POINTER*                    pointer
);
laszip_create_def laszip_create_ptr = 0;
LASZIP_API laszip_I32
laszip_create
(
    laszip_POINTER*                    pointer
)
{
  if (laszip_create_ptr)
  {
    return (*laszip_create_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_clean_def)
(
    laszip_POINTER                     pointer
);
laszip_clean_def laszip_clean_ptr = 0;
LASZIP_API laszip_I32
laszip_clean
(
    laszip_POINTER                     pointer
)
{
  if (laszip_clean_ptr)
  {
    return (*laszip_clean_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_error_def)
(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    error
);
laszip_get_error_def laszip_get_error_ptr = 0;
LASZIP_API laszip_I32
laszip_get_error
(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    error
)
{
  if (laszip_get_error_ptr)
  {
    return (*laszip_get_error_ptr)(pointer, error);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_warning_def)
(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    warning
);
laszip_get_warning_def laszip_get_warning_ptr = 0;
LASZIP_API laszip_I32
laszip_get_warning
(
    laszip_POINTER                     pointer
    , laszip_CHAR**                    warning
)
{
  if (laszip_get_warning_ptr)
  {
    return (*laszip_get_warning_ptr)(pointer, warning);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_destroy_def)
(
    laszip_POINTER       pointer
);
laszip_destroy_def laszip_destroy_ptr = 0;
LASZIP_API laszip_I32
laszip_destroy
(
    laszip_POINTER       pointer
)
{
  if (laszip_destroy_ptr)
  {
    return (*laszip_destroy_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
/*---------- DLL functions to write and read LAS and LAZ files --------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_header_pointer_def)
(
    laszip_POINTER                     pointer
    , laszip_header_struct**           header_pointer
);
laszip_get_header_pointer_def laszip_get_header_pointer_ptr = 0;
LASZIP_API laszip_I32
laszip_get_header_pointer
(
    laszip_POINTER                     pointer
    , laszip_header_struct**           header_pointer
)
{
  if (laszip_get_header_pointer_ptr)
  {
    return (*laszip_get_header_pointer_ptr)(pointer, header_pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_point_pointer_def)
(
    laszip_POINTER                     pointer
    , laszip_point_struct**            point_pointer
);
laszip_get_point_pointer_def laszip_get_point_pointer_ptr = 0;
LASZIP_API laszip_I32
laszip_get_point_pointer
(
    laszip_POINTER                     pointer
    , laszip_point_struct**            point_pointer
)
{
  if (laszip_get_point_pointer_ptr)
  {
    return (*laszip_get_point_pointer_ptr)(pointer, point_pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_point_count_def)
(
    laszip_POINTER                     pointer
    , laszip_I64*                      point_count
);
laszip_get_point_count_def laszip_get_point_count_ptr = 0;
LASZIP_API laszip_I32
laszip_get_point_count
(
    laszip_POINTER                     pointer
    , laszip_I64*                      point_count
)
{
  if (laszip_get_point_count_ptr)
  {
    return (*laszip_get_point_count_ptr)(pointer, point_count);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_header_def)
(
    laszip_POINTER                     pointer
    , const laszip_header_struct*      header
);
laszip_set_header_def laszip_set_header_ptr = 0;
LASZIP_API laszip_I32
laszip_set_header
(
    laszip_POINTER                     pointer
    , const laszip_header_struct*      header
)
{
  if (laszip_set_header_ptr)
  {
    return (*laszip_set_header_ptr)(pointer, header);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_point_type_and_size_def)
(
    laszip_POINTER                     pointer
    , laszip_U8                        point_type
    , laszip_U16                       point_size
);
laszip_set_point_type_and_size_def laszip_set_point_type_and_size_ptr = 0;
LASZIP_API laszip_I32
laszip_set_point_type_and_size
(
    laszip_POINTER                     pointer
    , laszip_U8                        point_type
    , laszip_U16                       point_size
)
{
  if (laszip_set_point_type_and_size_ptr)
  {
    return (*laszip_set_point_type_and_size_ptr)(pointer, point_type, point_size);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_check_for_integer_overflow_def)
(
    laszip_POINTER                     pointer
);
laszip_check_for_integer_overflow_def laszip_check_for_integer_overflow_ptr = 0;
LASZIP_API laszip_I32
laszip_check_for_integer_overflow
(
    laszip_POINTER                     pointer
)
{
  if (laszip_check_for_integer_overflow_ptr)
  {
    return (*laszip_check_for_integer_overflow_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_auto_offset_def)
(
    laszip_POINTER                     pointer
);
laszip_auto_offset_def laszip_auto_offset_ptr = 0;
LASZIP_API laszip_I32
laszip_auto_offset
(
    laszip_POINTER                     pointer
)
{
  if (laszip_auto_offset_ptr)
  {
    return (*laszip_auto_offset_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_point_def)
(
    laszip_POINTER                     pointer
    , const laszip_point_struct*       point
);
laszip_set_point_def laszip_set_point_ptr = 0;
LASZIP_API laszip_I32
laszip_set_point
(
    laszip_POINTER                     pointer
    , const laszip_point_struct*       point
)
{
  if (laszip_set_point_ptr)
  {
    return (*laszip_set_point_ptr)(pointer, point);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_coordinates_def)
(
    laszip_POINTER                     pointer
    , const laszip_F64*                coordinates
);
laszip_set_coordinates_def laszip_set_coordinates_ptr = 0;
LASZIP_API laszip_I32
laszip_set_coordinates
(
    laszip_POINTER                     pointer
    , const laszip_F64*                coordinates
)
{
  if (laszip_set_coordinates_ptr)
  {
    return (*laszip_set_coordinates_ptr)(pointer, coordinates);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_get_coordinates_def)
(
    laszip_POINTER                     pointer
    , laszip_F64*                      coordinates
);
laszip_get_coordinates_def laszip_get_coordinates_ptr = 0;
LASZIP_API laszip_I32
laszip_get_coordinates
(
    laszip_POINTER                     pointer
    , laszip_F64*                      coordinates
)
{
  if (laszip_get_coordinates_ptr)
  {
    return (*laszip_get_coordinates_ptr)(pointer, coordinates);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_geokeys_def)
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_geokey_struct*      key_entries
);
laszip_set_geokeys_def laszip_set_geokeys_ptr = 0;
LASZIP_API laszip_I32
laszip_set_geokeys
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_geokey_struct*      key_entries
)
{
  if (laszip_set_geokeys_ptr)
  {
    return (*laszip_set_geokeys_ptr)(pointer, number, key_entries);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_geodouble_params_def)
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_F64*                geodouble_params
);
laszip_set_geodouble_params_def laszip_set_geodouble_params_ptr = 0;
LASZIP_API laszip_I32
laszip_set_geodouble_params
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_F64*                geodouble_params
)
{
  if (laszip_set_geodouble_params_ptr)
  {
    return (*laszip_set_geodouble_params_ptr)(pointer, number, geodouble_params);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_geoascii_params_def)
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_CHAR*               geoascii_params
);
laszip_set_geoascii_params_def laszip_set_geoascii_params_ptr = 0;
LASZIP_API laszip_I32
laszip_set_geoascii_params
(
    laszip_POINTER                     pointer
    , laszip_U32                       number
    , const laszip_CHAR*               geoascii_params
)
{
  if (laszip_set_geoascii_params_ptr)
  {
    return (*laszip_set_geoascii_params_ptr)(pointer, number, geoascii_params);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_add_attribute_def)
(
    laszip_POINTER                     pointer
    , laszip_U32                       type
    , const laszip_CHAR*               name
    , const laszip_CHAR*               description
    , laszip_F64                       scale
    , laszip_F64                       offset
);
laszip_add_attribute_def laszip_add_attribute_ptr = 0;
LASZIP_API laszip_I32
laszip_add_attribute
(
    laszip_POINTER                     pointer
    , laszip_U32                       type
    , const laszip_CHAR*               name
    , const laszip_CHAR*               description
    , laszip_F64                       scale
    , laszip_F64                       offset
)
{
  if (laszip_add_attribute_ptr)
  {
    return (*laszip_add_attribute_ptr)(pointer, type, name, description, scale, offset);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_add_vlr_def)
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               user_id
    , laszip_U16                       record_id
    , laszip_U16                       record_length_after_header
    , const laszip_CHAR*               description
    , const laszip_U8*                 data
);
laszip_add_vlr_def laszip_add_vlr_ptr = 0;
LASZIP_API laszip_I32
laszip_add_vlr
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               user_id
    , laszip_U16                       record_id
    , laszip_U16                       record_length_after_header
    , const laszip_CHAR*               description
    , const laszip_U8*                 data
)
{
  if (laszip_add_vlr_ptr)
  {
    return (*laszip_add_vlr_ptr)(pointer, user_id, record_id, record_length_after_header, description, data);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_remove_vlr_def)
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               user_id
    , laszip_U16                       record_id
);
laszip_remove_vlr_def laszip_remove_vlr_ptr = 0;
LASZIP_API laszip_I32
laszip_remove_vlr
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               user_id
    , laszip_U16                       record_id
)
{
  if (laszip_remove_vlr_ptr)
  {
    return (*laszip_remove_vlr_ptr)(pointer, user_id, record_id);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_create_spatial_index_def)
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                create
    , const laszip_BOOL                append
);
laszip_create_spatial_index_def laszip_create_spatial_index_ptr = 0;
LASZIP_API laszip_I32
laszip_create_spatial_index
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                create
    , const laszip_BOOL                append
)
{
  if (laszip_create_spatial_index_ptr)
  {
    return (*laszip_create_spatial_index_ptr)(pointer, create, append);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_preserve_generating_software_def)
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                preserve
);
laszip_preserve_generating_software_def laszip_preserve_generating_software_ptr = 0;
LASZIP_API laszip_I32
laszip_preserve_generating_software
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                preserve
)
{
  if (laszip_preserve_generating_software_ptr)
  {
    return (*laszip_preserve_generating_software_ptr)(pointer, preserve);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_request_native_extension_def)
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                request
);
laszip_request_native_extension_def laszip_request_native_extension_ptr = 0;
LASZIP_API laszip_I32
laszip_request_native_extension
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                request
)
{
  if (laszip_request_native_extension_ptr)
  {
    return (*laszip_request_native_extension_ptr)(pointer, request);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_request_compatibility_mode_def)
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                request
);
laszip_request_compatibility_mode_def laszip_request_compatibility_mode_ptr = 0;
LASZIP_API laszip_I32
laszip_request_compatibility_mode
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                request
)
{
  if (laszip_request_compatibility_mode_ptr)
  {
    return (*laszip_request_compatibility_mode_ptr)(pointer, request);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_set_chunk_size_def)
(
    laszip_POINTER                     pointer
    , const laszip_U32                 chunk_size
);
laszip_set_chunk_size_def laszip_set_chunk_size_ptr = 0;
LASZIP_API laszip_I32
laszip_set_chunk_size
(
    laszip_POINTER                     pointer
    , const laszip_U32                 chunk_size
)
{
  if (laszip_set_chunk_size_ptr)
  {
    return (*laszip_set_chunk_size_ptr)(pointer, chunk_size);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_open_writer_def)
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL                      compress
);
laszip_open_writer_def laszip_open_writer_ptr = 0;
LASZIP_API laszip_I32
laszip_open_writer
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL                      compress
)
{
  if (laszip_open_writer_ptr)
  {
    return (*laszip_open_writer_ptr)(pointer, file_name, compress);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_write_point_def)
(
    laszip_POINTER                     pointer
);
laszip_write_point_def laszip_write_point_ptr = 0;
LASZIP_API laszip_I32
laszip_write_point
(
    laszip_POINTER                     pointer
)
{
  if (laszip_write_point_ptr)
  {
    return (*laszip_write_point_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_write_indexed_point_def)
(
    laszip_POINTER                     pointer
);
laszip_write_indexed_point_def laszip_write_indexed_point_ptr = 0;
LASZIP_API laszip_I32
laszip_write_indexed_point
(
    laszip_POINTER                     pointer
)
{
  if (laszip_write_indexed_point_ptr)
  {
    return (*laszip_write_indexed_point_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_update_inventory_def)
(
    laszip_POINTER                     pointer
);
laszip_update_inventory_def laszip_update_inventory_ptr = 0;
LASZIP_API laszip_I32
laszip_update_inventory
(
    laszip_POINTER                     pointer
)
{
  if (laszip_update_inventory_ptr)
  {
    return (*laszip_update_inventory_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_close_writer_def)
(
    laszip_POINTER                     pointer
);
laszip_close_writer_def laszip_close_writer_ptr = 0;
LASZIP_API laszip_I32
laszip_close_writer
(
    laszip_POINTER                     pointer
)
{
  if (laszip_close_writer_ptr)
  {
    return (*laszip_close_writer_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_exploit_spatial_index_def)
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                exploit
);
laszip_exploit_spatial_index_def laszip_exploit_spatial_index_ptr = 0;
LASZIP_API laszip_I32
laszip_exploit_spatial_index
(
    laszip_POINTER                     pointer
    , const laszip_BOOL                exploit
)
{
  if (laszip_exploit_spatial_index_ptr)
  {
    return (*laszip_exploit_spatial_index_ptr)(pointer, exploit);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_decompress_selective_def)
(
    laszip_POINTER                     pointer
    , const laszip_U32                 decompress_selective
);
laszip_decompress_selective_def laszip_decompress_selective_ptr = 0;
LASZIP_API laszip_I32
laszip_decompress_selective
(
    laszip_POINTER                     pointer
    , const laszip_U32                 decompress_selective
)
{
  if (laszip_decompress_selective_ptr)
  {
    return (*laszip_decompress_selective_ptr)(pointer, decompress_selective);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_open_reader_def)
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL*                     is_compressed
);
laszip_open_reader_def laszip_open_reader_ptr = 0;
LASZIP_API laszip_I32
laszip_open_reader
(
    laszip_POINTER                     pointer
    , const laszip_CHAR*               file_name
    , laszip_BOOL*                     is_compressed
)
{
  if (laszip_open_reader_ptr)
  {
    return (*laszip_open_reader_ptr)(pointer, file_name, is_compressed);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_has_spatial_index_def)
(
    laszip_POINTER                     pointer
    , laszip_BOOL*                     is_compressed
    , laszip_BOOL*                     is_appended
);
laszip_has_spatial_index_def laszip_has_spatial_index_ptr = 0;
LASZIP_API laszip_I32
laszip_has_spatial_index
(
    laszip_POINTER                     pointer
    , laszip_BOOL*                     is_compressed
    , laszip_BOOL*                     is_appended
)
{
  if (laszip_has_spatial_index_ptr)
  {
    return (*laszip_has_spatial_index_ptr)(pointer, is_compressed, is_appended);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_inside_rectangle_def)
(
    laszip_POINTER                     pointer
    , laszip_F64                       r_min_x
    , laszip_F64                       r_min_y
    , laszip_F64                       r_max_x
    , laszip_F64                       r_max_y
    , laszip_BOOL*                     is_empty
);
laszip_inside_rectangle_def laszip_inside_rectangle_ptr = 0;
LASZIP_API laszip_I32
laszip_inside_rectangle
(
    laszip_POINTER                     pointer
    , laszip_F64                       r_min_x
    , laszip_F64                       r_min_y
    , laszip_F64                       r_max_x
    , laszip_F64                       r_max_y
    , laszip_BOOL*                     is_empty
)
{
  if (laszip_inside_rectangle_ptr)
  {
    return (*laszip_inside_rectangle_ptr)(pointer, r_min_x, r_min_y, r_max_x, r_max_y, is_empty);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_seek_point_def)
(
    laszip_POINTER                     pointer
    , laszip_I64                       index
);
laszip_seek_point_def laszip_seek_point_ptr = 0;
LASZIP_API laszip_I32
laszip_seek_point(
    laszip_POINTER                     pointer
    , laszip_I64                       index
)
{
  if (laszip_seek_point_ptr)
  {
    return (*laszip_seek_point_ptr)(pointer, index);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_read_point_def)
(
    laszip_POINTER                     pointer
);
laszip_read_point_def laszip_read_point_ptr = 0;
LASZIP_API laszip_I32
laszip_read_point(
    laszip_POINTER                     pointer
)
{
  if (laszip_read_point_ptr)
  {
    return (*laszip_read_point_ptr)(pointer);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_read_inside_point_def)
(
    laszip_POINTER                     pointer
    , laszip_BOOL*                     is_done
);
laszip_read_inside_point_def laszip_read_inside_point_ptr = 0;
LASZIP_API laszip_I32
laszip_read_inside_point(
    laszip_POINTER                     pointer
    , laszip_BOOL*                     is_done
)
{
  if (laszip_read_inside_point_ptr)
  {
    return (*laszip_read_inside_point_ptr)(pointer, is_done);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
typedef laszip_I32 (*laszip_close_reader_def)
(
    laszip_POINTER                     pointer
);
laszip_close_reader_def laszip_close_reader_ptr = 0;
LASZIP_API laszip_I32
laszip_close_reader
(
    laszip_POINTER                     pointer
)
{
  if (laszip_close_reader_ptr)
  {
    return (*laszip_close_reader_ptr)(pointer);
  }
  return 1;
};

/*---------------------------------------------------------------------------*/
/*---------------- DLL functions to load and unload LASzip ------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
#ifdef _WIN32
  #include <windows.h>
#define FreeLibraryZeroMeansFail 1
#else
  #include <dlfcn.h>
  typedef void* HINSTANCE;
#ifndef NULL
#define NULL 0
#endif
#define LoadLibrary dlopen
#define GetProcAddress dlsym
#define FreeLibrary dlclose
#define FreeLibraryZeroMeansFail 0
#define TEXT
#endif
static HINSTANCE laszip_HINSTANCE = NULL;
laszip_I32 laszip_load_dll()
{
  // Assure DLL not yet loaded
  if (laszip_HINSTANCE != NULL) {
    return 1;
  }
  // Load DLL file
#ifdef _WIN32
#ifdef _WIN64
  laszip_HINSTANCE = LoadLibrary(TEXT("LASzip64.dll"));
#else
  laszip_HINSTANCE = LoadLibrary(TEXT("LASzip.dll"));
#endif // _WIN64
#elif __APPLE__
  laszip_HINSTANCE = LoadLibrary("liblaszip.dylib", RTLD_NOW);
#else
  laszip_HINSTANCE = LoadLibrary("liblaszip.so", RTLD_NOW);
#endif
  if (laszip_HINSTANCE == NULL) {
     return 1;
  }
  // Get function pointers
  laszip_get_version_ptr = (laszip_get_version_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_version");
  if (laszip_get_version_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_create_ptr = (laszip_create_def)GetProcAddress(laszip_HINSTANCE, "laszip_create");
  if (laszip_create_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_clean_ptr = (laszip_clean_def)GetProcAddress(laszip_HINSTANCE, "laszip_clean");
  if (laszip_clean_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_error_ptr = (laszip_get_error_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_error");
  if (laszip_get_error_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_warning_ptr = (laszip_get_warning_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_warning");
  if (laszip_get_warning_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_destroy_ptr = (laszip_destroy_def)GetProcAddress(laszip_HINSTANCE, "laszip_destroy");
  if (laszip_destroy_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_header_pointer_ptr = (laszip_get_header_pointer_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_header_pointer");
  if (laszip_get_header_pointer_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_point_pointer_ptr = (laszip_get_point_pointer_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_point_pointer");
  if (laszip_get_point_pointer_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_point_count_ptr = (laszip_get_point_count_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_point_count");
  if (laszip_get_point_count_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_header_ptr = (laszip_set_header_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_header");
  if (laszip_set_header_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_point_type_and_size_ptr = (laszip_set_point_type_and_size_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_point_type_and_size");
  if (laszip_set_point_type_and_size_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_check_for_integer_overflow_ptr = (laszip_check_for_integer_overflow_def)GetProcAddress(laszip_HINSTANCE, "laszip_check_for_integer_overflow");
  if (laszip_check_for_integer_overflow_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_auto_offset_ptr = (laszip_auto_offset_def)GetProcAddress(laszip_HINSTANCE, "laszip_auto_offset");
  if (laszip_auto_offset_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_point_ptr = (laszip_set_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_point");
  if (laszip_set_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_coordinates_ptr = (laszip_set_coordinates_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_coordinates");
  if (laszip_set_coordinates_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_get_coordinates_ptr = (laszip_get_coordinates_def)GetProcAddress(laszip_HINSTANCE, "laszip_get_coordinates");
  if (laszip_get_coordinates_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_geokeys_ptr = (laszip_set_geokeys_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_geokeys");
  if (laszip_set_geokeys_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_geodouble_params_ptr = (laszip_set_geodouble_params_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_geodouble_params");
  if (laszip_set_geodouble_params_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_geoascii_params_ptr = (laszip_set_geoascii_params_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_geoascii_params");
  if (laszip_set_geoascii_params_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_add_attribute_ptr = (laszip_add_attribute_def)GetProcAddress(laszip_HINSTANCE, "laszip_add_attribute");
  if (laszip_add_attribute_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_add_vlr_ptr = (laszip_add_vlr_def)GetProcAddress(laszip_HINSTANCE, "laszip_add_vlr");
  if (laszip_add_vlr_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_remove_vlr_ptr = (laszip_remove_vlr_def)GetProcAddress(laszip_HINSTANCE, "laszip_remove_vlr");
  if (laszip_remove_vlr_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_create_spatial_index_ptr = (laszip_create_spatial_index_def)GetProcAddress(laszip_HINSTANCE, "laszip_create_spatial_index");
  if (laszip_create_spatial_index_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_preserve_generating_software_ptr = (laszip_preserve_generating_software_def)GetProcAddress(laszip_HINSTANCE, "laszip_preserve_generating_software");
  if (laszip_preserve_generating_software_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_request_native_extension_ptr = (laszip_request_native_extension_def)GetProcAddress(laszip_HINSTANCE, "laszip_request_native_extension");
  if (laszip_request_native_extension_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_request_compatibility_mode_ptr = (laszip_request_compatibility_mode_def)GetProcAddress(laszip_HINSTANCE, "laszip_request_compatibility_mode");
  if (laszip_request_compatibility_mode_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_set_chunk_size_ptr = (laszip_set_chunk_size_def)GetProcAddress(laszip_HINSTANCE, "laszip_set_chunk_size");
  if (laszip_set_chunk_size_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_open_writer_ptr = (laszip_open_writer_def)GetProcAddress(laszip_HINSTANCE, "laszip_open_writer");
  if (laszip_open_writer_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_write_point_ptr = (laszip_write_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_write_point");
  if (laszip_write_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_write_indexed_point_ptr = (laszip_write_indexed_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_write_indexed_point");
  if (laszip_write_indexed_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_update_inventory_ptr = (laszip_update_inventory_def)GetProcAddress(laszip_HINSTANCE, "laszip_update_inventory");
  if (laszip_update_inventory_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_close_writer_ptr = (laszip_close_writer_def)GetProcAddress(laszip_HINSTANCE, "laszip_close_writer");
  if (laszip_close_writer_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_exploit_spatial_index_ptr = (laszip_exploit_spatial_index_def)GetProcAddress(laszip_HINSTANCE, "laszip_exploit_spatial_index");
  if (laszip_exploit_spatial_index_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_decompress_selective_ptr = (laszip_decompress_selective_def)GetProcAddress(laszip_HINSTANCE, "laszip_decompress_selective");
  if (laszip_decompress_selective_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_open_reader_ptr = (laszip_open_reader_def)GetProcAddress(laszip_HINSTANCE, "laszip_open_reader");
  if (laszip_open_reader_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_has_spatial_index_ptr = (laszip_has_spatial_index_def)GetProcAddress(laszip_HINSTANCE, "laszip_has_spatial_index");
  if (laszip_has_spatial_index_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_inside_rectangle_ptr = (laszip_inside_rectangle_def)GetProcAddress(laszip_HINSTANCE, "laszip_inside_rectangle");
  if (laszip_inside_rectangle_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_seek_point_ptr = (laszip_seek_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_seek_point");
  if (laszip_seek_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_read_point_ptr = (laszip_read_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_read_point");
  if (laszip_read_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_read_inside_point_ptr = (laszip_read_inside_point_def)GetProcAddress(laszip_HINSTANCE, "laszip_read_inside_point");
  if (laszip_read_inside_point_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  laszip_close_reader_ptr = (laszip_close_reader_def)GetProcAddress(laszip_HINSTANCE, "laszip_close_reader");
  if (laszip_close_reader_ptr == NULL) {
     FreeLibrary(laszip_HINSTANCE);
     return 1;
  }
  return 0;
};

/*---------------------------------------------------------------------------*/
laszip_I32 laszip_unload_dll()
{
  if (laszip_HINSTANCE == NULL) {
    return 1;
  }
  if (FreeLibraryZeroMeansFail)
  {
    if (!FreeLibrary(laszip_HINSTANCE)) {
      return 1;
    }
  }
  else
  {
    if (FreeLibrary(laszip_HINSTANCE)) {
      return 1;
    }
  }
  laszip_HINSTANCE = NULL;
  return 0;
}

#ifdef __cplusplus
}
#endif
