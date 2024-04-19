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

  FILE:  rangedecoder.cpp
  
  CONTENTS:
      
    see header file

  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2003 martin isenburg (isenburg@cs.unc.edu)
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see header file
  
===============================================================================
*/
#include "rangedecoder.hpp"
#include "rangemodel.hpp"

#include <string.h>
#include <assert.h>

RangeDecoder::RangeDecoder()
{
  instream = 0;
}

RangeDecoder::~RangeDecoder()
{
}

I32 RangeDecoder::init(ByteStreamIn* instream)
{
  assert(instream);
  this->instream = instream;
  buffer = instream->getByte();
  assert(buffer == HEADERBYTE);
  buffer = instream->getByte();
  low = buffer >> (8-EXTRA_BITS);
  range = (U32)1 << EXTRA_BITS;
  return 0;
}

void RangeDecoder::done()
{
  normalize(); /* use up all bytes */
  instream = 0;
}

EntropyModel* RangeDecoder::createBitModel()
{
  return createSymbolModel(2);
}

void RangeDecoder::initBitModel(EntropyModel* model)
{
  initSymbolModel(model);
}

void RangeDecoder::destroyBitModel(EntropyModel* model)
{
  destroySymbolModel(model);
}

EntropyModel* RangeDecoder::createSymbolModel(U32 n)
{
  RangeModel* m = new RangeModel(n, FALSE);
  return (EntropyModel*)m;
}

void RangeDecoder::initSymbolModel(EntropyModel* model, U32 *table)
{
  assert(model);
  RangeModel* m = (RangeModel*)model;
  m->init(table);
}

void RangeDecoder::destroySymbolModel(EntropyModel* model)
{
  RangeModel* m = (RangeModel*)model;
  delete m;
}

U32 RangeDecoder::decodeBit(EntropyModel* model)
{
  return decodeSymbol(model);
}

U32 RangeDecoder::decodeSymbol(EntropyModel* model)
{
  RangeModel* m = (RangeModel*)model;
  U32 sym;
  U32 ltfreq;
  U32 syfreq;
  U32 tmp;
  U32 lg_totf = m->lg_totf;

  normalize();
  help = this->range>>lg_totf;
  ltfreq = low/help;
#ifdef EXTRAFAST
  ltfreq = ltfreq;
#else
  ltfreq = ((ltfreq>>lg_totf) ? (1<<lg_totf)-1 : ltfreq);
#endif

  sym = m->getsym(ltfreq);
  m->getfreq(sym,&syfreq,&ltfreq);

  tmp = help * ltfreq;
  low -= tmp;
#ifdef EXTRAFAST
  this->range = help * syfreq;
#else
  if ((ltfreq + syfreq) < (1u<<lg_totf))
  {
    this->range = help * syfreq;
  }
  else
  {
    this->range -= tmp;
  }
#endif

  m->update(sym);

  return sym;
}

/* Decode a bit without modelling                           */
U32 RangeDecoder::readBit()
{
  U32 tmp;
  tmp = culshift(1);
  update(1, tmp, 2);
  return tmp;
}

/* Decode bits without modelling                            */
U32 RangeDecoder::readBits(U32 bits)
{
  U32 tmp;
  if (bits > 21) // 22 bits
  {
    tmp = readShort();
    U32 tmp1 = readBits(bits - 16) << 16;
    return (tmp1|tmp);
  }
  tmp = culshift(bits);
  update(1, tmp, 1u<<bits);
  return tmp;
}

/* Decode a byte without modelling                           */
U8 RangeDecoder::readByte()
{
  U8 tmp = culshift(8);
  update(1, tmp, 1u<<8);
  return tmp;
}

/* Decode a short without modelling                          */
U16 RangeDecoder::readShort()
{
  unsigned short tmp = culshift(16);
  update(1, tmp, 1u<<16);
  return tmp;
}

/* Decode an unsigned int without modelling                  */
U32 RangeDecoder::readInt()
{
  U32 lowerInt = readShort();
  U32 upperInt = readShort();
  return upperInt*U16_MAX_PLUS_ONE+lowerInt;
}

/* Decode a float without modelling                          */
F32 RangeDecoder::readFloat()
{
  U32F32 u32f32;
  u32f32.u32 = readInt();
  return u32f32.f32;
}

/* Decode an unsigned 64 bit int without modelling           */
U64 RangeDecoder::readInt64()
{
  U64 lowerInt = readInt();
  U64 upperInt = readInt();
  return upperInt*U32_MAX_PLUS_ONE+lowerInt;
}

/* Decode a double without modelling                         */
F64 RangeDecoder::readDouble()
{
  U64F64 u64f64;
  u64f64.u64 = readInt64();
  return u64f64.f64;
}

U32 RangeDecoder::culshift(U32 shift)
{
  U32 tmp;
  normalize();
  help = range>>shift;
  tmp = low/help;
#ifdef EXTRAFAST
  return tmp;
#else
  return (tmp>>shift ? (1u<<shift)-1 : tmp);
#endif
}

/* Update decoding state                                     */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
void RangeDecoder::update(U32 sy_f, U32 lt_f, U32 tot_f)
{
  U32 tmp;
  tmp = help * lt_f;
  low -= tmp;
#ifdef EXTRAFAST
  this->range = help * sy_f;
#else
  if (lt_f + sy_f < tot_f)
  {
    this->range = help * sy_f;
  }
  else
  {
    this->range -= tmp;
  }
#endif
}

inline void RangeDecoder::normalize()
{
  while (range <= BOTTOM_VALUE)
  {
    low = (low<<8) | ((buffer<<EXTRA_BITS)&0xff);
    buffer = instream->getByte();
    low |= buffer >> (8-EXTRA_BITS);
    range <<= 8;
  }
}

/*
U32 RangeDecoder::readRange(U32 range)
{
  U32 tmp;
  U32 tmp1;

  if (range > 4194303) // 22 bits
  {
    tmp = readShort();
    range = range >> 16;
    range++;
    tmp1 = readRange(range) << 16;
    return (tmp1|tmp);
  }
  
  normalize();
  help = this->range/range;
  tmp = low/help;
#ifdef EXTRAFAST
  tmp = tmp;
#else
  tmp = (tmp>=range ? range-1 : tmp);
#endif

  tmp1 = (help * tmp);
  low -= tmp1;
#ifdef EXTRAFAST
  this->range = help;
#else
  if (tmp+1 < range)
  {
    this->range = help;
  }
  else
  {
    this->range -= tmp1;
  }
#endif

  return tmp;
}

U64 RangeDecoder::readRange64(U64 range)
{
  if (range > U32_MAX) // 32 bits
  {
    U64 tmp;
    U64 tmp1;
    tmp = readInt();
    range = range >> 32;
    range++;
    tmp1 = ((U64)(readRange((U32)(range)))) << 32;
    return (tmp1|tmp);
  }
  else
  {
    return (U64)(readRange((U32)(range)));
  }
}
*/
