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

  FILE:  rangeencoder.hpp
  
  CONTENTS:
      
  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    Copyright (C) 2003 Martin Isenburg (isenburg@cs.unc.edu)
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    15 December 2010 -- unified framework for all entropy coders
    8 December 2010 -- unified framework for all entropy coders
    28 June 2004 -- added an option for NOT storing the code characters at all 
    14 January 2003 -- adapted from michael schindler's code before SIGGRAPH
  
===============================================================================
*/
#ifndef RANGEENCODER_HPP
#define RANGEENCODER_HPP

#include "entropyencoder.hpp"

class RangeEncoder : public EntropyEncoder
{
public:

/* Constructor & Deconstructor                               */
  RangeEncoder();
  ~RangeEncoder();

/* Manage the encoder                                        */
  I32 init(ByteStreamOut* outstream);
  void done();

/* Manage an entropy model for a single bit                  */
  EntropyModel* createBitModel();
  void initBitModel(EntropyModel* model);
  void destroyBitModel(EntropyModel* model);

/* Manage an entropy model for n symbols (table optional)    */
  EntropyModel* createSymbolModel(U32 n);
  void initSymbolModel(EntropyModel* model, U32 *init=0);
  void destroySymbolModel(EntropyModel* model);

/* Encode with modelling                                     */
  void encodeBit(EntropyModel* model, U32 sym);

/* Encode with modelling                                     */
  void encodeSymbol(EntropyModel* model, U32 sym);

/* Encode a bit without modelling                            */
  void writeBit(U32 sym);

/* Encode bits without modelling                             */
  void writeBits(U32 bits, U32 sym);

/* Encode an unsigned char without modelling                 */
  void writeByte(U8 sym);

/* Encode an unsigned short without modelling                */
  void writeShort(U16 sym);

/* Encode an unsigned int without modelling                  */
  void writeInt(U32 sym);

/* Encode a float without modelling                          */
  void writeFloat(F32 sym);

/* Encode an unsigned 64 bit int without modelling           */
  void writeInt64(U64 sym);

/* Encode a double without modelling                         */
  void writeDouble(F64 sym);

private:
  ByteStreamOut* outstream;

  inline void normalize();
  U32 low;           /* low end of interval */
  U32 range;         /* length of interval */
  U32 help;          /* bytes_to_follow resp. intermediate value */
  U8 buffer;         /* buffer for input/output */
  /* the following is used only when encoding */
  U32 bytecount;     /* counter for output bytes  */
};

#endif
