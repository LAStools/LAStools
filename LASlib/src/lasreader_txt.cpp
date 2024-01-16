/*
===============================================================================

  FILE:  lasreader_txt.cpp

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
#include "lasreader_txt.hpp"
#include "lastransform.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" FILE * fopen_compressed(const char* filename, const char* mode, bool* piped);

BOOL LASreaderTXT::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (file_name == 0)
  {
    fprintf(stderr, "ERROR: file name pointer is zero\n");
    return FALSE;
  }

  FILE* file = fopen_compressed(file_name, "r", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 10 * LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10 * LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  return open(file, file_name, point_type, parse_string, skip_lines, populate_header);
}

BOOL LASreaderTXT::open(FILE* file, const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  int i;
  BOOL has_column_description = FALSE;

  if (file == 0)
  {
    fprintf(stderr, "ERROR: file pointer is zero\n");
    return FALSE;
  }
  // clean the reader
  clean();
  // clean the header
  header.clean();
  // set the file pointer
  this->file = file;
  // add attributes in extra bytes
  if (number_attributes)
  {
    for (i = 0; i < number_attributes; i++)
    {
      I32 type = (attributes_data_types[i] - 1) % 10;
      try {
        LASattribute attribute(type, attribute_names[i], attribute_descriptions[i]);
        if (attribute_scales[i] != 1.0 || attribute_offsets[i] != 0.0)
        {
          attribute.set_scale(attribute_scales[i]);
        }
        if (attribute_offsets[i] != 0.0)
        {
          attribute.set_offset(attribute_offsets[i]);
        }
        if (attribute_no_datas[i] != F64_MAX)
        {
          attribute.set_no_data(attribute_no_datas[i]);
        }
        header.add_attribute(attribute);
      }
      catch (...) {
        fprintf(stderr, "ERROR: initializing attribute %s\n", attribute_descriptions[i]);
        return FALSE;
      }
    }
  }

  this->skip_lines = skip_lines;

  if (parse_string)
  {
    // User-input parse string has the precedence
    this->parse_string = LASCopyString(parse_string);
    // User provided a parse_string but the file may contain the column description
    for (i = 0; i < this->skip_lines; i++) fgets(line, 512, file);
    char* auto_parse_string = 0;
    has_column_description = parse_column_description(&auto_parse_string);
    fseek(file, 0, SEEK_SET);
    if (has_column_description && auto_parse_string == 0)
    {
      return FALSE;
    }
    if (has_column_description && strcmp(this->parse_string, auto_parse_string) != 0)
    {
      fprintf(stderr, "WARNING: input parse string is '%s' but the column description produced '%s'. User input has the precedence.\n", this->parse_string, auto_parse_string);
    }
    if (auto_parse_string) free(auto_parse_string);
  }
  else
  {
    // User did not provide a parse_string the file may contain the column description
    for (i = 0; i < this->skip_lines; i++) fgets(line, 512, file);
    has_column_description = parse_column_description(&this->parse_string);
    fseek(file, 0, SEEK_SET);
    // Column description found but nothing parsed.
    if (has_column_description && this->parse_string == 0) return FALSE;
  }

  // Parse the special extended string flags into non ascii char
  if (this->parse_string)
  {
    parse_string_unparsed = LASCopyString(this->parse_string);
    parse_extended_flags(this->parse_string);
  }

  if (this->parse_string && !check_parse_string(this->parse_string))
  {
    return FALSE;
  }

  // populate the header as much as it makes sense
  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderTXT (%d)", LAS_TOOLS_VERSION);

  // maybe set creation date
#ifdef _WIN32
  if (file_name)
  {
    WIN32_FILE_ATTRIBUTE_DATA attr;
    SYSTEMTIME creation;
    GetFileAttributesEx(file_name, GetFileExInfoStandard, &attr);
    FileTimeToSystemTime(&attr.ftCreationTime, &creation);
    int startday[13] = { -1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    header.file_creation_day = startday[creation.wMonth] + creation.wDay;
    header.file_creation_year = creation.wYear;
    // leap year handling
    if ((((creation.wYear) % 4) == 0) && (creation.wMonth > 2)) header.file_creation_day++;
  }
  else
  {
    header.file_creation_day = 1;
    header.file_creation_year = 2019;
  }
#else
  header.file_creation_day = 1;
  header.file_creation_year = 2019;
#endif
  if (point_type)
  {
    switch (point_type)
    {
    case 1:
      header.point_data_record_length = 28;
      break;
    case 2:
      header.point_data_record_length = 26;
      break;
    case 3:
      header.point_data_record_length = 34;
      break;
    case 6:
      header.point_data_record_length = 30;
      break;
    case 7:
      header.point_data_record_length = 36;
      break;
    case 8:
      header.point_data_record_length = 38;
      break;
    default:
      return FALSE;
    }
    header.point_data_format = point_type;
  }
  else if (this->parse_string)
  {
    // Is there non ascii extended flags?
    BOOL has_hsv_hsl = FALSE;
    for (I32 i = 0; i < (I32)strlen(this->parse_string); i++)
    {
      char c = this->parse_string[i];
      has_hsv_hsl = (c == HSV_H) || (c == HSV_S) || (c == HSV_V) || (c == HSL_h) || (c == HSL_H) || (c == HSL_S) || (c == HSL_L) || (c == HSL_h) || (c == HSL_s) || (c == HSL_l) || (c == HSV_h) || (c == HSV_s) || (c == HSV_v);
    }

    if (strstr(this->parse_string, "o") || strstr(this->parse_string, "l") || strstr(this->parse_string, "I"))
    {
      // new point types
      if (strstr(this->parse_string, "I"))
      {
        header.point_data_format = 8;
        header.point_data_record_length = 38;
      }
      else if (strstr(this->parse_string, "R") || strstr(this->parse_string, "G") || strstr(this->parse_string, "B") || strstr(this->parse_string, "H") || has_hsv_hsl)
      {
        header.point_data_format = 7;
        header.point_data_record_length = 36;
      }
      else
      {
        header.point_data_format = 6;
        header.point_data_record_length = 30;
      }
    }
    else if (strstr(this->parse_string, "t"))
    {
      if (strstr(this->parse_string, "R") || strstr(this->parse_string, "G") || strstr(this->parse_string, "B") || strstr(this->parse_string, "H") || has_hsv_hsl)
      {
        header.point_data_format = 3;
        header.point_data_record_length = 34;
      }
      else
      {
        header.point_data_format = 1;
        header.point_data_record_length = 28;
      }
    }
    else
    {
      if (strstr(this->parse_string, "R") || strstr(this->parse_string, "G") || strstr(this->parse_string, "B") || strstr(this->parse_string, "H") || has_hsv_hsl)
      {
        header.point_data_format = 2;
        header.point_data_record_length = 26;
      }
      else
      {
        header.point_data_format = 0;
        header.point_data_record_length = 20;
      }
    }
  }
  else
  {
    header.point_data_format = 0;
    header.point_data_record_length = 20;
  }

  this->point_type = header.point_data_format;

  // maybe update point size with extra bytes

  if (header.number_attributes)
  {
    header.point_data_record_length += header.get_attributes_size();
  }

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length, &header);

  // we do not know yet how many points to expect

  npoints = 0;

  // should we perform an extra pass to fully populate the header

  if (populate_header && file_name)
  {
    // create a cheaper parse string that only looks for 'x' 'y' 'z' 'r' and attributes in extra bytes

    char* parse_less;
    if (this->parse_string == 0)
    {
      parse_less = LASCopyString("xyz");
    }
    else
    {
      parse_less = LASCopyString(this->parse_string);
      for (i = 0; i < (int)strlen(parse_less); i++)
      {
        if (parse_less[i] != 'x' && parse_less[i] != 'y' && parse_less[i] != 'z' && parse_less[i] != 'r' && (parse_less[i] < '0' || parse_less[i] > '0'))
        {
          parse_less[i] = 's';
        }
      }
      do
      {
        parse_less[i] = '\0';
        i--;
      } while (parse_less[i] == 's');
    }

    // skip lines if we have to

    for (i = 0; i < this->skip_lines; i++) fgets(line, 512, file);
    if (has_column_description) fgets(line, 512, file);

    if (ipts)
    {
      if (fgets(line, 512, file))
      {
#ifdef _WIN32
        if (sscanf(line, "%I64d", &npoints) != 1)
#else
        if (sscanf(line, "%lld", &npoints) != 1)
#endif
        {
          fprintf(stderr, "ERROR: parsing number of points for '-itps'\n");
          return FALSE;
        }
#ifdef _WIN32
        fprintf(stderr, "PTS header states %I64d points. ignoring ...\n", npoints);
#else
        fprintf(stderr, "PTS header states %lld points. ignoring ...\n", npoints);
#endif
        npoints = 0;
      }
      else
      {
        fprintf(stderr, "ERROR: reading PTS header for '-itps'\n");
        return FALSE;
      }
    }
    else if (iptx || iptx_transform)
    {
      I32 ncols;
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%d", &ncols) != 1)
        {
          fprintf(stderr, "ERROR: parsing number of cols\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with number of cols\n");
        return FALSE;
      }
      I32 nrows;
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%d", &nrows) != 1)
        {
          fprintf(stderr, "ERROR: parsing number of rows\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with number of rows\n");
        return FALSE;
      }
      npoints = (I64)ncols * (I64)nrows;
#ifdef _WIN32
      fprintf(stderr, "PTX header states %d cols by %d rows aka %I64d points. ignoring ...\n", ncols, nrows, npoints);
#else
      fprintf(stderr, "PTX header states %d cols by %d rows aka %lld points. ignoring ...\n", ncols, nrows, npoints);
#endif
      F64 ptx_scan_pos[3];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf", &(ptx_scan_pos[0]), &(ptx_scan_pos[1]), &(ptx_scan_pos[2])) != 3)
        {
          fprintf(stderr, "ERROR: parsing ptx_scan_pos\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with ptx_scan_pos\n");
        return FALSE;
      }
      F64 ptx_scan_axis_x[3];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_x[0]), &(ptx_scan_axis_x[1]), &(ptx_scan_axis_x[2])) != 3)
        {
          fprintf(stderr, "ERROR: parsing rotation row 0\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with rotation row 0\n");
        return FALSE;
      }
      F64 ptx_scan_axis_y[3];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_y[0]), &(ptx_scan_axis_y[1]), &(ptx_scan_axis_y[2])) != 3)
        {
          fprintf(stderr, "ERROR: parsing rotation row 1\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with rotation row 1\n");
        return FALSE;
      }
      F64 ptx_scan_axis_z[3];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_z[0]), &(ptx_scan_axis_z[1]), &(ptx_scan_axis_z[2])) != 3)
        {
          fprintf(stderr, "ERROR: parsing rotation row 2\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with rotation row 2\n");
        return FALSE;
      }
      F64 ptx_matrix_row_0[4];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_0[0]), &(ptx_matrix_row_0[1]), &(ptx_matrix_row_0[2]), &(ptx_matrix_row_0[3])) != 4)
        {
          fprintf(stderr, "ERROR: parsing transformation row 0\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with transformation row 0\n");
        return FALSE;
      }
      F64 ptx_matrix_row_1[4];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_1[0]), &(ptx_matrix_row_1[1]), &(ptx_matrix_row_1[2]), &(ptx_matrix_row_1[3])) != 4)
        {
          fprintf(stderr, "ERROR: parsing transformation row 1\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with transformation row 1\n");
        return FALSE;
      }
      F64 ptx_matrix_row_2[4];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_2[0]), &(ptx_matrix_row_2[1]), &(ptx_matrix_row_2[2]), &(ptx_matrix_row_2[3])) != 4)
        {
          fprintf(stderr, "ERROR: parsing transformation row 2\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with transformation row 2\n");
        return FALSE;
      }
      F64 ptx_matrix_row_3[4];
      if (fgets(line, 512, file))
      {
        if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_3[0]), &(ptx_matrix_row_3[1]), &(ptx_matrix_row_3[2]), &(ptx_matrix_row_3[3])) != 4)
        {
          fprintf(stderr, "ERROR: parsing transformation row 3\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: reading line with transformation row 3\n");
        return FALSE;
      }
      npoints = 0;
    }

    // read the first line

    while (fgets(line, 512, file))
    {
      if (parse(parse_less))
      {
        // mark that we found the first point
        npoints++;
        // we can stop this loop
        break;
      }
      else
      {
        line[strlen(line) - 1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_less);
      }
    }

    // did we manage to parse a line

    if (npoints == 0)
    {
      fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", parse_less);
      fclose(file);
      file = 0;
      free(parse_less);
      return FALSE;
    }

    // init the bounding box

    header.min_x = header.max_x = point.coordinates[0];
    header.min_y = header.max_y = point.coordinates[1];
    header.min_z = header.max_z = point.coordinates[2];

    // create return histogram

    if (point.extended_point_type)
    {
      if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number - 1]++;
    }
    else
    {
      if (point.return_number >= 1 && point.return_number <= 7) header.extended_number_of_points_by_return[point.return_number - 1]++;
    }

    // init the min and max of attributes in extra bytes

    if (number_attributes)
    {
      for (i = 0; i < number_attributes; i++)
      {
        header.attributes[i].set_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].set_max(point.extra_bytes + attribute_starts[i]);
      }
    }

    // loop over the remaining lines

    while (fgets(line, 512, file))
    {
      if (parse(parse_less))
      {
        // count points
        npoints++;
        // create return histogram
        if (point.extended_point_type)
        {
          if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number - 1]++;
        }
        else
        {
          if (point.return_number >= 1 && point.return_number <= 7) header.extended_number_of_points_by_return[point.return_number - 1]++;
        }
        // update bounding box
        if (point.coordinates[0] < header.min_x) header.min_x = point.coordinates[0];
        else if (point.coordinates[0] > header.max_x) header.max_x = point.coordinates[0];
        if (point.coordinates[1] < header.min_y) header.min_y = point.coordinates[1];
        else if (point.coordinates[1] > header.max_y) header.max_y = point.coordinates[1];
        if (point.coordinates[2] < header.min_z) header.min_z = point.coordinates[2];
        else if (point.coordinates[2] > header.max_z) header.max_z = point.coordinates[2];
        // update the min and max of attributes in extra bytes
        if (number_attributes)
        {
          for (i = 0; i < number_attributes; i++)
          {
            header.attributes[i].update_min(point.extra_bytes + attribute_starts[i]);
            header.attributes[i].update_max(point.extra_bytes + attribute_starts[i]);
          }
        }
      }
      else
      {
        line[strlen(line) - 1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_less);
      }
    }

#ifdef _WIN32
    fprintf(stderr, "counted %I64d points in populate pass.\n", npoints);
#else
    fprintf(stderr, "counted %lld points in populate pass.\n", npoints);
#endif

    if (point.extended_point_type || (npoints > U32_MAX) || header.extended_number_of_points_by_return[5] || header.extended_number_of_points_by_return[6] || header.extended_number_of_points_by_return[7] || header.extended_number_of_points_by_return[8] || header.extended_number_of_points_by_return[9] || header.extended_number_of_points_by_return[10] || header.extended_number_of_points_by_return[11] || header.extended_number_of_points_by_return[12] || header.extended_number_of_points_by_return[13] || header.extended_number_of_points_by_return[14])
    {
      header.version_minor = 4;
      header.header_size = 375;
      header.offset_to_point_data = 375;
      header.number_of_point_records = 0;
      header.number_of_points_by_return[0] = 0;
      header.number_of_points_by_return[1] = 0;
      header.number_of_points_by_return[2] = 0;
      header.number_of_points_by_return[3] = 0;
      header.number_of_points_by_return[4] = 0;
      header.extended_number_of_point_records = npoints;
    }
    else
    {
      header.version_minor = 2;
      header.header_size = 227;
      header.offset_to_point_data = 227;
      header.number_of_point_records = (U32)npoints;
      header.number_of_points_by_return[0] = (U32)header.extended_number_of_points_by_return[0];
      header.number_of_points_by_return[1] = (U32)header.extended_number_of_points_by_return[1];
      header.number_of_points_by_return[2] = (U32)header.extended_number_of_points_by_return[2];
      header.number_of_points_by_return[3] = (U32)header.extended_number_of_points_by_return[3];
      header.number_of_points_by_return[4] = (U32)header.extended_number_of_points_by_return[4];
      header.extended_number_of_point_records = 0;
      header.extended_number_of_points_by_return[0] = 0;
      header.extended_number_of_points_by_return[1] = 0;
      header.extended_number_of_points_by_return[2] = 0;
      header.extended_number_of_points_by_return[3] = 0;
      header.extended_number_of_points_by_return[4] = 0;
    }

    // free the parse less string

    free(parse_less);

    // close the input file

    fclose(file);

    // populate scale and offset

    populate_scale_and_offset();

    // populate bounding box

    populate_bounding_box();

    // mark that header is already populated

    populated_header = TRUE;

    // reopen input file for the second pass

    file = fopen_compressed(file_name, "r", &piped);
    if (file == 0)
    {
      fprintf(stderr, "ERROR: could not open '%s' for second pass\n", file_name);
      return FALSE;
    }

    if (setvbuf(file, NULL, _IOFBF, 10 * LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
    {
      fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10 * LAS_TOOLS_IO_IBUFFER_SIZE);
    }
  }

  if (this->parse_string == 0)
  {
    this->parse_string_unparsed = LASCopyString("xyz");
    this->parse_string = LASCopyString("xyz");
  }

  // skip lines if we have to

  if (this->skip_lines)
  {
    for (i = 0; i < this->skip_lines; i++) fgets(line, 512, file);
  }
  else if (ipts)
  {
    if (fgets(line, 512, file))
    {
      if (!populated_header)
      {
#ifdef _WIN32
        if (sscanf(line, "%I64d", &npoints) != 1)
#else
        if (sscanf(line, "%lld", &npoints) != 1)
#endif
        {
          fprintf(stderr, "ERROR: parsing number of points for '-itps'\n");
          return FALSE;
        }
      }
      if (!populated_header)
      {
        if (npoints > U32_MAX)
        {
          header.version_minor = 4;
          header.header_size = 375;
          header.offset_to_point_data = 375;
          header.number_of_point_records = 0;
          header.number_of_points_by_return[0] = 0;
          header.number_of_points_by_return[1] = 0;
          header.number_of_points_by_return[2] = 0;
          header.number_of_points_by_return[3] = 0;
          header.number_of_points_by_return[4] = 0;
          header.extended_number_of_point_records = npoints;
        }
        else
        {
          header.version_minor = 2;
          header.header_size = 227;
          header.offset_to_point_data = 227;
          header.number_of_point_records = (U32)npoints;
          header.extended_number_of_point_records = 0;
        }
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading PTS header for '-itps'\n");
      return FALSE;
    }

    // add payload that informs about PTS

    U8* payload = new U8[32];
    memset(payload, 0, 32);
    ((F32*)payload)[0] = translate_intensity;
    ((F32*)payload)[1] = scale_intensity;
    strcpy((char*)(payload + 16), (parse_string ? parse_string : "xyz"));
    header.add_vlr("LAStools", 2000, 32, payload);
  }
  else if (iptx || iptx_transform)
  {
    I32 ncols;
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%d", &ncols) != 1)
      {
        fprintf(stderr, "ERROR: parsing number of cols\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with number of cols\n");
      return FALSE;
    }
    I32 nrows;
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%d", &nrows) != 1)
      {
        fprintf(stderr, "ERROR: parsing number of rows\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with number of rows\n");
      return FALSE;
    }
    npoints = (I64)ncols * (I64)nrows;
    if (!populated_header)
    {
      if (npoints > U32_MAX)
      {
        header.version_minor = 4;
        header.header_size = 375;
        header.offset_to_point_data = 375;
        header.number_of_point_records = 0;
        header.number_of_points_by_return[0] = 0;
        header.number_of_points_by_return[1] = 0;
        header.number_of_points_by_return[2] = 0;
        header.number_of_points_by_return[3] = 0;
        header.number_of_points_by_return[4] = 0;
        header.extended_number_of_point_records = npoints;
      }
      else
      {
        header.version_minor = 2;
        header.header_size = 227;
        header.offset_to_point_data = 227;
        header.number_of_point_records = (U32)npoints;
        header.extended_number_of_point_records = 0;
      }
    }
    F64 ptx_scan_pos[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(ptx_scan_pos[0]), &(ptx_scan_pos[1]), &(ptx_scan_pos[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing ptx_scan_pos\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with ptx_scan_pos\n");
      return FALSE;
    }
    F64 ptx_scan_axis_x[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_x[0]), &(ptx_scan_axis_x[1]), &(ptx_scan_axis_x[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 0\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 0\n");
      return FALSE;
    }
    F64 ptx_scan_axis_y[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_y[0]), &(ptx_scan_axis_y[1]), &(ptx_scan_axis_y[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 1\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 1\n");
      return FALSE;
    }
    F64 ptx_scan_axis_z[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(ptx_scan_axis_z[0]), &(ptx_scan_axis_z[1]), &(ptx_scan_axis_z[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 2\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 2\n");
      return FALSE;
    }
    F64 ptx_matrix_row_0[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_0[0]), &(ptx_matrix_row_0[1]), &(ptx_matrix_row_0[2]), &(ptx_matrix_row_0[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 0\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 0\n");
      return FALSE;
    }
    F64 ptx_matrix_row_1[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_1[0]), &(ptx_matrix_row_1[1]), &(ptx_matrix_row_1[2]), &(ptx_matrix_row_1[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 1\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 1\n");
      return FALSE;
    }
    F64 ptx_matrix_row_2[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_2[0]), &(ptx_matrix_row_2[1]), &(ptx_matrix_row_2[2]), &(ptx_matrix_row_2[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 2\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 2\n");
      return FALSE;
    }
    F64 ptx_matrix_row_3[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(ptx_matrix_row_3[0]), &(ptx_matrix_row_3[1]), &(ptx_matrix_row_3[2]), &(ptx_matrix_row_3[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 3\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 3\n");
      return FALSE;
    }

    if (iptx) {
      // add payload that informs about PTX
      U8* payload = new U8[32 + 240];
      memset(payload, 0, 32 + 240);
      ((F32*)payload)[0] = translate_intensity;
      ((F32*)payload)[1] = scale_intensity;
      strcpy((char*)(payload + 16), (parse_string ? parse_string : "xyz"));
      ((I64*)payload)[4] = (I64)ncols;
      ((I64*)payload)[5] = (I64)nrows;
      ((F64*)payload)[6] = ptx_scan_pos[0];
      ((F64*)payload)[7] = ptx_scan_pos[1];
      ((F64*)payload)[8] = ptx_scan_pos[2];
      ((F64*)payload)[9] = ptx_scan_axis_x[0];
      ((F64*)payload)[10] = ptx_scan_axis_x[1];
      ((F64*)payload)[11] = ptx_scan_axis_x[2];
      ((F64*)payload)[12] = ptx_scan_axis_y[0];
      ((F64*)payload)[13] = ptx_scan_axis_y[1];
      ((F64*)payload)[14] = ptx_scan_axis_y[2];
      ((F64*)payload)[15] = ptx_scan_axis_z[0];
      ((F64*)payload)[16] = ptx_scan_axis_z[1];
      ((F64*)payload)[17] = ptx_scan_axis_z[2];
      ((F64*)payload)[18] = ptx_matrix_row_0[0];
      ((F64*)payload)[19] = ptx_matrix_row_0[1];
      ((F64*)payload)[20] = ptx_matrix_row_0[2];
      ((F64*)payload)[21] = ptx_matrix_row_0[3];
      ((F64*)payload)[22] = ptx_matrix_row_1[0];
      ((F64*)payload)[23] = ptx_matrix_row_1[1];
      ((F64*)payload)[24] = ptx_matrix_row_1[2];
      ((F64*)payload)[25] = ptx_matrix_row_1[3];
      ((F64*)payload)[26] = ptx_matrix_row_2[0];
      ((F64*)payload)[27] = ptx_matrix_row_2[1];
      ((F64*)payload)[28] = ptx_matrix_row_2[2];
      ((F64*)payload)[29] = ptx_matrix_row_2[3];
      ((F64*)payload)[30] = ptx_matrix_row_3[0];
      ((F64*)payload)[31] = ptx_matrix_row_3[1];
      ((F64*)payload)[32] = ptx_matrix_row_3[2];
      ((F64*)payload)[33] = ptx_matrix_row_3[3];
      header.add_vlr("LAStools", 2001, 32 + 240, payload);
    }
    if (iptx_transform)
    {
      transform_matrix.r11 = ptx_matrix_row_0[0];
      transform_matrix.r12 = ptx_matrix_row_1[0];
      transform_matrix.r13 = ptx_matrix_row_2[0];
      transform_matrix.r21 = ptx_matrix_row_0[1];
      transform_matrix.r22 = ptx_matrix_row_1[1];
      transform_matrix.r23 = ptx_matrix_row_2[1];
      transform_matrix.r31 = ptx_matrix_row_0[2];
      transform_matrix.r32 = ptx_matrix_row_1[2];
      transform_matrix.r33 = ptx_matrix_row_2[2];
      transform_matrix.tr1 = ptx_matrix_row_3[0];
      transform_matrix.tr2 = ptx_matrix_row_3[1];
      transform_matrix.tr3 = ptx_matrix_row_3[2];
    }
  }
  else if (!populated_header)
  {
    if (this->point_type > 5)
    {
      header.version_minor = 4;
      header.header_size = 375;
      header.offset_to_point_data = 375;
    }
    else
    {
      header.version_minor = 2;
      header.header_size = 227;
      header.offset_to_point_data = 227;
    }
  }

  // maybe attributes in extra bytes

  if (header.number_attributes)
  {
    header.update_extra_bytes_vlr();
  }

  // read the first line with full parse_string

  i = 0;
  while (fgets(line, 512, file))
  {
    if (parse(this->parse_string))
    {
      // mark that we found the first point
      i = 1;
      break;
    }
    else
    {
      line[strlen(line) - 1] = '\0';
      fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, this->parse_string_unparsed);
    }
  }

  // did we manage to parse a line

  if (i != 1)
  {
    fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", this->parse_string_unparsed);
    fclose(this->file);
    this->file = 0;
    free(this->parse_string);
    this->parse_string = 0;
    return FALSE;
  }

  if (!populated_header)
  {
    // init the bounding box that we will incrementally compute

    header.min_x = header.max_x = point.coordinates[0];
    header.min_y = header.max_y = point.coordinates[1];
    header.min_z = header.max_z = point.coordinates[2];

    // init the min and max of attributes in extra bytes

    if (number_attributes)
    {
      for (i = 0; i < number_attributes; i++)
      {
        header.attributes[i].set_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].set_max(point.extra_bytes + attribute_starts[i]);
      }
    }

    // set scale and offset

    populate_scale_and_offset();
  }

  p_count = 0;

  return TRUE;
}

void LASreaderTXT::set_pts(BOOL pts)
{
  translate_intensity = 2048.0f;
  scale_intensity = 1.0f;
  this->ipts = pts;
}

void LASreaderTXT::set_ptx(BOOL ptx)
{
  translate_intensity = 0.0f;
  scale_intensity = 4095.0f;
  this->iptx = ptx;
}

void LASreaderTXT::set_ptx_transform(BOOL ptx_transform)
{
  translate_intensity = 0.0f;
  scale_intensity = 4095.0f;
  this->iptx_transform = ptx_transform;
}

void LASreaderTXT::set_translate_intensity(F32 translate_intensity)
{
  this->translate_intensity = translate_intensity;
}

void LASreaderTXT::set_scale_intensity(F32 scale_intensity)
{
  this->scale_intensity = scale_intensity;
}

void LASreaderTXT::set_translate_scan_angle(F32 translate_scan_angle)
{
  this->translate_scan_angle = translate_scan_angle;
}

void LASreaderTXT::set_scale_scan_angle(F32 scale_scan_angle)
{
  this->scale_scan_angle = scale_scan_angle;
}

void LASreaderTXT::set_scale_factor(const F64* scale_factor)
{
  if (scale_factor)
  {
    if (this->scale_factor == 0) this->scale_factor = new F64[3];
    this->scale_factor[0] = scale_factor[0];
    this->scale_factor[1] = scale_factor[1];
    this->scale_factor[2] = scale_factor[2];
  }
  else if (this->scale_factor)
  {
    delete[] this->scale_factor;
    this->scale_factor = 0;
  }
}

void LASreaderTXT::set_offset(const F64* offset)
{
  if (offset)
  {
    if (this->offset == 0) this->offset = new F64[3];
    this->offset[0] = offset[0];
    this->offset[1] = offset[1];
    this->offset[2] = offset[2];
  }
  else if (this->offset)
  {
    delete[] this->offset;
    this->offset = 0;
  }
}

void LASreaderTXT::add_attribute(I32 data_type, const char* name, const char* description, F64 scale, F64 offset, F64 pre_scale, F64 pre_offset, F64 no_data)
{
  attributes_data_types[number_attributes] = data_type;
  if (name)
  {
    attribute_names[number_attributes] = LASCopyString(name);
  }
  else
  {
    char temp[32];
    sprintf(temp, "attribute %d", number_attributes);
    attribute_names[number_attributes] = LASCopyString(temp);
  }
  if (description)
  {
    attribute_descriptions[number_attributes] = LASCopyString(description);
  }
  else
  {
    attribute_descriptions[number_attributes] = 0;
  }
  attribute_scales[number_attributes] = scale;
  attribute_offsets[number_attributes] = offset;
  attribute_pre_scales[number_attributes] = pre_scale;
  attribute_pre_offsets[number_attributes] = pre_offset;
  attribute_no_datas[number_attributes] = no_data;
  number_attributes++;
}

BOOL LASreaderTXT::seek(const I64 p_index)
{
  U32 delta = 0;
  if (p_index > p_count)
  {
    delta = (U32)(p_index - p_count);
  }
  else if (p_index < p_count)
  {
    if (piped) return FALSE;
    fseek(file, 0, SEEK_SET);
    // skip lines if we have to
    int i;
    for (i = 0; i < skip_lines; i++) fgets(line, 512, file);
    // read the first line with full parse_string
    i = 0;
    while (fgets(line, 512, file))
    {
      if (parse(this->parse_string))
      {
        // mark that we found the first point
        i = 1;
        break;
      }
      else
      {
        line[strlen(line) - 1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, this->parse_string_unparsed);
      }
    }
    // did we manage to parse a line
    if (i != 1)
    {
      fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", this->parse_string_unparsed);
      fclose(file);
      file = 0;
      free(this->parse_string);
      this->parse_string = 0;
      return FALSE;
    }
    delta = (U32)p_index;
  }
  while (delta)
  {
    read_point_default();
    delta--;
  }
  p_count = p_index;
  return TRUE;
}

BOOL LASreaderTXT::read_point_default()
{
  if (p_count)
  {
    while (true)
    {
      if (fgets(line, 512, file))
      {
        if (parse(parse_string))
        {
          break;
        }
        else
        {
          line[strlen(line) - 1] = '\0';
          fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_string_unparsed);
        }
      }
      else
      {
        if (populated_header)
        {
          if (p_count != npoints)
          {
#ifdef _WIN32
            fprintf(stderr, "WARNING: end-of-file after %I64d of %I64d points\n", p_count, npoints);
#else
            fprintf(stderr, "WARNING: end-of-file after %lld of %lld points\n", p_count, npoints);
#endif
          }
        }
        else
        {
          if (npoints)
          {
            if (p_count != npoints)
            {
#ifdef _WIN32
              fprintf(stderr, "WARNING: end-of-file after %I64d of %I64d points\n", p_count, npoints);
#else
              fprintf(stderr, "WARNING: end-of-file after %lld of %lld points\n", p_count, npoints);
#endif
            }
          }
          npoints = p_count;
          populate_bounding_box();
        }
        return FALSE;
      }
    }
  }
  // compute the quantized x, y, and z values
  point.set_X((I32)header.get_X(point.coordinates[0]));
  point.set_Y((I32)header.get_Y(point.coordinates[1]));
  point.set_Z((I32)header.get_Z(point.coordinates[2]));
  p_count++;
  if (!populated_header)
  {
    // update number of point records
    // create return histogram
    if (point.extended_point_type)
    {
      if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number - 1]++;
    }
    else if (header.version_minor == 4)
    {
      if (point.return_number >= 1 && point.return_number <= 7) header.extended_number_of_points_by_return[point.return_number - 1]++;
    }
    else
    {
      if (point.return_number >= 1 && point.return_number <= 5) header.number_of_points_by_return[point.return_number - 1]++;
    }
    // update bounding box
    if (point.coordinates[0] < header.min_x) header.min_x = point.coordinates[0];
    else if (point.coordinates[0] > header.max_x) header.max_x = point.coordinates[0];
    if (point.coordinates[1] < header.min_y) header.min_y = point.coordinates[1];
    else if (point.coordinates[1] > header.max_y) header.max_y = point.coordinates[1];
    if (point.coordinates[2] < header.min_z) header.min_z = point.coordinates[2];
    else if (point.coordinates[2] > header.max_z) header.max_z = point.coordinates[2];
    // update the min and max of attributes in extra bytes
    if (number_attributes)
    {
      for (I32 i = 0; i < number_attributes; i++)
      {
        header.attributes[i].update_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].update_max(point.extra_bytes + attribute_starts[i]);
      }
    }
  }
  return TRUE;
}

ByteStreamIn* LASreaderTXT::get_stream() const
{
  return 0;
}

void LASreaderTXT::close(BOOL close_stream)
{
  if (file)
  {
    if (piped) while (fgets(line, 512, file));
    fclose(file);
    file = 0;
  }
}

BOOL LASreaderTXT::reopen(const char* file_name)
{
  int i;

  if (file_name == 0)
  {
    fprintf(stderr, "ERROR: file name pointer is zero\n");
    return FALSE;
  }

  file = fopen_compressed(file_name, "r", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot reopen file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 10 * LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10 * LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  // skip lines if we have to

  for (i = 0; i < skip_lines; i++) fgets(line, 512, file);

  // read the first line with full parse_string

  i = 0;
  while (fgets(line, 512, file))
  {
    if (parse(parse_string))
    {
      // mark that we found the first point
      i = 1;
      break;
    }
    else
    {
      line[strlen(line) - 1] = '\0';
      fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_string_unparsed);
    }
  }

  // did we manage to parse a line

  if (i != 1)
  {
    fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", parse_string);
    fclose(file);
    file = 0;
    return FALSE;
  }

  p_count = 0;

  return TRUE;
}

void LASreaderTXT::clean()
{
  if (file)
  {
    fclose(file);
    file = 0;
  }
  if (parse_string)
  {
    free(parse_string);
    parse_string = 0;
  }
  if (parse_string_unparsed)
  {
    free(parse_string_unparsed);
    parse_string_unparsed = 0;
  }
  skip_lines = 0;
  populated_header = FALSE;
}

LASreaderTXT::LASreaderTXT()
{
  file = 0;
  piped = false;
  point_type = 0;
  parse_string = 0;
  parse_string_unparsed = 0;
  scale_factor = 0;
  offset = 0;
  ipts = FALSE;
  iptx = FALSE;
  iptx_transform = FALSE;
  translate_intensity = 0.0f;
  scale_intensity = 1.0f;
  translate_scan_angle = 0.0f;
  scale_scan_angle = 1.0f;
  number_attributes = 0;
  clean();
}

LASreaderTXT::~LASreaderTXT()
{
  clean();
  if (scale_factor)
  {
    delete[] scale_factor;
    scale_factor = 0;
  }
  if (offset)
  {
    delete[] offset;
    offset = 0;
  }
}

BOOL LASreaderTXT::parse_attribute(const char* lptr, I32 index)
{
  if (index >= header.number_attributes)
  {
    return FALSE;
  }
  F64 temp_d;
  if (sscanf(lptr, "%lf", &temp_d) != 1) return FALSE;
  if (attribute_pre_scales[index] != 1.0)
  {
    temp_d *= attribute_pre_scales[index];
  }
  if (attribute_pre_offsets[index] != 0.0)
  {
    temp_d -= attribute_pre_offsets[index];
  }
  if (header.attributes[index].data_type == 1)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U8_MIN || temp_i > U8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U8 is %d. clamped to [%d %d] range.\n", index, temp_i, U8_MIN, U8_MAX);
      point.set_attribute(attribute_starts[index], U8_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (U8)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 2)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I8_MIN || temp_i > I8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I8 is %d. clamped to [%d %d] range.\n", index, temp_i, I8_MIN, I8_MAX);
      point.set_attribute(attribute_starts[index], I8_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (I8)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 3)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U16_MIN || temp_i > U16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U16 is %d. clamped to [%d %d] range.\n", index, temp_i, U16_MIN, U16_MAX);
      point.set_attribute(attribute_starts[index], U16_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (U16)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 4)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I16_MIN || temp_i > I16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I16 is %d. clamped to [%d %d] range.\n", index, temp_i, I16_MIN, I16_MAX);
      point.set_attribute(attribute_starts[index], I16_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (I16)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 5)
  {
    U32 temp_u;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_u = U32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_u = U32_QUANTIZE(temp_d);
    }
    point.set_attribute(attribute_starts[index], temp_u);
  }
  else if (header.attributes[index].data_type == 6)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d / header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    point.set_attribute(attribute_starts[index], temp_i);
  }
  else if (header.attributes[index].data_type == 9)
  {
    F32 temp_f = (F32)temp_d;
    point.set_attribute(attribute_starts[index], temp_f);
  }
  else if (header.attributes[index].data_type == 10)
  {
    point.set_attribute(attribute_starts[index], temp_d);
  }
  else
  {
    fprintf(stderr, "WARNING: attribute %d not (yet) implemented.\n", index);
    return FALSE;
  }
  return TRUE;
}

BOOL LASreaderTXT::parse_extended_flags(CHAR* parse_string)
{
  const char* extended_flags[] = { "(HSV)", "(HSL)", "(hsv)", "(hsl)" };
  int nflags = (int)(sizeof(extended_flags) / sizeof(char*));

  for (int i = 0; i < nflags; i++)
  {
    char* found = strstr(parse_string, extended_flags[i]);

    while (found)
    {
      if (strcmp(extended_flags[i], "(HSV)") == 0)
      {
        found[0] = HSV_H; found[1] = HSV_S; found[2] = HSV_V;
        int len_flag = (int)strlen(extended_flags[i]);
        int len_remaining = (int)strlen(found + len_flag);
        memmove(found + 3, found + 5, len_remaining + 1);
        found = strstr(parse_string, extended_flags[i]);
      }
      else if (strcmp(extended_flags[i], "(HSL)") == 0)
      {
        found[0] = HSL_H; found[1] = HSL_S; found[2] = HSL_L;
        int len_flag = (int)strlen(extended_flags[i]);
        int len_remaining = (int)strlen(found + len_flag);
        memmove(found + 3, found + 5, len_remaining + 1);
        found = strstr(parse_string, extended_flags[i]);
      }
      else if (strcmp(extended_flags[i], "(hsv)") == 0)
      {
        found[0] = HSV_h; found[1] = HSV_s; found[2] = HSV_v;
        int len_flag = (int)strlen(extended_flags[i]);
        int len_remaining = (int)strlen(found + len_flag);
        memmove(found + 3, found + 5, len_remaining + 1);
        found = strstr(parse_string, extended_flags[i]);
      }
      else if (strcmp(extended_flags[i], "(hsl)") == 0)
      {
        found[0] = HSL_h; found[1] = HSL_s; found[2] = HSL_l;
        int len_flag = (int)strlen(extended_flags[i]);
        int len_remaining = (int)strlen(found + len_flag);
        memmove(found + 3, found + 5, len_remaining + 1);
        found = strstr(parse_string, extended_flags[i]);
      }
      else
      {
        found = 0;
      }
    }
  }

  return TRUE;
}

BOOL LASreaderTXT::parse_column_description(CHAR** parse_string)
{
  // Read the first line
  I32 here = ftell(file);
  fgets(line, 512, file);

  // If it contains column description
  if (strstr(line, "x") || strstr(line, "y") || strstr(line, "z") || strstr(line, "X") || strstr(line, "Y") || strstr(line, "Z"))
  {
    const char* delimiters = ",;.:- \t\n";
    char* token = strtok(line, delimiters);
    char* auto_parse_string = (char*)calloc(64, sizeof(char));

    U32 i = 0;
    while (token)
    {
      if (strcmp(token, "x") == 0) auto_parse_string[i] = 'x';
      else if (strcmp(token, "y") == 0) auto_parse_string[i] = 'y';
      else if (strcmp(token, "z") == 0) auto_parse_string[i] = 'z';
      else if (strcmp(token, "X") == 0) auto_parse_string[i] = 'X';
      else if (strcmp(token, "Y") == 0) auto_parse_string[i] = 'Y';
      else if (strcmp(token, "Z") == 0) auto_parse_string[i] = 'Z';
      else if (strcmp(token, "gps_time") == 0) auto_parse_string[i] = 't';
      else if (strcmp(token, "intensity") == 0) auto_parse_string[i] = 'i';
      else if (strcmp(token, "scan_angle") == 0) auto_parse_string[i] = 'a';
      else if (strcmp(token, "point_source_id") == 0) auto_parse_string[i] = 'p';
      else if (strcmp(token, "classification") == 0) auto_parse_string[i] = 'c';
      else if (strcmp(token, "user_data") == 0) auto_parse_string[i] = 'u';
      else if (strcmp(token, "return_number") == 0) auto_parse_string[i] = 'r';
      else if (strcmp(token, "number_of_returns") == 0) auto_parse_string[i] = 'n';
      else if (strcmp(token, "edge_of_flight_line") == 0)	auto_parse_string[i] = 'e';
      else if (strcmp(token, "scan_direction_flag") == 0)	auto_parse_string[i] = 'd';
      else if (strcmp(token, "withheld_flag") == 0) auto_parse_string[i] = 'h';
      else if (strcmp(token, "keypoint_flag") == 0) auto_parse_string[i] = 'k';
      else if (strcmp(token, "synthetic_flag") == 0) auto_parse_string[i] = 'g';
      else if (strcmp(token, "skip") == 0) auto_parse_string[i] = 's';
      else if (strcmp(token, "overlap_flag") == 0) auto_parse_string[i] = 'o';
      else if (strcmp(token, "scanner_channel") == 0) auto_parse_string[i] = 'l';
      else if (strcmp(token, "R") == 0) auto_parse_string[i] = 'R';
      else if (strcmp(token, "G") == 0) auto_parse_string[i] = 'G';
      else if (strcmp(token, "B") == 0) auto_parse_string[i] = 'B';
      else if (strcmp(token, "HSV_H") == 0)
      {
        memcpy(&auto_parse_string[i], "(HSV)", 5); i += 4;
        token = strtok(NULL, delimiters);

        // One flag for 3 columns implies that we assume they are consecutive
        BOOL error = TRUE;
        if (strcmp(token, "HSV_S") == 0)
        {
          token = strtok(NULL, delimiters);
          if (strcmp(token, "HSV_V") == 0) error = FALSE;
        }
        if (error)
        {
          fprintf(stderr, "ERROR: column 'HSV_H' is not followed by 'HSV_S' and 'HSV_V'\n");
          return true;
        }
      }
      else if (strcmp(token, "HSL_h") == 0)
      {
        memcpy(&auto_parse_string[i], "(hsl)", 5); i += 4;
        token = strtok(NULL, delimiters);

        // One flag for 3 columns implies that we assume they are consecutive
        BOOL error = TRUE;
        if (strcmp(token, "HSL_s") == 0)
        {
          token = strtok(NULL, delimiters);
          if (strcmp(token, "HSL_l") == 0) error = FALSE;
        }
        if (error)
        {
          fprintf(stderr, "ERROR: column 'HSL_h' is not followed by 'HSL_s' and 'HSL_l'\n");
          return true;
        }
      }
      else if (strcmp(token, "HSV_h") == 0)
      {
        memcpy(&auto_parse_string[i], "(hsv)", 5); i += 4;
        token = strtok(NULL, delimiters);

        // One flag for 3 columns implies that we assume they are consecutive
        BOOL error = TRUE;
        if (strcmp(token, "HSV_s") == 0)
        {
          token = strtok(NULL, delimiters);
          if (strcmp(token, "HSV_v") == 0) error = FALSE;
        }
        if (error)
        {
          fprintf(stderr, "ERROR: column 'HSV_h' is not followed by 'HSV_s' and 'HSV_v'\n");
          return true;
        }
      }
      else if (strcmp(token, "HSL_H") == 0)
      {
        memcpy(&auto_parse_string[i], "(HSL)", 5); i += 4;
        token = strtok(NULL, delimiters);

        // One flag for 3 columns implies that we assume they are consecutive
        BOOL error = TRUE;
        if (strcmp(token, "HSL_S") == 0)
        {
          token = strtok(NULL, delimiters);
          if (strcmp(token, "HSL_L") == 0) error = FALSE;
        }
        if (error)
        {
          fprintf(stderr, "ERROR: column 'HSL_H' is not followed by 'HSL_S' and 'HSL_L'\n");
          return true;
        }
      }
      else if (strcmp(token, "HSV_S") == 0 || strcmp(token, "HSV_V") == 0)
      {
        // Assuming that HSV columns are consecutive HSV_S should have already been parsed
        fprintf(stderr, "ERROR: column 'HSV_S' or 'HSV_V' is not led by 'HSV_H'\n");
        return true;
      }
      else if (strcmp(token, "HSV_s") == 0 || strcmp(token, "HSV_v") == 0)
      {
        fprintf(stderr, "ERROR: column 'HSV_s' or 'HSV_v' is not led by 'HSV_h'\n");
        return true;
      }
      else if (strcmp(token, "HSL_S") == 0 || strcmp(token, "HSL_L") == 0)
      {
        fprintf(stderr, "ERROR: column 'HSL_S' or 'HSL_L' is not led by 'HSL_H'\n");
        return true;
      }
      else if (strcmp(token, "HSL_s") == 0 || strcmp(token, "HSL_l") == 0)
      {
        fprintf(stderr, "ERROR: column 'HSL_s' or 'HSL_l' is not led by 'HSL_h'\n");
        return true;
      }
      else
      {
        fprintf(stderr, "WARNING: unknown parse item '%s'. skipping ...\n", token);
        auto_parse_string[i] = 's';
      }
      token = strtok(NULL, delimiters);
      i++;
    }

    fprintf(stderr, "Column description detected. Parse string is %s\n", auto_parse_string);

    if (*parse_string) free(*parse_string);
    *parse_string = LASCopyString(auto_parse_string);
    free(auto_parse_string);
    skip_lines++;
    return TRUE;
  }
  else
  {
    fseek(file, here, SEEK_SET);
    return FALSE;
  }
}

// first leading white spaces
BOOL LASreaderTXT::skip_pre() {
  while (lptr[0] && (lptr[0] == ' ' || lptr[0] == ',' || lptr[0] == '\t' || lptr[0] == ';')) lptr++;
  if (lptr[0] == 0) {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

// skip remaining white spaces
void LASreaderTXT::skip_post() {
  while (lptr[0] && lptr[0] != ' ' && lptr[0] != ',' && lptr[0] != '\t' && lptr[0] != ';') lptr++;
}

template<typename T>
BOOL LASreaderTXT::parse_item_i(I32* out, const I32 imin, const I32 imax, const CHAR* context, T addon) {
  I32 temp_i;
  if (!skip_pre()) return FALSE;
  if (sscanf(lptr, "%d", &temp_i) != 1) return FALSE;
  addon();
  if (temp_i < imin || temp_i > imax) fprintf(stderr, "WARNING: %s %d is out of range [%d,%d]\n", context, temp_i, imin, imax);
  *out = (temp_i <= imin) ? imin : ((temp_i >= imax) ? imax : temp_i);
  skip_post();
  return TRUE;
}

BOOL LASreaderTXT::parse_item_i(I32* out, const I32 imin, const I32 imax, const CHAR* context) {
  return parse_item_i(out, imin, imax, context, [&]() {});
}

template<typename T>
BOOL LASreaderTXT::parse_item_f(F32* out, const F32 imin, const F32 imax, const CHAR* context, T addon) {
  F32 temp_f;
  if (!skip_pre()) return FALSE;
  if (lptr[0] == 0) return FALSE;
  if (sscanf(lptr, "%f", &temp_f) != 1) return FALSE;
  addon();
  if (temp_f < imin || temp_f > imax) fprintf(stderr, "WARNING: %s %f is out of range [%f,%f]\n", context, temp_f, imin, imax);
  *out = (temp_f <= imin) ? imin : ((temp_f >= imax) ? imax : temp_f);
  skip_post();
  return TRUE;
}

BOOL LASreaderTXT::parse_item_f(F32* out, const F32 imin, const F32 imax, const CHAR* context) {
  return parse_item_f(out, imin, imax, context, [&]() {});
}

BOOL LASreaderTXT::parse(const char* parse_string)
{
  I32 temp_i;
  F32 temp_f;
  const char* p = parse_string;
  lptr = line;
  // HSL HSV special parsing
  BOOL has_hsl = false;
  BOOL has_hsv = false;
  F32 hsl[3];
  F32 hsv[3];

  while (p[0])
  {
    if (p[0] == 'x') // we expect the x coordinate
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%lf", &(point.coordinates[0])) != 1) return FALSE;
      skip_post();
    }
    else if (p[0] == 'y') // we expect the y coordinate
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%lf", &(point.coordinates[1])) != 1) return FALSE;
      skip_post();
    }
    else if (p[0] == 'z') // we expect the x coordinate
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%lf", &(point.coordinates[2])) != 1) return FALSE;
      skip_post();
    }
    else if (p[0] == 't') // we expect the gps time
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%lf", &(point.gps_time)) != 1) return FALSE;
      skip_post();
    }
    else if (p[0] == 'R') // we expect the red channel of the RGB field
    {
      if (parse_item_i(&temp_i, 0, 0xffff, "RGB red")) {
        point.rgb[0] = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 'G') // we expect the green channel of the RGB field
    {
      if (parse_item_i(&temp_i, 0, 0xffff, "RGB green")) {
        point.rgb[1] = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 'B') // we expect the blue channel of the RGB field
    {
      if (parse_item_i(&temp_i, 0, 0xffff, "RGB blue")) {
        point.rgb[2] = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 'I') // we expect the NIR channel of LAS 1.4 point type 8
    {
      if (parse_item_i(&temp_i, 0, 0xffff, "NIR")) {
        point.rgb[3] = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 's') // we expect a string or a number that we don't care about
    {
      if (!skip_pre()) return FALSE;
      skip_post();
    }
    else if (p[0] == 'i') // we expect the intensity
    {
      if (parse_item_f(&temp_f, 0.0f, 65535.5f, "intensity",
        [&]() {
          if (translate_intensity != 0.0f) temp_f = temp_f + translate_intensity;
          if (scale_intensity != 1.0f) temp_f = temp_f * scale_intensity;
        })) 
      {
        point.set_intensity(U16_QUANTIZE(temp_f));
      } 
      else return FALSE;
    }
    else if (p[0] == 'a') // we expect the scan angle
    {
      if (parse_item_f(&temp_f, -128.0f, 127.0f, "scan angle",
        [&]() {
          if (translate_scan_angle != 0.0f) temp_f = temp_f + translate_scan_angle;
          if (scale_scan_angle != 1.0f) temp_f = temp_f * scale_scan_angle;
        })) 
      {
        point.set_scan_angle(temp_f);
      }
      else return FALSE;
    }
    else if (p[0] == 'n') // we expect the number of returns of given pulse
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%d", &temp_i) != 1) return FALSE;
      if (point_type > 5)
      {
        if (temp_i < 0 || temp_i > 15) fprintf(stderr, "WARNING: number of returns of given pulse %d is out of range of four bits\n", temp_i);
        point.set_extended_number_of_returns(temp_i & 15);
      }
      else
      {
        if (temp_i < 0 || temp_i > 7) fprintf(stderr, "WARNING: number of returns of given pulse %d is out of range of three bits\n", temp_i);
        point.set_number_of_returns(temp_i & 7);
      }
      skip_post();
    }
    else if (p[0] == 'r') // we expect the number of the return
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%d", &temp_i) != 1) return FALSE;
      if (point_type > 5)
      {
        if (temp_i < 0 || temp_i > 15) fprintf(stderr, "WARNING: return number %d is out of range of four bits\n", temp_i);
        point.set_extended_return_number(temp_i & 15);
      }
      else
      {
        if (temp_i < 0 || temp_i > 7) fprintf(stderr, "WARNING: return number %d is out of range of three bits\n", temp_i);
        point.set_return_number(temp_i & 7);
      }
      skip_post();
    }
    else if (p[0] == 'h') // we expect the with<h>eld flag
    {
      if (parse_item_i(&temp_i, 0, 1, "withheld flag")) {
        point.set_withheld_flag(temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'k') // we expect the <k>eypoint flag
    {
      if (parse_item_i(&temp_i, 0, 1, "keypoint flag")) {
        point.set_keypoint_flag(temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'g') // we expect the synthetic fla<g>
    {
      if (parse_item_i(&temp_i, 0, 1, "synthetic flag")) {
        point.set_synthetic_flag(temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'o') // we expect the overlap flag
    {
      if (parse_item_i(&temp_i, 0, 1, "overlap flag")) {
        point.set_extended_overlap_flag(temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'l') // we expect the scanner channel
    {
      if (parse_item_i(&temp_i, 0, 3, "scanner channel")) {
        point.extended_scanner_channel = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 'E') // we expect a terrasolid echo encoding)
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 3) fprintf(stderr, "WARNING: terrasolid echo encoding %d is out of range of 0 to 3\n", temp_i);
      if (temp_i == 0) // only echo
      {
        point.number_of_returns = 1;
        point.return_number = 1;
      }
      else if (temp_i == 1) // first (of many)
      {
        point.number_of_returns = 2;
        point.return_number = 1;
      }
      else if (temp_i == 3) // last (of many)
      {
        point.number_of_returns = 2;
        point.return_number = 2;
      }
      else // intermediate
      {
        point.number_of_returns = 3;
        point.return_number = 2;
      }
      skip_post();
    }
    else if (p[0] == 'c') // we expect the classification
    {
      if (!skip_pre()) return FALSE;
      if (sscanf(lptr, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0)
      {
        fprintf(stderr, "WARNING: classification %d is negative. zeroing ...\n", temp_i);
        point.set_classification(0);
        point.set_extended_classification(0);
      }
      else if (point.extended_point_type)
      {
        if (temp_i > 255)
        {
          fprintf(stderr, "WARNING: extended classification %d is larger than 255. clamping ...\n", temp_i);
          point.set_extended_classification(255);
        }
        else
        {
          point.set_extended_classification((U8)temp_i);
        }
      }
      else
      {
        if (temp_i > 31)
        {
          fprintf(stderr, "WARNING: classification %d is larger than 31. clamping ...\n", temp_i);
          point.set_classification(31);
        }
        else
        {
          point.set_classification((U8)temp_i);
        }
      }
      skip_post();
    }
    else if (p[0] == 'u') // we expect the user data
    {
      if (parse_item_i(&temp_i, 0, 255, "user data")) {
        point.set_user_data((U8)temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'p') // we expect the point source ID
    {
      if (parse_item_i(&temp_i, 0, 0xffff, "point source ID")) {
        point.set_point_source_ID((U16)temp_i);
      }
      else return FALSE;
    }
    else if (p[0] == 'e') // we expect the edge of flight line flag
    {
      if (parse_item_i(&temp_i, 0, 1, "edge of flight line")) {
        point.edge_of_flight_line = temp_i;
      }
      else return FALSE;
    }
    else if (p[0] == 'd') // we expect the direction of scan flag
    {
      if (parse_item_i(&temp_i, 0, 1, "direction of scan")) {
        point.scan_direction_flag = temp_i;
      }
      else return FALSE;
    }
    else if ((p[0] >= '0') && (p[0] <= '9')) // we expect attribute number 0 to 9
    {
      if (!skip_pre()) return FALSE;
      I32 index = (I32)(p[0] - '0');
      if (!parse_attribute(lptr, index)) return FALSE;
      skip_post();
    }
    else if (p[0] == '(') // we expect attribute number 10 or higher
    {
      if (!skip_pre()) return FALSE;
      p++;
      I32 index = 0;
      while (p[0] >= '0' && p[0] <= '9')
      {
        index = 10 * index + (I32)(p[0] - '0');
        p++;
      }
      if (!parse_attribute(lptr, index)) return FALSE;
      skip_post();
    }
    else if (p[0] == 'H') // we expect a hexadecimal coded RGB color
    {
      I32 hex_value;
      char hex_string[3] = "__";
      while (lptr[0] && (lptr[0] == ' ' || lptr[0] == ',' || lptr[0] == '\t' || lptr[0] == ';' || lptr[0] == '\"')) lptr++; // first skip white spaces and quotes
      if (lptr[0] == 0) return FALSE;
      hex_string[0] = lptr[0]; hex_string[1] = lptr[1];
      sscanf(hex_string, "%x", &hex_value);
      point.rgb[0] = hex_value;
      hex_string[0] = lptr[2]; hex_string[1] = lptr[3];
      sscanf(hex_string, "%x", &hex_value);
      point.rgb[1] = hex_value;
      hex_string[0] = lptr[4]; hex_string[1] = lptr[5];
      sscanf(hex_string, "%x", &hex_value);
      point.rgb[2] = hex_value;
      lptr += 6;
      skip_post();
    }
    else if (p[0] == 'J') // we expect a hexadecimal coded intensity
    {
      I32 hex_value;
      while (lptr[0] && (lptr[0] == ' ' || lptr[0] == ',' || lptr[0] == '\t' || lptr[0] == ';' || lptr[0] == '\"')) lptr++; // first skip white spaces and quotes
      if (lptr[0] == 0) return FALSE;
      sscanf(lptr, "%x", &hex_value);
      point.intensity = U8_CLAMP(((F64)hex_value / (F64)0xFFFFFF) * 255);
      lptr += 6;
      skip_post();
    }
    else if (p[0] == HSL_H) // we expect the HSL hue representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 360, "HSL hue")) {
        hsl[0] = (F32)temp_i / 360.0f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSL_S) // we expect the HSL saturation representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 100, "HSL saturation")) {
        hsl[1] = (F32)temp_i / 100.0f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSL_L) // we expect the HSL lightness representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 100, "HSL lightness")) {
        hsl[2] = (F32)temp_i / 100.0f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSL_h) // we expect the HSL hue representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSL hue")) {
        hsl[0] = temp_f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSL_s) // we expect the HSL saturation representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSL saturation")) {
        hsl[1] = temp_f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSL_l) // we expect the HSL lightness representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSL lightness")) {
        hsl[2] = temp_f;
        has_hsl = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_H) // we expect the HSV hue representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 360, "HSV hue")) {
        hsv[0] = temp_i / 360.f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_S) // we expect the HSV saturation representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 100, "HSV saturation")) {
        hsv[1] = temp_i / 100.f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_V) // we expect the HSV value representation of RGB in range [0,255]
    {
      if (parse_item_i(&temp_i, 0, 100, "HSV value")) {
        hsv[2] = temp_i / 100.f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_h) // we expect the HSV hue representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSV hue")) {
        hsv[0] = temp_f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_s) // we expect the HSV saturation representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSV saturation")) {
        hsv[1] = temp_f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else if (p[0] == HSV_v) // we expect the HSV value representation of RGB in range [0,1]
    {
      if (parse_item_f(&temp_f, 0.0, 1.0, "HSV value")) {
        hsv[2] = temp_f;
        has_hsv = true;
      }
      else return FALSE;
    }
    else
    {
      fprintf(stderr, "ERROR: unknown symbol '%c' in parse string\n", p[0]);
    }
    p++;
  }
  if (has_hsl) {
    point.set_RGB_from_HSL(hsl);
  }
  else if (has_hsv) {
    point.set_RGB_from_HSV(hsv);
  }
  return TRUE;
}

BOOL LASreaderTXT::check_parse_string(const char* parse_string)
{
  const char* p = parse_string;
  while (p[0])
  {
    if ((p[0] != 'x') && // we expect the x coordinate
      (p[0] != 'y') && // we expect the y coordinate
      (p[0] != 'z') && // we expect the z coordinate
      (p[0] != 't') && // we expect the gps time
      (p[0] != 'R') && // we expect the red channel of the RGB field
      (p[0] != 'G') && // we expect the green channel of the RGB field
      (p[0] != 'B') && // we expect the blue channel of the RGB field
      (p[0] != 'I') && // we expect the NIR channel
      (p[0] != 's') && // we expect a string or a number that we don't care about
      (p[0] != 'i') && // we expect the intensity
      (p[0] != 'a') && // we expect the scan angle
      (p[0] != 'n') && // we expect the number of returns of given pulse
      (p[0] != 'r') && // we expect the number of the return
      (p[0] != 'h') && // we expect the with<h>eld flag
      (p[0] != 'k') && // we expect the <k>eypoint flag
      (p[0] != 'g') && // we expect the synthetic fla<g>
      (p[0] != 'o') && // we expect the <o>verlap flag
      (p[0] != 'l') && // we expect the scanner channe<l>
      (p[0] != 'E') && // we expect terrasolid echo encoding
      (p[0] != 'c') && // we expect the classification
      (p[0] != 'u') && // we expect the user data
      (p[0] != 'p') && // we expect the point source ID
      (p[0] != 'e') && // we expect the edge of flight line flag
      (p[0] != 'd') && // we expect the direction of scan flag
      (p[0] != 'H') && // we expect hexadecimal coded RGB(I) colors
      (p[0] != 'J') && // we expect a hexadecimal coded intensity
      (p[0] != HSL_H) && // we expect one of the non ascii special flags for HSL and HSV
      (p[0] != HSL_S) &&
      (p[0] != HSL_L) &&
      (p[0] != HSV_H) &&
      (p[0] != HSV_S) &&
      (p[0] != HSV_V) &&
      (p[0] != HSL_h) &&
      (p[0] != HSL_s) &&
      (p[0] != HSL_l) &&
      (p[0] != HSV_h) &&
      (p[0] != HSV_s) &&
      (p[0] != HSV_v))
    {
      if ((p[0] >= '0') && (p[0] <= '9'))
      {
        I32 index = (I32)(p[0] - '0');
        if (index >= header.number_attributes)
        {
          fprintf(stderr, "ERROR: extra bytes attribute '%d' was not described.\n", index);
          return FALSE;
        }
        attribute_starts[index] = header.get_attribute_start(index);
      }
      else if (p[0] == '(')
      {
        p++;
        if ((p[0] >= '0') && (p[0] <= '9'))
        {
          I32 index = 0;
          while ((p[0] >= '0') && (p[0] <= '9'))
          {
            index = 10 * index + (I32)(p[0] - '0');
            p++;
          }
          if (index >= header.number_attributes)
          {
            fprintf(stderr, "ERROR: extra bytes attribute '%d' was not described.\n", index);
            return FALSE;
          }
          if (p[0] != ')')
          {
            fprintf(stderr, "ERROR: extra bytes attribute '%d' misses closing bracket.\n", index);
            return FALSE;
          }
          attribute_starts[index] = header.get_attribute_start(index);
        }
        else
        {
          fprintf(stderr, "ERROR: parse string opening bracket '(' misses extra bytes index.\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: unknown symbol '%c' in parse string. valid are\n", p[0]);
        fprintf(stderr, "       'x' : the <x> coordinate\n");
        fprintf(stderr, "       'y' : the <y> coordinate\n");
        fprintf(stderr, "       'z' : the <z> coordinate\n");
        fprintf(stderr, "       't' : the gps <t>ime\n");
        fprintf(stderr, "       'R' : the <R>ed channel of the RGB field\n");
        fprintf(stderr, "       'G' : the <G>reen channel of the RGB field\n");
        fprintf(stderr, "       'B' : the <B>lue channel of the RGB field\n");
        fprintf(stderr, "       'I' : the N<I>R channel of LAS 1.4 point type 8\n");
        fprintf(stderr, "       's' : <s>kip a string or a number that we don't care about\n");
        fprintf(stderr, "       'i' : the <i>ntensity\n");
        fprintf(stderr, "       'a' : the scan <a>ngle\n");
        fprintf(stderr, "       'n' : the <n>umber of returns of that given pulse\n");
        fprintf(stderr, "       'r' : the number of the <r>eturn\n");
        fprintf(stderr, "       'h' : the with<h>eld flag\n");
        fprintf(stderr, "       'k' : the <k>eypoint flag\n");
        fprintf(stderr, "       'g' : the synthetic fla<g>\n");
        fprintf(stderr, "       'o' : the <o>verlap flag of LAS 1.4 point types 6, 7, 8\n");
        fprintf(stderr, "       'l' : the scanner channe<l> of LAS 1.4 point types 6, 7, 8\n");
        fprintf(stderr, "       'E' : terrasolid <E>hco Encoding\n");
        fprintf(stderr, "       'c' : the <c>lassification\n");
        fprintf(stderr, "       'u' : the <u>ser data\n");
        fprintf(stderr, "       'p' : the <p>oint source ID\n");
        fprintf(stderr, "       'e' : the <e>dge of flight line flag\n");
        fprintf(stderr, "       'd' : the <d>irection of scan flag\n");
        fprintf(stderr, "   '0'-'9' : additional attributes described as extra bytes (0 through 9)\n");
        fprintf(stderr, "    '(13)' : additional attributes described as extra bytes (10 and up)\n");
        fprintf(stderr, "       'H' : a hexadecimal string encoding the RGB color\n");
        fprintf(stderr, "       'J' : a hexadecimal string encoding the intensity\n");
        fprintf(stderr, "   '(hsv)' : the RGB colors in the HSV color model coded in [0, 1]\n");
        fprintf(stderr, "   '(hsl)' : the RGB colors in the hsv color model coded in [0, 1]\n");
        fprintf(stderr, "   '(HSV)' : the RGB colors in the HSV color model coded in [0,360|0,100|0,100]\n");
        fprintf(stderr, "   '(HSL)' : the RGB colors in the hsv color model coded in [0,360|0,100|0,100]\n");
        return FALSE;
      }
    }
    p++;
  }
  return TRUE;
}

void LASreaderTXT::populate_scale_and_offset()
{
  // if not specified in the command line, set a reasonable scale_factor
  if (scale_factor)
  {
    header.x_scale_factor = scale_factor[0];
    header.y_scale_factor = scale_factor[1];
    header.z_scale_factor = scale_factor[2];
  }
  else
  {
    if (-360 < header.min_x && -360 < header.min_y && header.max_x < 360 && header.max_y < 360) // do we have longitude / latitude coordinates
    {
      header.x_scale_factor = 1e-7;
      header.y_scale_factor = 1e-7;
    }
    else // then we assume utm or mercator / lambertian projections
    {
      header.x_scale_factor = 0.01;
      header.y_scale_factor = 0.01;
    }
    header.z_scale_factor = 0.01;
  }

  // if not specified in the command line, set a reasonable offset
  if (offset)
  {
    header.x_offset = offset[0];
    header.y_offset = offset[1];
    header.z_offset = offset[2];
  }
  else
  {
    if (F64_IS_FINITE(header.min_x) && F64_IS_FINITE(header.max_x))
      header.x_offset = ((I64)((header.min_x + header.max_x) / header.x_scale_factor / 20000000)) * 10000000 * header.x_scale_factor;
    else
      header.x_offset = 0;

    if (F64_IS_FINITE(header.min_y) && F64_IS_FINITE(header.max_y))
      header.y_offset = ((I64)((header.min_y + header.max_y) / header.y_scale_factor / 20000000)) * 10000000 * header.y_scale_factor;
    else
      header.y_offset = 0;

    if (F64_IS_FINITE(header.min_z) && F64_IS_FINITE(header.max_z))
      header.z_offset = ((I64)((header.min_z + header.max_z) / header.z_scale_factor / 20000000)) * 10000000 * header.z_scale_factor;
    else
      header.z_offset = 0;
  }
}

void LASreaderTXT::populate_bounding_box()
{
  // compute quantized and then unquantized bounding box

  F64 dequant_min_x = header.get_x((I32)(header.get_X(header.min_x)));
  F64 dequant_max_x = header.get_x((I32)(header.get_X(header.max_x)));
  F64 dequant_min_y = header.get_y((I32)(header.get_Y(header.min_y)));
  F64 dequant_max_y = header.get_y((I32)(header.get_Y(header.max_y)));
  F64 dequant_min_z = header.get_z((I32)(header.get_Z(header.min_z)));
  F64 dequant_max_z = header.get_z((I32)(header.get_Z(header.max_z)));

  // make sure there is not sign flip

  if ((header.min_x > 0) != (dequant_min_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_x from %g to %g.\n", header.min_x, dequant_min_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.min_x = dequant_min_x;
  }
  if ((header.max_x > 0) != (dequant_max_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_x from %g to %g.\n", header.max_x, dequant_max_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.max_x = dequant_max_x;
  }
  if ((header.min_y > 0) != (dequant_min_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_y from %g to %g.\n", header.min_y, dequant_min_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.min_y = dequant_min_y;
  }
  if ((header.max_y > 0) != (dequant_max_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_y from %g to %g.\n", header.max_y, dequant_max_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.max_y = dequant_max_y;
  }
  if ((header.min_z > 0) != (dequant_min_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_z from %g to %g.\n", header.min_z, dequant_min_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.min_z = dequant_min_z;
  }
  if ((header.max_z > 0) != (dequant_max_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_z from %g to %g.\n", header.max_z, dequant_max_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.max_z = dequant_max_z;
  }
}

LASreaderTXTrescale::LASreaderTXTrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderTXT()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderTXTrescale::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
  // do we need to change anything
  if (scale_factor[0] && (header.x_scale_factor != scale_factor[0]))
  {
    header.x_scale_factor = scale_factor[0];
  }
  if (scale_factor[1] && (header.y_scale_factor != scale_factor[1]))
  {
    header.y_scale_factor = scale_factor[1];
  }
  if (scale_factor[2] && (header.z_scale_factor != scale_factor[2]))
  {
    header.z_scale_factor = scale_factor[2];
  }
  return TRUE;
}

LASreaderTXTreoffset::LASreaderTXTreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderTXT()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderTXTreoffset::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
  // do we need to change anything
  if (header.x_offset != offset[0])
  {
    header.x_offset = offset[0];
  }
  if (header.y_offset != offset[1])
  {
    header.y_offset = offset[1];
  }
  if (header.z_offset != offset[2])
  {
    header.z_offset = offset[2];
  }
  return TRUE;
}

LASreaderTXTrescalereoffset::LASreaderTXTrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderTXTrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderTXTreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderTXTrescalereoffset::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
  // do we need to change anything
  if (scale_factor[0] && (header.x_scale_factor != scale_factor[0]))
  {
    header.x_scale_factor = scale_factor[0];
  }
  if (scale_factor[1] && (header.y_scale_factor != scale_factor[1]))
  {
    header.y_scale_factor = scale_factor[1];
  }
  if (scale_factor[2] && (header.z_scale_factor != scale_factor[2]))
  {
    header.z_scale_factor = scale_factor[2];
  }
  if (header.x_offset != offset[0])
  {
    header.x_offset = offset[0];
  }
  if (header.y_offset != offset[1])
  {
    header.y_offset = offset[1];
  }
  if (header.z_offset != offset[2])
  {
    header.z_offset = offset[2];
  }
  return TRUE;
}
