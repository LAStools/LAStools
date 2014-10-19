/*
===============================================================================

  FILE:  laszipdllexample.cpp
  
  CONTENTS:
  
    This source code implements three different  easy-to-follow examples on
    how to use the LASzip DLL. The first and the second examples implement a
    small compression and decompression utilitity. The third example shows
    how to use the DLL to export points to a proper geo-referenced LAZ file.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    11 August 2013 -- added third example for exporting geo-referenced points 
    29 July 2013 -- created for the LASzip DLL after returning to Sommerhausen 
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "laszip_dll.h"

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"laszipdllexample\n");
  fprintf(stderr,"laszipdllexample in.las out.laz\n");
  fprintf(stderr,"laszipdllexample in.laz out.las\n");
  fprintf(stderr,"laszipdllexample in.las out.las\n");
  fprintf(stderr,"laszipdllexample in.laz out.laz\n");
  fprintf(stderr,"laszipdllexample -h\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(1);
}

static void dll_error(laszip_POINTER laszip)
{
  if (laszip)
  {
    laszip_CHAR* error;
    if (laszip_get_error(laszip, &error))
    {
      fprintf(stderr,"DLL ERROR: getting error messages\n");
    }
    fprintf(stderr,"DLL ERROR MESSAGE: %s\n", error);
  }
}

static void byebye(bool error=false, bool wait=false, laszip_POINTER laszip=0)
{
  if (error)
  {  
    dll_error(laszip);
  }
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

#define EXAMPLE_ONE 1
#define EXAMPLE_TWO 2
#define EXAMPLE_THREE 3

#define EXAMPLE EXAMPLE_THREE

int main(int argc, char *argv[])
{
  double start_time = 0.0;
  char* file_name_in = 0;
  char* file_name_out = 0;

  // load LASzip DLL

  if (laszip_load_dll())
  {
    fprintf(stderr,"DLL ERROR: loading LASzip DLL\n");
    byebye(true, argc==1);
  }

  // get version of LASzip DLL

  laszip_U8 version_major;
  laszip_U8 version_minor;
  laszip_U16 version_revision;
  laszip_U32 version_build;

  if (laszip_get_version(&version_major, &version_minor, &version_revision, &version_build))
  {
    fprintf(stderr,"DLL ERROR: getting LASzip DLL version number\n");
    byebye(true, argc==1);
  }

  fprintf(stderr,"LASzip DLL v%d.%d r%d (build %d)\n", (int)version_major, (int)version_minor, (int)version_revision, (int)version_build);

  if (argc == 1)
  {
    char file_name[256];
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    fprintf(stderr,"enter input file%s: ", ((EXAMPLE == EXAMPLE_THREE) ? " (not used)" : "")); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    file_name_in = strdup(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    file_name_out = strdup(file_name);
  }
  else if (argc == 3)
  {
    file_name_in = strdup(argv[1]);
    file_name_out = strdup(argv[2]);
  }
  else
  {
    if ((argc != 2) || (strcmp(argv[1], "-h") != 0))
    {
      fprintf(stderr, "ERROR: cannot understand arguments\n");
    }
    usage();
  }

  start_time = taketime();

  if (EXAMPLE == EXAMPLE_ONE)
  {
    fprintf(stderr,"running EXAMPLE_ONE\n");

    // create the reader

    laszip_POINTER laszip_reader;
    if (laszip_create(&laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: creating laszip reader\n");
      byebye(true, argc==1);
    }

    // open the reader

    laszip_BOOL is_compressed;
    if (laszip_open_reader(laszip_reader, file_name_in, &is_compressed))
    {
      fprintf(stderr,"DLL ERROR: opening laszip reader for '%s'\n", file_name_in);
      byebye(true, argc==1, laszip_reader);
    }
  
    fprintf(stderr,"file '%s' is %scompressed\n", file_name_in, (is_compressed ? "" : "un"));

    // get a pointer to the header of the reader that was just populated

    laszip_header* header;

    if (laszip_get_header_pointer(laszip_reader, &header))
    {
      fprintf(stderr,"DLL ERROR: getting header pointer from laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // report how many points the file has

    fprintf(stderr,"file '%s' contains %u points\n", file_name_in, header->number_of_point_records);

    // get a pointer to the points that will be read

    laszip_point* point;

    if (laszip_get_point_pointer(laszip_reader, &point))
    {
      fprintf(stderr,"DLL ERROR: getting point pointer from laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // create the writer

    laszip_POINTER laszip_writer;
    if (laszip_create(&laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: creating laszip writer\n");
      byebye(true, argc==1);
    }

    // initialize the header for the writer using the header of the reader 

    if (laszip_set_header(laszip_writer, header))
    {
      fprintf(stderr,"DLL ERROR: setting header for laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // open the writer

    laszip_BOOL compress = (strstr(file_name_out, ".laz") != 0);

    if (laszip_open_writer(laszip_writer, file_name_out, compress))
    {
      fprintf(stderr,"DLL ERROR: opening laszip writer for '%d'\n", file_name_out);
      byebye(true, argc==1, laszip_writer);
    }
  
    fprintf(stderr,"writing file '%s' %scompressed\n", file_name_out, (compress ? "" : "un"));

    // read the points

    laszip_U32 count = 0;

    while (count < header->number_of_point_records)
    {
      // read a point

      if (laszip_read_point(laszip_reader))
      {
        fprintf(stderr,"DLL ERROR: reading point %u\n", count);
        byebye(true, argc==1, laszip_reader);
      }

      // copy the point

      if (laszip_set_point(laszip_writer, point))
      {
        fprintf(stderr,"DLL ERROR: setting point %u\n", count);
        byebye(true, argc==1, laszip_writer);
      }

      // write the point

      if (laszip_write_point(laszip_writer))
      {
        fprintf(stderr,"DLL ERROR: writing point %u\n", count);
        byebye(true, argc==1, laszip_writer);
      }

      count++;
    }

    fprintf(stderr,"successfully read and written %u points\n", count);

    // close the writer

    if (laszip_close_writer(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: closing laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // destroy the writer

    if (laszip_destroy(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: destroying laszip writer\n");
      byebye(true, argc==1);
    }

    // close the reader

    if (laszip_close_reader(laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: closing laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // destroy the reader

    if (laszip_destroy(laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: destroying laszip reader\n");
      byebye(true, argc==1);
    }

    fprintf(stderr,"total time: %g sec for reading %scompressed and writing %scompressed\n", taketime()-start_time, (is_compressed ? "" : "un"), (compress ? "" : "un"));

  } // EXAMPLE_ONE
  
  if (EXAMPLE == EXAMPLE_TWO)
  {
    fprintf(stderr,"running EXAMPLE_TWO\n");

    // create the reader

    laszip_POINTER laszip_reader;
    if (laszip_create(&laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: creating laszip reader\n");
      byebye(true, argc==1);
    }

    // open the reader

    laszip_BOOL is_compressed;
    if (laszip_open_reader(laszip_reader, file_name_in, &is_compressed))
    {
      fprintf(stderr,"DLL ERROR: opening laszip reader for '%s'\n", file_name_in);
      byebye(true, argc==1, laszip_reader);
    }
  
    fprintf(stderr,"file '%s' is %scompressed\n", file_name_in, (is_compressed ? "" : "un"));

    // get a pointer to the header of the reader that was just populated

    laszip_header* header_read;

    if (laszip_get_header_pointer(laszip_reader, &header_read))
    {
      fprintf(stderr,"DLL ERROR: getting header pointer from laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // report how many points the file has

    fprintf(stderr,"file '%s' contains %u points\n", file_name_in, header_read->number_of_point_records);

    // create the writer

    laszip_POINTER laszip_writer;
    if (laszip_create(&laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: creating laszip writer\n");
      byebye(true, argc==1);
    }

    // get a pointer to the header of the writer so we can populate it

    laszip_header* header_write;

    if (laszip_get_header_pointer(laszip_writer, &header_write))
    {
      fprintf(stderr,"DLL ERROR: getting header pointer from laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // copy entries from the reader header to the writer header

    laszip_U32 i;

    header_write->file_source_ID = header_read->file_source_ID;
    header_write->global_encoding = header_read->global_encoding;
    header_write->project_ID_GUID_data_1 = header_read->project_ID_GUID_data_1;
    header_write->project_ID_GUID_data_2 = header_read->project_ID_GUID_data_2;
    header_write->project_ID_GUID_data_3 = header_read->project_ID_GUID_data_3;
    memcpy(header_write->project_ID_GUID_data_4, header_read->project_ID_GUID_data_4, 8);
    header_write->version_major = header_read->version_major;
    header_write->version_minor = header_read->version_minor;
    memcpy(header_write->system_identifier, header_read->system_identifier, 32);
    memcpy(header_write->generating_software, header_read->generating_software, 32);
    header_write->file_creation_day = header_read->file_creation_day;
    header_write->file_creation_year = header_read->file_creation_year;
    header_write->header_size = header_read->header_size;
    header_write->offset_to_point_data = header_read->header_size; /* note !!! */ 
    header_write->number_of_variable_length_records = header_read->number_of_variable_length_records;
    header_write->point_data_format = header_read->point_data_format;
    header_write->point_data_record_length = header_read->point_data_record_length;
    header_write->number_of_point_records = header_read->number_of_point_records;
    for (i = 0; i < 5; i++)
    {
      header_write->number_of_points_by_return[i] = header_read->number_of_points_by_return[i];
    }
    header_write->x_scale_factor = header_read->x_scale_factor;
    header_write->y_scale_factor = header_read->y_scale_factor;
    header_write->z_scale_factor = header_read->z_scale_factor;
    header_write->x_offset = header_read->x_offset;
    header_write->y_offset = header_read->y_offset;
    header_write->z_offset = header_read->z_offset;
    header_write->max_x = header_read->max_x;
    header_write->min_x = header_read->min_x;
    header_write->max_y = header_read->max_y;
    header_write->min_y = header_read->min_y;
    header_write->max_z = header_read->max_z;
    header_write->min_z = header_read->min_z;

    // LAS 1.3 and higher only
    header_write->start_of_waveform_data_packet_record = header_read->start_of_waveform_data_packet_record;

    // LAS 1.4 and higher only
    header_write->start_of_first_extended_variable_length_record = header_read->start_of_first_extended_variable_length_record;
    header_write->number_of_extended_variable_length_records = header_read->number_of_extended_variable_length_records;
    header_write->extended_number_of_point_records = header_read->extended_number_of_point_records;
    for (i = 0; i < 15; i++)
    {
      header_write->extended_number_of_points_by_return[i] = header_read->extended_number_of_points_by_return[i];
    }

    // we may modify output because we omit any user defined data that may be ** the header

    if (header_read->user_data_in_header_size)
    {
      header_write->header_size -= header_read->user_data_in_header_size;
      header_write->offset_to_point_data -= header_read->user_data_in_header_size;
      fprintf(stderr,"omitting %d bytes of user_data_in_header\n", header_read->user_data_after_header_size);
    }

    // add all the VLRs

    if (header_read->number_of_variable_length_records)
    {
      fprintf(stderr,"offset_to_point_data before adding %u VLRs is %d\n", header_read->number_of_variable_length_records, (laszip_I32)header_write->offset_to_point_data);
      for (i = 0; i < header_read->number_of_variable_length_records; i++)
      {
        if (laszip_add_vlr(laszip_writer, &(header_read->vlrs[i])))
        {
          fprintf(stderr,"DLL ERROR: adding VLR %u of %u to the header of the laszip writer\n", i+i, header_read->number_of_variable_length_records);
          byebye(true, argc==1, laszip_writer);
        }
        fprintf(stderr,"                     after adding VLR number %u is %d\n", i+1, (laszip_I32)header_write->offset_to_point_data);
      }
    }

    // we may modify output because we omit any user defined data that may be *after* the header

    if (header_read->user_data_after_header_size)
    {
      fprintf(stderr,"omitting %d bytes of user_data_after_header\n", header_read->user_data_after_header_size);
    }

    // open the writer

    laszip_BOOL compress = (strstr(file_name_out, ".laz") != 0);

    if (laszip_open_writer(laszip_writer, file_name_out, compress))
    {
      fprintf(stderr,"DLL ERROR: opening laszip writer for '%d'\n", file_name_out);
      byebye(true, argc==1, laszip_writer);
    }
  
    fprintf(stderr,"writing file '%s' %scompressed\n", file_name_out, (compress ? "" : "un"));

    // get a pointer to the point of the reader will be read

    laszip_point* point_read;

    if (laszip_get_point_pointer(laszip_reader, &point_read))
    {
      fprintf(stderr,"DLL ERROR: getting point pointer from laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // get a pointer to the point of the writer that we will populate and write

    laszip_point* point_write;

    if (laszip_get_point_pointer(laszip_writer, &point_write))
    {
      fprintf(stderr,"DLL ERROR: getting point pointer from laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // read the points

    laszip_U32 count = 0;

    while (count < header_read->number_of_point_records)
    {
      // read a point

      if (laszip_read_point(laszip_reader))
      {
        fprintf(stderr,"DLL ERROR: reading point %u\n", count);
        byebye(true, argc==1, laszip_reader);
      }

      // copy the point

      point_write->X = point_read->X;
      point_write->Y = point_read->Y;
      point_write->Z = point_read->Z;
      point_write->intensity = point_read->intensity;
      point_write->return_number = point_read->return_number;
      point_write->number_of_returns_of_given_pulse = point_read->number_of_returns_of_given_pulse;
      point_write->scan_direction_flag = point_read->scan_direction_flag;
      point_write->edge_of_flight_line = point_read->edge_of_flight_line;
      point_write->classification = point_read->classification;
      point_write->scan_angle_rank = point_read->scan_angle_rank;
      point_write->user_data = point_read->user_data;
      point_write->point_source_ID = point_read->point_source_ID;

      point_write->gps_time = point_read->gps_time;
      memcpy(point_write->rgb, point_read->rgb, 8);
      memcpy(point_write->wave_packet, point_read->wave_packet, 29);

      // LAS 1.4 only
      point_write->extended_point_type = point_read->extended_point_type;
      point_write->extended_scanner_channel = point_read->extended_scanner_channel;
      point_write->extended_classification_flags = point_read->extended_classification_flags;
      point_write->extended_classification = point_read->extended_classification;
      point_write->extended_return_number = point_read->extended_return_number;
      point_write->extended_number_of_returns_of_given_pulse = point_read->extended_number_of_returns_of_given_pulse;
      point_write->extended_scan_angle = point_read->extended_scan_angle;

      if (point_read->num_extra_bytes)
      {
        memcpy(point_write->extra_bytes, point_read->extra_bytes, point_read->num_extra_bytes);
      }

      // write the point

      if (laszip_write_point(laszip_writer))
      {
        fprintf(stderr,"DLL ERROR: writing point %u\n", count);
        byebye(true, argc==1, laszip_writer);
      }

      count++;
    }

    fprintf(stderr,"successfully read and written %u points\n", count);

    // close the writer

    if (laszip_close_writer(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: closing laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // destroy the writer

    if (laszip_destroy(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: destroying laszip writer\n");
      byebye(true, argc==1);
    }

    // close the reader

    if (laszip_close_reader(laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: closing laszip reader\n");
      byebye(true, argc==1, laszip_reader);
    }

    // destroy the reader

    if (laszip_destroy(laszip_reader))
    {
      fprintf(stderr,"DLL ERROR: destroying laszip reader\n");
      byebye(true, argc==1);
    }
  
    fprintf(stderr,"total time: %g sec for reading %scompressed and writing %scompressed\n", taketime()-start_time, (is_compressed ? "" : "un"), (compress ? "" : "un"));

  } // EXAMPLE_TWO

  if (EXAMPLE == EXAMPLE_THREE)
  {
    fprintf(stderr,"running EXAMPLE_THREE\n");

    // create the writer

    laszip_POINTER laszip_writer;
    if (laszip_create(&laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: creating laszip writer\n");
      byebye(true, argc==1);
    }

    // get a pointer to the header of the writer so we can populate it

    laszip_header* header;

    if (laszip_get_header_pointer(laszip_writer, &header))
    {
      fprintf(stderr,"DLL ERROR: getting header pointer from laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // populate the header

    header->global_encoding = 0;             // see LAS specification for details
    header->version_major = 1;
    header->version_minor = 2;
    strncpy(header->system_identifier, "my LAS file writer", 32);
    header->file_creation_day = 120;
    header->file_creation_year = 2013;
    header->point_data_format = 1;
    header->point_data_record_length = 28;
    header->number_of_point_records = 5;
    header->number_of_points_by_return[0] = 3;
    header->number_of_points_by_return[1] = 2;
    header->max_x = 630499.95;
    header->min_x = 630498.56;
    header->max_y = 4834749.66;
    header->min_y = 4834748.73;
    header->max_z = 63.68;
    header->min_z = 61.33;

    // optional: use the bounding box and the scale factor to create a "good" offset

    if (laszip_auto_offset(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: during automatic offset creation\n");
      byebye(true, argc==1, laszip_writer);
    }

    // create some funny VLR

    laszip_vlr funny_vlr;
    memset(&funny_vlr, 0, sizeof(laszip_vlr));
    strcpy(funny_vlr.user_id, "funny");
    funny_vlr.record_id = 12345;
    funny_vlr.record_length_after_header = 0;
    strcpy(funny_vlr.description, "just a funny VLR");

    // add the funny VLR

    fprintf(stderr,"offset_to_point_data before adding funny VLR is    : %d\n", (laszip_I32)header->offset_to_point_data);

    if (laszip_add_vlr(laszip_writer, &funny_vlr))
    {
      fprintf(stderr,"DLL ERROR: adding funny VLR to the header\n");
      byebye(true, argc==1, laszip_writer);
    }

    // create the geokeys with the projection information

    laszip_geokey_struct key_entries[5];

    // projected coordinates
    key_entries[0].key_id = 1024; // GTModelTypeGeoKey
    key_entries[0].tiff_tag_location = 0;
    key_entries[0].count = 1;
    key_entries[0].value_offset = 1; // ModelTypeProjected

    // projection
    key_entries[1].key_id = 3072; // ProjectedCSTypeGeoKey
    key_entries[1].tiff_tag_location = 0;
    key_entries[1].count = 1;
    key_entries[1].value_offset = 32613; // PCS_WGS84_UTM_zone_13N

    // horizontal units
    key_entries[2].key_id = 3076; // ProjLinearUnitsGeoKey
    key_entries[2].tiff_tag_location = 0;
    key_entries[2].count = 1;
    key_entries[2].value_offset = 9001; // meters

    // vertical units
    key_entries[3].key_id = 4099; // VerticalUnitsGeoKey
    key_entries[3].tiff_tag_location = 0;
    key_entries[3].count = 1;
    key_entries[3].value_offset = 9001; // meters

    // vertical datum
    key_entries[4].key_id = 4096; // VerticalCSTypeGeoKey
    key_entries[4].tiff_tag_location = 0;
    key_entries[4].count = 1;
    key_entries[4].value_offset = 5030; // WGS84

    // add the geokeys (create or replace the appropriate VLR)

    fprintf(stderr,"                     before adding projection VLR  : %d\n", (laszip_I32)header->offset_to_point_data);

    if (laszip_set_geokeys(laszip_writer, 5, key_entries))
    {
      fprintf(stderr,"DLL ERROR: adding funny VLR to the header\n");
      byebye(true, argc==1, laszip_writer);
    }
    
    fprintf(stderr,"                     after adding two VLRs         : %d\n", (laszip_I32)header->offset_to_point_data);

    // open the writer

    laszip_BOOL compress = (strstr(file_name_out, ".laz") != 0);

    if (laszip_open_writer(laszip_writer, file_name_out, compress))
    {
      fprintf(stderr,"DLL ERROR: opening laszip writer for '%d'\n", file_name_out);
      byebye(true, argc==1, laszip_writer);
    }
  
    fprintf(stderr,"writing file '%s' %scompressed\n", file_name_out, (compress ? "" : "un"));

    // get a pointer to the point of the writer that we will populate and write

    laszip_point* point;

    if (laszip_get_point_pointer(laszip_writer, &point))
    {
      fprintf(stderr,"DLL ERROR: getting point pointer from laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // write five points

    laszip_I64 count = 0;
    laszip_F64 coordinates[3];

    // populate the first point

    coordinates[0] = 630499.95;
    coordinates[1] = 4834749.17;
    coordinates[2] = 62.15;

    if (laszip_set_coordinates(laszip_writer, coordinates))
    {
      fprintf(stderr,"DLL ERROR: setting coordinates for point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }

    point->intensity = 60;
    point->return_number = 2;
    point->number_of_returns_of_given_pulse = 2;
    point->classification = 2;
    point->scan_angle_rank = 21;
    point->gps_time = 413162.560400;

    // write the first point

    if (laszip_write_point(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: writing point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }
    count++;

    // populate the second point

    coordinates[0] = 630499.83;
    coordinates[1] = 4834748.88;
    coordinates[2] = 62.68;

    if (laszip_set_coordinates(laszip_writer, coordinates))
    {
      fprintf(stderr,"DLL ERROR: setting coordinates for point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }

    point->intensity = 90;
    point->return_number = 1;
    point->number_of_returns_of_given_pulse = 1;
    point->classification = 1;
    point->scan_angle_rank = 21;
    point->gps_time = 413162.563600;

    // write the second point

    if (laszip_write_point(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: writing point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }
    count++;
    
    // populate the third point

    coordinates[0] = 630499.54;  
    coordinates[1] = 4834749.66;
    coordinates[2] = 62.66;

    if (laszip_set_coordinates(laszip_writer, coordinates))
    {
      fprintf(stderr,"DLL ERROR: setting coordinates for point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }

    point->intensity = 70;
    point->return_number = 1;
    point->number_of_returns_of_given_pulse = 1;
    point->classification = 1;
    point->scan_angle_rank = 22;
    point->gps_time = 413162.566800;

    // write the third point

    if (laszip_write_point(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: writing point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }
    count++;

    // populate the fourth point

    coordinates[0] = 630498.56;     
    coordinates[1] = 4834749.41;
    coordinates[2] = 63.68;

    if (laszip_set_coordinates(laszip_writer, coordinates))
    {
      fprintf(stderr,"DLL ERROR: setting coordinates for point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }

    point->intensity = 20;
    point->return_number = 1;
    point->number_of_returns_of_given_pulse = 2;
    point->classification = 3;
    point->scan_angle_rank = 22;
    point->gps_time = 413162.580200;

    // write the fourth point

    if (laszip_write_point(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: writing point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }
    count++;

    // populate the fifth point

    coordinates[0] = 630498.80; 
    coordinates[1] = 4834748.73;
    coordinates[2] = 62.16;

    if (laszip_set_coordinates(laszip_writer, coordinates))
    {
      fprintf(stderr,"DLL ERROR: setting coordinates for point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }

    point->intensity = 110;
    point->return_number = 2;
    point->number_of_returns_of_given_pulse = 2;
    point->classification = 2;
    point->scan_angle_rank = 22;
    point->gps_time = 413162.580200;

    // write the fifth point

    if (laszip_write_point(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: writing point %u\n", (laszip_U32)count);
      byebye(true, argc==1, laszip_writer);
    }
    count++;
    
    // get the number of points written so far

    if (laszip_get_point_count(laszip_writer, &count))
    {
      fprintf(stderr,"DLL ERROR: getting point count\n");
      byebye(true, argc==1, laszip_writer);
    }

    fprintf(stderr,"successfully written %u points\n", (laszip_U32)count);

    // close the writer

    if (laszip_close_writer(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: closing laszip writer\n");
      byebye(true, argc==1, laszip_writer);
    }

    // destroy the writer

    if (laszip_destroy(laszip_writer))
    {
      fprintf(stderr,"DLL ERROR: destroying laszip writer\n");
      byebye(true, argc==1);
    }
  
    fprintf(stderr,"total time: %g sec for writing %scompressed\n", taketime()-start_time, (compress ? "" : "un"));

  } // EXAMPLE_THREE

  // unload LASzip DLL

  if (laszip_unload_dll())
  {
    fprintf(stderr,"DLL ERROR: unloading LASzip DLL\n");
    byebye(true, argc==1);
  }

  return 0;
}
