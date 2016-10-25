/*
===============================================================================

  FILE:  laswritepoint.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/

#include "laswritepoint.hpp"

#include "arithmeticencoder.hpp"
#include "laswriteitemraw.hpp"
#include "laswriteitemcompressed_v1.hpp"
#include "laswriteitemcompressed_v2.hpp"

#include <string.h>
#include <stdlib.h>

LASwritePoint::LASwritePoint()
{
  outstream = 0;
  num_writers = 0;
  writers = 0;
  writers_raw = 0;
  writers_compressed = 0;
  enc = 0;
  // used for chunking
  chunk_size = U32_MAX;
  chunk_count = 0;
  number_chunks = 0;
  alloced_chunks = 0;
  chunk_sizes = 0;
  chunk_bytes = 0;
  chunk_table_start_position = 0;
  chunk_start_position = 0;
}

bool LASwritePoint::setup(const U32 num_items, const LASitem* items, const LASzip* laszip)
{
  U32 i;

  // is laszip exists then we must use its items
  if (laszip)
  {
    if (num_items != laszip->num_items) return FALSE;
    if (items != laszip->items) return FALSE;
  }

  // create entropy encoder (if requested)
  enc = 0;
  if (laszip && laszip->compressor)
  {
    switch (laszip->coder)
    {
    case LASZIP_CODER_ARITHMETIC:
      enc = new ArithmeticEncoder();
      break;
    default:
      // entropy decoder not supported
      return FALSE;
    }
  }

  // initizalize the writers
  writers = 0;
  num_writers = num_items;

  // disable chunking
  chunk_size = U32_MAX;

  // always create the raw writers
  writers_raw = new LASwriteItem*[num_writers];
  memset(writers_raw, 0, num_writers*sizeof(LASwriteItem*));
  for (i = 0; i < num_writers; i++)
  {
    switch (items[i].type)
    {
    case LASitem::POINT10:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_POINT10_LE();
      else
        writers_raw[i] = new LASwriteItemRaw_POINT10_BE();
      break;
    case LASitem::GPSTIME11:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_GPSTIME11_LE();
      else
        writers_raw[i] = new LASwriteItemRaw_GPSTIME11_BE();
      break;
    case LASitem::RGB12:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_RGB12_LE();
      else
        writers_raw[i] = new LASwriteItemRaw_RGB12_BE();
      break;
    case LASitem::WAVEPACKET13:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_WAVEPACKET13_LE();
      else
        writers_raw[i] = new LASwriteItemRaw_WAVEPACKET13_BE();
      break;
    case LASitem::BYTE:
      writers_raw[i] = new LASwriteItemRaw_BYTE(items[i].size);
      break;
    case LASitem::POINT14:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_POINT14_LE();
      else
        return FALSE;
      break;
    case LASitem::RGBNIR14:
      if (IS_LITTLE_ENDIAN())
        writers_raw[i] = new LASwriteItemRaw_RGBNIR14_LE();
      else
        writers_raw[i] = new LASwriteItemRaw_RGBNIR14_BE();
      break;
    default:
      return FALSE;
    }
  }

  // if needed create the compressed writers and set versions
  if (enc)
  {
    writers_compressed = new LASwriteItem*[num_writers];
    memset(writers_compressed, 0, num_writers*sizeof(LASwriteItem*));
    for (i = 0; i < num_writers; i++)
    {
      switch (items[i].type)
      {
      case LASitem::POINT10:
        if (items[i].version == 1)
          writers_compressed[i] = new LASwriteItemCompressed_POINT10_v1(enc);
        else if (items[i].version == 2)
          writers_compressed[i] = new LASwriteItemCompressed_POINT10_v2(enc);
        else
          return FALSE;
        break;
      case LASitem::GPSTIME11:
        if (items[i].version == 1)
          writers_compressed[i] = new LASwriteItemCompressed_GPSTIME11_v1(enc);
        else if (items[i].version == 2)
          writers_compressed[i] = new LASwriteItemCompressed_GPSTIME11_v2(enc);
        else
          return FALSE;
        break;
      case LASitem::RGB12:
        if (items[i].version == 1)
          writers_compressed[i] = new LASwriteItemCompressed_RGB12_v1(enc);
        else if (items[i].version == 2)
          writers_compressed[i] = new LASwriteItemCompressed_RGB12_v2(enc);
        else
          return FALSE;
        break;
      case LASitem::WAVEPACKET13:
        if (items[i].version == 1)
         writers_compressed[i] = new LASwriteItemCompressed_WAVEPACKET13_v1(enc);
        else
          return FALSE;
        break;
      case LASitem::BYTE:
        if (items[i].version == 1)
         writers_compressed[i] = new LASwriteItemCompressed_BYTE_v1(enc, items[i].size);
        else if (items[i].version == 2)
          writers_compressed[i] = new LASwriteItemCompressed_BYTE_v2(enc, items[i].size);
        else
          return FALSE;
        break;
      default:
        return FALSE;
      }
    }
    if (laszip->compressor == LASZIP_COMPRESSOR_POINTWISE_CHUNKED)
    {
      if (laszip->chunk_size) chunk_size = laszip->chunk_size;
      chunk_count = 0;
      number_chunks = U32_MAX;
    }
  }
  return TRUE;
}

bool LASwritePoint::init(ByteStreamOut* outstream)
{
  if (!outstream) return FALSE;
  this->outstream = outstream;

  // if chunking is enabled
  if (number_chunks == U32_MAX)
  {
    number_chunks = 0;
    if (outstream->isSeekable())
    {
      chunk_table_start_position = outstream->tell();
    }
    else
    {
      chunk_table_start_position = -1;
    }
    outstream->put64bitsLE((U8*)&chunk_table_start_position);
    chunk_start_position = outstream->tell();
  }

  U32 i;
  for (i = 0; i < num_writers; i++)
  {
    ((LASwriteItemRaw*)(writers_raw[i]))->init(outstream);
  }

  if (enc)
  {
    writers = 0;
  }
  else
  {
    writers = writers_raw;
  }

  return TRUE;
}

bool LASwritePoint::write(const U8 * const * point)
{
  U32 i;

  if (chunk_count == chunk_size)
  {
    enc->done();
    add_chunk_to_table();
    init(outstream);
    chunk_count = 0;
  }
  chunk_count++;

  if (writers)
  {
    for (i = 0; i < num_writers; i++)
    {
      writers[i]->write(point[i]);
    }
  }
  else
  {
    for (i = 0; i < num_writers; i++)
    {
      writers_raw[i]->write(point[i]);
      ((LASwriteItemCompressed*)(writers_compressed[i]))->init(point[i]);
    }
    writers = writers_compressed;
    enc->init(outstream);
  }
  return TRUE;
}

bool LASwritePoint::chunk()
{
  if (chunk_start_position == 0 || chunk_size != U32_MAX)
  {
    return FALSE;
  }
  enc->done();
  add_chunk_to_table();
  init(outstream);
  chunk_count = 0;
  return TRUE;
}

bool LASwritePoint::done()
{
  if (writers == writers_compressed)
  {
    enc->done();
    if (chunk_start_position)
    {
      if (chunk_count) add_chunk_to_table();
      return write_chunk_table();
    }
  }
  else if (writers == 0)
  {
    if (chunk_start_position)
    {
      return write_chunk_table();
    }
  }

  return TRUE;
}

bool LASwritePoint::add_chunk_to_table()
{
  if (number_chunks == alloced_chunks)
  {
    if (chunk_bytes == 0)
    {
      alloced_chunks = 1024;
      if (chunk_size == U32_MAX) chunk_sizes = (U32*)malloc(sizeof(U32)*alloced_chunks);
      chunk_bytes = (U32*)malloc(sizeof(U32)*alloced_chunks);
    }
    else
    {
      alloced_chunks *= 2;
      if (chunk_size == U32_MAX) chunk_sizes = (U32*)realloc(chunk_sizes, sizeof(U32)*alloced_chunks);
      chunk_bytes = (U32*)realloc(chunk_bytes, sizeof(U32)*alloced_chunks);
    }
    if (chunk_size == U32_MAX && chunk_sizes == 0) return FALSE;
    if (chunk_bytes == 0) return FALSE;
  }
  I64 position = outstream->tell();
  if (chunk_size == U32_MAX) chunk_sizes[number_chunks] = chunk_count;
  chunk_bytes[number_chunks] = (U32)(position - chunk_start_position);
  chunk_start_position = position;
  number_chunks++;
  return TRUE;
}

bool LASwritePoint::write_chunk_table()
{
  U32 i;
  I64 position = outstream->tell();
  if (chunk_table_start_position != -1) // stream is seekable
  {
    if (!outstream->seek(chunk_table_start_position))
    {
      return FALSE;
    }
    if (!outstream->put64bitsLE((U8*)&position))
    {
      return FALSE;
    }
    if (!outstream->seek(position))
    {
      return FALSE;
    }
  }
  U32 version = 0;
  if (!outstream->put32bitsLE((U8*)&version))
  {
    return FALSE;
  }
  if (!outstream->put32bitsLE((U8*)&number_chunks))
  {
    return FALSE;
  }
  if (number_chunks > 0)
  {
    enc->init(outstream);
    IntegerCompressor ic(enc, 32, 2);
    ic.initCompressor();
    for (i = 0; i < number_chunks; i++)
    {
      if (chunk_size == U32_MAX) ic.compress((i ? chunk_sizes[i-1] : 0), chunk_sizes[i], 0);
      ic.compress((i ? chunk_bytes[i-1] : 0), chunk_bytes[i], 1);
    }
    enc->done();
  }
  if (chunk_table_start_position == -1) // stream is not-seekable
  {
    if (!outstream->put64bitsLE((U8*)&position))
    {
      return FALSE;
    }
  }
  return TRUE;
}

LASwritePoint::~LASwritePoint()
{
  U32 i;

  if (writers_raw)
  {
    for (i = 0; i < num_writers; i++)
    {
      delete writers_raw[i];
    }
    delete [] writers_raw;
  }
  if (writers_compressed)
  {
    for (i = 0; i < num_writers; i++)
    {
      delete writers_compressed[i];
    }
    delete [] writers_compressed;
  }
  if (enc)
  {
    delete enc;
  }

  if (chunk_bytes) free(chunk_bytes);
}
