/*
===============================================================================

  FILE:  lasspatial.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasspatial.hpp"

#include "bytestreamin.hpp"
#include "bytestreamout.hpp"

#include "lasquadtree.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LASspatial* LASspatialReadWrite::read(ByteStreamIn* stream) const
{
  char signature[4];
  try { stream->getBytes((U8*)signature, 4); } catch(...)
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): reading signature\n");
    return FALSE;
  }
  if (strncmp(signature, "LASS", 4) != 0)
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): wrong signature %4s instead of 'LASS'\n", signature);
    return FALSE;
  }
  U32 type;
  try { stream->getBytes((U8*)&type, 4); } catch(...)
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): reading type\n");
    return 0;
  }
  LASspatial* spatial;
  if (type == LAS_SPATIAL_QUAD_TREE)
  {
    spatial = new LASquadtree;
    if (!spatial->read(stream))
    {
      delete spatial;
      return 0;
    }
    return spatial;
  }
  else
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): unknown type %u\n", type);
    return 0;
  }
  return spatial;
}

bool LASspatialReadWrite::write(const LASspatial* spatial, ByteStreamOut* stream) const
{
  if (!stream->putBytes((U8*)"LASS", 4))
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): writing signature\n");
    return FALSE;
  }
  U32 type = LAS_SPATIAL_QUAD_TREE;
  if (!stream->put32bitsLE((U8*)&type))
  {
    fprintf(stderr,"ERROR (LASspatialReadWrite): writing type %u\n", type);
    return FALSE;
  }
  return spatial->write(stream);
}
