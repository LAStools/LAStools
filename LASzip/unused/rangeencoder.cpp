/******************************************************************************
 *
 * Project:  integrating laszip into liblas - http://liblas.org -
 * Purpose:
 * Author:   Martin Isenburg
 *           isenburg at cs.unc.edu
 *
 ******************************************************************************
 * Copyright (c) 2010, Martin Isenburg
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 *
 * See the COPYING file for more information.
 *
 ****************************************************************************/

/*
===============================================================================

  FILE:  rangeencoder.cpp
  
  CONTENTS:
      
    see header file

  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2003-10 martin isenburg (isenburg@cs.unc.edu)
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see header file
  
===============================================================================
*/
#include "rangeencoder.hpp"
#include "rangemodel.hpp"

#include <string.h>
#include <assert.h>

RangeEncoder::RangeEncoder()
{
  outstream = 0;
}

RangeEncoder::~RangeEncoder()
{
}

I32 RangeEncoder::init(ByteStreamOut* outstream)
{
  assert(outstream);
  this->outstream = outstream;

  low = 0;                /* Full code range */
  range = TOP_VALUE;
  /* this buffer is written as first byte in the datastream (header,...) */
  buffer = HEADERBYTE;
  help = 0;               /* No bytes to follow */
  return 0;
}

/* Finish encoding                                           */
/* actually not that many bytes need to be output, but who   */
/* cares. I output them because decode will read them :)     */
/* the return value is the number of bytes written           */
void RangeEncoder::done()
{
  U32 tmp;
  normalize();     /* now we have a normalized state */
  bytecount += 5;
  if ((low & (BOTTOM_VALUE-1)) < ((bytecount&0xffffffL)>>1))
  {
    tmp = low >> SHIFT_BITS;
  }
  else
  {
    tmp = (low >> SHIFT_BITS) + 1;
  }
  if (tmp > 0xff) /* we have a carry */
  {
    outstream->putByte(buffer+1);
    for(; help; help--)
    {
      outstream->putByte(0);
    }
  }
  else  /* no carry */
  {
    outstream->putByte(buffer);
    for(; help; help--)
    {
      outstream->putByte(0xff);
    }
  }
  outstream->putByte(tmp & 0xff);
  outstream->putByte((bytecount>>16) & 0xff);
  outstream->putByte((bytecount>>8) & 0xff);
  outstream->putByte(bytecount & 0xff);
  outstream = 0;
}

EntropyModel* RangeEncoder::createBitModel()
{
  return createSymbolModel(2);
}

void RangeEncoder::initBitModel(EntropyModel* model)
{
  initSymbolModel(model);
}

void RangeEncoder::destroyBitModel(EntropyModel* model)
{
  destroySymbolModel(model);
}

EntropyModel* RangeEncoder::createSymbolModel(U32 n)
{
  RangeModel* m = new RangeModel(n, TRUE);
  return (EntropyModel*)m;
}

void RangeEncoder::initSymbolModel(EntropyModel* model, U32 *table)
{
  assert(model);
  RangeModel* m = (RangeModel*)model;
  m->init(table);
}

void RangeEncoder::destroySymbolModel(EntropyModel* model)
{
  assert(model);
  RangeModel* m = (RangeModel*)model;
  delete m;
}

void RangeEncoder::encodeBit(EntropyModel* model, U32 bit)
{
  encodeSymbol(model, bit);
}

void RangeEncoder::encodeSymbol(EntropyModel* model, U32 sym)
{
  assert(model);
  RangeModel* m = (RangeModel*)model;
  assert(sym >= 0 && sym < (U32)(m->n));
  U32 syfreq;
  U32 ltfreq;
  U32 r, tmp;
  U32 lg_totf = m->lg_totf;
  
  m->getfreq(sym,&syfreq,&ltfreq);

  normalize();
  r = range >> lg_totf;
  tmp = r * ltfreq;
  low += tmp;
#ifdef EXTRAFAST
  range = r * syfreq;
#else
  if ((ltfreq+syfreq) >> lg_totf)
  {
    range -= tmp;
  }
  else
  {
    range = r * syfreq;
  }
#endif

  m->update(sym);
}

void RangeEncoder::writeBit(U32 sym)
{
  assert(sym < 2);

  U32 r, tmp;
  normalize();
  r = range >> 1;
  tmp = r * sym;
  low += tmp;
#ifdef EXTRAFAST
  range = r;
#else
  if ((sym+1) >> 1)
  {
    range -= tmp;
  }
  else
  {
    range = r;
  }
#endif
}

void RangeEncoder::writeBits(U32 bits, U32 sym)
{
  assert(bits && (sym < (1u<<bits)));

  if (bits > 21) // 22 bits
  {
    writeShort(sym&U16_MAX);
    sym = sym >> 16;
    bits = bits - 16;
  }

  U32 r, tmp;
  normalize();
  r = range >> bits;
  tmp = r * sym;
  low += tmp;
#ifdef EXTRAFAST
  range = r;
#else
  if ((sym+1) >> bits)
  {
    range -= tmp;
  }
  else
  {
    range = r;
  }
#endif
}

void RangeEncoder::writeByte(U8 sym)
{
  U32 r, tmp;
  normalize();
  r = range >> 8;
  tmp = r * (U32)(sym);
  low += tmp;
#ifdef EXTRAFAST
  range = r;
#else
  if (((U32)(sym)+1) >> 8)
  {
    range -= tmp;
  }
  else
  {
    range = r;
  }
#endif
}

void RangeEncoder::writeShort(U16 sym)
{
  U32 r, tmp;
  normalize();
  r = range >> 16;
  tmp = r * (U32)(sym);
  low += tmp;
#ifdef EXTRAFAST
  range = r;
#else
  if (((U32)(sym)+1) >> 16)
  {
    range -= tmp;
  }
  else
  {
    range = r;
  }
#endif
}

void RangeEncoder::writeInt(U32 sym)
{
  writeShort((U16)(sym % U16_MAX_PLUS_ONE)); // lower 16 bits
  writeShort((U16)(sym / U16_MAX_PLUS_ONE)); // UPPER 16 bits
}

void RangeEncoder::writeInt64(U64 sym)
{
  writeInt((U32)(sym % U32_MAX_PLUS_ONE)); // lower 32 bits
  writeInt((U32)(sym / U32_MAX_PLUS_ONE)); // UPPER 32 bits
}

void RangeEncoder::writeFloat(F32 sym)
{
  U32F32 u32f32;
  u32f32.f32 = sym;
  writeInt(u32f32.u32);
}

void RangeEncoder::writeDouble(F64 sym)
{
  U64F64 u64f64;
  u64f64.f64 = sym;
  writeInt64(u64f64.u64);
}

/* I do the normalization before I need a defined state instead of */
/* after messing it up. This simplifies starting and ending.       */
inline void RangeEncoder::normalize()
{
  while(range <= BOTTOM_VALUE) /* do we need renormalisation?  */
  {
    if (low < (U32)0xff<<SHIFT_BITS)  /* no carry possible --> output */
    {
      outstream->putByte(buffer);
      for(; help; help--)
      {
        outstream->putByte(0xff);
      }
      buffer = (U8)(low >> SHIFT_BITS);
    }
    else if (low & TOP_VALUE) /* carry now, no future carry */
    {
      outstream->putByte(buffer+1);
      for(; help; help--)
      {
        outstream->putByte(0);
      }
      buffer = (U8)(low >> SHIFT_BITS);
    }
    else                      /* passes on a potential carry */
    {
      help++;
    }
    range <<= 8;
    low = (low<<8) & (TOP_VALUE-1);
    bytecount++;
  }
}


/*
void RangeEncoder::writeRange(U32 range, U32 sym)
{
  assert(range && (sym < range));

  if (range > 4194303) // 22 bits
  {
    writeShort(sym&U16_MAX);
    sym = sym >> 16;
    range = range >> 16;
    range++;
  }
  U32 r, tmp;
  normalize();
  r = this->range / range;
  tmp = r * sym;
  low += tmp;
#ifdef EXTRAFAST
  this->range = r;
#else
  if (sym+1 < range)
  {
    this->range = r;
  }
  else
  {
    this->range -= tmp;
  }
#endif
}

void RangeEncoder::writeRange64(U64 range, U64 sym)
{
  assert(sym < range);
  if (range > U32_MAX) // 32 bits
  {
    writeInt((U32)(sym&U32_MAX));
    sym = sym >> 32;
    range = range >> 32;
    range++;
  }
  writeRange((U32)range, (U32)sym);
}
*/
