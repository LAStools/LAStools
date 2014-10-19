/*
===============================================================================

  FILE:  laszipper.cpp
  
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
#include "laszipper.hpp"

#include <string.h>
#include <stdlib.h>

#include "bytestreamout_file.hpp"
#include "bytestreamout_ostream.hpp"
#include "laswritepoint.hpp"

bool LASzipper::open(FILE* outfile, const LASzip* laszip)
{
  if (!outfile) return return_error("FILE* outfile pointer is NULL");
  if (!laszip) return return_error("const LASzip* laszip pointer is NULL");
  count = 0;
  if (writer) delete writer;
  writer = new LASwritePoint();
  if (!writer) return return_error("alloc of LASwritePoint failed");
  if (!writer->setup(laszip->num_items, laszip->items, laszip)) return return_error("setup() of LASwritePoint failed");
  if (stream) delete stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamOutFileLE(outfile);
  else
    stream = new ByteStreamOutFileBE(outfile);
  if (!stream) return return_error("alloc of ByteStreamOutFile failed");
  if (!writer->init(stream)) return return_error("init() of LASwritePoint failed");
  return true;
}

bool LASzipper::open(ostream& outstream, const LASzip* laszip)
{
  if (!laszip) return return_error("const LASzip* laszip pointer is NULL");
  count = 0;
  if (writer) delete writer;
  writer = new LASwritePoint();
  if (!writer) return return_error("alloc of LASwritePoint failed");
  if (!writer->setup(laszip->num_items, laszip->items, laszip)) return return_error("setup() of LASwritePoint failed");
  if (stream) delete stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamOutOstreamLE(outstream);
  else
    stream = new ByteStreamOutOstreamBE(outstream);
  if (!stream) return return_error("alloc of ByteStreamOutStream failed");
  if (!writer->init(stream)) return return_error("init() of LASwritePoint failed");
  return true;
}

bool LASzipper::write(const unsigned char * const * point)
{
  count++;
  return (writer->write(point) == TRUE);
}

bool LASzipper::chunk()
{
  if (!writer->chunk()) return return_error("chunk() of LASwritePoint failed");
  return true;
}

bool LASzipper::close()
{
  BOOL done = TRUE;
  if (writer)
  {
    done = writer->done();
    delete writer;
    writer = 0;
  }
  if (stream)
  {
    delete stream;
    stream = 0;
  }
  if (!done) return return_error("done() of LASwritePoint failed");
  return true;
}

const char* LASzipper::get_error() const
{
  return error_string;
}

bool LASzipper::return_error(const char* error)
{
  char err[256];
  sprintf(err, "%s (LASzip v%d.%dr%d)", error, LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION);
  if (error_string) free(error_string);
  error_string = LASCopyString(err);
  return false;
}

LASzipper::LASzipper()
{
  error_string = 0;
  count = 0;
  stream = 0;
  writer = 0;
}

LASzipper::~LASzipper()
{
  if (error_string) free(error_string);
  if (writer || stream) close();
}
