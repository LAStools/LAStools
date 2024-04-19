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

  FILE:  rangemodel.cpp
  
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
#include "rangemodel.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <stdexcept>

/* initialisation of model                             */
/* n   number of symbols in that model                 */
/* compress  set to 1 on compression, 0 on decompression */
/* targetrescale  desired rescaling interval, should be < 1<<(lg_totf+1) */
/* lg_totf  base2 log of total frequency count         */
RangeModel::RangeModel(U32 n, BOOL compress, I32 targetrescale, I32 lg_totf)
{
  this->n = n;
  this->targetrescale = targetrescale;
  this->lg_totf = lg_totf;
  cf = new U16[n+1];
  newf = new U16[n+1];
  if (lg_totf == 16)
  {
    cf[n] = 65535;
  }
  else
  {
    cf[n] = (1<<lg_totf);
  }
  cf[0] = 0;
  if (compress)
  {
    search = NULL;
  }
  else
  {
    searchshift = lg_totf - TBLSHIFT;
    search = new U16[(1<<TBLSHIFT)+1];
    search[1<<TBLSHIFT] = n-1;
  }
}

/* deletion                                            */
RangeModel::~RangeModel()
{
  delete [] cf;
  delete [] newf;
  if (search) delete [] search;
}

/* rescale frequency counts */
void RangeModel::dorescale()
{
  I32 i, c, missing;
  if (nextleft)  /* we have some more before actual rescaling */
  {
    incr++;
    left = nextleft;
    nextleft = 0;
    return;
  }
  if (rescale != targetrescale)  /* double rescale interval if needed */
  {
    rescale <<= 1;
    if (rescale > targetrescale)
    {
      rescale = targetrescale;
    }
  }
  c = missing = cf[n];  /* do actual rescaling */
  for(i=n-1; i; i--)
  {
    I32 tmp = newf[i];
    c -= tmp;
    cf[i] = c;
    tmp = tmp>>1 | 1;
    missing -= tmp;
    newf[i] = tmp;
  }
  if (c!=newf[0])
  {
//    fprintf(stderr,"BUG: rescaling left %d total frequency\n",c);
//    exit(1);
    throw std::runtime_error("internal error: RangeModel::dorescale()");
  }
  newf[0] = newf[0]>>1 | 1;
  missing -= newf[0];
  incr = missing / rescale;
  nextleft = missing % rescale;
  left = rescale - nextleft;
  if (search != NULL)
  {
    i=n;
    while (i)
    {
      I32 start, end;
      end = (cf[i]-1) >> searchshift;
      i--;
      start = cf[i] >> searchshift;
      while (start<=end)
      {
        search[start] = i;
        start++;
      }
    }
  }
}

/* reinitialisation of qsmodel                         */
/* table array of int's to be used for initialisation (NULL ok) */
void RangeModel::init(U32 *table)
{
  U32 i, end, initval;
  rescale = n >> 4 | 2;
  nextleft = 0;
  if (table)
  {
    I32 tmpn = 0;
    for(i=0; i<n; i++)
    {
      if (table[i])
      {
        tmpn++;
      }
    }
    initval = cf[n] / tmpn;
    end = cf[n] % tmpn;
    for (i=0; i<n; i++)
    {
      if (table[i])
      {
        if (end)
        {
          newf[i] = initval+1;
          end--;
        }
        else
        {
          newf[i] = initval;
        }
      }
      else
      {
        newf[i] = 0;        
      }
    }
  }
  else
  {
    initval = cf[n] / n;
    end = cf[n] % n;
    for (i=0; i<end; i++)
    {
      newf[i] = initval+1;
    }
    for (; i<n; i++)
    {
      newf[i] = initval;
    }
  }
  dorescale();
}

/* retrieval of estimated frequencies for a symbol     */
/* sym  symbol for which data is desired; must be <n   */
/* sy_f frequency of that symbol                       */
/* lt_f frequency of all smaller symbols together      */
/* the total frequency is 1<<LG_TOTF                   */
void RangeModel::getfreq(U32 sym, U32 *sy_f, U32 *lt_f )
{
  *sy_f = cf[sym+1] - (*lt_f = cf[sym]);
}

/* find out symbol for a given cumulative frequency    */
/* lt_f  cumulative frequency                          */
U32 RangeModel::getsym(U32 lt_f)
{
  U32 lo, hi;
  U16 *tmp;
  tmp = search+(lt_f>>searchshift);
  lo = *tmp;
  hi = *(tmp+1) + 1;
  while (lo+1 < hi )
  {
    I32 mid = (lo+hi)>>1;
    if (lt_f < cf[mid])
    {
      hi = mid;
    }
    else
    {
      lo = mid;
    }
  }
  return lo;
}

/* update model                                        */
/* sym  symbol that occurred (must be <n from init)    */
void RangeModel::update(U32 sym)
{
  if (left <= 0)
  {
    dorescale();
  }
  left--;
  newf[sym] += incr;
}
