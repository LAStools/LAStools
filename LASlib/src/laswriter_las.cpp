/*
===============================================================================

  FILE:  laswriter_las.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2017, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "laswriter_las.hpp"

#include "lasmessage.hpp"
#include "bytestreamout_nil.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamout_ostream.hpp"
#include "laswritepoint.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>

BOOL LASwriterLAS::refile(FILE* file)
{
  if (stream == 0) return FALSE;
  if (this->file) this->file = file;
  return ((ByteStreamOutFile*)stream)->refile(file);
}

BOOL LASwriterLAS::open(const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  ByteStreamOut* out = new ByteStreamOutNil();
  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(const char* file_name, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size, I32 io_buffer_size)
{
  if (file_name == 0)
  {
    laserror("file name pointer is zero");
    return FALSE;
  }

  file = LASfopen(file_name, "wb");

  if (file == 0)
  {
    laserror("cannot open file '%s' for write", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, io_buffer_size) != 0)
  {
    LASMessage(LAS_WARNING, "setvbuf() failed with buffer size %d", io_buffer_size);
  }

  ByteStreamOut* out;
  if (Endian::IS_LITTLE_ENDIAN)
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(FILE* file, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  if (file == 0)
  {
    laserror("file pointer is zero");
    return FALSE;
  }

#ifdef _WIN32
  if (file == stdout)
  {
    if (_setmode(_fileno(stdout), _O_BINARY) == -1)
    {
      laserror("cannot set stdout to binary (untranslated) mode");
    }
  }
#endif

  ByteStreamOut* out;
  if (Endian::IS_LITTLE_ENDIAN)
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(std::ostream& stream, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  ByteStreamOut* out;
  if (Endian::IS_LITTLE_ENDIAN)
    out = new ByteStreamOutOstreamLE(stream);
  else
    out = new ByteStreamOutOstreamBE(stream);

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(ByteStreamOut* stream, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  U32 i, j;

  if (stream == 0)
  {
    laserror("ByteStreamOut pointer is zero");
    return FALSE;
  }
  this->stream = stream;

  if (header == 0)
  {
    laserror("LASheader pointer is zero");
    return FALSE;
  }

  // check header contents

  if (!header->check()) return FALSE;

  // copy scale_and_offset
  quantizer.x_scale_factor = header->x_scale_factor;
  quantizer.y_scale_factor = header->y_scale_factor;
  quantizer.z_scale_factor = header->z_scale_factor;
  quantizer.x_offset = header->x_offset;
  quantizer.y_offset = header->y_offset;
  quantizer.z_offset = header->z_offset;

  // check if the requested point type is supported

  LASpoint point;
  U8 point_data_format;
  U16 point_data_record_length;
  BOOL point_is_standard = TRUE;

  if (header->laszip)
  {
    if (!point.init(&quantizer, header->laszip->num_items, header->laszip->items, header)) return FALSE;
    point_is_standard = header->laszip->is_standard(&point_data_format, &point_data_record_length);
  }
  else
  {
    if (!point.init(&quantizer, header->point_data_format, header->point_data_record_length, header)) return FALSE;
    point_data_format = header->point_data_format;
    point_data_record_length = header->point_data_record_length;
  }

  // fail if we don't use the layered compressor for the new LAS 1.4 point types

  if (compressor && (point_data_format > 5) && (compressor != LASZIP_COMPRESSOR_LAYERED_CHUNKED))
  {
    laserror("point type %d requires using \"native LAS 1.4 extension\" of LASzip", point_data_format);
    return FALSE;
  }

  // do we need a LASzip VLR (because we compress or use non-standard points?)

  LASzip* laszip = 0;
  U32 laszip_vlr_data_size = 0;
  if (compressor || point_is_standard == FALSE)
  {
    laszip = new LASzip();
    laszip->setup(point.num_items, point.items, compressor);
    if (chunk_size > -1) laszip->set_chunk_size((U32)chunk_size);
    if (compressor == LASZIP_COMPRESSOR_NONE) laszip->request_version(0);
    else if (chunk_size == 0 && (point_data_format <= 5)) {
      laserror("adaptive chunking is depricated for point type %d. only available for new LAS 1.4 point types 6 or higher.", point_data_format); 
      return FALSE;
    }
    else if (requested_version) laszip->request_version(requested_version);
    else laszip->request_version(laszip->get_default_version(point_data_format, header->version_major, header->version_minor));
    laszip_vlr_data_size = 34 + 6 * laszip->num_items;
  }

  // create and setup the point writer

  writer = new LASwritePoint();
  if (laszip)
  {
    if (!writer->setup(laszip->num_items, laszip->items, laszip))
    {
      laserror("point type %d of size %d not supported (with LASzip)", header->point_data_format, header->point_data_record_length);
      return FALSE;
    }
  }
  else
  {
    if (!writer->setup(point.num_items, point.items))
    {
      laserror("point type %d of size %d not supported", header->point_data_format, header->point_data_record_length);
      return FALSE;
    }
  }

  // LAS 1.5 doesn't allow point types 0-5 anymore. 
  // print a warning, but allow it if the user requests this 
  // Resulting file might not be supported by every reader though
  if ((header->version_minor >= 5) && ((point_data_format & 63) < 6))
  {
      LASMessage(LAS_WARNING, "point type %d is not supported for version %d.%d anymore", header->point_data_format, header->version_major, header->version_minor);
  }

  // save the position where we start writing the header

  header_start_position = stream->tell();

  // write header variable after variable (to avoid alignment issues)

  if (!stream->putBytes((const U8*)&(header->file_signature), 4))
  {
    laserror("writing header->file_signature");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->file_source_ID)))
  {
    laserror("writing header->file_source_ID");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->global_encoding)))
  {
    laserror("writing header->global_encoding");
    return FALSE;
  }
  if (!stream->put32bitsLE((const U8*)&(header->project_ID_GUID_data_1)))
  {
    laserror("writing header->project_ID_GUID_data_1");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->project_ID_GUID_data_2)))
  {
    laserror("writing header->project_ID_GUID_data_2");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->project_ID_GUID_data_3)))
  {
    laserror("writing header->project_ID_GUID_data_3");
    return FALSE;
  }
  if (!stream->putBytes((const U8*)header->project_ID_GUID_data_4, 8))
  {
    laserror("writing header->project_ID_GUID_data_4");
    return FALSE;
  }
  // check version major
  U8 version_major = header->version_major;
  if (header->version_major != 1)
  {
    LASMessage(LAS_WARNING, "header->version_major is %d. writing 1 instead.", header->version_major);
    version_major = 1;
  }
  if (!stream->putByte(version_major))
  {
    laserror("writing header->version_major");
    return FALSE;
  }
  // check version minor
  U8 version_minor = header->version_minor;
  if (version_minor > 5)
  {
    LASMessage(LAS_WARNING, "header->version_minor is %d. writing 5 instead.", version_minor);
    version_minor = 5;
  }
  if (!stream->putByte(version_minor))
  {
    laserror("writing header->version_minor");
    return FALSE;
  }
  if (!stream->putBytes((const U8*)header->system_identifier, 32))
  {
    laserror("writing header->system_identifier");
    return FALSE;
  }
  if (!stream->putBytes((const U8*)header->generating_software, 32))
  {
    laserror("writing header->generating_software");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->file_creation_day)))
  {
    laserror("writing header->file_creation_day");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->file_creation_year)))
  {
    laserror("writing header->file_creation_year");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->header_size)))
  {
    laserror("writing header->header_size");
    return FALSE;
  }
  U32 offset_to_point_data = header->offset_to_point_data;
  if (laszip) offset_to_point_data += (54 + laszip_vlr_data_size);
  if (header->vlr_lastiling) offset_to_point_data += (54 + 28);
  if (header->vlr_lasoriginal) offset_to_point_data += (54 + 176);
  if (!stream->put32bitsLE((const U8*)&offset_to_point_data))
  {
    laserror("writing header->offset_to_point_data");
    return FALSE;
  }
  U32 number_of_variable_length_records = header->number_of_variable_length_records;
  if (laszip) number_of_variable_length_records++;
  if (header->vlr_lastiling) number_of_variable_length_records++;
  if (header->vlr_lasoriginal) number_of_variable_length_records++;
  if (!stream->put32bitsLE((const U8*)&(number_of_variable_length_records)))
  {
    laserror("writing header->number_of_variable_length_records");
    return FALSE;
  }
  if (compressor) point_data_format |= 128;
  if (!stream->putByte(point_data_format))
  {
    laserror("writing header->point_data_format");
    return FALSE;
  }
  if (!stream->put16bitsLE((const U8*)&(header->point_data_record_length)))
  {
    laserror("writing header->point_data_record_length");
    return FALSE;
  }
  if (!stream->put32bitsLE((const U8*)&(header->number_of_point_records)))
  {
    laserror("writing header->number_of_point_records");
    return FALSE;
  }
  for (i = 0; i < 5; i++)
  {
    if (!stream->put32bitsLE((const U8*)&(header->number_of_points_by_return[i])))
    {
      laserror("writing header->number_of_points_by_return[%d]", i);
      return FALSE;
    }
  }
  if (!stream->put64bitsLE((const U8*)&(header->x_scale_factor)))
  {
    laserror("writing header->x_scale_factor");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->y_scale_factor)))
  {
    laserror("writing header->y_scale_factor");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->z_scale_factor)))
  {
    laserror("writing header->z_scale_factor");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->x_offset)))
  {
    laserror("writing header->x_offset");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->y_offset)))
  {
    laserror("writing header->y_offset");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->z_offset)))
  {
    laserror("writing header->z_offset");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->max_x)))
  {
    laserror("writing header->max_x");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->min_x)))
  {
    laserror("writing header->min_x");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->max_y)))
  {
    laserror("writing header->max_y");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->min_y)))
  {
    laserror("writing header->min_y");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->max_z)))
  {
    laserror("writing header->max_z");
    return FALSE;
  }
  if (!stream->put64bitsLE((const U8*)&(header->min_z)))
  {
    laserror("writing header->min_z");
    return FALSE;
  }

  // special handling for LAS 1.3 or higher.
  if (version_minor >= 3)
  {
    U64 start_of_waveform_data_packet_record = header->start_of_waveform_data_packet_record;
    if (start_of_waveform_data_packet_record != 0)
    {
      LASMessage(LAS_WARNING, "header->start_of_waveform_data_packet_record is %lld. writing 0 instead.", start_of_waveform_data_packet_record);
      start_of_waveform_data_packet_record = 0;
    }
    if (!stream->put64bitsLE((const U8*)&start_of_waveform_data_packet_record))
    {
      laserror("writing start_of_waveform_data_packet_record");
      return FALSE;
    }
  }

  // special handling for LAS 1.4 or higher.
  if (version_minor >= 4)
  {
    writing_las_1_4 = TRUE;
    if (header->point_data_format >= 6)
    {
      writing_new_point_type = TRUE;
    }
    else
    {
      writing_new_point_type = FALSE;
    }
    start_of_first_extended_variable_length_record = header->start_of_first_extended_variable_length_record;
    if (!stream->put64bitsLE((const U8*)&(start_of_first_extended_variable_length_record)))
    {
      laserror("writing header->start_of_first_extended_variable_length_record");
      return FALSE;
    }
    number_of_extended_variable_length_records = header->number_of_extended_variable_length_records;
    if (!stream->put32bitsLE((const U8*)&(number_of_extended_variable_length_records)))
    {
      laserror("writing header->number_of_extended_variable_length_records");
      return FALSE;
    }
    evlrs = header->evlrs;
    U64 extended_number_of_point_records;
    if (header->number_of_point_records)
      extended_number_of_point_records = header->number_of_point_records;
    else
      extended_number_of_point_records = header->extended_number_of_point_records;
    if (!stream->put64bitsLE((const U8*)&extended_number_of_point_records))
    {
      laserror("writing header->extended_number_of_point_records");
      return FALSE;
    }
    U64 extended_number_of_points_by_return;
    for (i = 0; i < 15; i++)
    {
      if ((i < 5) && header->number_of_points_by_return[i])
        extended_number_of_points_by_return = header->number_of_points_by_return[i];
      else
        extended_number_of_points_by_return = header->extended_number_of_points_by_return[i];
      if (!stream->put64bitsLE((const U8*)&extended_number_of_points_by_return))
      {
        laserror("writing header->extended_number_of_points_by_return[%d]", i);
        return FALSE;
      }
    }
  }
  else
  {
    writing_las_1_4 = FALSE;
    writing_new_point_type = FALSE;
  }

  // special handling for LAS 1.5 or higher.
  if (version_minor >= 5)
  {
      if (!stream->put64bitsLE((const U8*)&(header->max_gps_time)))
      {
          laserror("writing header->max_gps_time");
          return FALSE;
      }
      if (!stream->put64bitsLE((const U8*)&(header->min_gps_time)))
      {
          laserror("writing header->min_gps_time");
          return FALSE;
      }
      if (!stream->put16bitsLE((const U8*)&(header->time_offset)))
      {
          laserror("writing header->time_offset");
          return FALSE;
      }
  }


  // write any number of user-defined bytes that might have been added into the header

  if (header->user_data_in_header_size)
  {
    if (header->user_data_in_header)
    {
      if (!stream->putBytes((const U8*)header->user_data_in_header, header->user_data_in_header_size))
      {
        laserror("writing %d bytes of data from header->user_data_in_header", header->user_data_in_header_size);
        return FALSE;
      }
    }
    else
    {
      laserror("there should be %d bytes of data in header->user_data_in_header", header->user_data_in_header_size);
      return FALSE;
    }
  }

  // write variable length records variable after variable (to avoid alignment issues)

  for (i = 0; i < header->number_of_variable_length_records; i++)
  {
    // check variable length records contents

    if (header->vlrs[i].reserved != 0xAABB)
    {
      //      LASMessage(LAS_WARNING, "wrong header->vlrs[%d].reserved: %d != 0xAABB", i, header->vlrs[i].reserved);
    }

    // write variable length records variable after variable (to avoid alignment issues)

    if (!stream->put16bitsLE((const U8*)&(header->vlrs[i].reserved)))
    {
      laserror("writing header->vlrs[%d].reserved", i);
      return FALSE;
    }
    if (!stream->putBytes((const U8*)header->vlrs[i].user_id, 16))
    {
      laserror("writing header->vlrs[%d].user_id", i);
      return FALSE;
    }
    if (!stream->put16bitsLE((const U8*)&(header->vlrs[i].record_id)))
    {
      laserror("writing header->vlrs[%d].record_id", i);
      return FALSE;
    }
    if (!stream->put16bitsLE((const U8*)&(header->vlrs[i].record_length_after_header)))
    {
      laserror("writing header->vlrs[%d].record_length_after_header", i);
      return FALSE;
    }
    if (!stream->putBytes((const U8*)header->vlrs[i].description, 32))
    {
      laserror("writing header->vlrs[%d].description", i);
      return FALSE;
    }

    // write the data following the header of the variable length record

    if (header->vlrs[i].record_length_after_header)
    {
      if (header->vlrs[i].data)
      {
        if (!stream->putBytes((const U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          laserror("writing %d bytes of data from header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }
      else
      {
        laserror("there should be %d bytes of data in header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
        return FALSE;
      }
    }
  }

  // write laszip VLR with compression parameters

  if (laszip)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0; // used to be 0xAABB
    if (!stream->put16bitsLE((const U8*)&(reserved)))
    {
      laserror("writing reserved %d", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "laszip encoded\0";
    if (!stream->putBytes((const U8*)user_id, 16))
    {
      laserror("writing user_id %s", user_id);
      return FALSE;
    }
    U16 record_id = 22204;
    if (!stream->put16bitsLE((const U8*)&(record_id)))
    {
      laserror("writing record_id %d", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = laszip_vlr_data_size;
    if (!stream->put16bitsLE((const U8*)&(record_length_after_header)))
    {
      laserror("writing record_length_after_header %d", (I32)record_length_after_header);
      return FALSE;
    }
    char description[32];
    memset(description, 0, 32);
    snprintf(description, sizeof(description), "by laszip of LAStools (%d)", LAS_TOOLS_VERSION);
    if (!stream->putBytes((const U8*)description, 32))
    {
      laserror("writing description %s", description);
      return FALSE;
    }
    // write the data following the header of the variable length record
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

    if (!stream->put16bitsLE((const U8*)&(laszip->compressor)))
    {
      laserror("writing compressor %d", (I32)compressor);
      return FALSE;
    }
    if (!stream->put16bitsLE((const U8*)&(laszip->coder)))
    {
      laserror("writing coder %d", (I32)laszip->coder);
      return FALSE;
    }
    if (!stream->putByte(laszip->version_major))
    {
      laserror("writing version_major %d", laszip->version_major);
      return FALSE;
    }
    if (!stream->putByte(laszip->version_minor))
    {
      laserror("writing version_minor %d", laszip->version_minor);
      return FALSE;
    }
    if (!stream->put16bitsLE((const U8*)&(laszip->version_revision)))
    {
      laserror("writing version_revision %d", laszip->version_revision);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(laszip->options)))
    {
      laserror("writing options %d", (I32)laszip->options);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(laszip->chunk_size)))
    {
      laserror("writing chunk_size %d", laszip->chunk_size);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(laszip->number_of_special_evlrs)))
    {
      laserror("writing number_of_special_evlrs %d", (I32)laszip->number_of_special_evlrs);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(laszip->offset_to_special_evlrs)))
    {
      laserror("writing offset_to_special_evlrs %d", (I32)laszip->offset_to_special_evlrs);
      return FALSE;
    }
    if (!stream->put16bitsLE((const U8*)&(laszip->num_items)))
    {
      laserror("writing num_items %d", laszip->num_items);
      return FALSE;
    }
    for (i = 0; i < laszip->num_items; i++)
    {
      if (!stream->put16bitsLE((const U8*)&(laszip->items[i].type)))
      {
        laserror("writing type %d of item %d", laszip->items[i].type, i);
        return FALSE;
      }
      if (!stream->put16bitsLE((const U8*)&(laszip->items[i].size)))
      {
        laserror("writing size %d of item %d", laszip->items[i].size, i);
        return FALSE;
      }
      if (!stream->put16bitsLE((const U8*)&(laszip->items[i].version)))
      {
        laserror("writing version %d of item %d", laszip->items[i].version, i);
        return FALSE;
      }
    }

    delete laszip;
    laszip = 0;
  }

  // write lastiling VLR with the tile parameters

  if (header->vlr_lastiling)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0; // used to be 0xAABB
    if (!stream->put16bitsLE((const U8*)&(reserved)))
    {
      laserror("writing reserved %d", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "LAStools\0\0\0\0\0\0\0";
    if (!stream->putBytes((const U8*)user_id, 16))
    {
      laserror("writing user_id %s", user_id);
      return FALSE;
    }
    U16 record_id = 10;
    if (!stream->put16bitsLE((const U8*)&(record_id)))
    {
      laserror("writing record_id %d", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = 28;
    if (!stream->put16bitsLE((const U8*)&(record_length_after_header)))
    {
      laserror("writing record_length_after_header %d", (I32)record_length_after_header);
      return FALSE;
    }
    CHAR description[33] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    snprintf(description, sizeof(description), "tile %s buffer %s", (header->vlr_lastiling->buffer ? "with" : "without"), (header->vlr_lastiling->reversible ? ", reversible" : ""));
    if (!stream->putBytes((const U8*)description, 32))
    {
      laserror("writing description %s", description);
      return FALSE;
    }

    // write the payload of this VLR which contains 28 bytes
    //   U32  level                                          4 bytes
    //   U32  level_index                                    4 bytes
    //   U32  implicit_levels + buffer bit + reversible bit  4 bytes
    //   F32  min_x                                          4 bytes
    //   F32  max_x                                          4 bytes
    //   F32  min_y                                          4 bytes
    //   F32  max_y                                          4 bytes

    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->level)))
    {
      laserror("writing header->vlr_lastiling->level %u", header->vlr_lastiling->level);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->level_index)))
    {
      laserror("writing header->vlr_lastiling->level_index %u", header->vlr_lastiling->level_index);
      return FALSE;
    }
    if (!stream->put32bitsLE(((U8*)header->vlr_lastiling) + 8))
    {
      laserror("writing header->vlr_lastiling->implicit_levels %u", header->vlr_lastiling->implicit_levels);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->min_x)))
    {
      laserror("writing header->vlr_lastiling->min_x %g", header->vlr_lastiling->min_x);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->max_x)))
    {
      laserror("writing header->vlr_lastiling->max_x %g", header->vlr_lastiling->max_x);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->min_y)))
    {
      laserror("writing header->vlr_lastiling->min_y %g", header->vlr_lastiling->min_y);
      return FALSE;
    }
    if (!stream->put32bitsLE((const U8*)&(header->vlr_lastiling->max_y)))
    {
      laserror("writing header->vlr_lastiling->max_y %g", header->vlr_lastiling->max_y);
      return FALSE;
    }
  }

  // write lasoriginal VLR with the original (unbuffered) counts and bounding box extent

  if (header->vlr_lasoriginal)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0; // used to be 0xAABB
    if (!stream->put16bitsLE((U8*)&(reserved)))
    {
      laserror("writing reserved %d", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "LAStools\0\0\0\0\0\0\0";
    if (!stream->putBytes((U8*)user_id, 16))
    {
      laserror("writing user_id %s", user_id);
      return FALSE;
    }
    U16 record_id = 20;
    if (!stream->put16bitsLE((U8*)&(record_id)))
    {
      laserror("writing record_id %d", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = 176;
    if (!stream->put16bitsLE((U8*)&(record_length_after_header)))
    {
      laserror("writing record_length_after_header %d", (I32)record_length_after_header);
      return FALSE;
    }
    U8 description[32] = "counters and bbox of original\0\0";
    if (!stream->putBytes((U8*)description, 32))
    {
      laserror("writing description %s", description);
      return FALSE;
    }

    // save the position in the stream at which the payload of this VLR was written

    header->vlr_lasoriginal->position = stream->tell();

    // write the payload of this VLR which contains 176 bytes

    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->number_of_point_records)))
    {
      laserror("writing header->vlr_lasoriginal->number_of_point_records %u", (U32)header->vlr_lasoriginal->number_of_point_records);
      return FALSE;
    }
    for (j = 0; j < 15; j++)
    {
      if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->number_of_points_by_return[j])))
      {
        laserror("writing header->vlr_lasoriginal->number_of_points_by_return[%u] %u", j, (U32)header->vlr_lasoriginal->number_of_points_by_return[j]);
        return FALSE;
      }
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->min_x)))
    {
      laserror("writing header->vlr_lasoriginal->min_x %g", header->vlr_lasoriginal->min_x);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->max_x)))
    {
      laserror("writing header->vlr_lasoriginal->max_x %g", header->vlr_lasoriginal->max_x);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->min_y)))
    {
      laserror("writing header->vlr_lasoriginal->min_y %g", header->vlr_lasoriginal->min_y);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->max_y)))
    {
      laserror("writing header->vlr_lasoriginal->max_y %g", header->vlr_lasoriginal->max_y);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->min_z)))
    {
      laserror("writing header->vlr_lasoriginal->min_z %g", header->vlr_lasoriginal->min_z);
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->vlr_lasoriginal->max_z)))
    {
      laserror("writing header->vlr_lasoriginal->max_z %g", header->vlr_lasoriginal->max_z);
      return FALSE;
    }
  }

  // write any number of user-defined bytes that might have been added after the header

  if (header->user_data_after_header_size)
  {
    if (header->user_data_after_header)
    {
      if (!stream->putBytes((U8*)header->user_data_after_header, header->user_data_after_header_size))
      {
        laserror("writing %d bytes of data from header->user_data_after_header", header->user_data_after_header_size);
        return FALSE;
      }
    }
    else
    {
      laserror("there should be %d bytes of data in header->user_data_after_header", header->user_data_after_header_size);
      return FALSE;
    }
  }

  // initialize the point writer

  if (!writer->init(stream)) return FALSE;

  npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
  p_count = 0;

  return TRUE;
}

BOOL LASwriterLAS::write_point(const LASpoint* point)
{
  p_count++;
  return writer->write(point->point);
}

BOOL LASwriterLAS::chunk()
{
  return writer->chunk();
}

BOOL LASwriterLAS::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  I32 i;
  if (header == 0)
  {
    laserror("header pointer is zero");
    return FALSE;
  }
  if (stream == 0)
  {
    laserror("stream pointer is zero");
    return FALSE;
  }
  if (!stream->isSeekable())
  {
    LASMessage(LAS_WARNING, "stream not seekable. cannot update header.");
    return FALSE;
  }
  if (use_inventory)
  {
    U32 number;
    stream->seek(header_start_position + 107);
    if (header->point_data_format >= 6)
    {
      number = 0; // legacy counters are zero for new point types
    }
    else if (inventory.extended_number_of_point_records > U32_MAX)
    {
      if (header->version_minor >= 4)
      {
        number = 0;
      }
      else
      {
        LASMessage(LAS_WARNING, "too many points in LAS %d.%d file. limit is %u.", header->version_major, header->version_minor, U32_MAX);
        number = U32_MAX;
      }
    }
    else
    {
      number = (U32)inventory.extended_number_of_point_records;
    }
    if (!stream->put32bitsLE((const U8*)&number))
    {
      laserror("updating inventory.number_of_point_records");
      return FALSE;
    }
    npoints = inventory.extended_number_of_point_records;
    for (i = 0; i < 5; i++)
    {
      if (header->point_data_format >= 6)
      {
        number = 0; // legacy counters are zero for new point types
      }
      else if (inventory.extended_number_of_points_by_return[i + 1] > U32_MAX)
      {
        if (header->version_minor >= 4)
        {
          number = 0;
        }
        else
        {
          number = U32_MAX;
        }
      }
      else
      {
        number = (U32)inventory.extended_number_of_points_by_return[i + 1];
      }
      if (!stream->put32bitsLE((const U8*)&number))
      {
        laserror("updating inventory.number_of_points_by_return[%d]", i);
        return FALSE;
      }
    }
    stream->seek(header_start_position + 179);
    F64 value;
    value = quantizer.get_x(inventory.max_X);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.max_X");
      return FALSE;
    }
    value = quantizer.get_x(inventory.min_X);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.min_X");
      return FALSE;
    }
    value = quantizer.get_y(inventory.max_Y);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.max_Y");
      return FALSE;
    }
    value = quantizer.get_y(inventory.min_Y);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.min_Y");
      return FALSE;
    }
    value = quantizer.get_z(inventory.max_Z);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.max_Z");
      return FALSE;
    }
    value = quantizer.get_z(inventory.min_Z);
    if (!stream->put64bitsLE((const U8*)&value))
    {
      laserror("updating inventory.min_Z");
      return FALSE;
    }
    // special handling for LAS 1.4 or higher.
    if (header->version_minor >= 4)
    {
      stream->seek(header_start_position + 247);
      if (!stream->put64bitsLE((const U8*)&(inventory.extended_number_of_point_records)))
      {
        laserror("updating header->extended_number_of_point_records");
        return FALSE;
      }
      for (i = 0; i < 15; i++)
      {
        if (!stream->put64bitsLE((const U8*)&(inventory.extended_number_of_points_by_return[i + 1])))
        {
          laserror("updating header->extended_number_of_points_by_return[%d]", i);
          return FALSE;
        }
      }
    }
    // special handling for LAS 1.5 or higher.
    if (header->version_minor >= 5)
    {
        stream->seek(header_start_position + 375);
        if (!stream->put64bitsLE((const U8*)&(inventory.max_gps_time)))
        {
            laserror("updating inventory.max_gps_time");
            return FALSE;
        }
        if (!stream->put64bitsLE((const U8*)&(inventory.min_gps_time)))
        {
            laserror("updating inventory.min_gps_time");
            return FALSE;
        }
    }
  }
  else
  {
    U32 number;
    stream->seek(header_start_position + 107);
    if (header->point_data_format >= 6)
    {
      number = 0; // legacy counters are zero for new point types
    }
    else
    {
      number = header->number_of_point_records;
    }
    if (!stream->put32bitsLE((const U8*)&number))
    {
      laserror("updating header->number_of_point_records");
      return FALSE;
    }
    npoints = header->number_of_point_records;
    for (i = 0; i < 5; i++)
    {
      if (header->point_data_format >= 6)
      {
        number = 0; // legacy counters are zero for new point types
      }
      else
      {
        number = header->number_of_points_by_return[i];
      }
      if (!stream->put32bitsLE((const U8*)&number))
      {
        laserror("updating header->number_of_points_by_return[%d]", i);
        return FALSE;
      }
    }
    stream->seek(header_start_position + 179);
    if (!stream->put64bitsLE((const U8*)&(header->max_x)))
    {
      laserror("updating header->max_x");
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->min_x)))
    {
      laserror("updating header->min_x");
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->max_y)))
    {
      laserror("updating header->max_y");
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->min_y)))
    {
      laserror("updating header->min_y");
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->max_z)))
    {
      laserror("updating header->max_z");
      return FALSE;
    }
    if (!stream->put64bitsLE((const U8*)&(header->min_z)))
    {
      laserror("updating header->min_z");
      return FALSE;
    }
    // special handling for LAS 1.3 or higher.
    if (header->version_minor >= 3)
    {
      // nobody currently includes waveform. we set the field always to zero
      if (header->start_of_waveform_data_packet_record != 0)
      {
        LASMessage(LAS_WARNING, "header->start_of_waveform_data_packet_record is %lld. writing 0 instead.", header->start_of_waveform_data_packet_record);
        U64 start_of_waveform_data_packet_record = 0;
        if (!stream->put64bitsLE((const U8*)&start_of_waveform_data_packet_record))
        {
          laserror("updating start_of_waveform_data_packet_record");
          return FALSE;
        }
      }
      else
      {
        if (!stream->put64bitsLE((const U8*)&(header->start_of_waveform_data_packet_record)))
        {
          laserror("updating header->start_of_waveform_data_packet_record");
          return FALSE;
        }
      }
    }
    // special handling for LAS 1.4 or higher.
    if (header->version_minor >= 4)
    {
      stream->seek(header_start_position + 235);
      if (!stream->put64bitsLE((const U8*)&(header->start_of_first_extended_variable_length_record)))
      {
        laserror("updating header->start_of_first_extended_variable_length_record");
        return FALSE;
      }
      if (!stream->put32bitsLE((const U8*)&(header->number_of_extended_variable_length_records)))
      {
        laserror("updating header->number_of_extended_variable_length_records");
        return FALSE;
      }
      U64 value;
      if (header->number_of_point_records)
        value = header->number_of_point_records;
      else
        value = header->extended_number_of_point_records;
      if (!stream->put64bitsLE((const U8*)&value))
      {
        laserror("updating header->extended_number_of_point_records");
        return FALSE;
      }
      for (i = 0; i < 15; i++)
      {
        if ((i < 5) && header->number_of_points_by_return[i])
          value = header->number_of_points_by_return[i];
        else
          value = header->extended_number_of_points_by_return[i];
        if (!stream->put64bitsLE((const U8*)&value))
        {
          laserror("updating header->extended_number_of_points_by_return[%d]", i);
          return FALSE;
        }
      }
    }
    // special handling for LAS 1.5 or higher.
    if (header->version_minor >= 5)
    {
        stream->seek(header_start_position + 375);
        if (!stream->put64bitsLE((const U8*)&(header->max_gps_time)))
        {
            laserror("updating header->max_gps_time");
            return FALSE;
        }
        if (!stream->put64bitsLE((const U8*)&(header->min_gps_time)))
        {
            laserror("updating header->min_gps_time");
            return FALSE;
        }
        if (!stream->put16bitsLE((const U8*)&(header->time_offset)))
        {
            laserror("updating header->time_offset");
            return FALSE;
        }
    }
  }
  stream->seekEnd();
  if (update_extra_bytes)
  {
    if (header == 0)
    {
      laserror("header pointer is zero");
      return FALSE;
    }
    if (header->number_attributes)
    {
      I64 start = header_start_position + header->header_size;
      for (i = 0; i < (I32)header->number_of_variable_length_records; i++)
      {
        start += 54;
        if ((header->vlrs[i].record_id == 4) && (strcmp(header->vlrs[i].user_id, "LASF_Spec") == 0))
        {
          break;
        }
        else
        {
          start += header->vlrs[i].record_length_after_header;
        }
      }
      if (i == (I32)header->number_of_variable_length_records)
      {
        LASMessage(LAS_WARNING, "could not find extra bytes VLR for update");
      }
      else
      {
        stream->seek(start);
        if (!stream->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          laserror("writing %d bytes of data from header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }
    }
    stream->seekEnd();
  }

  // COPC EPT hierarchy EVLR is computed and added to the header after the last point is written.
  // Therefore, it cannot be added when opening the writer. We use update_header to propagate the EVLR
  // just before closing the writer. EVLRs are written when closing. This trick allows us to be COPC
  // compatible with minimal changes to the code.
  for (i = 0; i < (I32)header->number_of_extended_variable_length_records; i++)
  {
    if ((strcmp(header->evlrs[i].user_id, "copc") == 0) && header->evlrs[i].record_id == 1000)
    {
      evlrs = header->evlrs;
    }
  }

  return TRUE;
}

I64 LASwriterLAS::close(BOOL update_npoints)
{
  I64 bytes = 0;

  if (p_count != npoints)
  {
    if (npoints || !update_npoints)
    {
      LASMessage(LAS_WARNING, "written %lld points but expected %lld points", p_count, npoints);
    }
  }

  if (writer)
  {
    writer->done();
    delete writer;
    writer = 0;
  }

  if (writing_las_1_4 && number_of_extended_variable_length_records)
  {
    I64 real_start_of_first_extended_variable_length_record = stream->tell();

    // write extended variable length records variable after variable (to avoid alignment issues)
    U64 copc_root_hier_size = 0;
    U64 copc_root_hier_offset = 0;
    for (U32 i = 0; i < number_of_extended_variable_length_records; i++)
    {
      if ((strcmp(evlrs[i].user_id, "copc") == 0) && evlrs[i].record_id == 1000)
        copc_root_hier_offset = stream->tell() + 60;

      // check variable length records contents

      if (evlrs[i].reserved != 0xAABB)
      {
        //      LASMessage(LAS_WARNING, "wrong evlrs[%d].reserved: %d != 0xAABB", i, evlrs[i].reserved);
      }

      // write variable length records variable after variable (to avoid alignment issues)

      if (!stream->put16bitsLE((const U8*)&(evlrs[i].reserved)))
      {
        laserror("writing evlrs[%d].reserved", i);
        return FALSE;
      }
      if (!stream->putBytes((const U8*)evlrs[i].user_id, 16))
      {
        laserror("writing evlrs[%d].user_id", i);
        return FALSE;
      }
      if (!stream->put16bitsLE((const U8*)&(evlrs[i].record_id)))
      {
        laserror("writing evlrs[%d].record_id", i);
        return FALSE;
      }
      if (!stream->put64bitsLE((const U8*)&(evlrs[i].record_length_after_header)))
      {
        laserror("writing evlrs[%d].record_length_after_header", i);
        return FALSE;
      }
      if (!stream->putBytes((const U8*)evlrs[i].description, 32))
      {
        laserror("writing evlrs[%d].description", i);
        return FALSE;
      }

      // write the data following the header of the variable length record

      if (evlrs[i].record_length_after_header)
      {
        if (evlrs[i].data)
        {
          if (!stream->putBytes((const U8*)evlrs[i].data, (U32)evlrs[i].record_length_after_header))
          {
            laserror("writing %u bytes of data from evlrs[%d].data", (U32)evlrs[i].record_length_after_header, i);
            return FALSE;
          }
        }
        else
        {
          laserror("there should be %u bytes of data in evlrs[%d].data", (U32)evlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }

      if ((strcmp(evlrs[i].user_id, "copc") == 0) && evlrs[i].record_id == 1000)
      {
        copc_root_hier_size = evlrs[i].record_length_after_header;
        stream->seek(375 + 54 + 40);
        stream->put64bitsLE((const U8*)&copc_root_hier_offset);
        stream->put64bitsLE((const U8*)&copc_root_hier_size);
        stream->seekEnd();
      }
    }

    if (real_start_of_first_extended_variable_length_record != start_of_first_extended_variable_length_record)
    {
      stream->seek(header_start_position + 235);
      stream->put64bitsLE((const U8*)&real_start_of_first_extended_variable_length_record);
      stream->seekEnd();
    }
  }

  if (stream)
  {
    if (update_npoints && p_count != npoints)
    {
      if (!stream->isSeekable())
      {
        LASMessage(LAS_WARNING, "stream not seekable. cannot update header from %lld to %lld points.", npoints, p_count);
      }
      else
      {
        U32 number;
        if (writing_new_point_type)
        {
          number = 0;
        }
        else if (p_count > U32_MAX)
        {
          if (writing_las_1_4)
          {
            number = 0;
          }
          else
          {
            number = U32_MAX;
          }
        }
        else
        {
          number = (U32)p_count;
        }
        stream->seek(header_start_position + 107);
        stream->put32bitsLE((const U8*)&number);
        if (writing_las_1_4)
        {
          stream->seek(header_start_position + 235 + 12);
          stream->put64bitsLE((const U8*)&p_count);
        }
        stream->seekEnd();
      }
    }
    bytes = stream->tell() - header_start_position;
    if (delete_stream)
    {
      delete stream;
    }
    stream = 0;
  }

  if (file)
  {
    fclose(file);
    file = 0;
  }

  npoints = p_count;
  p_count = 0;

  return bytes;
}

I64 LASwriterLAS::tell()
{
  return stream->tell();
}

LASwriterLAS::LASwriterLAS()
{
  file = 0;
  stream = 0;
  delete_stream = TRUE;
  writer = 0;
  writing_las_1_4 = FALSE;
  writing_new_point_type = FALSE;
  // for delayed write of EVLRs
  start_of_first_extended_variable_length_record = 0;
  number_of_extended_variable_length_records = 0;
  evlrs = 0;
  header_start_position = 0;
}

LASwriterLAS::~LASwriterLAS()
{
  if (writer || stream) close();
}
