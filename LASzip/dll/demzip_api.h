/*
===============================================================================

  FILE:  demzip_api.h

  CONTENTS:

    A simple DLL interface to read and write RasterLAZ files

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    31 August 2019 -- created during codesprint after FOSS4G 2019 in Bucharest 

===============================================================================
*/

#ifndef DEMZIP_API_H
#define DEMZIP_API_H

#ifdef _WIN32
#   ifdef DEMZIP_DYN_LINK
#       ifdef DEMZIP_SOURCE
#           define DEMZIP_API __declspec(dllexport)
#       else
#           define DEMZIP_API __declspec(dllimport)
#       endif
#   else
#       define DEMZIP_API
#   endif
#else
#   define DEMZIP_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/*--------------- DLL variables to pass data to/from DEMzip -----------------*/
/*---------------------------------------------------------------------------*/

#ifdef _WIN32
typedef int                demzip_BOOL;
typedef unsigned char      demzip_U8;
typedef unsigned short     demzip_U16;
typedef unsigned int       demzip_U32;
typedef unsigned __int64   demzip_U64;
typedef char               demzip_I8;
typedef short              demzip_I16;
typedef int                demzip_I32;
typedef __int64            demzip_I64;
typedef char               demzip_CHAR;
typedef float              demzip_F32;
typedef double             demzip_F64;
typedef void*              demzip_POINTER;
#else
#include <stdint.h>
typedef int                demzip_BOOL;
typedef uint8_t            demzip_U8;
typedef uint16_t           demzip_U16;
typedef uint32_t           demzip_U32;
typedef uint64_t           demzip_U64;
typedef int8_t             demzip_I8;
typedef int16_t            demzip_I16;
typedef int32_t            demzip_I32;
typedef int64_t            demzip_I64;
typedef char               demzip_CHAR;
typedef float              demzip_F32;
typedef double             demzip_F64;
typedef void*              demzip_POINTER;
#endif

typedef struct demzip_geokey
{
  demzip_U16 key_id;
  demzip_U16 tiff_tag_location;
  demzip_U16 count;
  demzip_U16 value_offset;
} demzip_geokey_struct;

typedef struct demzip_vlr
{
  demzip_U16 reserved;
  demzip_CHAR user_id[16];
  demzip_U16 record_id;
  demzip_U16 record_length_after_header;
  demzip_CHAR description[32];
  demzip_U8* data;
} demzip_vlr_struct;

typedef struct demzip_header
{
  demzip_U16 file_source_ID;
  demzip_U16 global_encoding;
  demzip_U32 project_ID_GUID_data_1;
  demzip_U16 project_ID_GUID_data_2;
  demzip_U16 project_ID_GUID_data_3;
  demzip_CHAR project_ID_GUID_data_4[8];
  demzip_U8 version_major;
  demzip_U8 version_minor;
  demzip_CHAR system_identifier[32];
  demzip_CHAR generating_software[32];
  demzip_U16 file_creation_day;
  demzip_U16 file_creation_year;
  demzip_U16 header_size;
  demzip_U32 offset_to_point_data;
  demzip_U32 number_of_variable_length_records;
  demzip_U8 point_data_format;
  demzip_U16 point_data_record_length;
  demzip_U32 number_of_point_records;
  demzip_U32 number_of_points_by_return[5];
  demzip_F64 x_scale_factor;
  demzip_F64 y_scale_factor;
  demzip_F64 z_scale_factor;
  demzip_F64 x_offset;
  demzip_F64 y_offset;
  demzip_F64 z_offset;
  demzip_F64 max_x;
  demzip_F64 min_x;
  demzip_F64 max_y;
  demzip_F64 min_y;
  demzip_F64 max_z;
  demzip_F64 min_z;

  // LAS 1.3 and higher only
  demzip_U64 start_of_waveform_data_packet_record;

  // LAS 1.4 and higher only
  demzip_U64 start_of_first_extended_variable_length_record;
  demzip_U32 number_of_extended_variable_length_records;
  demzip_U64 extended_number_of_point_records;
  demzip_U64 extended_number_of_points_by_return[15];

  // optional
  demzip_U32 user_data_in_header_size;
  demzip_U8* user_data_in_header;

  // optional VLRs
  demzip_vlr_struct* vlrs;

  // optional
  demzip_U32 user_data_after_header_size;
  demzip_U8* user_data_after_header;

} demzip_header_struct;

typedef struct demzip_point
{
  demzip_I32 X;
  demzip_I32 Y;
  demzip_I32 Z;
  demzip_U16 intensity;
  demzip_U8 return_number : 3;
  demzip_U8 number_of_returns : 3;
  demzip_U8 scan_direction_flag : 1;
  demzip_U8 edge_of_flight_line : 1;
  demzip_U8 classification : 5;
  demzip_U8 synthetic_flag : 1;
  demzip_U8 keypoint_flag  : 1;
  demzip_U8 withheld_flag  : 1;
  demzip_I8 scan_angle_rank;
  demzip_U8 user_data;
  demzip_U16 point_source_ID;

  // LAS 1.4 only
  demzip_I16 extended_scan_angle;
  demzip_U8 extended_point_type : 2;
  demzip_U8 extended_scanner_channel : 2;
  demzip_U8 extended_classification_flags : 4;
  demzip_U8 extended_classification;
  demzip_U8 extended_return_number : 4;
  demzip_U8 extended_number_of_returns : 4;

  // for 8 byte alignment of the GPS time
  demzip_U8 dummy[7];

  demzip_F64 gps_time;
  demzip_U16 rgb[4];
  demzip_U8 wave_packet[29];

  demzip_I32 num_extra_bytes;
  demzip_U8* extra_bytes;

} demzip_point_struct;

/*---------------------------------------------------------------------------*/
/*------ DLL constants for selective decompression via LASzip DLL -----------*/
/*---------------------------------------------------------------------------*/

#define demzip_DECOMPRESS_SELECTIVE_ALL                0xFFFFFFFF

#define demzip_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY 0x00000000
#define demzip_DECOMPRESS_SELECTIVE_Z                  0x00000001
#define demzip_DECOMPRESS_SELECTIVE_CLASSIFICATION     0x00000002
#define demzip_DECOMPRESS_SELECTIVE_FLAGS              0x00000004
#define demzip_DECOMPRESS_SELECTIVE_INTENSITY          0x00000008
#define demzip_DECOMPRESS_SELECTIVE_SCAN_ANGLE         0x00000010
#define demzip_DECOMPRESS_SELECTIVE_USER_DATA          0x00000020
#define demzip_DECOMPRESS_SELECTIVE_POINT_SOURCE       0x00000040
#define demzip_DECOMPRESS_SELECTIVE_GPS_TIME           0x00000080
#define demzip_DECOMPRESS_SELECTIVE_RGB                0x00000100
#define demzip_DECOMPRESS_SELECTIVE_NIR                0x00000200
#define demzip_DECOMPRESS_SELECTIVE_WAVEPACKET         0x00000400
#define demzip_DECOMPRESS_SELECTIVE_BYTE0              0x00010000
#define demzip_DECOMPRESS_SELECTIVE_BYTE1              0x00020000
#define demzip_DECOMPRESS_SELECTIVE_BYTE2              0x00040000
#define demzip_DECOMPRESS_SELECTIVE_BYTE3              0x00080000
#define demzip_DECOMPRESS_SELECTIVE_BYTE4              0x00100000
#define demzip_DECOMPRESS_SELECTIVE_BYTE5              0x00200000
#define demzip_DECOMPRESS_SELECTIVE_BYTE6              0x00400000
#define demzip_DECOMPRESS_SELECTIVE_BYTE7              0x00800000
#define demzip_DECOMPRESS_SELECTIVE_EXTRA_BYTES        0xFFFF0000

/*---------------------------------------------------------------------------*/
/*---------------- DLL functions to manage the LASzip DLL -------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_version
(
    demzip_U8*                         version_major
    , demzip_U8*                       version_minor
    , demzip_U16*                      version_revision
    , demzip_U32*                      version_build
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_create(
    demzip_POINTER*                    pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_error
(
    demzip_POINTER                     pointer
    , demzip_CHAR**                    error
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_warning
(
    demzip_POINTER                     pointer
    , demzip_CHAR**                    warning
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_clean(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_destroy(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
/*---------- DLL functions to write and read LAS and LAZ files --------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_header_pointer(
    demzip_POINTER                     pointer
    , demzip_header_struct**           header_pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_point_pointer(
    demzip_POINTER                     pointer
    , demzip_point_struct**            point_pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_point_count(
    demzip_POINTER                     pointer
    , demzip_I64*                      count
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_header(
    demzip_POINTER                     pointer
    , const demzip_header_struct*      header
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_point_type_and_size(
    demzip_POINTER                     pointer
    , demzip_U8                        point_type
    , demzip_U16                       point_size
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_check_for_integer_overflow(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_auto_offset(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_point(
    demzip_POINTER                     pointer
    , const demzip_point_struct*       point
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_coordinates(
    demzip_POINTER                     pointer
    , const demzip_F64*                coordinates
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_coordinates(
    demzip_POINTER                     pointer
    , demzip_F64*                      coordinates
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geokeys(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_geokey_struct*      key_entries
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geodouble_params(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_F64*                geodouble_params
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geoascii_params(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_CHAR*               geoascii_params
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_add_attribute(
    demzip_POINTER                     pointer
    , demzip_U32                       type
    , const demzip_CHAR*               name
    , const demzip_CHAR*               description
    , demzip_F64                       scale
    , demzip_F64                       offset
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_add_vlr(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               user_id
    , demzip_U16                       record_id
    , demzip_U16                       record_length_after_header
    , const demzip_CHAR*               description
    , const demzip_U8*                 data
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_remove_vlr(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               user_id
    , demzip_U16                       record_id
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_create_spatial_index(
    demzip_POINTER                     pointer
    , const demzip_BOOL                create
    , const demzip_BOOL                append
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_preserve_generating_software(
    demzip_POINTER                     pointer
    , const demzip_BOOL                preserve
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_request_native_extension(
    demzip_POINTER                     pointer
    , const demzip_BOOL                request
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_request_compatibility_mode(
    demzip_POINTER                     pointer
    , const demzip_BOOL                request
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_chunk_size(
    demzip_POINTER                     pointer
    , const demzip_U32                 chunk_size
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_writer(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               file_name
    , demzip_BOOL                      compress
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_write_point(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_write_indexed_point(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_update_inventory(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_close_writer(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_exploit_spatial_index(
    demzip_POINTER                     pointer
    , const demzip_BOOL                exploit
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_decompress_selective(
    demzip_POINTER                     pointer
    , const demzip_U32                 decompress_selective
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_reader(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               file_name
    , demzip_BOOL*                     is_compressed
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_has_spatial_index(
    demzip_POINTER                     pointer
    , demzip_BOOL*                     is_indexed
    , demzip_BOOL*                     is_appended
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_inside_rectangle(
    demzip_POINTER                     pointer
    , demzip_F64                       min_x
    , demzip_F64                       min_y
    , demzip_F64                       max_x
    , demzip_F64                       max_y
    , demzip_BOOL*                     is_empty
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_seek_point(
    demzip_POINTER                     pointer
    , demzip_I64                       index
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_read_point(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_read_inside_point(
    demzip_POINTER                     pointer
    , demzip_BOOL*                     is_done
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_close_reader(
    demzip_POINTER                     pointer
);

/*---------------------------------------------------------------------------*/
/*---------------- DLL functions to load and unload LASzip ------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_load_dll
(
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_unload_dll
(
);

#ifdef __cplusplus
} // extern "C"

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_reader_stream(
    demzip_POINTER                     pointer
    , istream&                         stream
    , demzip_BOOL*                     is_compressed
);

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_writer_stream(
    demzip_POINTER                     pointer
    , ostream&                         stream
    , demzip_BOOL                      compress
    , demzip_BOOL                      do_not_write_header
);

/*---------------------------------------------------------------------------*/
// make LASzip VLR for point type and point size already specified earlier
DEMZIP_API demzip_I32
demzip_create_demzip_vlr(
    demzip_POINTER                     pointer
    , demzip_U8**                      vlr
    , demzip_U32*                      vlr_size
);

#endif  // __cplusplus

#endif /* DEMZIP_API_H */
