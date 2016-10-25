/*
===============================================================================

  FILE:  bytestreamin_array.hpp
  
  CONTENTS:
      
  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    19 July 2015 -- moved from LASlib to LASzip for "compatibility mode" in DLL
     9 April 2012 -- created after cooking Zuccini/Onion/Potatoe dinner for Mara
  
===============================================================================
*/
#ifndef BYTE_STREAM_IN_ARRAY_H
#define BYTE_STREAM_IN_ARRAY_H

#include "bytestreamin.hpp"

class ByteStreamInArray : public ByteStreamIn
{
public:
  ByteStreamInArray(U8* data, I64 size);
/* read a single byte                                        */
  U32 getByte();
/* read an array of bytes                                    */
  void getBytes(U8* bytes, const U32 num_bytes);
/* is the stream seekable (e.g. stdin is not)                */
  bool isSeekable() const;
/* get current position of stream                            */
  I64 tell() const;
/* seek to this position in the stream                       */
  bool seek(const I64 position);
/* seek to the end of the stream                             */
  bool seekEnd(const I64 distance=0);
/* destructor                                                */
  ~ByteStreamInArray(){};
protected:
  U8* data;
  I64 size;
  I64 curr;
};

class ByteStreamInArrayLE : public ByteStreamInArray
{
public:
  ByteStreamInArrayLE(U8* data, I64 size);
/* read 16 bit low-endian field                              */
  void get16bitsLE(U8* bytes);
/* read 32 bit low-endian field                              */
  void get32bitsLE(U8* bytes);
/* read 64 bit low-endian field                              */
  void get64bitsLE(U8* bytes);
/* read 16 bit big-endian field                              */
  void get16bitsBE(U8* bytes);
/* read 32 bit big-endian field                              */
  void get32bitsBE(U8* bytes);
/* read 64 bit big-endian field                              */
  void get64bitsBE(U8* bytes);
private:
  U8 swapped[8];
};

class ByteStreamInArrayBE : public ByteStreamInArray
{
public:
  ByteStreamInArrayBE(U8* data, I64 size);
/* read 16 bit low-endian field                              */
  void get16bitsLE(U8* bytes);
/* read 32 bit low-endian field                              */
  void get32bitsLE(U8* bytes);
/* read 64 bit low-endian field                              */
  void get64bitsLE(U8* bytes);
/* read 16 bit big-endian field                              */
  void get16bitsBE(U8* bytes);
/* read 32 bit big-endian field                              */
  void get32bitsBE(U8* bytes);
/* read 64 bit big-endian field                              */
  void get64bitsBE(U8* bytes);
private:
  U8 swapped[8];
};

inline ByteStreamInArray::ByteStreamInArray(U8* data, I64 size)
{
  this->data = data;
  this->size = size;
  this->curr = 0;
}

inline U32 ByteStreamInArray::getByte()
{
  if (curr == size)
  {
    throw EOF;
  }
  U32 byte = data[curr];
  curr++;
  return byte;
}

inline void ByteStreamInArray::getBytes(U8* bytes, const U32 num_bytes)
{
  if ((curr + num_bytes) > size)
  {
    throw EOF;
  }
  memcpy((void*)bytes, (void*)(data+curr), num_bytes);
  curr += num_bytes;
}

inline bool ByteStreamInArray::isSeekable() const
{
  return TRUE;
}

inline I64 ByteStreamInArray::tell() const
{
  return curr;
}

inline bool ByteStreamInArray::seek(const I64 position)
{
  if ((0 <= position) && (position <= size))
  {
    curr = position;
    return TRUE;
  }
  return FALSE;
}

inline bool ByteStreamInArray::seekEnd(const I64 distance)
{
  if ((0 <= distance) && (distance <= size))
  {
    curr = size - distance;
    return TRUE;
  }
  return FALSE;
}

inline ByteStreamInArrayLE::ByteStreamInArrayLE(U8* data, I64 size) : ByteStreamInArray(data, size)
{
}

inline void ByteStreamInArrayLE::get16bitsLE(U8* bytes)
{
  getBytes(bytes, 2);
}

inline void ByteStreamInArrayLE::get32bitsLE(U8* bytes)
{
  getBytes(bytes, 4);
}

inline void ByteStreamInArrayLE::get64bitsLE(U8* bytes)
{
  getBytes(bytes, 8);
}

inline void ByteStreamInArrayLE::get16bitsBE(U8* bytes)
{
  getBytes(swapped, 2);
  bytes[0] = swapped[1];
  bytes[1] = swapped[0];
}

inline void ByteStreamInArrayLE::get32bitsBE(U8* bytes)
{
  getBytes(swapped, 4);
  bytes[0] = swapped[3];
  bytes[1] = swapped[2];
  bytes[2] = swapped[1];
  bytes[3] = swapped[0];
}

inline void ByteStreamInArrayLE::get64bitsBE(U8* bytes)
{
  getBytes(swapped, 8);
  bytes[0] = swapped[7];
  bytes[1] = swapped[6];
  bytes[2] = swapped[5];
  bytes[3] = swapped[4];
  bytes[4] = swapped[3];
  bytes[5] = swapped[2];
  bytes[6] = swapped[1];
  bytes[7] = swapped[0];
}

inline ByteStreamInArrayBE::ByteStreamInArrayBE(U8* data, I64 size) : ByteStreamInArray(data, size)
{
}

inline void ByteStreamInArrayBE::get16bitsLE(U8* bytes)
{
  getBytes(swapped, 2);
  bytes[0] = swapped[1];
  bytes[1] = swapped[0];
}

inline void ByteStreamInArrayBE::get32bitsLE(U8* bytes)
{
  getBytes(swapped, 4);
  bytes[0] = swapped[3];
  bytes[1] = swapped[2];
  bytes[2] = swapped[1];
  bytes[3] = swapped[0];
}

inline void ByteStreamInArrayBE::get64bitsLE(U8* bytes)
{
  getBytes(swapped, 8);
  bytes[0] = swapped[7];
  bytes[1] = swapped[6];
  bytes[2] = swapped[5];
  bytes[3] = swapped[4];
  bytes[4] = swapped[3];
  bytes[5] = swapped[2];
  bytes[6] = swapped[1];
  bytes[7] = swapped[0];
}

inline void ByteStreamInArrayBE::get16bitsBE(U8* bytes)
{
  getBytes(bytes, 2);
}

inline void ByteStreamInArrayBE::get32bitsBE(U8* bytes)
{
  getBytes(bytes, 4);
}

inline void ByteStreamInArrayBE::get64bitsBE(U8* bytes)
{
  getBytes(bytes, 8);
}

#endif
