/*
===============================================================================

  FILE:  LASreaditem.hpp
  
  CONTENTS:
  
    Common interface for all classes that read the items that compose a point.

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
  
    10 January 2011 -- licensing change for LGPL release and liblas integration
    7 December 2010 -- refactored after getting invited to KAUST in Saudi Arabia
  
===============================================================================
*/
#ifndef LAS_READ_ITEM_H
#define LAS_READ_ITEM_H

#include "mydefs.hpp"

class ByteStreamIn;

class LASreadItem
{
public:
  virtual void read(U8* item)=0;

  virtual ~LASreadItem(){};
};

class LASreadItemRaw : public LASreadItem
{
public:
  LASreadItemRaw()
  {
    instream = 0;
  };
  BOOL init(ByteStreamIn* instream)
  {
    if (!instream) return FALSE;
    this->instream = instream;
    return TRUE;
  };
  virtual ~LASreadItemRaw(){};
protected:
  ByteStreamIn* instream;
};

class LASreadItemCompressed : public LASreadItem
{
public:
  virtual BOOL init(const U8* item)=0;

  virtual ~LASreadItemCompressed(){};
};

#endif
