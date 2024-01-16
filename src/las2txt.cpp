/*
===============================================================================

  FILE:  las2txt.cpp

  CONTENTS:

    This tool converts LIDAR data from the binary LAS format to a human
    readable ASCII format. The tool can create different formattings for
    the textual representation that are controlable via the 'parse' and
    'sep' commandline flags. Optionally the header can be placed at the
    beginning of the file each line preceeded by some comment symbol.

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

    19 September 2023 -- added support of custom extented -parse flags. Support of (hsl) and (hsv) flags
    18 September 2023 -- added -coldesc argument to add column description
     7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
    22 November 2017 -- parse attributes with indices > 9 by bracketing (12) them
    19 April 2017 -- 1st example for selective decompression for new LAS 1.4 points
    11 January 2017 -- added with<h>eld and scanner channe<l> for the parse string
    24 April 2015 -- added 'k'eypoint and 'o'verlap flags for the parse string
    30 March 2015 -- support LAS 1.4 extended return counts and number of returns
    25 October 2011 -- changed LAS 1.3 parsing to use new LASwaveform13reader
    17 May 2011 -- enabling batch processing with wildcards or multiple file names
    13 May 2011 -- moved indexing, filtering, transforming into LASreader
    15 March 2011 -- added the 'E' option to place an '-extra STRING'
    26 January 2011 -- added the LAStransform to modify before output
     4 January 2011 -- added the LASfilter to drop or keep points
     1 January 2011 -- added LAS 1.3 waveforms while homesick for Livermore
     1 December 2010 -- support output of raw unscaled XYZ coordinates
    12 March 2009 -- updated to ask for input if started without arguments
    17 September 2008 -- updated to deal with LAS format version 1.2
    13 June 2007 -- added 'e' and 'd' for the parse string and fixed 'n'
     6 June 2007 -- added lidardouble2string() after Vinton Valentine's bug report
     4 May 2007 -- completed one month later because my mother passed away
     4 April 2007 -- created in the ICE from Frankfurt Airport to Wuerzburg

===============================================================================
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "laswaveform13reader.hpp"
#include "laswriter.hpp"

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"las2txt -i test.las -parse Mxyzrna -stdout | more\n");
  fprintf(stderr,"las2txt -i *.las -parse xyzt\n");
  fprintf(stderr,"las2txt -i flight1*.las flight2*.las -parse xyziarn\n");
  fprintf(stderr,"las2txt -i *.las -parse xyzrn -sep comma -verbose\n");
  fprintf(stderr,"las2txt -i lidar.las -parse xyztE -extra 99 -o ascii.txt\n");
  fprintf(stderr,"las2txt -h\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"The '-parse txyz' flag specifies how to format each\n");
  fprintf(stderr,"each line of the ASCII file. For example, 'txyzia'\n");
  fprintf(stderr,"means that the first number of each line should be the\n");
  fprintf(stderr,"gpstime, the next three numbers should be the x, y, and\n");
  fprintf(stderr,"z coordinate, the next number should be the intensity\n");
  fprintf(stderr,"and the next number should be the scan angle.\n");
  fprintf(stderr,"The supported entries are a - scan angle, i - intensity,\n");
  fprintf(stderr,"n - number of returns for given pulse, r - number of\n");
  fprintf(stderr,"this return, t - gpstime, c - classification, u - user data,\n");
  fprintf(stderr,"p - point source ID, e - edge of flight line flag, and\n");
  fprintf(stderr,"d - direction of scan flag, l - extended scanner channel,\n");
  fprintf(stderr,"h - withheld flag, k - keypoint flag, g - synthetic flag,\n");
  fprintf(stderr,"o - extended overlap flag, R - red channel of RGB color,\n");
  fprintf(stderr,"G - green channel of RGB color, B - blue channel of RGB color,\n");
  fprintf(stderr,"I - NIR channel, m - count for each point (starting at zero),\n");
  fprintf(stderr,"M - count for each point (starting at one),\n");
  fprintf(stderr,"X, Y, and Z - the unscaled, raw LAS integer coordinates\n");
  fprintf(stderr,"w and W - for the wavepacket information (LAS 1.3 only)\n");
  fprintf(stderr,"V - for the waVeform from the *.wdp file (LAS 1.3 only)\n");
  fprintf(stderr,"E - for an extra string. specify it with '-extra <string>'\n");
  fprintf(stderr,"0 through 9 - for additional attributes stored as extra bytes\n");
  fprintf(stderr,"Additionnal strings (HSV), (HSL), (hsv) and (hsl) convert RGB\n");
	fprintf(stderr,"into HSV or HSL colors models in range [0..360/100] or [0..1]\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"The '-sep space' flag specifies what separator to use. The\n");
  fprintf(stderr,"default is a space but 'tab', 'comma', 'colon', 'hyphen',\n");
  fprintf(stderr,"'dot', or 'semicolon' are other possibilities.\n");
  fprintf(stderr,"---------------------------------------------\n");
  fprintf(stderr,"The '-header pound' flag results in the header information\n");
  fprintf(stderr,"being printed at the beginning of the ASCII file in form of\n");
  fprintf(stderr,"a comment that starts with the special character '#'. Also\n");
  fprintf(stderr,"possible are 'percent', 'dollar', 'comma', 'star',\n");
  fprintf(stderr,"'colon', or 'semicolon' as that special character.\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static void byebye(bool error=false, bool wait=false)
{
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

static void lidardouble2string(CHAR* string, double value)
{
  int len;
  len = sprintf(string, "%.15f", value) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
}

static void lidardouble2string(CHAR* string, double value, double precision)
{
  if (precision == 0.01)
    sprintf(string, "%.2f", value);
  else if (precision == 0.001)
    sprintf(string, "%.3f", value);
  else if (precision == 0.0001)
    sprintf(string, "%.4f", value);
  else if (precision == 0.1)
    sprintf(string, "%.1f", value);
  else if (precision == 0.00001)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000001)
    sprintf(string, "%.6f", value);
  else if (precision == 0.0000001)
    sprintf(string, "%.7f", value);
  else if (precision == 0.00000001)
    sprintf(string, "%.8f", value);
  else if (precision == 0.000000001)
    sprintf(string, "%.9f", value);
  else if (precision == 0.0025)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00025)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000025)
    sprintf(string, "%.6f", value);
  else if (precision == 0.005)
    sprintf(string, "%.3f", value);
  else if (precision == 0.0005)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00005)
    sprintf(string, "%.5f", value);
  else if (precision == 0.0000000001)
    sprintf(string, "%.10f", value);
  else if (precision == 0.00000000001)
    sprintf(string, "%.11f", value);
  else if (precision == 0.000000000001)
    sprintf(string, "%.12f", value);
  else if (precision == 0.0000000000001)
    sprintf(string, "%.13f", value);
  else if (precision == 0.00000000000001)
    sprintf(string, "%.14f", value);
  else if (precision == 0.000000000000001)
    sprintf(string, "%.15f", value);
  else
    lidardouble2string(string, value);
}

static void output_waveform(FILE* file_out, CHAR separator_sign, LASwaveform13reader* laswaveform13reader)
{
  U32 i;
  fprintf(file_out, "%d", laswaveform13reader->nbits);
  fprintf(file_out, "%c%d", separator_sign, laswaveform13reader->nsamples);
  if (laswaveform13reader->nbits == 8)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      fprintf(file_out, "%c%d", separator_sign, laswaveform13reader->samples[i]);
    }
  }
  else if (laswaveform13reader->nbits == 16)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      fprintf(file_out, "%c%d", separator_sign, ((U16*)laswaveform13reader->samples)[i]);
    }
  }
  else if (laswaveform13reader->nbits == 32)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      fprintf(file_out, "%c%d", separator_sign, ((I32*)laswaveform13reader->samples)[i]);
    }
  }
}

static I32 attribute_starts[32];

static BOOL print_attribute(FILE* file, const LASheader* header, const LASpoint* point, I32 index, CHAR* printstring)
{
  if (index >= header->number_attributes)
  {
    return FALSE;
  }
  if (header->attributes[index].data_type == 1)
  {
    U8 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%d", (I32)value);
      }
    }
  }
  else if (header->attributes[index].data_type == 2)
  {
    I8 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%d", (I32)value);
      }
    }
  }
  else if (header->attributes[index].data_type == 3)
  {
    U16 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%d", (I32)value);
      }
    }
  }
  else if (header->attributes[index].data_type == 4)
  {
    I16 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%d", (I32)value);
      }
    }
  }
  else if (header->attributes[index].data_type == 5)
  {
    U32 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%u", value);
      }
    }
  }
  else if (header->attributes[index].data_type == 6)
  {
    I32 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%d", value);
      }
    }
  }
  else if (header->attributes[index].data_type == 7)
  {
    U64 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*((I64)value) + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*((I64)value);
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + ((I64)value);
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
#ifdef _WIN32
        fprintf(file, "%I64u", value);
#else
        fprintf(file, "%llu", value);
#endif
      }
    }
  }
  else if (header->attributes[index].data_type == 8)
  {
    I64 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
#ifdef _WIN32
        fprintf(file, "%I64d", value);
#else
        fprintf(file, "%lld", value);
#endif
      }
    }
  }
  else if (header->attributes[index].data_type == 9)
  {
    F32 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%g", value);
      }
    }
  }
  else if (header->attributes[index].data_type == 10)
  {
    F64 value;
    point->get_attribute(attribute_starts[index], value);
    if (header->attributes[index].has_scale())
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].scale[0]*value + header->attributes[index].offset[0];
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
      else
      {
        F64 temp_d = header->attributes[index].scale[0]*value;
        lidardouble2string(printstring, temp_d, header->attributes[index].scale[0]);
        fprintf(file, "%s", printstring);
      }
    }
    else
    {
      if (header->attributes[index].has_offset())
      {
        F64 temp_d = header->attributes[index].offset[0] + value;
        lidardouble2string(printstring, temp_d);
        fprintf(file, "%s", printstring);
      }
      else
      {
        fprintf(file, "%g", value);
      }
    }
  }
  else
  {
    fprintf(file, "-");
    fprintf(stderr, "WARNING: data type %d of attribute %d not implemented.\n", header->attributes[index].data_type, index);
    return FALSE;
  }
  return TRUE;
}

enum extended_flags{HSV = -1, HSL = -2, HSV255 = -3, HSL255 = -4};

static void parse_extended_flags(char *parse_string)
{
  const char *extended_flags[] = {"(HSV)", "(HSL)", "(hsv)", "(hsl)"};
  const char replacement_codes[] = {HSV255, HSL255, HSV, HSL};
  I32 nflags = (I32)(sizeof(extended_flags) / sizeof(char*));

  for (I32 i = 0; i < nflags; i++)
  {
    char *found = strstr(parse_string, extended_flags[i]);

    while (found)
    {
      int len_flag = (int)strlen(extended_flags[i]);
      int len_remaining = (int)strlen(found + len_flag);
      *found = replacement_codes[i];
      memmove(found + 1, found + len_flag, len_remaining + 1);
      found = strstr(parse_string, extended_flags[i]);
    }
  }
}

#ifdef COMPILE_WITH_GUI
extern int las2txt_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern int las2txt_multi_core(int argc, char *argv[], LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores, BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  int i;
#ifdef COMPILE_WITH_GUI
  bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
  I32 cores = 1;
  BOOL cpu64 = FALSE;
#endif
  bool diff = false;
  bool verbose = false;
  CHAR separator_sign = ' ';
  CHAR const* separator = "space";
  bool opts = false;
  bool optx = false;
  CHAR header_comment_sign = '\0';
  U32 decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY;
  CHAR* parse_string = 0;
  CHAR* extra_string = 0;
  CHAR printstring[512];
  double start_time = 0.0;
  bool coldesc = false;

  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  laswriteopener.set_format("txt");

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return las2txt_gui(argc, argv, 0);
#else
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    CHAR file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
#endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == 0x96) argv[i][0] = '-';
      if (strcmp(argv[i],"-opts") == 0)
      {
        opts = TRUE;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-optx") == 0)
      {
        optx = TRUE;
        *argv[i]='\0';
      }
    }
    if (!lasreadopener.parse(argc, argv)) byebye(true);
    if (!laswriteopener.parse(argc, argv)) byebye(true);
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      fprintf(stderr, "LAStools (by info@rapidlasso.de) version %d\n", LAS_TOOLS_VERSION);
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = true;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "LAStools (by info@rapidlasso.de) version %d\n", LAS_TOOLS_VERSION);
      byebye();
    }
    else if (strcmp(argv[i],"-fail") == 0)
    {
    }
    else if (strcmp(argv[i],"-gui") == 0)
    {
#ifdef COMPILE_WITH_GUI
      gui = true;
#else
      fprintf(stderr, "WARNING: not compiled with GUI support. ignoring '-gui' ...\n");
#endif
    }
    else if (strcmp(argv[i],"-cores") == 0)
    {
#ifdef COMPILE_WITH_MULTI_CORE
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        usage(true);
      }
      argv[i][0] = '\0';
      i++;
      cores = atoi(argv[i]);
      argv[i][0] = '\0';
#else
      fprintf(stderr, "WARNING: not compiled with multi-core batching. ignoring '-cores' ...\n");
      i++;
#endif
    }
    else if (strcmp(argv[i],"-cpu64") == 0)
    {
#ifdef COMPILE_WITH_MULTI_CORE
      cpu64 = TRUE;
#else
      fprintf(stderr, "WARNING: not compiled with 64 bit support. ignoring '-cpu64' ...\n");
#endif
      argv[i][0] = '\0';
    }
    else if (strcmp(argv[i],"-parse") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: string\n", argv[i]);
        usage(true);
      }
      i++;
      if (parse_string) free(parse_string);
      parse_string = LASCopyString(argv[i]);
      parse_extended_flags(parse_string);
    }
    else if (strcmp(argv[i],"-parse_all") == 0)
    {
      if (parse_string) free(parse_string);
      parse_string = LASCopyString("txyzirndecaup");
    }
    else if (strcmp(argv[i],"-extra") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: string\n", argv[i]);
        usage(true);
      }
      i++;
      extra_string = argv[i];
    }
    else if (strcmp(argv[i],"-sep") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: separator\n", argv[i]);
        usage(true);
      }
      i++;
      separator = argv[i];
      if (strcmp(separator,"comma") == 0 || strcmp(separator,"komma") == 0)
      {
        separator_sign = ',';
      }
      else if (strcmp(separator,"tab") == 0)
      {
        separator_sign = '\t';
      }
      else if (strcmp(separator,"dot") == 0 || strcmp(separator,"period") == 0)
      {
        separator_sign = '.';
      }
      else if (strcmp(separator,"colon") == 0)
      {
        separator_sign = ':';
      }
      else if (strcmp(separator,"semicolon") == 0)
      {
        separator_sign = ';';
      }
      else if (strcmp(separator,"hyphen") == 0 || strcmp(separator,"minus") == 0)
      {
        separator_sign = '-';
      }
      else if (strcmp(separator,"space") == 0)
      {
        separator_sign = ' ';
      }
      else
      {
        fprintf(stderr, "ERROR: unknown seperator '%s'\n",separator);
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-header") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: comment\n", argv[i]);
        usage(true);
      }
      i++;
      if (strcmp(argv[i],"comma") == 0 || strcmp(argv[i],"komma") == 0)
      {
        header_comment_sign = ',';
      }
      else if (strcmp(argv[i],"colon") == 0)
      {
        header_comment_sign = ':';
      }
      else if (strcmp(argv[i],"scolon") == 0 || strcmp(argv[i],"semicolon") == 0)
      {
        header_comment_sign = ';';
      }
      else if (strcmp(argv[i],"pound") == 0 || strcmp(argv[i],"hash") == 0)
      {
        header_comment_sign = '#';
      }
      else if (strcmp(argv[i],"percent") == 0)
      {
        header_comment_sign = '%';
      }
      else if (strcmp(argv[i],"dollar") == 0)
      {
        header_comment_sign = '$';
      }
      else if (strcmp(argv[i],"star") == 0)
      {
        header_comment_sign = '*';
      }
      else
      {
        fprintf(stderr, "ERROR: unknown header comment symbol '%s'\n",argv[i]);
        usage(true);
      }
    }
    else if (strcmp(argv[i],"-coldesc") == 0)
    {
      coldesc = true;
    }
    else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage(true);
    }
  }

#ifdef COMPILE_WITH_GUI
  if (gui)
  {
    return las2txt_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (cores > 1)
  {
    if (lasreadopener.get_file_name_number() < 2)
    {
      fprintf(stderr,"WARNING: only %u input files. ignoring '-cores %d' ...\n", lasreadopener.get_file_name_number(), cores);
    }
    else if (lasreadopener.is_merged())
    {
      fprintf(stderr,"WARNING: input files merged on-the-fly. ignoring '-cores %d' ...\n", cores);
    }
    else
    {
      return las2txt_multi_core(argc, argv, &lasreadopener, &laswriteopener, cores, cpu64);
    }
  }
  if (cpu64)
  {
    return las2txt_multi_core(argc, argv, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    byebye(true, argc == 1);
  }

  // what layers do we need (for selective LAS 1.4 decompression)

  if (opts || optx)
  {
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_Z;
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_INTENSITY;
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_RGB;
  }
  else if (parse_string == 0)
  {
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_Z;
  }
  else
  {
    // check requested fields and print warnings of necessary
    i = 0;
    while (parse_string[i])
    {
      switch (parse_string[i])
      {
      case '_': // diff of unscaled raw integer X to prev point
      case '!': // diff of unscaled raw integer Y to prev point
        diff = true;
      case 'x': // the x coordinate
      case 'y': // the y coordinate
      case 'X': // the unscaled raw integer X coordinate
      case 'Y': // the unscaled raw integer Y coordinate
      case 'r': // the number of the return
      case 'n': // the number of returns of given pulse
      case 'l': // the (extended) scanner channe<l>
        break;
      case '@': // diff of unscaled raw integer Z to prev point
        diff = true;
      case 'z': // the z coordinate
      case 'Z': // the unscaled raw integer Z coordinate
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_Z;
        break;
      case 'i': // the intensity
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_INTENSITY;
        break;
      case 'a': // the scan angle
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE;
        break;
      case 'c': // the classification
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION;
        break;
      case 'u': // the user data
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_USER_DATA;
        break;
      case 'p': // the point source ID
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE;
        break;
      case 'e': // the edge of flight line flag
      case 'd': // the direction of scan flag
      case 'h': // the with<h>eld flag
      case 'k': // the <k>eypoint flag
      case 'g': // the synthetic fla<g>
      case 'o': // the (extended) <o>verlap flag
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_FLAGS;
        break;
      case 'm': // the index of the point (count starts at 0)
      case 'M': // the index of the point (count starts at 1)
        break;
      case '#': // diff of gps-time to prev point
        diff = true;
      case 't': // the gps-time
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME;
        break;
      case '$': // the R difference to the last point
      case '&': // the byte-wise R difference to the last point
      case '%': // the G difference to the last point
      case '*': // the byte-wise G difference to the last point
      case '^': // the B difference to the last point
      case '+': // the byte-wise B difference to the last point
        diff = true;
      case 'R': // the red channel of the RGB field
      case 'B': // the blue channel of the RGB field
      case 'G': // the green channel of the RGB field
      case HSV: // HSV conversion in range [0, 1] (extended flag)
      case HSL: // HSL conversion in range [0, 1] (extended flag)
      case HSV255: // HSV conversion in range [0,360] (extended flag)
      case HSL255: // HSL conversion in range [0,360] (extended flag)
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_RGB;
        break;
      case 'I': // the near infrared channel of the RGBI field
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_NIR;
        break;
      case 'w': // the wavepacket index
      case 'W': // all wavepacket attributes
      case 'V': // the waveform data
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_WAVEPACKET;
        break;
      case 'E':
        if (extra_string == 0)
        {
          fprintf (stderr, "WARNING: requested 'E' but no '-extra' specified. skipping ...\n");
          parse_string[i] = 's';
        }
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '(':
      case ')':
        decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES;
        break;
      default:
        fprintf (stderr, "WARNING: requested unknown parse item '%c'. skipping ...\n", parse_string[i]);
        parse_string[i] = 's';
      }
      i++;
    }
  }

  // only decompress the layers we need (for new LAS 1.4 point types only)

  lasreadopener.set_decompress_selective(decompress_selective);

  // possibly loop over multiple input files

  while (lasreadopener.active())
  {
    if (verbose) start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

    // (maybe) open laswaveform13reader

    LASwaveform13reader* laswaveform13reader = lasreadopener.open_waveform13(&lasreader->header);

    // get a pointer to the header

    LASheader* header = &(lasreader->header);

    // open output file

    FILE* file_out;

    if (laswriteopener.is_piped())
    {
      file_out = stdout;
    }
    else
    {
      // create output file name if needed

      if (laswriteopener.get_file_name() == 0)
      {
        if (lasreadopener.get_file_name() == 0)
        {
          fprintf(stderr, "ERROR: no output file specified\n");
          byebye(true, argc==1);
        }
        laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
      }

      const CHAR* file_name_out = laswriteopener.get_file_name();

      // open output file

      file_out = fopen(file_name_out, "w");

      // fail if output file does not open

      if (file_out == 0)
      {
        fprintf(stderr, "ERROR: could not open '%s' for write\n", file_name_out);
        byebye(true, argc==1);
      }

      laswriteopener.set_file_name(0);
    }

    // maybe PTS or PTX format

    if (opts)
    {
      // look for VRL with PTS or PTX info
      const LASvlr* ptsVLR = 0;
      const LASvlr* ptxVLR = 0;
      if ((ptsVLR = header->get_vlr("LAStools", 2000)) || (ptxVLR = header->get_vlr("LAStools", 2001)))
      {
        if ((parse_string == 0) || (strcmp(parse_string, "original") == 0))
        {
          if (parse_string) free(parse_string);
          if (ptsVLR && (ptsVLR->record_length_after_header >= 32))
          {
            parse_string = LASCopyString((CHAR*)(ptsVLR->data + 16));
          }
          else if (ptxVLR && (ptxVLR->record_length_after_header >= 32))
          {
            parse_string = LASCopyString((CHAR*)(ptxVLR->data + 16));
          }
          else if (ptsVLR)
          {
            fprintf(stderr, "WARNING: found VLR for PTS with wrong payload size of %d.\n", ptsVLR->record_length_after_header);
          }
          else if (ptxVLR)
          {
            fprintf(stderr, "WARNING: found VLR for PTX with wrong payload size of %d.\n", ptxVLR->record_length_after_header);
          }
        }
      }
      else
      {
        fprintf(stderr, "WARNING: found no VLR with PTS or PTX info.\n");
      }
      if (header->version_minor >= 4)
      {
#ifdef _WIN32
        fprintf(file_out, "%I64d       \012", header->extended_number_of_point_records);
#else
        fprintf(file_out, "%lld       \012", header->extended_number_of_point_records);
#endif
      }
      else
      {
        fprintf(file_out, "%u       \012", header->number_of_point_records);
      }
      if (parse_string && strcmp(parse_string, "xyz") && strcmp(parse_string, "xyzi") && strcmp(parse_string, "xyziRGB") && strcmp(parse_string, "xyzRGB"))
      {
        fprintf(stderr, "WARNING: the parse string for PTS should be 'xyz', 'xyzi', 'xyziRGB', or 'xyzRGB'\n");
      }
      if (separator_sign != ' ')
      {
        fprintf(stderr, "WARNING: the separator for PTS should be 'space' not '%s'\n", separator);
      }
    }
    else if (optx)
    {
      // look for VRL with PTX info
      const LASvlr* ptxVLR = header->get_vlr("LAStools", 2001);
      if (ptxVLR && (ptxVLR->record_length_after_header == 272))
      {
        U8* payload = ptxVLR->data;
        if ((parse_string == 0) || (strcmp(parse_string, "original") == 0))
        {
          if (parse_string) free(parse_string);
          parse_string = LASCopyString((CHAR*)(payload + 16));
        }
        fprintf(file_out, "%u     \012", (U32)((I64*)payload)[4]); // ncols
        fprintf(file_out, "%u     \012", (U32)((I64*)payload)[5]); // nrows
        fprintf(file_out, "%g %g %g\012", ((F64*)payload)[6], ((F64*)payload)[7], ((F64*)payload)[8]); // scanner registered position
        fprintf(file_out, "%g %g %g\012", ((F64*)payload)[9], ((F64*)payload)[10], ((F64*)payload)[11]); // scanner registered axis 'X'
        fprintf(file_out, "%g %g %g\012", ((F64*)payload)[12], ((F64*)payload)[13], ((F64*)payload)[14]); // scanner registered axis 'Y'
        fprintf(file_out, "%g %g %g\012", ((F64*)payload)[15], ((F64*)payload)[16], ((F64*)payload)[17]); // scanner registered axis 'Z'
        fprintf(file_out, "%g %g %g %g\012", ((F64*)payload)[18], ((F64*)payload)[19], ((F64*)payload)[20], ((F64*)payload)[21]); // r11 r12 r13 - rotation matrix
        fprintf(file_out, "%g %g %g %g\012", ((F64*)payload)[22], ((F64*)payload)[23], ((F64*)payload)[24], ((F64*)payload)[25]); // r21 r22 r23
        fprintf(file_out, "%g %g %g %g\012", ((F64*)payload)[26], ((F64*)payload)[27], ((F64*)payload)[28], ((F64*)payload)[29]); // r31 r32 r33
        fprintf(file_out, "%g %g %g %g\012", ((F64*)payload)[30], ((F64*)payload)[31], ((F64*)payload)[32], ((F64*)payload)[33]); // tr1 tr2 tr3 - transformation 
      }
      else
      {
        if (ptxVLR)
        {
          fprintf(stderr, "WARNING: found VLR for PTX with wrong payload size of %d.\n", ptxVLR->record_length_after_header);
        }
        else
        {
          fprintf(stderr, "WARNING: found no VLR with PTX info.\n");
        }
        fprintf(stderr, "         outputting PTS instead ...\n");
        if (header->version_minor >= 4)
        {
#ifdef _WIN32
          fprintf(file_out, "%I64d       \012", header->extended_number_of_point_records);
#else
          fprintf(file_out, "%lld       \012", header->extended_number_of_point_records);
#endif
        }
        else
        {
          fprintf(file_out, "%u       \012", header->number_of_point_records);
        }
      }
      if (parse_string && strcmp(parse_string, "xyz") && strcmp(parse_string, "xyzi") && strcmp(parse_string, "xyziRGB") && strcmp(parse_string, "xyzRGB"))
      {
        fprintf(stderr, "WARNING: the parse string for PTX should be 'xyz', 'xyzi', 'xyziRGB', or 'xyzRGB'\n");
      }
      if (separator_sign != ' ')
      {
        fprintf(stderr, "WARNING: the separator for PTX should be 'space' not '%s'\n", separator);
      }
    }
    else if (header_comment_sign)
    {
      // output header info
      fprintf(file_out, "%c file signature:            '%.4s'\012", header_comment_sign, header->file_signature);
      fprintf(file_out, "%c file source ID:            %d\012", header_comment_sign, header->file_source_ID);
      fprintf(file_out, "%c reserved (global encoding):%d\012", header_comment_sign, header->global_encoding);
      fprintf(file_out, "%c project ID GUID data 1-4:  %d %d %d '%.8s'\012", header_comment_sign, header->project_ID_GUID_data_1, header->project_ID_GUID_data_2, header->project_ID_GUID_data_3, header->project_ID_GUID_data_4);
      fprintf(file_out, "%c version major.minor:       %d.%d\012", header_comment_sign, header->version_major, header->version_minor);
      fprintf(file_out, "%c system_identifier:         '%.32s'\012", header_comment_sign, header->system_identifier);
      fprintf(file_out, "%c generating_software:       '%.32s'\012", header_comment_sign, header->generating_software);
      fprintf(file_out, "%c file creation day/year:    %d/%d\012", header_comment_sign, header->file_creation_day, header->file_creation_year);
      fprintf(file_out, "%c header size                %d\012", header_comment_sign, header->header_size);
      fprintf(file_out, "%c offset to point data       %u\012", header_comment_sign, header->offset_to_point_data);
      fprintf(file_out, "%c number var. length records %u\012", header_comment_sign, header->number_of_variable_length_records);
      fprintf(file_out, "%c point data format          %d\012", header_comment_sign, header->point_data_format);
      fprintf(file_out, "%c point data record length   %d\012", header_comment_sign, header->point_data_record_length);
      fprintf(file_out, "%c number of point records    %u\012", header_comment_sign, header->number_of_point_records);
      fprintf(file_out, "%c number of points by return %u %u %u %u %u\012", header_comment_sign, header->number_of_points_by_return[0], header->number_of_points_by_return[1], header->number_of_points_by_return[2], header->number_of_points_by_return[3], header->number_of_points_by_return[4]);
      fprintf(file_out, "%c scale factor x y z         %g %g %g\012", header_comment_sign, header->x_scale_factor, header->y_scale_factor, header->z_scale_factor);
      fprintf(file_out, "%c offset x y z               ", header_comment_sign); lidardouble2string(printstring, header->x_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, header->y_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, header->z_offset); fprintf(file_out, "%s\012", printstring);
      fprintf(file_out, "%c min x y z                  ", header_comment_sign); lidardouble2string(printstring, header->min_x, header->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, header->min_y, header->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, header->min_z, header->z_scale_factor); fprintf(file_out, "%s\012", printstring);
      fprintf(file_out, "%c max x y z                  ", header_comment_sign); lidardouble2string(printstring, header->max_x, header->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, header->max_y, header->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, header->max_z, header->z_scale_factor); fprintf(file_out, "%s\012", printstring);
      // if LAS 1.4
      if (header->version_minor >= 4)
      {
#ifdef _WIN32
        fprintf(file_out, "%c extended number of point records    %I64d\012", header_comment_sign, header->extended_number_of_point_records);
        fprintf(file_out, "%c extended number of points by return %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d\012", header_comment_sign, header->extended_number_of_points_by_return[0], header->extended_number_of_points_by_return[1], header->extended_number_of_points_by_return[2], header->extended_number_of_points_by_return[3], header->extended_number_of_points_by_return[4], header->extended_number_of_points_by_return[5], header->extended_number_of_points_by_return[6], header->extended_number_of_points_by_return[7], header->extended_number_of_points_by_return[8], header->extended_number_of_points_by_return[9], header->extended_number_of_points_by_return[10], header->extended_number_of_points_by_return[11], header->extended_number_of_points_by_return[12], header->extended_number_of_points_by_return[13], header->extended_number_of_points_by_return[14]);
#else
        fprintf(file_out, "%c extended number of point records    %lld\012", header_comment_sign, header->extended_number_of_point_records);
        fprintf(file_out, "%c extended number of points by return %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\012", header_comment_sign, header->extended_number_of_points_by_return[0], header->extended_number_of_points_by_return[1], header->extended_number_of_points_by_return[2], header->extended_number_of_points_by_return[3], header->extended_number_of_points_by_return[4], header->extended_number_of_points_by_return[5], header->extended_number_of_points_by_return[6], header->extended_number_of_points_by_return[7], header->extended_number_of_points_by_return[8], header->extended_number_of_points_by_return[9], header->extended_number_of_points_by_return[10], header->extended_number_of_points_by_return[11], header->extended_number_of_points_by_return[12], header->extended_number_of_points_by_return[13], header->extended_number_of_points_by_return[14]);
#endif
      }
    }

    // maybe create default parse string

    if (parse_string == 0) parse_string = LASCopyString("xyz");

    // check requested fields and print warnings if attributes do not exist

    I32 index;
    i = 0;
    while (parse_string[i])
    {
      switch (parse_string[i])
      {
      case '_': // diff of unscaled raw integer X to prev point
      case '!': // diff of unscaled raw integer Y to prev point
      case 'x': // the x coordinate
      case 'y': // the y coordinate
      case 'X': // the unscaled raw integer X coordinate
      case 'Y': // the unscaled raw integer Y coordinate
      case 'r': // the number of the return
      case 'n': // the number of returns of given pulse
      case '@': // diff of unscaled raw integer Z to prev point
      case 'z': // the z coordinate
      case 'Z': // the unscaled raw integer Z coordinate
      case 'i': // the intensity
      case 'a': // the scan angle
      case 'c': // the classification
      case 'u': // the user data
      case 'p': // the point source ID
      case 'e': // the edge of flight line flag
      case 'd': // the direction of scan flag
      case 'h': // the with<h>eld flag
      case 'k': // the <k>eypoint flag
      case 'g': // the synthetic fla<g>
      case 'm': // the index of the point (count starts at 0)
      case 'M': // the index of the point (count starts at 1)
        break;
      case 'l': // the (extended) scanner channe<l>
        if (lasreader->point.extended_point_type == 0)
          fprintf (stderr, "WARNING: requested 'l' but points are not of extended type\n");
        break;
      case 'o': // the (extended) <o>verlap flag
        if (lasreader->point.extended_point_type == 0)
          fprintf (stderr, "WARNING: requested 'o' but points are not of extended type\n");
        break;
      case '#': // diff of gps-time to prev point
      case 't': // the gps-time
        if (lasreader->point.have_gps_time == false)
          fprintf (stderr, "WARNING: requested 't' but points do not have gps time\n");
        break;
      case '$': // the R difference to the last point
      case '&': // the byte-wise R difference to the last point
      case 'R': // the red channel of the RGB field
        if (lasreader->point.have_rgb == false)
          fprintf (stderr, "WARNING: requested 'R' but points do not have RGB\n");
        break;
      case '%': // the G difference to the last point
      case '*': // the byte-wise G difference to the last point
      case 'G': // the green channel of the RGB field
        if (lasreader->point.have_rgb == false)
          fprintf (stderr, "WARNING: requested 'G' but points do not have RGB\n");
        break;
      case '^': // the B difference to the last point
      case '+': // the byte-wise B difference to the last point
      case 'B': // the blue channel of the RGB field
        if (lasreader->point.have_rgb == false)
          fprintf (stderr, "WARNING: requested 'B' but points do not have RGB\n");
        break;
      case HSL:
      case HSV:
      case HSL255:
      case HSV255:
        if (lasreader->point.have_rgb == false)
          fprintf (stderr, "WARNING: requested 'HSV' or 'HSL' but points do not have RGB\n");
        break;
      case 'I': // the near infrared channel of the RGBI field
        if (lasreader->point.have_nir == false)
          fprintf (stderr, "WARNING: requested 'I' but points do not have NIR\n");
        break;
      case 'w': // the wavepacket index
        if (lasreader->point.have_wavepacket == false)
          fprintf (stderr, "WARNING: requested 'w' but points do not have wavepacket\n");
        break;
      case 'W': // all wavepacket attributes
        if (lasreader->point.have_wavepacket == false)
          fprintf (stderr, "WARNING: requested 'W' but points do not have wavepacket\n");
        break;
      case 'V': // the waveform data
        if (laswaveform13reader == 0)
        {
          fprintf (stderr, "WARNING: requested 'V' but no waveform data available\n");
          fprintf (stderr, "         omitting ...\n");
        }
        break;
      case 'E':
        if (extra_string == 0)
        {
          fprintf (stderr, "WARNING: requested 'E' but no '-extra' specified. skipping ...\n");
          parse_string[i] = 's';
        }
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if ((parse_string[i] - '0') >= lasreader->header.number_attributes)
        {
          fprintf(stderr, "WARNING: attribute '%d' does not exist. skipping ...\n", (parse_string[i] - '0'));
          parse_string[i] = 's';
        }
        else
        {
          attribute_starts[(parse_string[i] - '0')] = lasreader->header.get_attribute_start((parse_string[i] - '0'));
        }
        break;
      case '(':
        index = 0;
        i++;
        while (parse_string[i] && ('0' <= parse_string[i]) && (parse_string[i] <= '9'))
        {
          index = 10*index + (parse_string[i] - '0');
          i++;
        }
        if (index >= lasreader->header.number_attributes)
        {
          fprintf(stderr, "ERROR: attribute '%d' does not exist. skipping ...\n", index);
          byebye(true);
        }
        else
        {
          attribute_starts[index] = lasreader->header.get_attribute_start(index);
        }
        break;
      case 's':
        break;
      default:
        fprintf (stderr, "WARNING: requested unknown parse item '%c'\n", parse_string[i]);
      }
      i++;
    }

    // in case diff is requested

    int last_XYZ[3] = {0,0,0};
    unsigned short last_RGB[4] = {0,0,0};
    double last_GPSTIME = 0;

    // read and convert the points to ASCII

#ifdef _WIN32
    if (verbose) fprintf(stderr,"processing %I64d points with '%s'.\n", lasreader->npoints, parse_string);
#else
    if (verbose) fprintf(stderr,"processing %lld points with '%s'.\n", lasreader->npoints, parse_string);
#endif

    // print the column names in the first line
    if (coldesc)
    {
      int i = 0;
      while (true)
      {
        switch (parse_string[i])
        {
        case 'x': fprintf(file_out, "x"); break; // the z coordinate
        case 'y': fprintf(file_out, "y"); break; // the y coordinate
        case 'z': fprintf(file_out, "z"); break; // the z coordinate
        case 'X': fprintf(file_out, "X"); break; // the unscaled and unoffset integer X coordinate
        case 'Y': fprintf(file_out, "Y"); break;// the unscaled and unoffset integer Y coordinate
        case 'Z': fprintf(file_out, "Z"); break; // the unscaled and unoffset integer Z coordinate
        case 't': fprintf(file_out, "gps_time"); break; // the gps-time
        case 'i': fprintf(file_out, "intensity"); break; // the intensity
        case 'a': fprintf(file_out, "scan_angle"); break; // the scan angle
        case 'r': fprintf(file_out, "return_number"); break; // the number of the return
        case 'c': fprintf(file_out, "classification"); break; // the classification
        case 'u': fprintf(file_out, "user_data"); break; // the user data
        case 'n': fprintf(file_out, "number_of_returns"); break; // the number of returns of given pulse
        case 'p': fprintf(file_out, "point_source_id"); break; // the point source ID
        case 'e': fprintf(file_out, "edge_of_flight_line"); break; // the edge of flight line flag
        case 'd': fprintf(file_out, "scan_direction_flag"); break; // the direction of scan flag
        case 'h': fprintf(file_out, "withheld_flag"); break; // the withheld flag
        case 'k': fprintf(file_out, "keypoint_flag"); break; // the keypoint flag
        case 'g': fprintf(file_out, "synthetic_flag"); break; // the synthetic flag
        case 'o': fprintf(file_out, "overlap_flag"); break; // the overlap flag
        case 'l': fprintf(file_out, "scanner_channel"); break; // the scanner channel
        case 'R': fprintf(file_out, "R"); break;// the red channel of the RGB field
        case 'G': fprintf(file_out, "G"); break; // the green channel of the RGB field
        case 'B': fprintf(file_out, "B"); break; // the blue channel of the RGB field
        case 'm': fprintf(file_out, "0-index"); break; // the index of the point (count starts at 0)
        case 'M': fprintf(file_out, "1-index"); break; // the index of the point (count starts at 1)
        case '_': fprintf(file_out, "raw_int_X_diff"); break; // the raw integer X difference to the last point
        case '!': fprintf(file_out, "raw_int_Y_diff"); break; // the raw integer Y difference to the last point
        case '@': fprintf(file_out, "raw_int_Z_diff"); break; // the raw integer Z difference to the last point
        case '#': fprintf(file_out, "gpstime_diff"); break; // the gps-time difference to the last point
        case '$': fprintf(file_out, "R_diff"); break; // the R difference to the last point
        case '%': fprintf(file_out, "G_diff"); break; // the G difference to the last point
        case '^': fprintf(file_out, "B_diff"); break; // the B difference to the last point
        case '&': fprintf(file_out, "bitwise_R_diff"); break; // the byte-wise R difference to the last point
        case '+': fprintf(file_out, "bitwise_B_diff"); break; // the byte-wise B difference to the last point
        case 'w': fprintf(file_out, "wavepacket_descriptor_index"); break;// the wavepacket descriptor index
        case 'W': fprintf(file_out, "wavepacket_descriptor_index%cwavepacket_offset%cwavepacket_size%cwavepacket_location%cXt%cYt%cZt", separator_sign, separator_sign, separator_sign, separator_sign, separator_sign, separator_sign); break; // all wavepacket attributes
        case 'V': break;
        case 'E': break;
        case HSV255: fprintf(file_out, "HSV_H%cHSV_S%cHSV_V", separator_sign, separator_sign); break; // the HSV representation of RGB in [0,360|100]
        case HSL255: fprintf(file_out, "HSL_H%cHSL_S%cHSL_L", separator_sign, separator_sign); break; // the HSL representation of RGB in [0,360|100]
        case HSV: fprintf(file_out, "HSV_h%cHSV_s%cHSV_v", separator_sign, separator_sign); break; // the HSV representation of RGB in [0, 1]
        case HSL: fprintf(file_out, "HSL_h%cHSL_s%cHSL_l", separator_sign, separator_sign); break; // the HSL representation of RGB in [0, 1]
        default: break; // must handle extra bytes
        }

        i++;
        if (parse_string[i])
        {
          fprintf(file_out, "%c", separator_sign);
        }
        else
        {
          fprintf(file_out, "\012");
          break;
        }
      }
    }

    while (lasreader->read_point())
    {
      i = 0;
      while (true)
      {
        switch (parse_string[i])
        {
        case 'x': // the x coordinate
          lidardouble2string(printstring, lasreader->point.get_x(), lasreader->header.x_scale_factor); fprintf(file_out, "%s", printstring);
          break;
        case 'y': // the y coordinate
          lidardouble2string(printstring, lasreader->point.get_y(), lasreader->header.y_scale_factor); fprintf(file_out, "%s", printstring);
          break;
        case 'z': // the z coordinate
          lidardouble2string(printstring, lasreader->point.get_z(), lasreader->header.z_scale_factor); fprintf(file_out, "%s", printstring);
          break;
        case 'X': // the unscaled raw integer X coordinate
          fprintf(file_out, "%d", lasreader->point.get_X());
          break;
        case 'Y': // the unscaled raw integer Y coordinate
          fprintf(file_out, "%d", lasreader->point.get_Y());
          break;
        case 'Z': // the unscaled raw integer Z coordinate
          fprintf(file_out, "%d", lasreader->point.get_Z());
          break;
        case 't': // the gps-time
          fprintf(file_out, "%.6f", lasreader->point.get_gps_time());
          break;
        case 'i': // the intensity
          if (opts)
            fprintf(file_out, "%d", -2048 + lasreader->point.get_intensity());
          else if (optx)
          {
            int len;
            len = sprintf(printstring, "%.3f", 1.0f/4095.0f * lasreader->point.get_intensity()) - 1;
            while (printstring[len] == '0') len--;
            if (printstring[len] != '.') len++;
            printstring[len] = '\0';
            fprintf(file_out, "%s", printstring);
          }
          else
            fprintf(file_out, "%d", lasreader->point.get_intensity());
          break;
        case 'a': // the scan angle
          fprintf(file_out, "%g", lasreader->point.get_scan_angle());
          break;
        case 'r': // the number of the return
          if (header->point_data_format > 5)
          {
            fprintf(file_out, "%d", lasreader->point.get_extended_return_number());
          }
          else
          {
            fprintf(file_out, "%d", lasreader->point.get_return_number());
          }
          break;
        case 'c': // the classification
          if (header->point_data_format > 5)
          {
            if (lasreader->point.get_extended_classification())
            {
              fprintf(file_out, "%d", lasreader->point.get_extended_classification());
            }
            else
            {
              fprintf(file_out, "%d", lasreader->point.get_classification());
            }
          }
          else
          {
            fprintf(file_out, "%d", lasreader->point.get_classification());
          }
          break;
        case 'u': // the user data
          fprintf(file_out, "%d", lasreader->point.get_user_data());
          break;
        case 'n': // the number of returns of given pulse
          if (header->point_data_format > 5)
          {
            fprintf(file_out, "%d", lasreader->point.get_extended_number_of_returns());
          }
          else
          {
            fprintf(file_out, "%d", lasreader->point.get_number_of_returns());
          }
          break;
        case 'p': // the point source ID
          fprintf(file_out, "%d", lasreader->point.get_point_source_ID());
          break;
        case 'e': // the edge of flight line flag
          fprintf(file_out, "%d", lasreader->point.get_edge_of_flight_line());
          break;
        case 'd': // the direction of scan flag
          fprintf(file_out, "%d", lasreader->point.get_scan_direction_flag());
          break;
        case 'h': // the withheld flag
          fprintf(file_out, "%d", lasreader->point.get_withheld_flag());
          break;
        case 'k': // the keypoint flag
          fprintf(file_out, "%d", lasreader->point.get_keypoint_flag());
          break;
        case 'g': // the synthetic flag
          fprintf(file_out, "%d", lasreader->point.get_synthetic_flag());
          break;
        case 'o': // the (extended) overlap flag
          fprintf(file_out, "%d", lasreader->point.get_extended_overlap_flag());
          break;
        case 'l': // the (extended) scanner channel
          fprintf(file_out, "%d", lasreader->point.get_extended_scanner_channel());
          break;
        case 'R': // the red channel of the RGB field
          fprintf(file_out, "%d", lasreader->point.rgb[0]);
          break;
        case 'G': // the green channel of the RGB field
          fprintf(file_out, "%d", lasreader->point.rgb[1]);
          break;
        case 'B': // the blue channel of the RGB field
          fprintf(file_out, "%d", lasreader->point.rgb[2]);
          break;
        case 'I': // the near-infrared channel of the RGBI field
          fprintf(file_out, "%d", lasreader->point.rgb[3]);
          break;
        case 'm': // the index of the point (count starts at 0)
#ifdef _WIN32
          fprintf(file_out, "%I64d", lasreader->p_count-1);
#else
          fprintf(file_out, "%lld", lasreader->p_count-1);
#endif
          break;
        case 'M': // the index of the point  (count starts at 1)
#ifdef _WIN32
          fprintf(file_out, "%I64d", lasreader->p_count);
#else
          fprintf(file_out, "%lld", lasreader->p_count);
#endif
          break;
        case '_': // the raw integer X difference to the last point
          fprintf(file_out, "%d", lasreader->point.get_X()-last_XYZ[0]);
          break;
        case '!': // the raw integer Y difference to the last point
          fprintf(file_out, "%d", lasreader->point.get_Y()-last_XYZ[1]);
          break;
        case '@': // the raw integer Z difference to the last point
          fprintf(file_out, "%d", lasreader->point.get_Z()-last_XYZ[2]);
          break;
        case '#': // the gps-time difference to the last point
          lidardouble2string(printstring,lasreader->point.gps_time-last_GPSTIME); fprintf(file_out, "%s", printstring);
          break;
        case '$': // the R difference to the last point
          fprintf(file_out, "%d", lasreader->point.rgb[0]-last_RGB[0]);
          break;
        case '%': // the G difference to the last point
          fprintf(file_out, "%d", lasreader->point.rgb[1]-last_RGB[1]);
          break;
        case '^': // the B difference to the last point
          fprintf(file_out, "%d", lasreader->point.rgb[2]-last_RGB[2]);
          break;
        case '&': // the byte-wise R difference to the last point
          fprintf(file_out, "%d%c%d", (lasreader->point.rgb[0]>>8)-(last_RGB[0]>>8), separator_sign, (lasreader->point.rgb[0]&255)-(last_RGB[0]&255));
          break;
        case '*': // the byte-wise G difference to the last point
          fprintf(file_out, "%d%c%d", (lasreader->point.rgb[1]>>8)-(last_RGB[1]>>8), separator_sign, (lasreader->point.rgb[1]&255)-(last_RGB[1]&255));
          break;
        case '+': // the byte-wise B difference to the last point
          fprintf(file_out, "%d%c%d", (lasreader->point.rgb[2]>>8)-(last_RGB[2]>>8), separator_sign, (lasreader->point.rgb[2]&255)-(last_RGB[2]&255));
          break;
        case 'w': // the wavepacket index
          fprintf(file_out, "%d", lasreader->point.wavepacket.getIndex());
          break;
        case 'W': // all wavepacket attributes
          fprintf(file_out, "%d%c%d%c%d%c%g%c%g%c%g%c%g", lasreader->point.wavepacket.getIndex(), separator_sign, (U32)lasreader->point.wavepacket.getOffset(), separator_sign, lasreader->point.wavepacket.getSize(), separator_sign, lasreader->point.wavepacket.getLocation(), separator_sign, lasreader->point.wavepacket.getXt(), separator_sign, lasreader->point.wavepacket.getYt(), separator_sign, lasreader->point.wavepacket.getZt());
          break;
        case 'V': // the waVeform
          if (laswaveform13reader && laswaveform13reader->read_waveform(&lasreader->point))
          {
            output_waveform(file_out, separator_sign, laswaveform13reader);
          }
          else
          {
            fprintf(file_out, "no_waveform");
          }
          break;
        case 'E': // the extra string
          fprintf(file_out, "%s", extra_string);
          break;
        case HSV255: { // the HSV representation of RGB
          F32 hsv[3];
          lasreader->point.get_hsv(hsv);
          fprintf(file_out, "%d%c%d%c%d", (U16)(hsv[0]*360), separator_sign, (U8)(hsv[1]*100), separator_sign, (U8)(hsv[2]*100));
          break;
        }
        case HSV: { // the HSV representation of RGB
          F32 hsv[3];
          lasreader->point.get_hsv(hsv);
          fprintf(file_out, "%.3f%c%.3f%c%.3f", hsv[0], separator_sign, hsv[1], separator_sign, hsv[2]);
          break;
        }
        case HSL255: { // the HSL representation of RGB
          F32 hsl[3];
          lasreader->point.get_hsl(hsl);
          fprintf(file_out, "%d%c%d%c%d", (U16)(hsl[0]*360), separator_sign, (U8)(hsl[1]*100), separator_sign, (U8)(hsl[2]*100));
          break;
        }
        case HSL: { // the HSL representation of RGB
          F32 hsl[3];
          lasreader->point.get_hsl(hsl);
          fprintf(file_out, "%.3f%c%.3f%c%.3f", hsl[0], separator_sign, hsl[1], separator_sign, hsl[2]);
          break;
        }
        case '0': // the extra attributes
        case '1': // the extra attributes
        case '2': // the extra attributes
        case '3': // the extra attributes
        case '4': // the extra attributes
        case '5': // the extra attributes
        case '6': // the extra attributes
        case '7': // the extra attributes
        case '8': // the extra attributes
        case '9': // the extra attributes
          print_attribute(file_out, &lasreader->header, &lasreader->point, (I32)(parse_string[i]-'0'), printstring);
          break;
        default:
          index = 0;
          i++;
          while (parse_string[i] && ('0' <= parse_string[i]) && (parse_string[i] <= '9'))
          {
            index = 10*index + (parse_string[i] - '0');
            i++;
          }
          print_attribute(file_out, &lasreader->header, &lasreader->point, index, printstring);
        }
        i++;
        if (parse_string[i])
        {
          fprintf(file_out, "%c", separator_sign);
        }
        else
        {
          fprintf(file_out, "\012");
          break;
        }
      }
      if (diff)
      {
        last_XYZ[0] = lasreader->point.get_X();
        last_XYZ[1] = lasreader->point.get_Y();
        last_XYZ[2] = lasreader->point.get_Z();
        last_GPSTIME = lasreader->point.gps_time;
        last_RGB[0] = lasreader->point.rgb[0];
        last_RGB[1] = lasreader->point.rgb[1];
        last_RGB[2] = lasreader->point.rgb[2];
      }
    }

#ifdef _WIN32
    if (verbose) fprintf(stderr,"converting %I64d points of '%s' took %g sec.\n", lasreader->p_count, lasreadopener.get_file_name(), taketime()-start_time);
#else
    if (verbose) fprintf(stderr,"converting %lld points of '%s' took %g sec.\n", lasreader->p_count, lasreadopener.get_file_name(), taketime()-start_time);
#endif

    // close the reader
    lasreader->close();
    delete lasreader;

    // (maybe) close the waveform reader
    if (laswaveform13reader)
    {
      laswaveform13reader->close();
      delete laswaveform13reader;
    }

    // close the files

    if (file_out != stdout) fclose(file_out);
  }

  free(parse_string);

  byebye(false, argc==1);

  return 0;
}
