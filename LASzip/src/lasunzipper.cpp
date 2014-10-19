/*
===============================================================================

  FILE:  lasunzipper.cpp
  
  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/
#include "lasunzipper.hpp"

#include <string.h>
#include <stdlib.h>

#include "bytestreamin_file.hpp"
#include "bytestreamin_istream.hpp"
#include "lasreadpoint.hpp"

bool LASunzipper::open(FILE* infile, const LASzip* laszip)
{
  if (!infile) return return_error("FILE* infile pointer is NULL");
  if (!laszip) return return_error("const LASzip* laszip pointer is NULL");
  count = 0;
  if (reader) delete reader;
  reader = new LASreadPoint();
  if (!reader) return return_error("alloc of LASreadPoint failed");
  if (!reader->setup(laszip->num_items, laszip->items, laszip)) return return_error("setup() of LASreadPoint failed");
  if (stream) delete stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamInFileLE(infile);
  else
    stream = new ByteStreamInFileBE(infile);
  if (!stream) return return_error("alloc of ByteStreamInFile failed");
  if (!reader->init(stream)) return return_error("init() of LASreadPoint failed");
  return true;
}

bool LASunzipper::open(istream& instream, const LASzip* laszip)
{
  if (!laszip) return return_error("const LASzip* laszip pointer is NULL");
  count = 0;
  if (reader) delete reader;
  reader = new LASreadPoint();
  if (!reader) return return_error("alloc of LASreadPoint failed");
  if (!reader->setup(laszip->num_items, laszip->items, laszip)) return return_error("setup() of LASreadPoint failed");
  if (stream) delete stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamInIstreamLE(instream);
  else
    stream = new ByteStreamInIstreamBE(instream);
  if (!stream) return return_error("alloc of ByteStreamInStream failed");
  if (!reader->init(stream)) return return_error("init() of LASreadPoint failed");
  return true;
}

bool LASunzipper::seek(const unsigned int position)
{
  if (!reader->seek(count, position)) return return_error("seek() of LASreadPoint failed");
  count = position;
  return true;
}

unsigned int LASunzipper::tell() const
{
  return count;
}

bool LASunzipper::read(unsigned char * const * point)
{
  count++;
  return (reader->read(point) == TRUE);
}

bool LASunzipper::close()
{
  BOOL done = TRUE;
  if (reader)
  {
    done = reader->done();
    delete reader;
    reader = 0;
  }
  if (stream)
  {
    delete stream;
    stream = 0;
  }
  if (!done) return return_error("done() of LASreadPoint failed");
  return true;
}

const char* LASunzipper::get_error() const
{
  return error_string;
}

bool LASunzipper::return_error(const char* error)
{
  char err[256];
  sprintf(err, "%s (LASzip v%d.%dr%d)", error, LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION);
  if (error_string) free(error_string);
  error_string = LASCopyString(err);
  return false;
}

LASunzipper::LASunzipper()
{
  error_string = 0;
  count = 0;
  stream = 0;
  reader = 0;
}

LASunzipper::~LASunzipper()
{
  if (error_string) free(error_string);
  if (reader || stream) close();
}
