/*
===============================================================================

  FILE:  laswriter.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2017, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "laswriter.hpp"

#include "lasmessage.hpp"
#include "laswriter_las.hpp"
#include "laswriter_bin.hpp"
#include "laswriter_qfit.hpp"
#include "laswriter_wrl.hpp"
#include "laswriter_txt.hpp"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void LASwriter::dealloc()
{
  delete this;
}

BOOL LASwriteOpener::is_piped() const
{
  return ((file_name == 0) && use_stdout);
}

LASwriter* LASwriteOpener::open(const LASheader* header)
{
  if (use_nil)
  {
    LASwriterLAS* laswriterlas = new LASwriterLAS();
    if (!laswriterlas->open(header, (format == LAS_TOOLS_FORMAT_LAZ ? (native ? LASZIP_COMPRESSOR_LAYERED_CHUNKED : LASZIP_COMPRESSOR_CHUNKED) : LASZIP_COMPRESSOR_NONE), requested_version, chunk_size))
    {
      laserror("cannot open laswriterlas to NULL");
      delete laswriterlas;
      return 0;
    }
    return laswriterlas;
  }
  else if (file_name)
  {
    if (format <= LAS_TOOLS_FORMAT_LAZ)
    {
      LASwriterLAS* laswriterlas = new LASwriterLAS();
      if (!laswriterlas->open(file_name, header, (format == LAS_TOOLS_FORMAT_LAZ ? (native ? LASZIP_COMPRESSOR_LAYERED_CHUNKED : LASZIP_COMPRESSOR_CHUNKED) : LASZIP_COMPRESSOR_NONE), requested_version, chunk_size, io_obuffer_size))
      {
        laserror("cannot open laswriterlas with file name '%s'", file_name);
        delete laswriterlas;
        return 0;
      }
      return laswriterlas;
    }
    else if (format == LAS_TOOLS_FORMAT_TXT)
    {
      LASwriterTXT* laswritertxt = new LASwriterTXT();
      if (opts) laswritertxt->set_pts(TRUE);
      else if (optx) laswritertxt->set_ptx(TRUE);
      if (!laswritertxt->open(file_name, header, parse_string, separator))
      {
        laserror("cannot open laswritertxt with file name '%s'", file_name);
        delete laswritertxt;
        return 0;
      }
      if (scale_rgb != 1.0f) laswritertxt->set_scale_rgb(scale_rgb);
      return laswritertxt;
    }
    else if (format == LAS_TOOLS_FORMAT_BIN)
    {
      LASwriterBIN* laswriterbin = new LASwriterBIN();
      if (!laswriterbin->open(file_name, header, "ts8"))
      {
        laserror("cannot open laswriterbin with file name '%s'", file_name);
        delete laswriterbin;
        return 0;
      }
      return laswriterbin;
    }
    else if (format == LAS_TOOLS_FORMAT_QFIT)
    {
      LASwriterQFIT* laswriterqfit = new LASwriterQFIT();
      if (!laswriterqfit->open(file_name, header, 40))
      {
        laserror("cannot open laswriterqfit with file name '%s'", file_name);
        delete laswriterqfit;
        return 0;
      }
      return laswriterqfit;
    }
    else if (format == LAS_TOOLS_FORMAT_VRML)
    {
      LASwriterWRL* laswriterwrl = new LASwriterWRL();
      if (!laswriterwrl->open(file_name, header, parse_string))
      {
        laserror("cannot open laswriterwrl with file name '%s'", file_name);
        delete laswriterwrl;
        return 0;
      }
      return laswriterwrl;
    }
    else
    {
      laserror("unknown format %d", format);
      return 0;
    }
  }
  else if (use_stdout)
  {
    if (format <= LAS_TOOLS_FORMAT_LAZ)
    {
      LASwriterLAS* laswriterlas = new LASwriterLAS();
      if (!laswriterlas->open(stdout, header, (format == LAS_TOOLS_FORMAT_LAZ ? (native ? LASZIP_COMPRESSOR_LAYERED_CHUNKED : LASZIP_COMPRESSOR_CHUNKED) : LASZIP_COMPRESSOR_NONE), requested_version, chunk_size))
      {
        laserror("cannot open laswriterlas to stdout");
        delete laswriterlas;
        return 0;
      }
      return laswriterlas;
    }
    else if (format == LAS_TOOLS_FORMAT_TXT)
    {
      LASwriterTXT* laswritertxt = new LASwriterTXT();
      if (opts) laswritertxt->set_pts(TRUE);
      else if (optx) laswritertxt->set_ptx(TRUE);
      if (!laswritertxt->open(stdout, header, parse_string, separator))
      {
        laserror("cannot open laswritertxt to stdout");
        delete laswritertxt;
        return 0;
      }
      if (scale_rgb != 1.0f) laswritertxt->set_scale_rgb(scale_rgb);
      return laswritertxt;
    }
    else if (format == LAS_TOOLS_FORMAT_BIN)
    {
      LASwriterBIN* laswriterbin = new LASwriterBIN();
      if (!laswriterbin->open(stdout, header, "ts8"))
      {
        laserror("cannot open laswriterbin to stdout");
        delete laswriterbin;
        return 0;
      }
      return laswriterbin;
    }
    else if (format == LAS_TOOLS_FORMAT_QFIT)
    {
      LASwriterQFIT* laswriterqfit = new LASwriterQFIT();
      if (!laswriterqfit->open(stdout, header, 40))
      {
        laserror("cannot open laswriterbin to stdout");
        delete laswriterqfit;
        return 0;
      }
      return laswriterqfit;
    }
    else if (format == LAS_TOOLS_FORMAT_VRML)
    {
      LASwriterWRL* laswriterwrl = new LASwriterWRL();
      if (!laswriterwrl->open(stdout, header, parse_string))
      {
        laserror("cannot open laswriterwrl with file name '%s'", file_name);
        delete laswriterwrl;
        return 0;
      }
      return laswriterwrl;
    }
    else
    {
      laserror("unknown format %d", format);
      return 0;
    }
  }
  else
  {
    laserror("no laswriter output specified");
    return 0;
  }
}

LASwaveform13writer* LASwriteOpener::open_waveform13(const LASheader* lasheader)
{
  if (lasheader->point_data_format < 4) return 0;
  if ((lasheader->point_data_format > 5) && (lasheader->point_data_format < 9)) return 0;
  if (lasheader->vlr_wave_packet_descr == 0) return 0;
  if (get_file_name() == 0) return 0;
  LASwaveform13writer* waveform13writer = new LASwaveform13writer();
  if (waveform13writer->open(get_file_name(), lasheader->vlr_wave_packet_descr))
  {
    return waveform13writer;
  }
  delete waveform13writer;
  return 0;
}

void LASwriteOpener::usage() const
{
  //pre-formated multiline message
  LASMessage(LAS_INFO, "Supported LAS Outputs\n" \
                       "  -o lidar.las\n" \
                       "  -o lidar.laz\n" \
                       "  -o xyzta.txt -oparse xyzta (on-the-fly to ASCII)\n" \
                       "  -o terrasolid.bin\n" \
                       "  -odir C:%cdata%cground (specify output directory)\n" \
                       "  -odix _classified (specify file name appendix)\n" \
                       "  -ocut 2 (cut the last two characters from name)\n" \
                       "  -olas -olaz -otxt -obin -oqi (specify format)\n" \
                       "  -stdout (pipe to stdout)\n" \
                       "  -nil    (pipe to NULL)\n", DIRECTORY_SLASH, DIRECTORY_SLASH);
}

BOOL LASwriteOpener::parse(int argc, char* argv[])
{
  int i;
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
    {
      return TRUE;
    }
    else if (strcmp(argv[i], "-hh") == 0)
    {
      usage();
      return TRUE;
    }
    else if (strcmp(argv[i],"-o") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: file_name", argv[i]);
        return FALSE;
      }
      set_file_name(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-odir") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: directory", argv[i]);
        return FALSE;
      }
      if (!set_directory(argv[i+1]))
      {
        laserror("'%s' is not a valid directory", argv[i+1]);
        return FALSE;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-odix") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: appendix", argv[i]);
        return FALSE;
      }
      set_appendix(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-ocut") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: number of characters to cut", argv[i]);
        return FALSE;
      }
      set_cut(atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-oforce") == 0)
    {
      set_force(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-native") == 0)
    {
      set_native(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-compatible") == 0)
    {
      set_native(FALSE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-olas") == 0)
    {
      set_format(LAS_TOOLS_FORMAT_LAS);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-olaz") == 0)
    {
      set_format(LAS_TOOLS_FORMAT_LAZ);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i], "-ojs") == 0)
    {
      specified = TRUE;
      set_format(LAS_TOOLS_FORMAT_JSON);
      *argv[i] = '\0';
    }
    else if (strcmp(argv[i],"-otxt") == 0)
    {
      specified = TRUE;
      format = LAS_TOOLS_FORMAT_TXT;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-obin") == 0)
    {
      set_format(LAS_TOOLS_FORMAT_BIN);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-oqi") == 0)
    {
      set_format(LAS_TOOLS_FORMAT_QFIT);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-owrl") == 0)
    {
      set_format(LAS_TOOLS_FORMAT_VRML);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-stdout") == 0)
    {
      use_stdout = TRUE;
      use_nil = FALSE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-nil") == 0)
    {
      use_nil = TRUE;
      use_stdout = FALSE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-chunk_size") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: number_points", argv[i]);
        return FALSE;
      }
      set_chunk_size(atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-oparse") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: string", argv[i]);
        return FALSE;
      }
      set_parse_string(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-osep") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: separator", argv[i]);
        return FALSE;
      }
      set_separator(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-oscale_rgb") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: scale", argv[i]);
        return FALSE;
      }
      set_scale_rgb((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-opts") == 0)
    {
      opts = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-optx") == 0)
    {
      optx = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-io_obuffer") == 0)
    {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: size", argv[i]);
        return FALSE;
      }
      set_io_obuffer_size((I32)atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
  }
  return TRUE;
}

void LASwriteOpener::set_io_obuffer_size(I32 io_obuffer_size)
{
  this->io_obuffer_size = io_obuffer_size;
}

BOOL LASwriteOpener::set_directory(const CHAR* directory)
{
  if (this->directory) free(this->directory);
  if (directory)
  {
    if (strstr(directory, ";") || strstr(directory, "\""))
    {
      LASMessage(LAS_WARNING, "specified '-odir' seems to contain a substring '\\\"' such\n" \
                              "\tas -odir \"D:\\\" or -odir \"..\\tiles\\\". this command will\n" \
                              "\tprobably fail. please use -odir \"D:\" or -odir \"..\\tiles\"\n" \
                              "\tinstead.");
    }
    this->directory = LASCopyString(directory);
    if (this->directory == nullptr) 
    {
      return FALSE;
    }
#pragma warning(push)
#pragma warning(disable : 6001)
    I32 len = (I32)strlen(this->directory);
    if ((len > 0) && ((this->directory[len-1] == '\\') || (this->directory[len-1] == '/')))
    {
      this->directory[len-1] = '\0';
    }
    if (file_name) add_directory();
#pragma warning(pop)
    // return FALSE if it does not exist or is no directory

    if ((len > 0) && (this->directory[len-1] != ':'))
    {
      las_stat_t info;

      if (stat_las(this->directory, &info) != 0)
      {
        return FALSE;
      }
      else if (!(info.st_mode & S_IFDIR))
      {
        return FALSE;
      }
    }
  }
  else
  {
    this->directory = 0;
  }
  return TRUE;
}

void LASwriteOpener::set_file_name(const CHAR* file_name)
{
  if (this->file_name) free(this->file_name);
  if (file_name)
  {
    this->file_name = LASCopyString(file_name);

    // get length of file name
    I32 len = (I32)strlen(this->file_name);

    // remove tailing white spaces
    while (len && (this->file_name[len-1] == ' '))
    {
      len--;
      this->file_name[len] = '\0';
    }

    // does file name have extension???
    int ext = len;

    while (ext && (this->file_name[ext-1] != '.'))
    {
      ext--;
      if ((len - ext) > 4)
      {
        ext = 0;
        break;
      }
    }

    if (ext) // file name has extension!!!
    {
      if (specified)
      {
      }
      else
      {
        const CHAR* extension = this->file_name + ext;
        if (strstr(extension, "laz") || strstr(extension, "LAZ"))
        {
          format = LAS_TOOLS_FORMAT_LAZ;
        }
        else if (strstr(extension, "las") || strstr(extension, "LAS"))
        {
          format = LAS_TOOLS_FORMAT_LAS;
        }
        else if (strstr(extension, "bin") || strstr(extension, "BIN")) // terrasolid
        {
          format = LAS_TOOLS_FORMAT_BIN;
        }
        else if (strstr(extension, "qi") || strstr(extension, "QI")) // QFIT
        {
          format = LAS_TOOLS_FORMAT_QFIT;
        }
        else if (strstr(extension, "wrl") || strstr(extension, "WRL")) // VRML
        {
          format = LAS_TOOLS_FORMAT_VRML;
        }
        else if (strstr(extension, "json") || strstr(extension, "JSON"))  // json
        {
          format = LAS_TOOLS_FORMAT_JSON;
        }
        else // assume ascii output
        {
          format = LAS_TOOLS_FORMAT_TXT;
        }
      }
    }
    else
    {
      CHAR* temp_file_name = (CHAR*)malloc_las(len + (format == LAS_TOOLS_FORMAT_QFIT ? 4 : (format == LAS_TOOLS_FORMAT_JSON ? 6 : 5)));
      if (temp_file_name) 
      {
        strcpy(temp_file_name, this->file_name);
        free(this->file_name);
        this->file_name = temp_file_name;
        this->file_name[len] = '.';
      }
      if (format == LAS_TOOLS_FORMAT_LAZ)
      {
        len++;
        this->file_name[len] = 'l';
        len++;
        this->file_name[len] = 'a';
        len++;
        this->file_name[len] = 'z';
      }
      else if (format == LAS_TOOLS_FORMAT_LAS)
      {
        len++;
        this->file_name[len] = 'l';
        len++;
        this->file_name[len] = 'a';
        len++;
        this->file_name[len] = 's';
      }
      else if (format == LAS_TOOLS_FORMAT_BIN) // terrasolid
      {
        len++;
        this->file_name[len] = 'b';
        len++;
        this->file_name[len] = 'i';
        len++;
        this->file_name[len] = 'n';
      }
      else if (format == LAS_TOOLS_FORMAT_QFIT) // QFIT
      {
        len++;
        this->file_name[len] = 'q';
        len++;
        this->file_name[len] = 'i';       ;
      }
      else if (format == LAS_TOOLS_FORMAT_VRML) // VRML
      {
        len++;
        this->file_name[len] = 'w';
        len++;
        this->file_name[len] = 'r';
        len++;
        this->file_name[len] = 'l';
      }
      else if (format == LAS_TOOLS_FORMAT_JSON)  // json
      {
        len++;
        this->file_name[len] = 'j';
        len++;
        this->file_name[len] = 's';
        len++;
        this->file_name[len] = 'o';
        len++;
        this->file_name[len] = 'n';
      }
      else // assume ascii output
      {
        len++;
        this->file_name[len] = 't';
        len++;
        this->file_name[len] = 'x';
        len++;
        this->file_name[len] = 't';
      }
      len++;
      this->file_name[len] = '\0';
    }

    if (directory) add_directory();
    if (cut) cut_characters();
    if (appendix) add_appendix();
  }
  else
  {
    this->file_name = 0;
  }
}

void LASwriteOpener::set_appendix(const CHAR* appendix)
{
  if (this->appendix) free(this->appendix);
  if (appendix)
  {
    this->appendix = LASCopyString(appendix);
    if (file_name) add_appendix();
  }
  else
  {
    this->appendix = 0;
  }
}

void LASwriteOpener::set_cut(U32 cut)
{
  this->cut = cut;
  if (cut && file_name) cut_characters();
}

void LASwriteOpener::set_native(BOOL native)
{
  this->native = native;
}

BOOL LASwriteOpener::set_format(I32 format)
{
  if ((format < LAS_TOOLS_FORMAT_DEFAULT) || ((format > LAS_TOOLS_FORMAT_TXT) && format != LAS_TOOLS_FORMAT_JSON))
  {
    return FALSE;
  }

  specified = TRUE;
  this->format = format;

  if (file_name)
  {
    I32 len = (I32)strlen(file_name);
    while (len > 0 && this->file_name[len] != '.')
    {
      len--;
    }
    if (len)
    {
      if (format <= LAS_TOOLS_FORMAT_LAS)
      {
        file_name[len+1] = 'l';
        file_name[len+2] = 'a';
        file_name[len+3] = 's';
      }
      else if (format == LAS_TOOLS_FORMAT_LAZ)
	    {
        file_name[len+1] = 'l';
        file_name[len+2] = 'a';
        file_name[len+3] = 'z';
	    }
      else if (format == LAS_TOOLS_FORMAT_BIN)
	    {
        file_name[len+1] = 'l';
        file_name[len+2] = 'a';
        file_name[len+3] = 'z';
	    }
      else if (format == LAS_TOOLS_FORMAT_QFIT)
	    {
        file_name[len+1] = 'q';
        file_name[len+2] = 'i';
        file_name[len+3] = '\0';
	    }
      else if (format == LAS_TOOLS_FORMAT_VRML)
	    {
        file_name[len+1] = 'w';
        file_name[len+2] = 'r';
        file_name[len+3] = 'l';
	    }
      else if (format == LAS_TOOLS_FORMAT_JSON)
        {
          file_name[len+1] = 'j';
          file_name[len+2] = 's';
          file_name[len+3] = 'o';
          file_name[len+4] = 'n';
        }
      else if (format == LAS_TOOLS_FORMAT_TXT)
	    {
        file_name[len+1] = 't';
        file_name[len+2] = 'x';
        file_name[len+3] = 't';
	    }
      else
      {
        return FALSE;
      }
      if (format == LAS_TOOLS_FORMAT_JSON) {
        file_name[len + 5] = '\0';
      }
      else
      {
        file_name[len + 4] = '\0';
      }
    }
  }
  return TRUE;
}

BOOL LASwriteOpener::set_format(const CHAR* format)
{
  if (format)
  {
    if (strstr(format, "laz") || strstr(format, "LAZ"))
    {
      return set_format(LAS_TOOLS_FORMAT_LAZ);
    }
    else if (strstr(format, "las") || strstr(format, "LAS"))
    {
      return set_format(LAS_TOOLS_FORMAT_LAS);
    }
    else if (strstr(format, "bin") || strstr(format, "BIN")) // terrasolid
    {
      return set_format(LAS_TOOLS_FORMAT_BIN);
    }
    else if (strstr(format, "qi") || strstr(format, "QI")) // QFIT
    {
      return set_format(LAS_TOOLS_FORMAT_QFIT);
    }
    else if (strstr(format, "wrl") || strstr(format, "WRL")) // VRML
    {
      return set_format(LAS_TOOLS_FORMAT_VRML);
    }
    else if (strstr(format, "json") || strstr(format, "JSON"))  // json
    {
      return set_format(LAS_TOOLS_FORMAT_JSON);
    }
    else // assume ascii output
    {
      return set_format(LAS_TOOLS_FORMAT_TXT);
    }
  }
  else
  {
    specified = FALSE;
    this->format = LAS_TOOLS_FORMAT_DEFAULT;
  }
  return TRUE;
}

void LASwriteOpener::set_force(BOOL force)
{
  this->force = force;
}

// current default item version: 2 for point types 0-5, 3 for point types 6-10 (will be automatically selected. 
// using version 4 is for the new, slightly fixed encoding of point types 6-10 
// (not the default, so the supporting software can be spread first, along with LAS 1.5)
void LASwriteOpener::set_requested_version(U32 requested_version ) 
{
    this->requested_version = requested_version;
}



void LASwriteOpener::set_chunk_size(U32 chunk_size)
{
  this->chunk_size = chunk_size;
}

void LASwriteOpener::make_numbered_file_name(const CHAR* file_name, I32 digits)
{
  I32 len;
  if (file_name)
  {
    if (this->file_name != 0) free(this->file_name);
    len = (I32)strlen(file_name);
    this->file_name = (CHAR*)malloc_las(len + digits + 2);
    if (this->file_name) strcpy(this->file_name, file_name);
  }
  else
  {
    if (this->file_name == 0) this->file_name = LASCopyString("output.xxx");
    len = (I32)strlen(this->file_name);
    this->file_name = (CHAR*)realloc_las(this->file_name, len + digits + 2);
  }
  if (this->file_name != nullptr) 
  {
    while (len > 0 && this->file_name[len] != '.')
    {
      len--;
    }
    if (len > 0)
    {
      this->file_name[len] = '_';
      len++;
    }
    while (digits > 0)
    {
      this->file_name[len] = '0';
      digits--;
      len++;
    }
    this->file_name[len] = '.';
    len++;
    this->file_name[len] = 'x';
    this->file_name[len+1] = 'x';
    this->file_name[len+2] = 'x';
    this->file_name[len+3] = '\0';
  }
}

void LASwriteOpener::make_file_name(const CHAR* file_name, I32 file_number)
{
  I32 len;

  if (file_number > -1)
  {
    if (file_name)
    {
      if (this->file_name) free(this->file_name);
      len = (I32)strlen(file_name);
      this->file_name = (CHAR*)malloc_las(len + 10);
      strcpy(this->file_name, file_name);
      if (cut)
      {
        cut_characters();
        len = (I32)strlen(file_name);
      }
    }
    else
    {
      if (this->file_name == 0)
      {
        this->file_name = LASCopyString("output_0000000.xxx");
      }
      len = (I32)strlen(this->file_name);
    }
    while (len > 0 && this->file_name[len] != '.') len--;
    len++;
    int num = len - 2;
    I32 file_num = file_number;
    while (num > 0 && this->file_name[num] >= '0' && this->file_name[num] <= '9')
    {
      this->file_name[num] = '0' + (file_num%10);
      file_num = file_num/10;
      num--;
    }
    if (file_num)
    {
      LASMessage(LAS_WARNING, "file name number %d too big to store in '%s'. use more digits.", file_number, this->file_name);
    }
  }
  else
  {
    if (this->file_name) free(this->file_name);
    if (file_name)
    {
      len = (I32)strlen(file_name);
      this->file_name = (CHAR*)malloc_las(len + (appendix ? strlen(appendix) + 5 : 10));
      strcpy(this->file_name, file_name);
      while (len > 0 && this->file_name[len] != '.') len--;
      if (cut)
      {
        len -= cut;
        if (len < 0) len = 0;
      }
      if (appendix != 0)
      {
        strcpy(&(this->file_name[len]), appendix);
        len += (I32)strlen(appendix);
      }
      else if ((directory == 0) && (cut == 0) && (file_number == -1))
      {
        this->file_name[len] = '_';
        this->file_name[len+1] = '1';
        len += 2;
      }
      this->file_name[len] = '.';
      len++;
    }
    else
    {
      len = 7;
      this->file_name = LASCopyString("output.xxx");
    }
  }
  if (format <= LAS_TOOLS_FORMAT_LAS)
  {
    this->file_name[len] = 'l';
    this->file_name[len+1] = 'a';
    this->file_name[len+2] = 's';
  }
  else if (format == LAS_TOOLS_FORMAT_LAZ)
  {
    this->file_name[len] = 'l';
    this->file_name[len+1] = 'a';
    this->file_name[len+2] = 'z';
  }
  else if (format == LAS_TOOLS_FORMAT_BIN)
  {
    this->file_name[len] = 'b';
    this->file_name[len+1] = 'i';
    this->file_name[len+2] = 'n';
  }
  else if (format == LAS_TOOLS_FORMAT_QFIT)
  {
    this->file_name[len] = 'q';
    this->file_name[len+1] = 'i';
    this->file_name[len+2] = '\0';
  }
  else if (format == LAS_TOOLS_FORMAT_JSON)
  {
    this->file_name[len] = 'j';
    this->file_name[len + 1] = 's';
    this->file_name[len + 2] = 'o';
    this->file_name[len + 3] = 'n';
  }
  else // if (format == LAS_TOOLS_FORMAT_TXT)
  {
    this->file_name[len] = 't';
    this->file_name[len+1] = 'x';
    this->file_name[len+2] = 't';
  }
  if (format == LAS_TOOLS_FORMAT_JSON) {
    this->file_name[len + 4] = '\0';
  }
  else
  {
    this->file_name[len + 3] = '\0';
  }

  if (directory) add_directory();

  if (file_name && (strcmp(this->file_name, file_name) == 0))
  {
    if (!force)
    {
      free(this->file_name);
      if (format <= LAS_TOOLS_FORMAT_LAS)
      {
        this->file_name = LASCopyString("temp.las");
      }
      else if (format == LAS_TOOLS_FORMAT_LAZ)
      {
        this->file_name = LASCopyString("temp.laz");
      }
      else if (format == LAS_TOOLS_FORMAT_BIN)
      {
        this->file_name = LASCopyString("temp.bin");
      }
      else if (format == LAS_TOOLS_FORMAT_QFIT)
      {
        this->file_name = LASCopyString("temp.qi");
      }
      else if (format == LAS_TOOLS_FORMAT_VRML)
      {
        this->file_name = LASCopyString("temp.wrl");
      }
      else if (format == LAS_TOOLS_FORMAT_JSON)
      {
        this->file_name = LASCopyString("temp.json");
      }
      else // if (format == LAS_TOOLS_FORMAT_TXT)
      {
        this->file_name = LASCopyString("temp.txt");
      }
      LASMessage(LAS_WARNING, "generated output name '%s'\n" \
                              "\tidentical to input name. changed to '%s'.\n" \
                              "\tyou can override this safety measure with '-oforce'.", file_name, this->file_name);
    }
  }
}

const CHAR* LASwriteOpener::get_directory() const
{
  return directory;
}

const CHAR* LASwriteOpener::get_file_name() const
{
  return file_name;
}

CHAR* LASwriteOpener::get_file_name_base() const
{
  CHAR* file_name_base = 0;

  if (file_name)
  {
    file_name_base = LASCopyString(file_name);
    // remove extension
    I32 len = (I32)strlen(file_name_base);
    while ((len > 0) && (file_name_base[len] != '.') && (file_name_base[len] != '\\') && (file_name_base[len] != '/') && (file_name_base[len] != ':')) len--;
    if (file_name_base[len] == '.')
    {
      file_name_base[len] = '\0';
    }
  }
  else if (directory)
  {
    I32 len = (I32)strlen(directory);
    file_name_base = (CHAR*)malloc_las(len + 2);
    snprintf(file_name_base, len + 2, "%s%c", directory, DIRECTORY_SLASH);
  }

  return file_name_base;
}

const CHAR* LASwriteOpener::get_file_name_only() const
{
  const CHAR* file_name_only = 0;

  if (file_name)
  {
    I32 len = (I32)strlen(file_name);
    while ((len > 0) && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;
    if (len)
    {
      file_name_only = file_name + len + 1;
    }
    else
    {
      file_name_only = file_name;
    }
  }

  return file_name_only;
}

const CHAR* LASwriteOpener::get_appendix() const
{
  return appendix;
}

U32 LASwriteOpener::get_cut() const
{
  return cut;
}

BOOL LASwriteOpener::get_native() const
{
  return native;
}

BOOL LASwriteOpener::format_was_specified() const
{
  return specified;
}

static const CHAR* LAS_TOOLS_FORMAT_NAMES[12] = {"las", "las", "laz", "bin", "qi", "wrl", "txt", "shp", "asc", "bil", "flt", "json"};

const CHAR* LASwriteOpener::get_format_name() const
{
  return LAS_TOOLS_FORMAT_NAMES[get_format()];
}

I32 LASwriteOpener::get_format() const
{
  if (specified || (file_name == 0))
  {
    return format;
  }
  else
  {
    if (HasFileExt(std::string(file_name), ".laz")) {
      return LAS_TOOLS_FORMAT_LAZ;
    } else if (HasFileExt(std::string(file_name), ".las"))
    {
      return LAS_TOOLS_FORMAT_LAS;
    } else if (HasFileExt(std::string(file_name), ".bin"))  // terrasolid
    {
      return LAS_TOOLS_FORMAT_BIN;
    }
    else if (HasFileExt(std::string(file_name), ".qi")) // QFIT
    {
      return LAS_TOOLS_FORMAT_QFIT;
    }
    else if (HasFileExt(std::string(file_name), ".wrl")) // VRML
    {
      return LAS_TOOLS_FORMAT_VRML;
    }
    else if (HasFileExt(std::string(file_name), ".json"))  // json
    {
      return LAS_TOOLS_FORMAT_JSON;
    }
    else // assume ascii output
    {
      return LAS_TOOLS_FORMAT_TXT;
    }
  }
}

void LASwriteOpener::set_parse_string(const CHAR* parse_string)
{
  if (this->parse_string) free(this->parse_string);
  if (parse_string)
  {
    this->parse_string = LASCopyString(parse_string);
  }
  else
  {
    this->parse_string = 0;
  }
}

void LASwriteOpener::set_separator(const CHAR* separator)
{
  if (this->separator) free(this->separator);
  if (separator)
  {
    this->separator = LASCopyString(separator);
  }
  else
  {
    this->separator = 0;
  }
}

void LASwriteOpener::set_scale_rgb(F32 scale_rgb)
{
  this->scale_rgb = scale_rgb;
}

BOOL LASwriteOpener::active() const
{
  return (file_name != 0 || use_stdout || use_nil);
}

void LASwriteOpener::add_directory(const CHAR* directory)
{
  if (directory == 0) directory = this->directory;

  if (file_name && directory)
  {
    I32 len = (I32)strlen(file_name);
    while ((len > 0) && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;
    if (len > 0) len++;
    CHAR* new_file_name = (CHAR*)malloc_las(strlen(directory) + strlen(&(file_name[len])) + 5);
    snprintf(new_file_name, strlen(directory) + strlen(&(file_name[len])) + 5, "%s%c%s", directory, DIRECTORY_SLASH, &(file_name[len]));
    free(file_name);
    file_name = new_file_name;
  }
}

void LASwriteOpener::add_appendix(const CHAR* appendix)
{
  if (appendix == 0) appendix = this->appendix;

  if (file_name && appendix)
  {
    I32 len = (I32)strlen(file_name);
    CHAR* new_file_name = (CHAR*)malloc_las(len + strlen(appendix) + 5);
    while ((len > 0) && (file_name[len] != '.') && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;

    if ((len == 0) || (file_name[len] == '\\') || (file_name[len] == '/') || (file_name[len] == ':'))
    {
      snprintf(new_file_name, strlen(appendix) + 5, "%s%s", file_name, appendix);
    }
    else if (new_file_name) 
    {
      strncpy_las(new_file_name, len + strlen(appendix) + 5, file_name, len);
      snprintf(&(new_file_name[len]), len + strlen(appendix) + 5, "%s%s", appendix, &(file_name[len]));
    }
    free(file_name);
    file_name = new_file_name;
  }
}

#ifdef __GNUC__ 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
void LASwriteOpener::cut_characters() {
  if (file_name && cut)
  {
    size_t len = strlen(file_name);
    size_t new_len = (len > cut) ? (len - cut) : 0;
    CHAR* new_file_name = (CHAR*)malloc_las(new_len + 5);
    while ((len > 0) && (file_name[len] != '.') && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;

    if (new_file_name)
    {
      if ((len == 0) || (file_name[len] == '\\') || (file_name[len] == '/') || (file_name[len] == ':'))
      {
        len = strlen(file_name);
        memcpy(new_file_name, file_name, new_len);
      }
      else
      {
        strncpy_las(new_file_name, new_len + 5, file_name, new_len);
        snprintf(&(new_file_name[new_len]), new_len + 5, "%s", &(file_name[len]));
      }
    }
    free(file_name);
    file_name = new_file_name;
  }
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

LASwriteOpener::LASwriteOpener()
{
  io_obuffer_size = LAS_TOOLS_IO_OBUFFER_SIZE;
  directory = nullptr;
  file_name = 0;
  appendix = 0;
  cut = 0;
  opts = FALSE;
  optx = FALSE;
  parse_string = 0;
  separator = 0;
  scale_rgb = 1.0f;
  native = TRUE;
  format = LAS_TOOLS_FORMAT_DEFAULT;
  specified = FALSE;
  force = FALSE;
  chunk_size = LASZIP_CHUNK_SIZE_DEFAULT;
  use_stdout = FALSE;
  use_nil = FALSE;
  requested_version = 0;
}

LASwriteOpener::~LASwriteOpener()
{
  if (directory) free(directory);
  if (file_name) free(file_name);
  if (appendix) free(appendix);
  if (parse_string) free(parse_string);
  if (separator) free(separator);
}
