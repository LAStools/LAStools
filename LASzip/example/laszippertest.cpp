/*
===============================================================================

  FILE    laszippertest.cpp
  
  CONTENTS  
  
    This tool reads and writes point data in the LAS 1.X format compressed
    or uncompressed via the laszipper and lasunzipper interfaces that are
    not (!) used by LASzip or LASlib but are the binding to libLAS.

  PROGRAMMERS:

    martin.isenburg@gmail.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    14 January 2011 -- mpg@flaxen.com adds randomness and commandline controls
    10 January 2011 -- licensing change for LGPL release and liblas integration
    13 December 2010 -- created to test the remodularized laszip compressor
  
===============================================================================
*/

#include "laszipper.hpp"
#include "lasunzipper.hpp"

#ifdef LZ_WIN32_VC6
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//---------------------------------------------------------------------------

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

//---------------------------------------------------------------------------
// abstractions for doing I/O, which support VC6 streams, modern streams, and FILE*

class OStream
{
public:
  OStream(bool use_iostream, const char* filename) :
    m_use_iostream(use_iostream),
    m_filename(filename),
    ofile(NULL),
    streamo(NULL)
  {
    if (m_use_iostream)
    {
#ifdef LZ_WIN32_VC6 
      ofb.open(filename, ios::out);
      ofb.setmode(filebuf::binary);
      streamo = new ostream(&ofb);
#else
      streamo = new ofstream();
      streamo->open(filename, std::ios::out | std::ios::binary );
#endif 
    }
    else
    {
      ofile = fopen(filename, "wb");
    }
  };

  ~OStream()
  {
    if (m_use_iostream)
    {
      delete streamo;
#ifdef LZ_WIN32_VC6 
      ofb.close();
#endif
    }
    else
    {
      if (ofile)
        fclose(ofile);
    }
  };

  bool m_use_iostream;
  const char* m_filename;
  FILE* ofile;
  filebuf ofb;
#ifdef LZ_WIN32_VC6 
  ostream* streamo;
#else
  ofstream* streamo;
#endif
};

//---------------------------------------------------------------------------

struct IStream
{
public:
  IStream(bool use_iostream, const char* filename) :
    m_use_iostream(use_iostream),
    m_filename(filename),
    ifile(NULL),
    streami(NULL)
  {
    if (m_use_iostream)
    {
#ifdef LZ_WIN32_VC6 
      ifb.open(filename, ios::in);
      ifb.setmode(filebuf::binary);
      streami = new istream(&ifb);
#else
      streami = new ifstream();
      streami->open(filename, std::ios::in | std::ios::binary);
#endif 
    }
    else
    {
      ifile = fopen(filename, "rb");
    }
  };

  ~IStream()
  {
    if (m_use_iostream)
    {
      delete streami;
#ifdef LZ_WIN32_VC6 
      ifb.close();
#endif
    }
    else
    {
      if (ifile)
        fclose(ifile);
    }
  };

  bool m_use_iostream;
  const char* m_filename;
  FILE* ifile;
  filebuf ifb;
#ifdef LZ_WIN32_VC6 
  istream* streami;
#else
  ifstream* streami;
#endif
};


//---------------------------------------------------------------------------

class PointData
{
public:
  PointData(unsigned char type=5, unsigned short size=70)
  {
    point_type = type;
    point_size = size;
    point = 0;
    point_data = 0;
  }

  bool setup(unsigned int num_items, const LASitem* items)
  {
    unsigned int offset = 0;
    if (point) delete [] point;
    point = new unsigned char*[num_items];
    if (point_data) delete [] point_data;
    point_data = new unsigned char[point_size];
    for (unsigned int i = 0; i < num_items; i++)
    {
      point[i] = &(point_data[offset]);
      offset += items[i].size;
    }
    return (offset == point_size);
  }

  ~PointData()
  {
    if (point) delete [] point;
    if (point_data) delete [] point_data;
  }

  unsigned char point_type;
  unsigned short point_size;
  unsigned char** point;
  unsigned char* point_data;
};


//---------------------------------------------------------------------------

class Settings
{
public:
  Settings(unsigned int num_pts, bool random, bool use_stream) :
    num_points(num_pts),
    use_random(random),
    use_iostream(use_stream)
  {
    logfile = fopen("zippertest.log","w");
    return;
  }

  ~Settings()
  {
    fclose(logfile);
    return;
  }

  unsigned num_points;
  bool use_random;
  unsigned int seed;
  bool use_iostream;
  FILE* logfile;
};

static Settings* settings = NULL; // singleton


//---------------------------------------------------------------------------

static void log(const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);

  va_start(args, format);
  vfprintf(settings->logfile, format, args);
  va_end(args);  
  fflush(settings->logfile);

  return;
}

//---------------------------------------------------------------------------

static LASzipper* make_zipper(OStream* ost, const LASzip* laszip)
{
  LASzipper* zipper = new LASzipper();
  if (zipper == 0)
  {
    log("ERROR: could not alloc laszipper\n");
    exit(1);
  }
  bool success;
  if (ost->m_use_iostream)
    success = zipper->open(*ost->streamo, laszip);
  else
    success = zipper->open(ost->ofile, laszip);
  if (!success)
  {
    log("ERROR: could not open laszipper with %s because %s\n", ost->m_filename, zipper->get_error());
    exit(1);
  }
  return zipper;
}

//---------------------------------------------------------------------------

static LASunzipper* make_unzipper(IStream* ist, const LASzip* laszip)
{
  LASunzipper* unzipper = new LASunzipper();
  if (unzipper == 0)
  {
    log("ERROR: could not alloc lasunzipper\n");
    exit(1);
  }
  bool success;
  if (ist->m_use_iostream)
    success = unzipper->open(*ist->streami, laszip);
  else
    success = unzipper->open(ist->ifile, laszip);
  if (!success)
  {
    log("ERROR: could not open laszipper with %s because %s\n", ist->m_filename, unzipper->get_error());
    exit(1);
  }
  return unzipper;
}

//---------------------------------------------------------------------------

static void write_points(LASzipper* zipper, PointData& data)
{
  if (zipper==NULL)
    return;

  double start_time, end_time;
  unsigned char c;
  unsigned int i,j;

  // the two branches of this IF are the same, except for the use of a random number;
  // we keep the random case separate, so that we can get fastest timing tests w/o random data
  if (settings->use_random)
  {
    srand(settings->seed);
    start_time = taketime();
    c = rand() % 256;
    for (i = 0; i < settings->num_points; i++)
    {
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c = rand() % 256;
      }
      zipper->write(data.point);
    }
  }
  else
  {
    start_time = taketime();
    c = 0;
    for (i = 0; i < settings->num_points; i++)
    {
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c++;
      }
      zipper->write(data.point);
    }
  }

  if (!zipper->close())
  {
    log("ERROR on zipper->close(): %s\n", zipper->get_error());
  }
  end_time = taketime();

  log("laszipper wrote %u points in %g seconds\n", settings->num_points, end_time-start_time);

  return;
}


//---------------------------------------------------------------------------

static void read_points(LASunzipper* unzipper, PointData& data)
{
  if (unzipper==NULL)
    return;

  unsigned char c;
  unsigned int i,j;
  unsigned int num_errors;
  double start_time, end_time;

  start_time = taketime();
  num_errors = 0;

  if (settings->use_random)
  {
    srand(settings->seed);
    c = rand() % 256;
    for (i = 0; i < settings->num_points; i++)
    {
      unzipper->read(data.point);
      for (j = 0; j < data.point_size; j++)
      {
        if (data.point_data[j] != c)
        {
          log("%d %d %d != %d\n", i, j, data.point_data[j], c);
          num_errors++;
          if (num_errors > 20) break;
        }
        c = rand() % 256;
      }
      if (num_errors > 20) break;
    }
  }
  else
  {
    c = 0;
    for (i = 0; i < settings->num_points; i++)
    {
      unzipper->read(data.point);
      for (j = 0; j < data.point_size; j++)
      {
        if (data.point_data[j] != c)
        {
          log("%d %d %d != %d\n", i, j, data.point_data[j], c);
          num_errors++;
          if (num_errors > 20) break;
        }
        c++;
      }
      if (num_errors > 20) break;
    }
  }

  if (!unzipper->close())
  {
    log("ERROR on unzipper->close(): %s\n", unzipper->get_error());
  }
  end_time = taketime();

  if (num_errors)
  {
    log("ERROR: with lasunzipper %d\n", num_errors);
    getc(stdin);
  }
  else
  {
    log("SUCCESS: lasunzipper read %u points in %g seconds\n", settings->num_points, end_time-start_time);
  }

  return;
}

//---------------------------------------------------------------------------

static void write_points_seek(LASzipper* zipper, PointData& data)
{
  if (zipper==NULL)
    return;

  double start_time, end_time;
  unsigned char c;
  unsigned int i,j;

  start_time = taketime();

  // the two branches of this IF are the same, except for the use of a random number;
  // we keep the random case separate, so that we can get fastest timing tests w/o random data
  if (settings->use_random)
  {
    for (i = 0; i < settings->num_points; i++)
    {
      srand(i);
      c = rand() % 256;
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c = rand() % 256;
      }
      zipper->write(data.point);
    }
  }
  else
  {
    for (i = 0; i < settings->num_points; i++)
    {
      c = (unsigned char)i;
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c++;
      }
      zipper->write(data.point);
    }
  }

  if (!zipper->close())
  {
    log("ERROR on zipper->close(): %s\n", zipper->get_error());
  }
  end_time = taketime();

  log("laszipper wrote %u points in %g seconds\n", settings->num_points, end_time-start_time);

  return;
}

//---------------------------------------------------------------------------

static void read_points_seek(LASunzipper* unzipper, PointData& data)
{
  if (unzipper==NULL)
    return;

  unsigned char c;
  unsigned int i,j;
  unsigned int num_errors, num_seeks;
  double start_time, end_time;

  start_time = taketime();
  num_errors = 0;
  num_seeks = 0;

  if (settings->use_random)
  {
    for (i = 0; i < settings->num_points; i++)
    {
      if (i%1000 == 0)
      {
        if (num_seeks > 100) break;
        int s = (rand()*rand())%settings->num_points;
        fprintf(stderr, "at position %d seeking to %d\n", i, s);
        unzipper->seek(s);
        num_seeks++;
        i = s;
      }
      unzipper->read(data.point);
      srand(i);
      c = rand() % 256;
      for (j = 0; j < data.point_size; j++)
      {
        if (data.point_data[j] != c)
        {
          log("%d %d %d != %d\n", i, j, data.point_data[j], c);
          num_errors++;
          if (num_errors > 20) break;
        }
        c = rand() % 256;
      }
      if (num_errors > 20) break;
    }
  }
  else
  {
    for (i = 0; i < settings->num_points; i++)
    {
      if (i%1000 == 0)
      {
        if (num_seeks > 100) break;
        int s = (rand()*rand())%settings->num_points;
        fprintf(stderr, "at position %d seeking to %d\n", i, s);
        unzipper->seek(s);
        num_seeks++;
        i = s;
      }
      unzipper->read(data.point);
      c = (unsigned char)i;
      for (j = 0; j < data.point_size; j++)
      {
        if (data.point_data[j] != c)
        {
          log("%d %d %d != %d\n", i, j, data.point_data[j], c);
          num_errors++;
          if (num_errors > 20) break;
        }
        c++;
      }
      if (num_errors > 20) break;
    }
  }

  if (!unzipper->close())
  {
    log("ERROR on unzipper->close(): %s\n", unzipper->get_error());
  }
  end_time = taketime();

  if (num_errors)
  {
    log("ERROR: with lasunzipper %d\n", num_errors);
    getc(stdin);
  }
  else
  {
    log("SUCCESS: lasunzipper read %u bytes in %g seconds\n", settings->num_points, end_time-start_time);
  }

  return;
}

//---------------------------------------------------------------------------

static void write_points_explicit_chunk(LASzipper* zipper, PointData& data)
{
  if (zipper==NULL)
    return;

  double start_time, end_time;
  unsigned char c;
  unsigned int i,j;
  unsigned int next_chunk = settings->num_points/10;

  // the two branches of this IF are the same, except for the use of a random number;
  // we keep the random case separate, so that we can get fastest timing tests w/o random data
  if (settings->use_random)
  {
    srand(settings->seed);
    start_time = taketime();
    c = rand() % 256;
    for (i = 0; i < settings->num_points; i++)
    {
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c = rand() % 256;
      }
      zipper->write(data.point);
      if (i == next_chunk)
      {
        zipper->chunk();
        next_chunk += settings->num_points/10;
        next_chunk -= c;
      }
    }
  }
  else
  {
    start_time = taketime();
    c = 0;
    for (i = 0; i < settings->num_points; i++)
    {
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c++;
      }
      zipper->write(data.point);
      if (i == next_chunk)
      {
        zipper->chunk();
        next_chunk += settings->num_points/10;
        next_chunk -= (rand() % 256);
      }
    }
  }

  if (!zipper->close())
  {
    log("ERROR on zipper->close(): %s\n", zipper->get_error());
  }
  end_time = taketime();

  log("laszipper wrote %u points in %g seconds\n", settings->num_points, end_time-start_time);

  return;
}

//---------------------------------------------------------------------------

static void write_points_explicit_chunk_seek(LASzipper* zipper, PointData& data)
{
  if (zipper==NULL)
    return;

  double start_time, end_time;
  unsigned char c;
  unsigned int i,j;
  unsigned int next_chunk = settings->num_points/10;

  start_time = taketime();

  // the two branches of this IF are the same, except for the use of a random number;
  // we keep the random case separate, so that we can get fastest timing tests w/o random data
  if (settings->use_random)
  {
    for (i = 0; i < settings->num_points; i++)
    {
      srand(i);
      c = rand() % 256;
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c = rand() % 256;
      }
      zipper->write(data.point);
      if (i == next_chunk)
      {
        zipper->chunk();
        next_chunk += settings->num_points/10;
        next_chunk -= c;
      }
    }
  }
  else
  {
    for (i = 0; i < settings->num_points; i++)
    {
      c = (unsigned char)i;
      for (j = 0; j < data.point_size; j++)
      {
        data.point_data[j] = c;
        c++;
      }
      zipper->write(data.point);
      if (i == next_chunk)
      {
        zipper->chunk();
        next_chunk += settings->num_points/10;
        next_chunk -= (rand() % 256);
      }
    }
  }

  if (!zipper->close())
  {
    log("ERROR on zipper->close(): %s\n", zipper->get_error());
  }
  end_time = taketime();

  log("laszipper wrote %u points in %g seconds\n", settings->num_points, end_time-start_time);

  return;
}

//---------------------------------------------------------------------------

static void run_test(const char* filename, PointData& data, unsigned short compressor, int requested_version=-1, int chunk_size=-1, bool random_seeks=false)
{
  //
  // COMPRESSION
  //

  // setting up LASzip parameters
  LASzip laszip;
  if (!laszip.setup(data.point_type, data.point_size, compressor))
  {
    log("ERROR on laszip.setup(): %s\n", laszip.get_error());
  }
  if (requested_version > -1) laszip.request_version((unsigned short)requested_version);
  if (chunk_size > -1) laszip.set_chunk_size((unsigned int)chunk_size);

  // packing up LASzip
  unsigned char* bytes;
  int num;
  if (!laszip.pack(bytes, num))
  {
    log("ERROR on laszip.pack(): %s\n", laszip.get_error());
  }

  // creating the output stream
  OStream* ost = new OStream(settings->use_iostream, filename);

  // creating the zipper
  LASzipper* laszipper = make_zipper(ost, &laszip);

  // allocating the data to read from
  data.setup(laszip.num_items, laszip.items);

  // writing the points
  if (chunk_size == 0)
  {
    if (random_seeks)
      write_points_explicit_chunk_seek(laszipper, data);
    else
      write_points_explicit_chunk(laszipper, data);
  }
  else
  {
    if (random_seeks)
      write_points_seek(laszipper, data);
    else
      write_points(laszipper, data);
  }

  // cleaning up
  delete laszipper;
  delete ost;

  //
  // DECOMPRESSION
  //

  // setting up LASzip parameters
  LASzip laszip_dec;
  if (!laszip_dec.unpack(bytes, num))
  {
    log("ERROR on laszip_dec.unpack(): %s\n", laszip_dec.get_error());
  }

  // creating the input stream
  IStream* ist = new IStream(settings->use_iostream, filename);

  // creating the unzipper
  LASunzipper* lasunzipper = make_unzipper(ist, &laszip_dec);

  // allocating the data to write into
  data.setup(laszip_dec.num_items, laszip_dec.items);
  
  // reading the points
  if (random_seeks)
    read_points_seek(lasunzipper, data);
  else
    read_points(lasunzipper, data);

  // cleaning up
  delete lasunzipper;
  delete ist;

  return;
}

//---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  unsigned int num_points = 100000;
  bool use_iostream = false;
  bool run_forever = false;
  bool use_random = false;
  unsigned int user_seed = (unsigned int)time(NULL);

  for (int i=1; i<argc; i++)
  {
    const char* p = argv[i];
    if (strcmp(p,"-n")==0)
    {
      ++i;
      num_points = atoi(argv[i]);
    }
    else if (strcmp(p,"-s")==0)
    {
      use_iostream = true;
    }
    else if (strcmp(p,"-f")==0)
    {
      run_forever = true;
    }
    else if (strcmp(p,"-r")==0)
    {
      use_random = true;
    }
    else if (strcmp(p,"-x")==0)
    {
      ++i;
      user_seed = atoi(argv[i]);
    }    
    else
    {
      fprintf(stderr, "Usage   ziptest [-n NUMBER] [-s] [-f] [-r]\n");
      fprintf(stderr, "   -n    number of points to process (default   100000)\n");
      fprintf(stderr, "   -s    use C++ stream I/O (default   use FILE* I/O)\n");
      fprintf(stderr, "   -f    run forever (default   run just one pass)\n");
      fprintf(stderr, "   -r    use random input data (default   use fixed data)\n");
      fprintf(stderr, "   -x    set random seed, instead of current time (default: use time)\n");
      exit(1);
    }
  }

  settings = new Settings(num_points, use_random, use_iostream);

  log("Settings:\n");
  log("  num_points=%u, use_iostream=%s, run_forever=%s, use_random=%s, user_seed=%d\n",
    num_points, use_iostream?"true":"false", run_forever?"true":"false", use_random?"true":"false", user_seed);

  settings->seed = user_seed;

  unsigned int run = 1;
  do
  {
  PointData data;

  // use a seed based on the current time
  if (run_forever && settings->use_random)
  {
    settings->seed = (unsigned int)time(NULL);
    log("Seed: %u\n", settings->seed);
  }

  run_test("test.tmp", data, LASZIP_COMPRESSOR_NONE);

  // not chunked version 1.0 and sequential version 2.0
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_NOT_CHUNKED, 1);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_NOT_CHUNKED, 1);
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_NOT_CHUNKED, 2);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_NOT_CHUNKED, 2);

  // chunk every 10000 points
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 1, 10000);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 1, 10000);
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 2, 10000);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 2, 10000);

  // explicit chunk() calls
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 1, 0);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 1, 0); 
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 2, 0);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 2, 0);

  // chunk every 10000 points and random seeks during read
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 1, 10000, true);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 1, 10000, true);
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 2, 10000, true);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 2, 10000, true);

  // explicit chunk() calls and random seeks during read
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 1, 0, true);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 1, 0, true);
  log("run_test(test.tmp, data, LASZIP_COMPRESSOR_CHUNKED, 2, 0, true);\n");
  run_test("test.tmp", data, LASZIP_COMPRESSOR_CHUNKED, 2, 0, true);

  log("Finished %u runs\n\n", run);
  ++run;
  } while (run_forever);

  return 0;
}
