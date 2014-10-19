/*
===============================================================================

  FILE:  laszipper.hpp
  
  CONTENTS:
  
    Writes (optionally compressed) LIDAR points to LAS formats 1.0 - 1.3. This
    particular class is only used for adding LASzip to libLAS (not to LASlib).

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    8 May 2011 -- added an option for variable chunking via chunk()
    23 April 2011 -- changed interface for simplicity and chunking support
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from LASwriter/LASreader after Howard got pushy (-;
  
===============================================================================
*/
#ifndef LAS_ZIPPER_HPP
#define LAS_ZIPPER_HPP

#include <stdio.h>

#include "laszip.hpp"

#ifdef LZ_WIN32_VC6
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

class ByteStreamOut;
class LASwritePoint;

class LASZIP_DLL LASzipper
{
public:
  bool open(FILE* outfile, const LASzip* laszip);
  bool open(ostream& outstream, const LASzip* laszip);

  bool write(const unsigned char* const * point);
  bool chunk();
  bool close();

  LASzipper();
  ~LASzipper();

  // in case a function returns false this string describes the problem
  const char* get_error() const;

private:
  unsigned int count;
  ByteStreamOut* stream;
  LASwritePoint* writer;
  bool return_error(const char* err);
  char* error_string;
};

#endif
