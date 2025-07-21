/*
===============================================================================

  FILE:  lasdiff.cpp

  CONTENTS:

    This tool reads two LIDAR files in LAS format and checks whether they are
    identical. Both the standard header, the variable header, and all points
    are checked.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-17, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    4 November 2019 -- new option '-idir' takes two input directories and compares
    7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
    13 July 2017 -- added missing checks for LAS 1.4 EVLR size and payloads
    18 August 2015 -- fixed report for truncated files (fewer or more points)
    11 September 2014 -- added missing checks for LAS 1.4 files and points
    27 July 2011 -- added capability to create a difference output file
    2 December 2010 -- updated to merely warn when the point scaling is off
    12 March 2009 -- updated to ask for input if started without arguments
    17 September 2008 -- updated to deal with LAS format version 1.2
    11 July 2007 -- added more complete reporting about differences
    23 February 2007 -- created just before getting ready for the cabin trip

===============================================================================
*/

#include "lasreader.hpp"
#include "laswriter.hpp"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lastool.hpp"

class LasTool_lasdiff : public LasTool
{
private:
public:
  void usage() override
  {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "lasdiff lidar1.las lidar1.laz\n");
    fprintf(stderr, "lasdiff *.las\n");
    fprintf(stderr, "lasdiff lidar1.las lidar2.las -shutup 20\n");
    fprintf(stderr, "lasdiff lidar1.txt lidar2.txt -iparse xyzti\n");
    fprintf(stderr, "lasdiff lidar1.las lidar1.laz\n");
    fprintf(stderr, "lasdiff lidar1.las lidar1.laz -random_seeks\n");
    fprintf(stderr, "lasdiff -i lidar1.las -i lidar2.las -o diff.las\n");
    fprintf(stderr, "lasdiff -wildcards folder1 folder2\n");
    fprintf(stderr, "lasdiff -h\n");
  };
};

static int lidardouble2string(char* string, double value0, double value1)
{
  int len;
  len = sprintf(string, "%.9f", value0) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  len += sprintf(&(string[len]), " %.9f", value1) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
  return len;
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

static bool scaled_offset_difference;

static int check_header(LASreader* lasreader1, LASreader* lasreader2)
{
  int i,j;
  int different_header = 0;

  scaled_offset_difference = false;

  int memcmp_until = (int)(((const char*)&(lasreader1->header.user_data_in_header_size))-((const char*)&(lasreader1->header)));

  if (memcmp((const void*)&(lasreader1->header), (const void*)&(lasreader2->header), memcmp_until))
  {
    char printstring[128];

    LASheader* lasheader1 = &(lasreader1->header);
    LASheader* lasheader2 = &(lasreader2->header);

    bool fatal_difference = false;

    if (strncmp(lasheader1->file_signature, lasheader2->file_signature, 4))
    {
      fprintf(stderr, "  different file_signature: '%4s' '%4s'\n", lasheader1->file_signature, lasheader2->file_signature);
      different_header++;
    }
    if (lasheader1->file_source_ID != lasheader2->file_source_ID)
    {
      fprintf(stderr, "  different file_source_ID: %d %d\n", lasheader1->file_source_ID, lasheader2->file_source_ID);
      different_header++;
    }
    if (lasheader1->global_encoding != lasheader2->global_encoding)
    {
      fprintf(stderr, "  different reserved (global_encoding): %d %d\n", lasheader1->global_encoding, lasheader2->global_encoding);
      different_header++;
    }
    if (lasheader1->get_GUID() != lasheader2->get_GUID())
    {
      fprintf(stderr, "  different project_ID_GUID: %s %s\n", lasheader1->get_GUID().c_str(), lasheader2->get_GUID().c_str());
      different_header++;
    }
    if (lasheader1->version_major != lasheader2->version_major || lasheader1->version_minor != lasheader2->version_minor)
    {
      fprintf(stderr, "  different version: %d.%d %d.%d\n", lasheader1->version_major, lasheader1->version_minor, lasheader2->version_major, lasheader2->version_minor);
      different_header++;
    }
    if (strncmp(lasheader1->system_identifier, lasheader2->system_identifier, 32))
    {
      fprintf(stderr, "  different system_identifier: '%.32s' '%.32s'\n", lasheader1->system_identifier, lasheader2->system_identifier);
      different_header++;
    }
    if (strncmp(lasheader1->generating_software, lasheader2->generating_software, 32))
    {
      fprintf(stderr, "  different generating_software: '%.32s' '%.32s'\n", lasheader1->generating_software, lasheader2->generating_software);
      different_header++;
    }
    if (lasheader1->file_creation_day != lasheader2->file_creation_day || lasheader1->file_creation_year != lasheader2->file_creation_year)
    {
      fprintf(stderr, "  different file_creation day.year: %d.%d %d.%d\n", lasheader1->file_creation_day, lasheader1->file_creation_year, lasheader2->file_creation_day, lasheader2->file_creation_year);
      different_header++;
    }
    if (lasheader1->header_size != lasheader2->header_size)
    {
      fprintf(stderr, "  different header_size: %d %d\n", lasheader1->header_size, lasheader2->header_size);
      fatal_difference = true;
    }
    if (lasheader1->offset_to_point_data != lasheader2->offset_to_point_data)
    {
      fprintf(stderr, "  different offset_to_point_data: %d %d\n", lasheader1->offset_to_point_data, lasheader2->offset_to_point_data);
      different_header++;
    }
    if (lasheader1->number_of_variable_length_records != lasheader2->number_of_variable_length_records)
    {
      fprintf(stderr, "  different number_of_variable_length_records: %d %d\n", lasheader1->number_of_variable_length_records, lasheader2->number_of_variable_length_records);
      different_header++;
    }
    if (lasheader1->point_data_format != lasheader2->point_data_format)
    {
      fprintf(stderr, "  different point_data_format: %d %d\n", lasheader1->point_data_format, lasheader2->point_data_format);
      different_header++;
    }
    if (lasheader1->point_data_record_length != lasheader2->point_data_record_length)
    {
      fprintf(stderr, "  different point_data_record_length: %d %d\n", lasheader1->point_data_record_length, lasheader2->point_data_record_length);
      different_header++;
    }
    if (lasheader1->number_of_point_records != lasheader2->number_of_point_records)
    {
      fprintf(stderr, "  different number_of_point_records: %d %d\n", lasheader1->number_of_point_records, lasheader2->number_of_point_records);
      different_header++;
    }
    for (i = 0; i < 5; i++)
    {
      if (lasheader1->number_of_points_by_return[i] != lasheader2->number_of_points_by_return[i])
      {
        fprintf(stderr, "  different number_of_points_by_return[%d]: %u %u\n", i, lasheader1->number_of_points_by_return[i], lasheader2->number_of_points_by_return[i]);
        different_header++;
      }
    }
    if (lasheader1->x_scale_factor != lasheader2->x_scale_factor)
    {
      lidardouble2string(printstring, lasheader1->x_scale_factor, lasheader2->x_scale_factor); fprintf(stderr, "  WARNING: different x_scale_factor: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->y_scale_factor != lasheader2->y_scale_factor)
    {
      lidardouble2string(printstring, lasheader1->y_scale_factor, lasheader2->y_scale_factor); fprintf(stderr, "  WARNING: different y_scale_factor: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->z_scale_factor != lasheader2->z_scale_factor)
    {
      lidardouble2string(printstring, lasheader1->z_scale_factor, lasheader2->z_scale_factor); fprintf(stderr, "  WARNING: different z_scale_factor: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->x_offset != lasheader2->x_offset)
    {
      lidardouble2string(printstring, lasheader1->x_offset, lasheader2->x_offset); fprintf(stderr, "  WARNING: different x_offset: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->y_offset != lasheader2->y_offset)
    {
      lidardouble2string(printstring, lasheader1->y_offset, lasheader2->y_offset); fprintf(stderr, "  WARNING: different y_offset: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->z_offset != lasheader2->z_offset)
    {
      lidardouble2string(printstring, lasheader1->z_offset, lasheader2->z_offset); fprintf(stderr, "  WARNING: different z_offset: %s\n", printstring);
      scaled_offset_difference = true;
      different_header++;
    }
    if (lasheader1->max_x != lasheader2->max_x)
    {
      lidardouble2string(printstring, lasheader1->max_x, lasheader2->max_x); fprintf(stderr, "  different max_x: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->min_x != lasheader2->min_x)
    {
      lidardouble2string(printstring, lasheader1->min_x, lasheader2->min_x); fprintf(stderr, "  different min_x: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->max_y != lasheader2->max_y)
    {
      lidardouble2string(printstring, lasheader1->max_y, lasheader2->max_y); fprintf(stderr, "  different max_y: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->min_y != lasheader2->min_y)
    {
      lidardouble2string(printstring, lasheader1->min_y, lasheader2->min_y); fprintf(stderr, "  different min_y: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->max_z != lasheader2->max_z)
    {
      lidardouble2string(printstring, lasheader1->max_z, lasheader2->max_z); fprintf(stderr, "  different max_z: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->min_z != lasheader2->min_z)
    {
      lidardouble2string(printstring, lasheader1->min_z, lasheader2->min_z); fprintf(stderr, "  different min_z: %s\n", printstring);
      different_header++;
    }
    if (lasheader1->start_of_waveform_data_packet_record != lasheader2->start_of_waveform_data_packet_record)
    {
      fprintf(stderr, "  different start_of_waveform_data_packet_record: %u %u\n", (U32)lasheader1->start_of_waveform_data_packet_record, (U32)lasheader2->start_of_waveform_data_packet_record);
      different_header++;
    }
    if (lasheader1->start_of_first_extended_variable_length_record != lasheader2->start_of_first_extended_variable_length_record)
    {
      fprintf(stderr, "  different start_of_first_extended_variable_length_record: %u %u\n", (U32)lasheader1->start_of_first_extended_variable_length_record, (U32)lasheader2->start_of_first_extended_variable_length_record);
      different_header++;
    }
    if (lasheader1->number_of_extended_variable_length_records != lasheader2->number_of_extended_variable_length_records)
    {
      fprintf(stderr, "  different number_of_extended_variable_length_records: %d %d\n", lasheader1->number_of_extended_variable_length_records, lasheader2->number_of_extended_variable_length_records);
      different_header++;
    }
    if (lasheader1->extended_number_of_point_records != lasheader2->extended_number_of_point_records)
    {
      fprintf(stderr, "  different extended_number_of_point_records: %u %u\n", (U32)lasheader1->extended_number_of_point_records, (U32)lasheader2->extended_number_of_point_records);
      different_header++;
    }
    for (i = 0; i < 15; i++)
    {
      if (lasheader1->extended_number_of_points_by_return[i] != lasheader2->extended_number_of_points_by_return[i])
      {
        fprintf(stderr, "  different extended_number_of_points_by_return[%d]: %u %u\n", i, (U32)lasheader1->extended_number_of_points_by_return[i], (U32)lasheader2->extended_number_of_points_by_return[i]);
        different_header++;
      }
    }
    if (fatal_difference)
    {
      fprintf(stderr, "difference was fatal ... no need to check points\n");
      byebye();
    }
  }

  // check user-defined data in header

  if (lasreader1->header.user_data_in_header_size == lasreader2->header.user_data_in_header_size)
  {
    for (i = 0; i < (I32)lasreader1->header.user_data_in_header_size; i++)
    {
      if (lasreader1->header.user_data_in_header[i] != lasreader2->header.user_data_in_header[i])
      {
        different_header++;
        fprintf(stderr, "user-defined data in header is different at byte %d of %d\n", i, lasreader1->header.user_data_in_header_size);
        break;
      }
    }
  }
  else
  {
    different_header++;
    fprintf(stderr, "skipping check of user-defined data in header due to length difference (%d != %d)\n", lasreader1->header.user_data_in_header_size, lasreader2->header.user_data_in_header_size);
  }

  // check variable length headers

  if (lasreader1->header.number_of_variable_length_records == lasreader2->header.number_of_variable_length_records)
  {
    for (i = 0; i < (int)lasreader1->header.number_of_variable_length_records; i++)
    {
      if (lasreader1->header.vlrs[i].reserved != lasreader2->header.vlrs[i].reserved)
      {
        different_header++;
        fprintf(stderr, "variable length record %d reserved field is different: %d %d\n", i, lasreader1->header.vlrs[i].reserved, lasreader2->header.vlrs[i].reserved);
      }
      if (memcmp(lasreader1->header.vlrs[i].user_id, lasreader2->header.vlrs[i].user_id, 16) != 0)
      {
        different_header++;
        fprintf(stderr, "variable length record %d user_id field is different: '%s' '%s'\n", i, lasreader1->header.vlrs[i].user_id, lasreader2->header.vlrs[i].user_id);
      }
      if (lasreader1->header.vlrs[i].record_id != lasreader2->header.vlrs[i].record_id)
      {
        different_header++;
        fprintf(stderr, "variable length record %d record_id field is different: %d %d\n", i, lasreader1->header.vlrs[i].record_id, lasreader2->header.vlrs[i].record_id);
      }
      if (lasreader1->header.vlrs[i].record_length_after_header != lasreader2->header.vlrs[i].record_length_after_header)
      {
        different_header++;
        fprintf(stderr, "variable length record %d record_length_after_header field is different: %d %d\n", i, lasreader1->header.vlrs[i].record_length_after_header, lasreader2->header.vlrs[i].record_length_after_header);
      }
      if (memcmp(lasreader1->header.vlrs[i].description, lasreader2->header.vlrs[i].description, 32) != 0)
      {
        different_header++;
        fprintf(stderr, "variable length record %d description field is different: '%s' '%s'\n", i, lasreader1->header.vlrs[i].description, lasreader2->header.vlrs[i].description);
      }
      if (memcmp(lasreader1->header.vlrs[i].data, lasreader2->header.vlrs[i].data, lasreader1->header.vlrs[i].record_length_after_header))
      {
        for (j = 0; j < lasreader1->header.vlrs[i].record_length_after_header; j++)
        {
          if (lasreader1->header.vlrs[i].data[j] != lasreader2->header.vlrs[i].data[j])
          {
            different_header++;
            fprintf(stderr, "variable length record %d data field is different at byte %d: %d %d\n", i, j, lasreader1->header.vlrs[i].data[j], lasreader2->header.vlrs[i].data[j]);
            break;
          }
        }
      }
    }
  }
  else
  {
    fprintf(stderr, "skipping check of variable length records due to different number (%d != %d)\n", lasreader1->header.number_of_variable_length_records, lasreader2->header.number_of_variable_length_records);
  }

  // check user-defined data after header

  if (lasreader1->header.user_data_after_header_size == lasreader2->header.user_data_after_header_size)
  {
    for (i = 0; i < (I32)lasreader1->header.user_data_after_header_size; i++)
    {
      if (lasreader1->header.user_data_after_header[i] != lasreader2->header.user_data_after_header[i])
      {
        different_header++;
        fprintf(stderr, "user-defined data after header is different at byte %d of %d\n", i, lasreader1->header.user_data_in_header_size);
        break;
      }
    }
  }
  else
  {
    different_header++;
    fprintf(stderr, "skipping check of user-defined data in header due to length difference (%d != %d)\n", lasreader1->header.user_data_after_header_size, lasreader2->header.user_data_after_header_size);
  }

  // check extended variable length headers

  if (lasreader1->header.number_of_extended_variable_length_records == lasreader2->header.number_of_extended_variable_length_records)
  {
    for (i = 0; i < (int)lasreader1->header.number_of_extended_variable_length_records; i++)
    {
      if (lasreader1->header.evlrs[i].reserved != lasreader2->header.evlrs[i].reserved)
      {
        different_header++;
        fprintf(stderr, "extended variable length record %d reserved field is different: %d %d\n", i, lasreader1->header.evlrs[i].reserved, lasreader2->header.evlrs[i].reserved);
      }
      if (memcmp(lasreader1->header.evlrs[i].user_id, lasreader2->header.evlrs[i].user_id, 16) != 0)
      {
        different_header++;
        fprintf(stderr, "extended variable length record %d user_id field is different: '%s' '%s'\n", i, lasreader1->header.evlrs[i].user_id, lasreader2->header.evlrs[i].user_id);
      }
      if (lasreader1->header.evlrs[i].record_id != lasreader2->header.evlrs[i].record_id)
      {
        different_header++;
        fprintf(stderr, "extended variable length record %d record_id field is different: %d %d\n", i, lasreader1->header.evlrs[i].record_id, lasreader2->header.evlrs[i].record_id);
      }
      if (lasreader1->header.evlrs[i].record_length_after_header != lasreader2->header.evlrs[i].record_length_after_header)
      {
        different_header++;
        fprintf(stderr, "extended variable length record %d record_length_after_header field is different: %lld %lld\n", i, lasreader1->header.evlrs[i].record_length_after_header, lasreader2->header.evlrs[i].record_length_after_header);
      }
      if (memcmp(lasreader1->header.evlrs[i].description, lasreader2->header.evlrs[i].description, 32) != 0)
      {
        different_header++;
        fprintf(stderr, "extended variable length record %d description field is different: '%s' '%s'\n", i, lasreader1->header.evlrs[i].description, lasreader2->header.evlrs[i].description);
      }
      if (lasreader1->header.evlrs[i].record_length_after_header == lasreader2->header.evlrs[i].record_length_after_header)
      {
        if (memcmp(lasreader1->header.vlrs[i].data, lasreader2->header.vlrs[i].data, lasreader1->header.vlrs[i].record_length_after_header))
        {
          for (j = 0; j < lasreader1->header.vlrs[i].record_length_after_header; j++)
          {
            if (lasreader1->header.vlrs[i].data[j] != lasreader2->header.vlrs[i].data[j])
            {
              different_header++;
              fprintf(stderr, "extended variable length record %d data field is different at byte %d: %d %d\n", i, j, lasreader1->header.vlrs[i].data[j], lasreader2->header.vlrs[i].data[j]);
              break;
            }
          }
        }
      }
      else
      {
        fprintf(stderr, "skipping check of extended variable length record %d due to different size (%lld != %lld)\n", i, lasreader1->header.evlrs[i].record_length_after_header, lasreader2->header.evlrs[i].record_length_after_header);
      }
    }
  }
  else
  {
    fprintf(stderr, "skipping check of extended variable length records due to different number (%d != %d)\n", lasreader1->header.number_of_variable_length_records, lasreader2->header.number_of_variable_length_records);
  }

  if (different_header)
    fprintf(stderr, "headers have %d difference%s.\n", different_header, (different_header > 1 ? "s" : ""));
  else
    fprintf(stderr, "headers are identical.\n");

  return different_header;
};

static int shutup = 5;
static int different_scaled_offset_coordinates;

static int check_points(const CHAR* file_name1, LASreader* lasreader1, const CHAR* file_name2, LASreader* lasreader2, LASwriter* laswriter, I32 random_seeks)
{
  int seeking = random_seeks;

  int different_points = 0;
  double diff;
  double max_diff_x = 0.0;
  double max_diff_y = 0.0;
  double max_diff_z = 0.0;

  different_scaled_offset_coordinates = 0;

  while (true)
  {
    bool difference = false;
    if (seeking)
    {
      if (lasreader1->p_idx%100000 == 25000)
      {
        I64 s = (rand()*rand())%lasreader1->npoints;
        fprintf(stderr, "at p_idx %u seeking to %u\n", (U32)lasreader1->p_idx, (U32)s);
        lasreader1->seek(s);
        lasreader2->seek(s);
        seeking--;
      }
    }
    if (lasreader1->read_point())
    {
      if (lasreader2->read_point())
      {
        if (memcmp((const void*)&(lasreader1->point), (const void*)&(lasreader2->point), 20))
        {
          if (scaled_offset_difference)
          {
            if (lasreader1->get_x() != lasreader2->get_x())
            {
              diff = lasreader1->get_x() - lasreader2->get_x();
              if (diff < 0) diff = -diff;
              if (diff > max_diff_x) max_diff_x = diff;
              if (different_scaled_offset_coordinates < 9) fprintf(stderr, "  x: %d %d scaled offset x %g %g\n", lasreader1->point.get_X(), lasreader2->point.get_X(), lasreader1->get_x(), lasreader2->get_x());
              different_scaled_offset_coordinates++;
            }
            if (lasreader1->get_y() != lasreader2->get_y())
            {
              diff = lasreader1->get_y() - lasreader2->get_y();
              if (diff < 0) diff = -diff;
              if (diff > max_diff_y) max_diff_y = diff;
              if (different_scaled_offset_coordinates < 9) fprintf(stderr, "  y: %d %d scaled offset y %g %g\n", lasreader1->point.get_Y(), lasreader2->point.get_Y(), lasreader1->get_y(), lasreader2->get_y());
              different_scaled_offset_coordinates++;
            }
            if (lasreader1->get_z() != lasreader2->get_z())
            {
              diff = lasreader1->get_z() - lasreader2->get_z();
              if (diff < 0) diff = -diff;
              if (diff > max_diff_z)
              {
                max_diff_z = diff;
                if (max_diff_z > 0.001)
                {
                  max_diff_z = diff;
                }
              }
              if (different_scaled_offset_coordinates < 9) fprintf(stderr, "  z: %d %d scaled offset z %g %g\n", lasreader1->point.get_Z(), lasreader2->point.get_Z(), lasreader1->get_z(), lasreader2->get_z());
              different_scaled_offset_coordinates++;
            }
          }
          else
          {
            if (lasreader1->point.get_X() != lasreader2->point.get_X())
            {
              if (different_points < shutup) fprintf(stderr, "  x: %d %d\n", lasreader1->point.get_X(), lasreader2->point.get_X());
              difference = true;
            }
            if (lasreader1->point.get_Y() != lasreader2->point.get_Y())
            {
              if (different_points < shutup) fprintf(stderr, "  y: %d %d\n", lasreader1->point.get_Y(), lasreader2->point.get_Y());
              difference = true;
            }
            if (lasreader1->point.get_Z() != lasreader2->point.get_Z())
            {
              if (different_points < shutup) fprintf(stderr, "  z: %d %d\n", lasreader1->point.get_Z(), lasreader2->point.get_Z());
              difference = true;
            }
          }
          if (lasreader1->point.intensity != lasreader2->point.intensity)
          {
            if (different_points < shutup) fprintf(stderr, "  intensity: %d %d\n", lasreader1->point.intensity, lasreader2->point.intensity);
            difference = true;
          }
          if (lasreader1->point.return_number != lasreader2->point.return_number)
          {
            if (different_points < shutup) fprintf(stderr, "  return_number: %d %d\n", lasreader1->point.return_number, lasreader2->point.return_number);
            difference = true;
          }
          if (lasreader1->point.number_of_returns != lasreader2->point.number_of_returns)
          {
            if (different_points < shutup) fprintf(stderr, "  number_of_returns: %d %d\n", lasreader1->point.number_of_returns, lasreader2->point.number_of_returns);
            difference = true;
          }
          if (lasreader1->point.scan_direction_flag != lasreader2->point.scan_direction_flag)
          {
            if (different_points < shutup) fprintf(stderr, "  scan_direction_flag: %d %d\n", lasreader1->point.scan_direction_flag, lasreader2->point.scan_direction_flag);
            difference = true;
          }
          if (lasreader1->point.edge_of_flight_line != lasreader2->point.edge_of_flight_line)
          {
            if (different_points < shutup) fprintf(stderr, "  edge_of_flight_line: %d %d\n", lasreader1->point.edge_of_flight_line, lasreader2->point.edge_of_flight_line);
            difference = true;
          }
          if (lasreader1->point.get_classification() != lasreader2->point.get_classification())
          {
            if (different_points < shutup) fprintf(stderr, "  classification: %d %d\n", lasreader1->point.get_classification(), lasreader2->point.get_classification());
            difference = true;
          }
          if (lasreader1->point.get_synthetic_flag() != lasreader2->point.get_synthetic_flag())
          {
            if (different_points < shutup) fprintf(stderr, "  synthetic_flag: %d %d\n", lasreader1->point.get_synthetic_flag(), lasreader2->point.get_synthetic_flag());
            difference = true;
          }
          if (lasreader1->point.get_keypoint_flag() != lasreader2->point.get_keypoint_flag())
          {
            if (different_points < shutup) fprintf(stderr, "  keypoint_flag: %d %d\n", lasreader1->point.get_keypoint_flag(), lasreader2->point.get_keypoint_flag());
            difference = true;
          }
          if (lasreader1->point.get_withheld_flag() != lasreader2->point.get_withheld_flag())
          {
            if (different_points < shutup) fprintf(stderr, "  withheld_flag: %d %d\n", lasreader1->point.get_withheld_flag(), lasreader2->point.get_withheld_flag());
            difference = true;
          }
          if (lasreader1->point.get_scan_angle() != lasreader2->point.get_scan_angle())
          {
            if (different_points < shutup)
              fprintf(stderr, "  scan_angle_rank: %s %s\n", lasreader1->point.get_scan_angle_string().c_str(), lasreader2->point.get_scan_angle_string().c_str());
            difference = true;
          }
          if (lasreader1->point.user_data != lasreader2->point.user_data)
          {
            if (different_points < shutup) fprintf(stderr, "  user_data: %d %d\n", lasreader1->point.user_data, lasreader2->point.user_data);
            difference = true;
          }
          if (lasreader1->point.point_source_ID != lasreader2->point.point_source_ID)
          {
            if (different_points < shutup) fprintf(stderr, "  point_source_ID: %d %d\n", lasreader1->point.point_source_ID, lasreader2->point.point_source_ID);
            difference = true;
          }
          if (difference) if (different_points < shutup) fprintf(stderr, "point %u of %u is different\n", (U32)lasreader1->p_idx, (U32)lasreader1->npoints);
        }
        if (lasreader1->point.have_gps_time || lasreader2->point.have_gps_time)
        {
          if (lasreader1->point.gps_time != lasreader2->point.gps_time)
          {
            if (different_points < shutup) fprintf(stderr, "gps time of point %u of %u is different: %f != %f\n", (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.gps_time, lasreader2->point.gps_time);
            difference = true;
          }
        }
        if (lasreader1->point.have_rgb || lasreader2->point.have_rgb)
        {
          if (lasreader1->point.have_nir || lasreader2->point.have_nir)
          {
            if (memcmp((const void*)&(lasreader1->point.rgb), (const void*)&(lasreader2->point.rgb), sizeof(short[4])))
            {
              if (different_points < shutup) fprintf(stderr, "RGBI of point %u of %u is different: (%d %d %d %d) != (%d %d %d %d)\n", (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.rgb[0], lasreader1->point.rgb[1], lasreader1->point.rgb[2], lasreader1->point.rgb[3], lasreader2->point.rgb[0], lasreader2->point.rgb[1], lasreader2->point.rgb[2], lasreader2->point.rgb[3]);
              difference = true;
            }
          }
          else
          {
            if (memcmp((const void*)&(lasreader1->point.rgb), (const void*)&(lasreader2->point.rgb), sizeof(short[3])))
            {
              if (different_points < shutup) fprintf(stderr, "RGB of point %u of %u is different: (%d %d %d) != (%d %d %d)\n", (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.rgb[0], lasreader1->point.rgb[1], lasreader1->point.rgb[2], lasreader2->point.rgb[0], lasreader2->point.rgb[1], lasreader2->point.rgb[2]);
              difference = true;
            }
          }
        }
        if (lasreader1->point.have_wavepacket || lasreader2->point.have_wavepacket)
        {
          if (memcmp((const void*)&(lasreader1->point.wavepacket), (const void*)&(lasreader2->point.wavepacket), sizeof(LASwavepacket)))
          {
            if (different_points < shutup) fprintf(stderr, "wavepacket of point %u of %u is different: (%d %d %d %g %g %g %g) != (%d %d %d %g %g %g %g)\n", (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.wavepacket.getIndex(), (I32)lasreader1->point.wavepacket.getOffset(), lasreader1->point.wavepacket.getSize(), lasreader1->point.wavepacket.getLocation(), lasreader1->point.wavepacket.getXt(), lasreader1->point.wavepacket.getYt(), lasreader1->point.wavepacket.getZt(), lasreader2->point.wavepacket.getIndex(), (I32)lasreader2->point.wavepacket.getOffset(), lasreader2->point.wavepacket.getSize(), lasreader2->point.wavepacket.getLocation(), lasreader2->point.wavepacket.getXt(), lasreader2->point.wavepacket.getYt(), lasreader2->point.wavepacket.getZt());
            difference = true;
          }
        }
        if (lasreader1->point.extra_bytes_number)
        {
          if (memcmp((const void*)lasreader1->point.extra_bytes, (const void*)lasreader2->point.extra_bytes, lasreader1->point.extra_bytes_number))
          {
            if (different_points < shutup)
            {
              if (lasreader1->point.extra_bytes_number == 1)
              {
                fprintf(stderr, "%d extra_byte of point %u of %u are different: %d != %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader2->point.extra_bytes[0]);
              }
              else if (lasreader1->point.extra_bytes_number == 2)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d != %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1]);
              }
              else if (lasreader1->point.extra_bytes_number == 3)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d != %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2]);
              }
              else if (lasreader1->point.extra_bytes_number == 4)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d != %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3]);
              }
              else if (lasreader1->point.extra_bytes_number == 5)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d != %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4]);
              }
              else if (lasreader1->point.extra_bytes_number == 6)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d != %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5]);
              }
              else if (lasreader1->point.extra_bytes_number == 7)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d != %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6]);
              }
              else if (lasreader1->point.extra_bytes_number == 8)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7]);
              }
              else if (lasreader1->point.extra_bytes_number == 9)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8]);
              }
              else if (lasreader1->point.extra_bytes_number == 10)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9]);
              }
              else if (lasreader1->point.extra_bytes_number == 11)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10]);
              }
              else if (lasreader1->point.extra_bytes_number == 12)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11]);
              }
              else if (lasreader1->point.extra_bytes_number == 13)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12]);
              }
              else if (lasreader1->point.extra_bytes_number == 14)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader1->point.extra_bytes[13], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12], lasreader2->point.extra_bytes[13]);
              }
              else if (lasreader1->point.extra_bytes_number == 15)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader1->point.extra_bytes[13], lasreader1->point.extra_bytes[14], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12], lasreader2->point.extra_bytes[13], lasreader2->point.extra_bytes[14]);
              }
              else if (lasreader1->point.extra_bytes_number == 16)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader1->point.extra_bytes[13], lasreader1->point.extra_bytes[14], lasreader1->point.extra_bytes[15], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12], lasreader2->point.extra_bytes[13], lasreader2->point.extra_bytes[14], lasreader2->point.extra_bytes[15]);
              }
              else if (lasreader1->point.extra_bytes_number == 17)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader1->point.extra_bytes[13], lasreader1->point.extra_bytes[14], lasreader1->point.extra_bytes[15], lasreader1->point.extra_bytes[16], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12], lasreader2->point.extra_bytes[13], lasreader2->point.extra_bytes[14], lasreader2->point.extra_bytes[15], lasreader2->point.extra_bytes[16]);
              }
              else
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ... != %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ...\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader1->point.extra_bytes[8], lasreader1->point.extra_bytes[9], lasreader1->point.extra_bytes[10], lasreader1->point.extra_bytes[11], lasreader1->point.extra_bytes[12], lasreader1->point.extra_bytes[13], lasreader1->point.extra_bytes[14], lasreader1->point.extra_bytes[15], lasreader1->point.extra_bytes[16], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7], lasreader2->point.extra_bytes[8], lasreader2->point.extra_bytes[9], lasreader2->point.extra_bytes[10], lasreader2->point.extra_bytes[11], lasreader2->point.extra_bytes[12], lasreader2->point.extra_bytes[13], lasreader2->point.extra_bytes[14], lasreader2->point.extra_bytes[15], lasreader2->point.extra_bytes[16]);
              }
            }
            difference = true;
          }
        }
        else if (lasreader2->point.extra_bytes_number)
        {
          if (memcmp((const void*)lasreader1->point.extra_bytes, (const void*)lasreader2->point.extra_bytes, lasreader2->point.extra_bytes_number))
          {
            if (different_points < shutup)
            {
              if (lasreader1->point.extra_bytes_number == 1)
              {
                fprintf(stderr, "%d extra_byte of point %u of %u are different: %d != %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader2->point.extra_bytes[0]);
              }
              else if (lasreader1->point.extra_bytes_number == 2)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d != %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1]);
              }
              else if (lasreader1->point.extra_bytes_number == 3)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d != %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2]);
              }
              else if (lasreader1->point.extra_bytes_number == 4)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d != %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3]);
              }
              else if (lasreader1->point.extra_bytes_number == 5)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d != %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4]);
              }
              else if (lasreader1->point.extra_bytes_number == 6)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d != %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5]);
              }
              else if (lasreader1->point.extra_bytes_number == 7)
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d != %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6]);
              }
              else
              {
                fprintf(stderr, "%d extra_bytes of point %u of %u are different: %d %d %d %d %d %d %d %d != %d %d %d %d %d %d %d %d\n", lasreader1->point.extra_bytes_number,  (U32)lasreader1->p_idx, (U32)lasreader1->npoints, lasreader1->point.extra_bytes[0], lasreader1->point.extra_bytes[1], lasreader1->point.extra_bytes[2], lasreader1->point.extra_bytes[3], lasreader1->point.extra_bytes[4], lasreader1->point.extra_bytes[5], lasreader1->point.extra_bytes[6], lasreader1->point.extra_bytes[7], lasreader2->point.extra_bytes[0], lasreader2->point.extra_bytes[1], lasreader2->point.extra_bytes[2], lasreader2->point.extra_bytes[3], lasreader2->point.extra_bytes[4], lasreader2->point.extra_bytes[5], lasreader2->point.extra_bytes[6], lasreader2->point.extra_bytes[7]);
              }
            }
            difference = true;
          }
        }
        if (lasreader1->point.extended_point_type || lasreader2->point.extended_point_type )
        {
          if (lasreader1->point.extended_scan_angle != lasreader2->point.extended_scan_angle)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_scan_angle: %s %s (point index %u)\n", lasreader1->point.get_scan_angle_string().c_str(), lasreader2->point.get_scan_angle_string().c_str(), (U32)(lasreader1->p_idx-1));
            difference = true;
          }
          if (lasreader1->point.extended_scanner_channel != lasreader2->point.extended_scanner_channel)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_scanner_channel: %d %d\n", lasreader1->point.extended_scanner_channel, lasreader2->point.extended_scanner_channel);
            difference = true;
          }
          if (lasreader1->point.extended_classification_flags != lasreader2->point.extended_classification_flags)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_classification_flags: %d %d\n", lasreader1->point.extended_classification_flags, lasreader2->point.extended_classification_flags);
            difference = true;
          }
          if (lasreader1->point.extended_classification != lasreader2->point.extended_classification)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_classification: %d %d\n", lasreader1->point.extended_classification, lasreader2->point.extended_classification);
            difference = true;
          }
          if (lasreader1->point.extended_return_number != lasreader2->point.extended_return_number)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_return_number: %d %d\n", lasreader1->point.extended_return_number, lasreader2->point.extended_return_number);
            difference = true;
          }
          if (lasreader1->point.extended_number_of_returns != lasreader2->point.extended_number_of_returns)
          {
            if (different_points < shutup) fprintf(stderr, "  extended_number_of_returns: %d %d\n", lasreader1->point.extended_number_of_returns, lasreader2->point.extended_number_of_returns);
            difference = true;
          }
        }
      }
      else
      {
        while (lasreader1->read_point());
        fprintf(stderr, "%s (%u) has %u fewer points than %s (%u)\n", file_name2, (U32)lasreader2->p_idx, (U32)(lasreader1->p_idx - lasreader2->p_idx), file_name1, (U32)lasreader1->p_idx);
        break;
      }
    }
    else
    {
      if (lasreader2->read_point())
      {
        while (lasreader2->read_point());
        fprintf(stderr, "%s (%u) has %u more points than %s (%u)\n", file_name2, (U32)lasreader2->p_idx, (U32)(lasreader2->p_idx - lasreader1->p_idx), file_name1, (U32)lasreader1->p_idx);
        break;
      }
      else
      {
        break;
      }
    }
    if (difference)
    {
      different_points++;
      if (different_points == shutup) fprintf(stderr, "already %d points are different ... shutting up.\n", shutup);
    }
    if (laswriter)
    {
      lasreader1->point.set_z(lasreader1->point.get_z()-lasreader2->point.get_z());
      laswriter->write_point(&lasreader1->point);
      laswriter->update_inventory(&lasreader1->point);
    }
  }

  if (laswriter)
  {
    laswriter->update_header(&lasreader1->header, TRUE);
    laswriter->close();
    delete laswriter;
    laswriter = 0;
  }

  if (scaled_offset_difference)
  {
    if (different_scaled_offset_coordinates)
    {
      fprintf(stderr, "scaled offset points are different (max diff: %g %g %g).\n", max_diff_x, max_diff_y, max_diff_z);
    }
    else
    {
      fprintf(stderr, "scaled offset points are identical.\n");
    }
  }
  else
  {
    if (different_points)
    {
      fprintf(stderr, "%u points are different.\n", different_points);
    }
    else
    {
      fprintf(stderr, "raw points are identical.\n");
    }
  }

  return different_points;
};

#ifdef COMPILE_WITH_GUI
extern int lasdiff_gui(int argc, char *argv[], LASreadOpener* lasreadopener);
#endif

int main(int argc, char *argv[])
{
  LasTool_lasdiff lastool;
  lastool.init(argc, argv, "lasdiff");
  int i;
  int random_seeks = 0;
  const CHAR* wildcard1 = 0;
  const CHAR* wildcard2 = 0;
  double start_time = 0.0;

  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return lasdiff_gui(argc, argv, 0);
#else
    wait_on_exit();
    char file_name[256];
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    fprintf(stderr,"enter input file1: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr,"enter input file2: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
#endif
  }
  else
  {
    for (i = 1; i < argc; i++)
    {
      if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
    }
    lasreadopener.parse(argc, argv);
    laswriteopener.parse(argc, argv);
  }

  auto arg_local = [&](int& i) -> bool {
    if (strcmp(argv[i],"-random_seeks") == 0)
    {
      random_seeks = 10;
    }
    else if (strcmp(argv[i],"-wildcards") == 0)
    {
      if ((i+2) >= argc)
      {
        laserror("'%s' needs 2 arguments: wildcard1 wildcard2", argv[i]);
      }
      wildcard1 = argv[i+1];
      wildcard2 = argv[i+2];
      i+=2;
    }
    else if (strcmp(argv[i],"-shutup") == 0)
    {
      i++;
      shutup = atoi(argv[i]);;
    }
    else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
    else
    {
      return false;
    }
    return true;
  };

  lastool.parse(arg_local);

#ifdef COMPILE_WITH_GUI
  if (lastool.gui)
  {
    return lasdiff_gui(argc, argv, &lasreadopener);
  }
#endif

  char* file_name1;
  char* file_name2;

  LASreader* lasreader1;
  LASreader* lasreader2;
  LASwriter* laswriter = 0;

  int different_header;

  // possibly loop over two wild cards

  if (wildcard1 && wildcard2)
  {
    LASreadOpener lasreadopener1;
    LASreadOpener lasreadopener2;

    lasreadopener1.add_file_name(wildcard1);
    lasreadopener2.add_file_name(wildcard2);

    if (!lasreadopener1.active())
    {
      laserror("wildcard1 specifies no input");
    }

    if (!lasreadopener2.active())
    {
      laserror("wildcard2 specifies no input");
    }

    if (lasreadopener1.get_file_name_number() != lasreadopener2.get_file_name_number())
    {
      laserror("wildcard1 (%u) and wildcard2 (%u) specify different number of files", lasreadopener1.get_file_name_number(), lasreadopener2.get_file_name_number());
    }

    // possibly multiple input files

    while (lasreadopener1.active())
    {
      lasreader1 = lasreadopener1.open();
      file_name1 = LASCopyString(lasreadopener1.get_file_name());
      if (lasreader1 == 0)
      {
        laserror("cannot open '%s'", file_name1);
      }
      lasreader2 = lasreadopener2.open();
      file_name2 = LASCopyString(lasreadopener2.get_file_name());
      if (lasreader2 == 0)
      {
        laserror("cannot open '%s'", file_name2);
      }

      LASMessage(LAS_INFO, "checking '%s' against '%s'", file_name1, file_name2);

      // check header

      different_header = check_header(lasreader1, lasreader2);

      // check points

      int different_points = check_points(file_name1, lasreader1, file_name2, lasreader2, laswriter, random_seeks);

      // output final verdicts

      if (!different_header && !different_points && !different_scaled_offset_coordinates) fprintf(stderr, "files are identical. ");

      if (lasreader1->p_idx == lasreader2->p_idx)
      {
        LASMessage(LAS_INFO, "both have %lld points. took %g secs.", lasreader1->p_idx, taketime()-start_time);
      }
      else
      {
        LASMessage(LAS_INFO, "one has %lld the other %lld points. took %g secs.", lasreader1->p_idx, lasreader2->p_idx, taketime()-start_time);
      }
      lasreader1->close();
      delete lasreader1;
      free(file_name1);

      lasreader2->close();
      delete lasreader2;
      free(file_name2);
    }
  }
  else
  {
    if (!lasreadopener.active())
    {
      laserror("no input specified");
    }

    // possibly multiple input files

    while (lasreadopener.active())
    {
      start_time = taketime();

      if (lasreadopener.get_file_name_number() == 2)
      {
        lasreader1 = lasreadopener.open();
        file_name1 = LASCopyString(lasreadopener.get_file_name());
        if (lasreader1 == 0)
        {
          laserror("cannot open '%s'", file_name1);
        }
        lasreader2 = lasreadopener.open();
        file_name2 = LASCopyString(lasreadopener.get_file_name());
        if (lasreader2 == 0)
        {
          laserror("cannot open '%s'", file_name2);
        }
      }
      else
      {
        lasreader1 = lasreadopener.open();
        file_name1 = LASCopyString(lasreadopener.get_file_name());
        if (lasreader1 == 0)
        {
          laserror("cannot open '%s'", file_name1);
        }
        file_name2 = LASCopyString(lasreadopener.get_file_name());
        I32 len = (I32)strlen(file_name1);
        if (strncmp(&file_name1[len-4], ".las", 4) == 0)
        {
          file_name2[len-1] = 'z';
        }
        else if (strncmp(&file_name1[len-4], ".laz", 4) == 0)
        {
          file_name2[len-1] = 's';
        }
        else if (strncmp(&file_name1[len-4], ".LAS", 4) == 0)
        {
          file_name2[len-1] = 'Z';
        }
        else if (strncmp(&file_name1[len-4], ".LAZ", 4) == 0)
        {
          file_name2[len-1] = 'S';
        }
        else
        {
          laserror("file '%s' not ending in *.las or *.laz", file_name1);
        }
        LASreadOpener lasreadopener_other;
        lasreadopener_other.set_file_name(file_name2);
        lasreader2 = lasreadopener_other.open();
        if (lasreader2 == 0)
        {
          laserror("cannot open '%s'", file_name2);
        }
      }

      LASMessage(LAS_INFO, "checking '%s' against '%s'", file_name1, file_name2);

      // check header

      different_header = check_header(lasreader1, lasreader2);

      // maybe we should create a difference file

      if (laswriteopener.active() || laswriteopener.get_directory() || laswriteopener.get_appendix())
      {
        if (!laswriteopener.active())
        {
          laswriteopener.make_file_name(lasreadopener.get_file_name());
        }
        // prepare the header
        memset(lasreader1->header.system_identifier, 0, LAS_HEADER_CHAR_LEN);
        memset(lasreader1->header.generating_software, 0, LAS_HEADER_CHAR_LEN);
        snprintf(lasreader1->header.system_identifier, sizeof(lasreader1->header.system_identifier), LAS_TOOLS_COPYRIGHT);
        snprintf(lasreader1->header.generating_software, sizeof(lasreader1->header.generating_software), "lasdiff%s (version %d)", (IS64 ? "64" : ""), LAS_TOOLS_VERSION);
        laswriter = laswriteopener.open(&lasreader1->header);
        if (laswriter == 0)
        {
          laserror("cannot open '%s'", laswriteopener.get_file_name());
        }
        laswriteopener.set_file_name(0);
      }

      // check points

      int different_points = check_points(file_name1, lasreader1, file_name2, lasreader2, laswriter, random_seeks);

      // output final verdicts

      if (!different_header && !different_points && !different_scaled_offset_coordinates) fprintf(stderr, "files are identical. ");

      if (lasreader1->p_idx == lasreader2->p_idx)
      {
        LASMessage(LAS_INFO, "both have %lld points. took %g secs.", lasreader1->p_idx, taketime()-start_time);
      }
      else
      {
        LASMessage(LAS_INFO, "one has %lld the other %lld points. took %g secs.", lasreader1->p_idx, lasreader2->p_idx, taketime()-start_time);
      }
      lasreader1->close();
      delete lasreader1;
      free(file_name1);
      //
      lasreader2->close();
      delete lasreader2;
      free(file_name2);
    }
  }

  byebye();

  return 0;
}
