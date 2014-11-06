/*
===============================================================================

  FILE:  laswriter_las.hpp
  
  CONTENTS:
  
    Writes LIDAR points to the LAS format (Version 1.x , April 29, 2008).

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    13 October 2014 -- changed default IO buffer size with setvbuf() to 262144
    5 November 2011 -- changed default IO buffer size with setvbuf() to 65536
    8 May 2011 -- added an option for variable chunking via chunk()
    23 April 2011 -- added additional open parameters to support chunking
    21 January 2011 -- adapted from laswriter to create abstract reader class
    3 December 2010 -- updated to (somewhat) support LAS format 1.3
    7 September 2008 -- updated to support LAS format 1.2 
    21 February 2007 -- created after eating Sarah's veggies with peanutsauce
  
===============================================================================
*/
#ifndef LAS_WRITER_LAS_HPP
#define LAS_WRITER_LAS_HPP

#include "laswriter.hpp"

#include <stdio.h>

#ifdef LZ_WIN32_VC6
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

class ByteStreamOut;
class LASwritePoint;

class LASwriterLAS : public LASwriter
{
public:

  bool refile(FILE* file);

  bool open(const LASheader* header, U32 compressor=LASZIP_COMPRESSOR_NONE, I32 requested_version=0, I32 chunk_size=50000);
  bool open(const char* file_name, const LASheader* header, U32 compressor=LASZIP_COMPRESSOR_NONE, I32 requested_version=0, I32 chunk_size=50000, I32 io_buffer_size=262144);
  bool open(FILE* file, const LASheader* header, U32 compressor=LASZIP_COMPRESSOR_NONE, I32 requested_version=0, I32 chunk_size=50000);
  bool open(ostream& ostream, const LASheader* header, U32 compressor=LASZIP_COMPRESSOR_NONE, I32 requested_version=0, I32 chunk_size=50000);

  bool write_point(const LASpoint* point);
  bool chunk();

  bool update_header(const LASheader* header, bool use_inventory=FALSE, bool update_extra_bytes=FALSE);
  I64 close(bool update_npoints=TRUE);

  LASwriterLAS();
  ~LASwriterLAS();

private:
  bool open(ByteStreamOut* stream, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size);
  ByteStreamOut* stream;
  LASwritePoint* writer;
  FILE* file;
  I64 header_start_position;
  bool writing_las_1_4;
};

#endif
