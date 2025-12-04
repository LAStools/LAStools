/*
===============================================================================

  FILE:  las2las.cpp

  CONTENTS:

    This tool reads and writes LIDAR data in the LAS format and is typically
    used to modify the contents of a LAS file. Examples are keeping or dropping
    those points lying within a certain region (-keep_xy, -drop_x_above, ...),
    or points with a certain elevation (-keep_z, -drop_z_below, -drop_z_above)
    eliminating points that are the second return (-drop_return 2), that have a
    scan angle above a certain threshold (-drop_scan_angle_above 5), or that are
    below a certain intensity (-drop_intensity_below 15).
    Another typical use may be to extract only first (-first_only) returns or
    only last returns (-last_only). Extracting the first return is actually the
    same as eliminating all others (e.g. -keep_return 2 -keep_return 3, ...).

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2020, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    30 October 2020 -- fail / exit with error code when input file is corrupt
     9 September 2019 -- warn if modifying x or y coordinates for tiles with VLR
    30 November 2017 -- set OGC WKT with '-set_ogc_wkt "PROJCS[\"WGS84\",GEOGCS[\"GCS_ ..."
    10 October 2017 -- allow piped input *and* output if no filter or coordinate change
    14 July 2017 -- fixed missing 'comma' in compound (COMPD_CS) OGC WKT string
    23 October 2016 -- OGC WKT string stores COMPD_CS for projection + vertical
    22 October 2016 -- new '-set_ogc_wkt_in_evlr' store to EVLR instead of VLR
     1 January 2016 -- option '-set_ogc_wkt' to store CRS as OGC WKT string
     3 May 2015 -- improved up-conversion via '-set_version 1.4 -set_point_type 6'
     5 July 2012 -- added option to '-remove_original_vlr'
     6 May 2012 -- added option to '-remove_tiling_vlr'
     5 January 2012 -- added option to clip points to the bounding box
    17 May 2011 -- enabling batch processing with wildcards or multiple file names
    13 May 2011 -- moved indexing, filtering, transforming into LASreader
    18 April 2011 -- can set projection tags or reproject horizontally
    26 January 2011 -- added LAStransform for simply manipulations of points
    21 January 2011 -- added LASreadOpener and reading of multiple LAS files
     3 January 2011 -- added -reoffset & -rescale + -keep_circle via LASfilter
    10 January 2010 -- added -subseq for selecting a [start, end] interval
    10 June 2009 -- added -scale_rgb_down and -scale_rgb_up to scale rgb values
    12 March 2009 -- updated to ask for input if started without arguments
     9 March 2009 -- added ability to remove user defined headers or vlrs
    17 September 2008 -- updated to deal with LAS format version 1.2
    17 September 2008 -- dropping or keeping in double precision and based on z
    10 July 2007 -- created after talking with Linda about the H1B process

===============================================================================
*/

#include <cstdint>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "mydefs.hpp"
#include "lastool.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lastransform.hpp"
#include "geoprojectionconverter.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamin_file.hpp"

class LasTool_las2las : public LasTool
{
private:
public:
  void usage() override
  {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "las2las -i *.las -utm 13N\n");
    fprintf(stderr, "las2las -i *.laz -first_only -olaz\n");
    fprintf(stderr, "las2las -i *.las -drop_return 4 5 -olaz\n");
    fprintf(stderr, "las2las -latlong -target_utm 12T -i in.las -o out.las\n");
    fprintf(stderr, "las2las -i in.laz -target_epsg 2972 -o out.laz\n");
    fprintf(stderr, "las2las -set_point_type 0 -lof file_list.txt -merged -o out.las\n");
    fprintf(stderr, "las2las -remove_vlr 2 -scale_rgb_up -i in.las -o out.las\n");
    fprintf(stderr, "las2las -i in.las -keep_xy 630000 4834500 630500 4835000 -keep_z 10 100 -o out.las\n");
    fprintf(stderr, "las2las -i in.txt -iparse xyzit -keep_circle 630200 4834750 100 -oparse xyzit -o out.txt\n");
    fprintf(stderr, "las2las -i in.las -remove_padding -keep_scan_angle -15 15 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -rescale 0.01 0.01 0.01 -reoffset 0 300000 0 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -set_version 1.2 -keep_gpstime 46.5 47.5 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -drop_intensity_below 10 -olaz -stdout > out.laz\n");
    fprintf(stderr, "las2las -i in.las -last_only -drop_gpstime_below 46.75 -otxt -oparse xyzt -stdout > out.txt\n");
    fprintf(stderr, "las2las -i in.las -remove_all_vlrs -keep_class 2 3 4 -olas -stdout > out.las\n");
    fprintf(stderr, "las2las -h\n");
  };
};

static double taketime()
{
  return (double)(clock()) / CLOCKS_PER_SEC;
}

static bool save_vlrs_to_file(const LASheader* header)
{
  U32 i;
  FILE* file =  LASfopen("vlrs.vlr", "wb");
  if (file == 0) {
      return false;
  }
  ByteStreamOut* out;
  if (Endian::IS_LITTLE_ENDIAN)
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);
  // write number of VLRs
  if (!out->put32bitsLE((U8*)&(header->number_of_variable_length_records)))
  {
    laserror("writing header->number_of_variable_length_records");
  }
  // loop over VLRs
  for (i = 0; i < header->number_of_variable_length_records; i++)
  {
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].reserved)))
    {
      laserror("writing header->vlrs[%d].reserved", i);
    }
    if (!out->putBytes((U8*)header->vlrs[i].user_id, 16))
    {
      laserror("writing header->vlrs[%d].user_id", i);
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_id)))
    {
      laserror("writing header->vlrs[%d].record_id", i);
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_length_after_header)))
    {
      laserror("writing header->vlrs[%d].record_length_after_header", i);
    }
    if (!out->putBytes((U8*)header->vlrs[i].description, 32))
    {
      laserror("writing header->vlrs[%d].description", i);
    }

    // write the data following the header of the variable length record

    if (header->vlrs[i].record_length_after_header)
    {
      if (header->vlrs[i].data)
      {
        if (!out->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          laserror("writing %d bytes of data from header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
        }
      }
      else
      {
        laserror("there should be %d bytes of data in header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
      }
    }
  }
  delete out;
  fclose(file);
  return true;
}

static bool load_vlrs_from_file(LASheader* header)
{
  U32 i;
  FILE* file = LASfopen("vlrs.vlr", "rb");
  if (file == 0) {
      return false;
  }
  ByteStreamIn* in;
  if (Endian::IS_LITTLE_ENDIAN)
    in = new ByteStreamInFileLE(file);
  else
    in = new ByteStreamInFileBE(file);
  // read number of VLRs
  U32 number_of_variable_length_records;
  try {
    in->get32bitsLE((U8*)&number_of_variable_length_records);
  }
  catch (...)
  {
    laserror("reading number_of_variable_length_records");
  }
  // loop over VLRs
  LASvlr vlr;
  for (i = 0; i < number_of_variable_length_records; i++)
  {
    try {
      in->get16bitsLE((U8*)&(vlr.reserved));
    }
    catch (...)
    {
      laserror("reading vlr.reserved");
    }
    try {
      in->getBytes((U8*)vlr.user_id, 16);
    }
    catch (...)
    {
      laserror("reading vlr.user_id");
    }
    try {
      in->get16bitsLE((U8*)&(vlr.record_id));
    }
    catch (...)
    {
      laserror("reading vlr.record_id");
    }
    try {
      in->get16bitsLE((U8*)&(vlr.record_length_after_header));
    }
    catch (...)
    {
      laserror("reading vlr.record_length_after_header");
    }
    try {
      in->getBytes((U8*)vlr.description, 32);
    }
    catch (...)
    {
      laserror("reading vlr.description");
    }

    // write the data following the header of the variable length record

    if (vlr.record_length_after_header)
    {
      vlr.data = new U8[vlr.record_length_after_header];
      try {
        in->getBytes((U8*)vlr.data, vlr.record_length_after_header);
      }
      catch (...)
      {
        laserror("reading %d bytes into vlr.data", vlr.record_length_after_header);
      }
    }
    else
    {
      vlr.data = 0;
    }
    header->add_vlr(vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.data, TRUE, vlr.description);
  }
  delete in;
  fclose(file);
  return true;
}

/// Checks if the specified filename has a ".vlr" extension.
static bool has_vlr_extension(const CHAR* filename) {
  const CHAR* extension = ".vlr";
  const CHAR* dot = strrchr(filename, '.'); // Last occurrence of '.' in the filename

  if (dot != nullptr) {
    if (strcmp(dot, extension) == 0) {
      return true;
    }
  }
  return false;
}

// Validating and determining the VLR index and the parameters
static U32 determine_vlr_index(const LASheader* header, const int& vlr_index, const CHAR* vlr_user_id, const int& vlr_record_id) {
  U32 index = UINT32_MAX;

  if (vlr_user_id == nullptr && vlr_record_id == -1) {
    //Using index for '-save_vlr'
    if (vlr_index != -1) {
      if (vlr_index < 0 || static_cast<unsigned int>(vlr_index) >= header->number_of_variable_length_records) {  //HIER NOCHMAL KONTROLLIEREN ' >= '
        laserror("Invalid index: The specified index '%d' for '-save_vlr' is out of range", vlr_index);
      }
      index = (U32)vlr_index;
    }
    //Default to index = 0 if no parameters are specified when using '-save_vlr'
    else {
      index = 0;
    }
  }
  else if (vlr_index == -1 && vlr_user_id != nullptr && vlr_record_id != -1) {
    //Using user ID and record ID for '-save_vlr'
    for (U32 idx = 0; idx < header->number_of_variable_length_records; idx++)
    {
      if (strcmp(header->vlrs[idx].user_id, vlr_user_id) == 0 && header->vlrs[idx].record_id == vlr_record_id)
      {
        index = idx;
        break;
      }
    }
    if (index == UINT32_MAX) {
      laserror("The specified user ID '%s' or record ID '%d' could not be found when using '-save_vlr'", vlr_user_id, vlr_record_id);
    }
  } 
  else {
    laserror("Invalid parameters: Either specify an index, or specify both user ID and record ID for '-save_vlr'");
  }
  return index;
}

/// Saves a single VLR from a LAS header to a specified or default file, ensuring data integrity and file format validation.
static bool save_single_vlr_to_file(const LASheader* header, const int& vlr_index, const CHAR* vlr_user_id, const int& vlr_record_id, const CHAR* vlr_output_filename)
{
  U32 i = UINT32_MAX;
  FILE* file  = nullptr;
  // Save VLR to specified or default filename
  if (vlr_output_filename == nullptr) {
    file = LASfopen("save.vlr", "wb");
  }
  else {
    if (has_vlr_extension(vlr_output_filename))
      file = LASfopen(vlr_output_filename, "wb");
    else
      laserror("Invalid file format: The specified file must have a .vlr extension");
  }
  if (file == nullptr) {
    return false;
  }
  if (vlr_index != -1) {
    LASMessage(LAS_VERBOSE, "save VLR with vlr_index '%d' to '%s' file", vlr_index, vlr_output_filename);
  }
  else if (vlr_record_id != -1 && vlr_user_id)
  {
    LASMessage(LAS_VERBOSE, "save VLR with user_id '%s' and record_id '%d' to '%s' file", vlr_user_id, vlr_record_id, vlr_output_filename);
  }
  // validating and determining the vlr index and the parameters
  i = determine_vlr_index(header, vlr_index, vlr_user_id, vlr_record_id);

  ByteStreamOut* out;
  if (Endian::IS_LITTLE_ENDIAN)
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  U32 number_of_single_variable_length_records = 1;
  // write number of VLRs
  if (!out->put32bitsLE((U8*)&(number_of_single_variable_length_records)))
  {
    laserror("writing number_of_single_variable_length_records in -save_vlr");
  }

  if (i >= 0) {
    // using specified vlr [i]
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].reserved)))
    {
      laserror("writing header->vlrs[%d].reserved", i);
    }
    if (!out->putBytes((U8*)header->vlrs[i].user_id, 16))
    {
      laserror("writing header->vlrs[%d].user_id", i);
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_id)))
    {
      laserror("writing header->vlrs[%d].record_id", i);
    }
    if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_length_after_header)))
    {
      laserror("writing header->vlrs[%d].record_length_after_header", i);
    }
    if (!out->putBytes((U8*)header->vlrs[i].description, 32))
    {
      laserror("writing header->vlrs[%d].description", i);
    }

    // write the data following the header of the variable length record
    if (header->vlrs[i].record_length_after_header)
    {
      if (header->vlrs[i].data)
      {
        if (!out->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          laserror("writing %d bytes of data from header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
        }
      }
      else
      {
        laserror("there should be %d bytes of data in header->vlrs[%d].data", header->vlrs[i].record_length_after_header, i);
      }
    }
  }
  else {
    laserror("specified vlr could not be located");
  }
  delete out;
  fclose(file);
  return true;
}

/// Loads a single VLR from a specified vlr file based on index or user ID and record ID, and adds it to the LAS header.
static bool load_single_vlr_from_file(LASheader* header, const int& vlr_index, const CHAR* vlr_user_id, const int& vlr_record_id, const CHAR* vlr_input_filename)
{
  U32 i;
  FILE* file = nullptr;
  
  if (vlr_input_filename == nullptr) {
    //try to open default save.vlr file
    file = LASfopen("save.vlr", "rb");
  }
  else {
    file = LASfopen(vlr_input_filename, "rb");
  }
  if (file == nullptr) {
    laserror("VLR file '%s' could not be found", vlr_input_filename);
  }
  ByteStreamIn* in;
  if (Endian::IS_LITTLE_ENDIAN)
    in = new ByteStreamInFileLE(file);
  else
    in = new ByteStreamInFileBE(file);
  // read number of VLRs
  U32 number_of_variable_length_records;
  try {
    in->get32bitsLE((U8*)&number_of_variable_length_records);
  }
  catch (...)
  {
    laserror("reading number_of_variable_length_records");
  } 
  // Check whether the specified index is valid
  if (vlr_index >= 0 && static_cast<unsigned int>(vlr_index) >= number_of_variable_length_records) {
    laserror("specified vlr index '%d' is out of range", vlr_index);
  }
  // loop over VLRs
  LASvlr vlr;
  for (i = 0; i < number_of_variable_length_records; i++)
  {
    try {
      in->get16bitsLE((U8*)&(vlr.reserved));
    }
    catch (...)
    {
      laserror("reading vlr.reserved");
    }
    try {
      in->getBytes((U8*)vlr.user_id, 16);
    }
    catch (...)
    {
      laserror("reading vlr.user_id");
    }
    try {
      in->get16bitsLE((U8*)&(vlr.record_id));
    }
    catch (...)
    {
      laserror("reading vlr.record_id");
    }
    try {
      in->get16bitsLE((U8*)&(vlr.record_length_after_header));
    }
    catch (...)
    {
      laserror("reading vlr.record_length_after_header");
    }
    try {
      in->getBytes((U8*)vlr.description, 32);
    }
    catch (...)
    {
      laserror("reading vlr.description");
    }

    // write the data following the header of the variable length record
    if (vlr.record_length_after_header)
    {
      vlr.data = new U8[vlr.record_length_after_header];
      try {
        in->getBytes((U8*)vlr.data, vlr.record_length_after_header);
      }
      catch (...)
      {
        laserror("reading %d bytes into vlr.data", vlr.record_length_after_header);
      }
    }
    else
    {
      vlr.data = 0;
    }

    if (vlr_index >= 0 && i == (U32)vlr_index) {
      header->add_vlr(vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.data, TRUE, vlr.description);
      LASMessage(LAS_VERBOSE, "load VLR with vlr_index '%d' from '%s' file", vlr_index, vlr_input_filename);
      break;
    }
    else if (vlr_user_id != nullptr && vlr_record_id >= 0 &&
             strcmp(vlr.user_id, vlr_user_id) == 0 && vlr.record_id == vlr_record_id)
    {
        header->add_vlr(vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.data, TRUE, vlr.description);
        LASMessage(LAS_VERBOSE, "load VLR with user_id '%s' and record_id '%d' from '%s' file", vlr_user_id, vlr_record_id, vlr_input_filename);
        break;
    }
  }
  delete in;
  fclose(file);
  return true;
}

/// Parse and handle arguments for -save_vlr or -load_vlr, based on specified vlr index or user ID and record ID and optional output filename
static void parse_save_load_vlr_args(int& i, int argc, char* argv[], bool& save_vlr, int& vlr_index, char*& vlr_user_id, int& vlr_record_id, char*& vlr_filename) {
  if (i + 1 >= argc) {
    vlr_index = 0; // Default value if no arguments are provided
  }
  // Check for user ID and record ID
  else if (i + 2 < argc && argv[i + 1][0] != '-' && isalnum(argv[i + 1][0]) && argv[i + 2][0] != '-' && isdigit(argv[i + 2][0])) {
    vlr_user_id = argv[++i];
    vlr_record_id = atoi(argv[++i]);
    // Optional output filename
    if (i + 1 < argc && argv[i + 1][0] != '-') {
      vlr_filename = argv[++i];
    }
  }
  // Check for numeric index
  else if (i + 1 < argc && argv[i + 1][0] != '-' && isdigit(argv[i + 1][0])) {
    vlr_index = atoi(argv[++i]);
    // Optional output filename
    if (i + 1 < argc && argv[i + 1][0] != '-') {
      vlr_filename = argv[++i];
    }
  }
  //Only filename arguments is provided
  else if (i + 1 < argc && argv[i + 1][0] != '-') {
    vlr_index = 0; // Default value if just filename arguments is provided
    vlr_filename = argv[++i];
  }
}

// for point type conversions
const U8 convert_point_type_from_to[11][11] =
{
  {  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  0,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  0,  1,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 },
};

#ifdef COMPILE_WITH_GUI
extern int las2las_gui(int argc, char* argv[], LASreadOpener* lasreadopener);
#endif

#ifdef COMPILE_WITH_MULTI_CORE
extern void las2las_multi_core(int argc, char* argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores, BOOL cpu64);
#endif

int main(int argc, char* argv[])
{
  LasTool_las2las lastool;
  lastool.init(argc, argv, "las2las");
  int i;
  // fixed header changes
  int set_version_major = -1;
  int set_version_minor = -1;
  int set_point_data_format = -1;
  int set_point_data_record_length = -1;
  int set_global_encoding_gps_bit = -1;
  int set_time_offset = -1;
  int set_lastiling_buffer_flag = -1;
  // variable header changes
  bool ogc_wkt_in_header = false;
  bool set_ogc_wkt = false;
  bool set_ogc_wkt_in_evlr = false;
  CHAR* set_ogc_wkt_string = 0;
  bool remove_header_padding = false;
  bool remove_all_variable_length_records = false;
  int remove_variable_length_record = -1;
  int remove_variable_length_record_from = -1;
  int remove_variable_length_record_to = -1;
  bool remove_all_extended_variable_length_records = false;
  int remove_extended_variable_length_record = -1;
  int remove_extended_variable_length_record_from = -1;
  int remove_extended_variable_length_record_to = -1;
  CHAR* add_empty_vlr_user_ID = 0;
  int add_empty_vlr_record_ID = -1;
  CHAR* add_empty_vlr_description = 0;
  bool move_evlrs_to_vlrs = false;
  bool save_vlrs = false;
  bool load_vlrs = false;
  bool save_vlr = false;
  bool load_vlr = false;
  int vlr_index = -1;
  int vlr_record_id = -1;
  CHAR* vlr_user_id = nullptr;
  CHAR* vlr_filename = nullptr;
  int set_attribute_scales = 0;
  int set_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
  double set_attribute_scale_scale[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
  int set_attribute_offsets = 0;
  int set_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
  double set_attribute_offset_offset[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
  int unset_attribute_scales = 0;
  int unset_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
  int unset_attribute_offsets = 0;
  int unset_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
  bool remove_tiling_vlr = false;
  bool remove_original_vlr = false;
  bool remove_empty_files = true;
  // extract a subsequence
  I64 subsequence_start = 0;
  I64 subsequence_stop = I64_MAX;
  // fix files with corrupt points
  bool clip_to_bounding_box = false;
  double start_time = 0;
  
  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  enum class gps_time_type
  {
      NONE,
      GPS_WEEK_TIME,
      ADJUSTED_GPS_TIME,
      OFFSET_GPS_TIME,
      ADJUSTED_OR_OFFSET_GPS_TIME
  };

  gps_time_type verify_current_gps_time_type = gps_time_type::NONE;


  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    return las2las_gui(argc, argv, 0);
#else
    wait_on_exit();
    fprintf(stderr, "%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr, "enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name) - 1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr, "enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name) - 1] = '\0';
    laswriteopener.set_file_name(file_name);
#endif
  }
  else
  {
      for (i = 1; i < argc; i++)
      {
          if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
          if (strcmp(argv[i], "-week_to_adjusted") == 0)
          {
              set_global_encoding_gps_bit = 1;
              verify_current_gps_time_type = gps_time_type::GPS_WEEK_TIME;
          }
          else if (strcmp(argv[i], "-offset_to_adjusted") == 0) {
              set_global_encoding_gps_bit = 1;
              verify_current_gps_time_type = gps_time_type::OFFSET_GPS_TIME;
          }
          else if (strcmp(argv[i], "-adjusted_or_offset_to_adjusted") == 0) {
              set_global_encoding_gps_bit = 1;
              verify_current_gps_time_type = gps_time_type::ADJUSTED_OR_OFFSET_GPS_TIME;
          }
          else if (strcmp(argv[i], "-adjusted_to_week") == 0)
          {
              set_global_encoding_gps_bit = 0;
              verify_current_gps_time_type = gps_time_type::ADJUSTED_GPS_TIME;
          }
          else if (strcmp(argv[i], "-adjusted_or_offset_to_week") == 0)
          {
              set_global_encoding_gps_bit = 0;
              verify_current_gps_time_type = gps_time_type::ADJUSTED_OR_OFFSET_GPS_TIME;
          }
          else if (strcmp(argv[i], "-offset_to_week") == 0)
          {
              set_global_encoding_gps_bit = 0;
              verify_current_gps_time_type = gps_time_type::OFFSET_GPS_TIME;
          }
          else if (strcmp(argv[i], "-adjusted_to_offset") == 0)
          {
              lastool.parse_arg_cnt_check(i, 1, "time_offset");
              if (sscanf_las(argv[i + 1], "%u", &set_time_offset) != 1)
              {
                  lastool.error_parse_arg_n_invalid(i, 1);
              }
              if (set_time_offset < 0) {
                  set_time_offset = 0;
              }
              verify_current_gps_time_type = gps_time_type::ADJUSTED_GPS_TIME;
          }
          else if (strcmp(argv[i], "-adjusted_or_offset_to_offset") == 0)
          {
              lastool.parse_arg_cnt_check(i, 1, "time_offset");
              if (sscanf_las(argv[i + 1], "%u", &set_time_offset) != 1)
              {
                  lastool.error_parse_arg_n_invalid(i, 1);
              }
              if (set_time_offset < 0) {
                  set_time_offset = 0;
              }
              verify_current_gps_time_type = gps_time_type::ADJUSTED_OR_OFFSET_GPS_TIME;
          }
          else if (strcmp(argv[i], "-week_to_offset") == 0)
          {
              lastool.parse_arg_cnt_check(i, 2, "week time_offset");
              // week only used in lastransform, so we can skip it here
              if (sscanf_las(argv[i + 2], "%u", &set_time_offset) != 1)
              {
                  lastool.error_parse_arg_n_invalid(i, 2);
              }
              if (set_time_offset < 0) {  
                  set_time_offset = 0;
              }
              verify_current_gps_time_type = gps_time_type::GPS_WEEK_TIME;
          }
          else if (strcmp(argv[i], "-offset_to_offset") == 0)
          {
              lastool.parse_arg_cnt_check(i, 1, "time_offset");
              if (sscanf_las(argv[i + 1], "%u", &set_time_offset) != 1)
              {
                  lastool.error_parse_arg_n_invalid(i, 1);
              }
              if (set_time_offset < 0) {
                  set_time_offset = 0;
              }
              verify_current_gps_time_type = gps_time_type::OFFSET_GPS_TIME;
          }
      }
      geoprojectionconverter.parse(argc, argv);
      lasreadopener.parse(argc, argv);
      laswriteopener.parse(argc, argv);
  }

  auto arg_local = [&](int& i) -> bool {
    if (strcmp(argv[i], "-subseq") == 0)
    {
      lastool.parse_arg_cnt_check(i, 2, "start stop");
      if (sscanf_las(argv[i + 1], "%lld", &subsequence_start) != 1)
      {
        lastool.error_parse_arg_n_invalid(i, 1);
      }
      if (sscanf_las(argv[i + 2], "%lld", &subsequence_stop) != 1)
      {
        lastool.error_parse_arg_n_invalid(i, 2);
      }
      i += 2;
    }
    else if (strcmp(argv[i], "-start_at_point") == 0)
    {
      lastool.parse_arg_cnt_check(i, 1, "index of start point");
      if (sscanf_las(argv[i + 1], "%lld", &subsequence_start) != 1)
      {
        lastool.error_parse_arg_n_invalid(i, 1);
      }
      i += 1;
    }
    else if (strcmp(argv[i], "-stop_at_point") == 0)
    {
      lastool.parse_arg_cnt_check(i, 1, "index of stop point");
      if (sscanf_las(argv[i + 1], "%lld", &subsequence_stop) != 1)
      {
        lastool.error_parse_arg_n_invalid(i, 1);
      }
      i += 1;
    }
    else if (strncmp(argv[i], "-set_", 5) == 0)
    {
      if (strncmp(argv[i], "-set_point_", 11) == 0)
      {
        if (strcmp(argv[i], "-set_point_type") == 0 || strcmp(argv[i], "-set_point_data_format") == 0)
        {
          lastool.parse_arg_cnt_check(i, 1, "type");
          if (sscanf_las(argv[i + 1], "%u", &set_point_data_format) != 1) {
            lastool.error_parse_arg_n_invalid(i, 1);
          } else {
            // preset point type 
            lasreadopener.set_point_type(set_point_data_format);
          }
          i++;
        }
        else if (strcmp(argv[i], "-set_point_data_record_length") == 0 || strcmp(argv[i], "-set_point_size") == 0)
        {
          lastool.parse_arg_cnt_check(i, 1, "size");
          if (sscanf_las(argv[i + 1], "%u", &set_point_data_record_length) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 1);
          }
          i++;
        }
        else
        {
          lastool.parse_arg_invalid_n(i);
        }
      }
      else if (strcmp(argv[i], "-set_global_encoding_gps_bit") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "0 or 1");
        if (sscanf_las(argv[i + 1], "%u", &set_global_encoding_gps_bit) != 1)
        {
          lastool.error_parse_arg_n_invalid(i, 1);
        }
        i += 1;
      }
      else if (strcmp(argv[i], "-set_version") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "major.minor");
        if (sscanf_las(argv[i + 1], "%u.%u", &set_version_major, &set_version_minor) != 2)
        {
          lastool.error_parse_arg_n_invalid(i, 1);
        }
        i += 1;
      }
      else if (strcmp(argv[i], "-set_version_major") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "major");
        if (sscanf_las(argv[i + 1], "%u", &set_version_major) != 1)
        {
          lastool.error_parse_arg_n_invalid(i, 1);
        }
        i += 1;
      }
      else if (strcmp(argv[i], "-set_version_minor") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "minor");
        if (sscanf_las(argv[i + 1], "%u", &set_version_minor) != 1)
        {
          lastool.error_parse_arg_n_invalid(i, 1);
        }
        i += 1;
      }
      else if (strcmp(argv[i], "-set_lastiling_buffer_flag") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "0 or 1");
        if (sscanf_las(argv[i + 1], "%u", &set_lastiling_buffer_flag) != 1)
        {
          lastool.error_parse_arg_n_invalid(i, 1);
        }
        if (set_lastiling_buffer_flag > 1)
        {
          lastool.parse_arg_cnt_check(i, 1, "0 or 1");
        }
        i += 1;
      }
      else if (strncmp(argv[i], "-set_ogc_wkt", 12) == 0)
      {
        if (!geoprojectionconverter.is_proj_request) {         
          if (strcmp(argv[i], "-set_ogc_wkt") == 0)
          {
            set_ogc_wkt = true;
            set_ogc_wkt_in_evlr = false;
          }
          else if (strcmp(argv[i], "-set_ogc_wkt_in_evlr") == 0)
          {
            set_ogc_wkt = true;
            set_ogc_wkt_in_evlr = true;
          }
          else
          {
            lastool.parse_arg_invalid_n(i);
          }
        }
        if ((i + 1) < argc)
        {
          if ((argv[i + 1][0] != '-') && (argv[i + 1][0] != '\0'))
          {
            set_ogc_wkt_string = argv[i + 1];
            i++;
          }
        }
      } else if (strncmp(argv[i], "-set_proj_wkt", 13) == 0) {
        if (!geoprojectionconverter.is_proj_request) {
          // When using the PROJ functionalities, the PROJ lib must be loaded dynamically
          geoprojectionconverter.is_proj_request = load_proj_library(nullptr, false);

          if (strcmp(argv[i], "-set_proj_wkt") == 0) {
            set_ogc_wkt = true;
            set_ogc_wkt_in_evlr = false;
          } else if (strcmp(argv[i], "-set_proj_wkt_in_evlr") == 0) {
            set_ogc_wkt = true;
            set_ogc_wkt_in_evlr = true;
          } else {
            lastool.parse_arg_invalid_n(i);
          }
        }
        if ((i + 1) < argc) {
          if ((argv[i + 1][0] != '-') && (argv[i + 1][0] != '\0')) {
            set_ogc_wkt_string = argv[i + 1];
            i++;
          }
        }
      }
      else if (strcmp(argv[i], "-set_attribute_scale") == 0)
      {
        lastool.parse_arg_cnt_check(i, 2, "index scale");
        if (set_attribute_scales < 5)
        {
          if (sscanf_las(argv[i + 1], "%u", &(set_attribute_scale_index[set_attribute_scales])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 1);
          }
          if (sscanf_las(argv[i + 2], "%lf", &(set_attribute_scale_scale[set_attribute_scales])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 2);
          }
          set_attribute_scales++;
        }
        else
        {
          laserror("cannot '%s' more than 5 times", argv[i]);
        }
        i += 2;
      }
      else if (strcmp(argv[i], "-set_attribute_offset") == 0)
      {
        lastool.parse_arg_cnt_check(i, 2, "index offset");
        if (set_attribute_offsets < 5)
        {
          if (sscanf_las(argv[i + 1], "%u", &(set_attribute_offset_index[set_attribute_offsets])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 1);
          }
          if (sscanf_las(argv[i + 2], "%lf", &(set_attribute_offset_offset[set_attribute_offsets])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 2);
          }
        }
        else
        {
          laserror("cannot '%s' more than 5 times", argv[i]);
        }
        i += 2;
      }
      else
      {
        lastool.parse_arg_invalid_n(i);
      }
    }
    else if (strncmp(argv[i], "-remove_", 8) == 0)
    {
      if (strcmp(argv[i], "-remove_padding") == 0)
      {
        remove_header_padding = true;
      }
      else if (strcmp(argv[i], "-remove_all_vlrs") == 0)
      {
        remove_all_variable_length_records = true;
      }
      else if (strcmp(argv[i], "-remove_vlr") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "number");
        remove_variable_length_record = atoi(argv[i + 1]);
        remove_variable_length_record_from = -1;
        remove_variable_length_record_to = -1;
        i++;
      }
      else if (strcmp(argv[i], "-remove_vlrs_from_to") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "start end");
        remove_variable_length_record = -1;
        remove_variable_length_record_from = atoi(argv[i + 1]);
        remove_variable_length_record_to = atoi(argv[i + 2]);
        i += 2;
      }
      else if (strcmp(argv[i], "-remove_all_evlrs") == 0)
      {
        remove_all_extended_variable_length_records = true;
      }
      else if (strcmp(argv[i], "-remove_evlr") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "number");
        remove_extended_variable_length_record = atoi(argv[i + 1]);
        remove_extended_variable_length_record_from = -1;
        remove_extended_variable_length_record_to = -1;
        i++;
      }
      else if (strcmp(argv[i], "-remove_evlrs_from_to") == 0)
      {
        lastool.parse_arg_cnt_check(i, 2, "start end");
        remove_extended_variable_length_record = -1;
        remove_extended_variable_length_record_from = atoi(argv[i + 1]);
        remove_extended_variable_length_record_to = atoi(argv[i + 2]);
        i += 2;
      }
      else if (strcmp(argv[i], "-remove_tiling_vlr") == 0)
      {
        remove_tiling_vlr = true;
      }
      else if (strcmp(argv[i], "-remove_original_vlr") == 0)
      {
        remove_original_vlr = true;
      }
      else
      {
        lastool.parse_arg_invalid_n(i);
      }
    }
    else if (strncmp(argv[i], "-add_", 5) == 0)
    {
      if (strcmp(argv[i], "-add_attribute") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "data_type name description ...");
        if (((i + 4) < argc) && (atof(argv[i + 4]) != 0.0))
        {
          if (((i + 5) < argc) && ((atof(argv[i + 5]) != 0.0) || (strcmp(argv[i + 5], "0") == 0) || (strcmp(argv[i + 5], "0.0") == 0)))
          {
            if (((i + 6) < argc) && ((atof(argv[i + 6]) != 0.0) || (strcmp(argv[i + 6], "0") == 0) || (strcmp(argv[i + 6], "0.0") == 0)))
            {
              lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]), 1.0, 0.0, atof(argv[i + 6]));
              i += 6;
            }
            else
            {
              lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]));
              i += 5;
            }
          }
          else
          {
            lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]));
            i += 4;
          }
        }
        else
        {
          lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3]);
          i += 3;
        }
      }
      else if (strcmp(argv[i], "-add_empty_vlr") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "user_ID record_ID ...");
        add_empty_vlr_user_ID = argv[i + 1];
        add_empty_vlr_record_ID = atoi(argv[i + 2]);
        i += 2;
        if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
        {
          add_empty_vlr_description = argv[i + 1];
          i += 1;
        }
      }
      else
      {
        lastool.parse_arg_invalid_n(i);
      }
    }
    else if (strncmp(argv[i], "-unset_", 7) == 0)
    {
      if (strcmp(argv[i], "-unset_attribute_scale") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "index");
        if (unset_attribute_scales < 5)
        {
          if (sscanf_las(argv[i + 1], "%u", &(unset_attribute_scale_index[unset_attribute_scales])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 1);
          }
          unset_attribute_scales++;
        }
        else
        {
          laserror("cannot '%s' more than 5 times", argv[i]);
        }
        i += 1;
      }
      else if (strcmp(argv[i], "-unset_attribute_offset") == 0)
      {
        lastool.parse_arg_cnt_check(i, 1, "index");
        if (unset_attribute_offsets < 5)
        {
          if (sscanf_las(argv[i + 1], "%u", &(unset_attribute_offset_index[unset_attribute_offsets])) != 1)
          {
            lastool.error_parse_arg_n_invalid(i, 1);
          }
          unset_attribute_offsets++;
        }
        else
        {
          laserror("cannot '%s' more than 5 times", argv[i]);
        }
        i += 1;
      }
      else
      {
        lastool.parse_arg_invalid_n(i);
      }
    }
    else if (strcmp(argv[i], "-move_evlrs_to_vlrs") == 0)
    {
      move_evlrs_to_vlrs = true;
    }
    else if (strcmp(argv[i], "-save_vlrs") == 0)
    {
      save_vlrs = true;
    }
    else if (strcmp(argv[i], "-load_vlrs") == 0)
    {
      load_vlrs = true;
    }
    else if (strcmp(argv[i], "-save_vlr") == 0)
    {
      save_vlr = true;
      parse_save_load_vlr_args(i, argc, argv, save_vlr, vlr_index, vlr_user_id, vlr_record_id, vlr_filename);
    }
    else if (strcmp(argv[i], "-load_vlr") == 0)
    {
      load_vlr = true;
      parse_save_load_vlr_args(i, argc, argv, save_vlr, vlr_index, vlr_user_id, vlr_record_id, vlr_filename);
    }
    else if (strcmp(argv[i], "-load_ogc_wkt") == 0)
    {
      lastool.parse_arg_cnt_check(i, 1, "file name");
      if ((argv[i + 1][0] != '-') && (argv[i + 1][0] != '\0'))
      {
        FILE* file = LASfopen(argv[i + 1], "r");

        if (file)
        {
          set_ogc_wkt = true;
          set_ogc_wkt_in_evlr = false;
          U32 buff_size = 5; I32 c = 0; U32 k = 0;
          set_ogc_wkt_string = (CHAR*)calloc(buff_size, sizeof(CHAR));

          while (c != EOF && c != '\n' && set_ogc_wkt_string != nullptr)
          {
            c = fgetc(file);

            if (k == buff_size)
              set_ogc_wkt_string = (CHAR*)realloc_las(set_ogc_wkt_string, (buff_size *= 2) * sizeof(CHAR));

            if (c == EOF || c == '\n')
              set_ogc_wkt_string[k] = '\0';
            else
              set_ogc_wkt_string[k++] = (CHAR)c;
          }

          fclose(file);
          i++;
        }
        else
        {
          laserror("cannot open file '%s' for read", argv[i + 1]);
        }
      }
      else
      {
        lastool.parse_arg_cnt_check(i, 1, "file name");
      }
    }
    else if (strcmp(argv[i], "-dont_remove_empty_files") == 0)
    {
      remove_empty_files = false;
    }
    // lasfilter abort on -clip_... due obsolete in general. we change -clip_ to -crop
    else if (strcmp(argv[i], "-crop_to_bounding_box") == 0 || strcmp(argv[i], "-crop_to_bb") == 0)
    {
      clip_to_bounding_box = true;
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
    return las2las_gui(argc, argv, &lasreadopener);
  }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
  if (lastool.cores > 1)
  {
    if (lasreadopener.get_use_stdin())
    {
      LASMessage(LAS_WARNING, "using stdin. ignoring '-cores %d' ...", lastool.cores);
    }
    else if (lasreadopener.get_file_name_number() < 2)
    {
      LASMessage(LAS_WARNING, "only %u input files. ignoring '-cores %d' ...", lasreadopener.get_file_name_number(), lastool.cores);
    }
    else if (lasreadopener.is_merged())
    {
      LASMessage(LAS_WARNING, "input files merged on-the-fly. ignoring '-cores %d' ...", lastool.cores);
    }
    else
    {
      las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, lastool.cores, lastool.cpu64);
    }
  }
  if (lastool.cpu64)
  {
    las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active())
  {
    laserror("no input specified");
  }

  // check proj options

  if (geoprojectionconverter.is_proj_request == true) {
    geoprojectionconverter.load_proj();
  }

  BOOL extra_pass = laswriteopener.is_piped();

  // we only really need an extra pass if the coordinates are altered or if points are filtered

  if (extra_pass)
  {
    if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
    {
      extra_pass = FALSE;
    }
  }

  // for piped output we need an extra pass

  if (extra_pass)
  {
    if (lasreadopener.is_piped())
    {
      laserror("input and output cannot both be piped");
    }
  }
    // only save or load
    if (save_vlrs && load_vlrs)
  {
    laserror("cannot save and load VLRs at the same time");
  }
  if (save_vlr && load_vlr)
  {
    laserror("cannot save and load VLR at the same time");
  }

  // only save/load a single vlr or multible vlrs
    if (save_vlrs && save_vlr)
  {
    laserror("cannot save a single VLR and multiple VLRs at the same time");
  }
  if (load_vlrs && load_vlr)
  {
    laserror("cannot load a single VLR and multiple VLRs at the same time");
  }

  // possibly loop over multiple input files
  while (lasreadopener.active())
  {
    try
    {
      start_time = taketime();

      // open lasreader

      LASreader* lasreader = lasreadopener.open();

      if (lasreader == 0)
      {
        laserror("could not open lasreader");
      }

      // store the inventory for the header

      LASinventory lasinventory;

      // the point we write sometimes needs to be copied

      LASpoint* point = 0;

      // prepare the header for output
      
      /*
      if (set_global_encoding_gps_bit != -1)
      {
        if (set_global_encoding_gps_bit == 0)
        {
          if ((lasreader->header.global_encoding & 1) == 0)
          {
            lastool.laswarnforce("global encoding indicates file already in GPS week time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
          }
          else
          {
            lasreader->header.global_encoding &= ~1;
          }
        }
        else if (set_global_encoding_gps_bit == 1)
        {
          if ((lasreader->header.global_encoding & 1) == 1)
          {
            lastool.laswarnforce("global encoding indicates file already in Adjusted Standard GPS time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
          }
          else
          {
            lasreader->header.global_encoding |= 1;
          }
        }
        else
        {
          LASMessage(LAS_WARNING, "ignoring invalid option '-set_global_encoding_gps_bit %d'", set_global_encoding_gps_bit);
        }
      }
      */

      // verify gps time type matches specified source type
      if (verify_current_gps_time_type != gps_time_type::NONE) {
          if (verify_current_gps_time_type == gps_time_type::GPS_WEEK_TIME) {
              if ((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE)) != 0)
              {
                  lastool.laswarnforce("global encoding indicates file is not in GPS week time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
              }
          }
          else if (verify_current_gps_time_type == gps_time_type::ADJUSTED_GPS_TIME) {
              if (((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE)) == 0) || ((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG)) != 0))
              {
                  lastool.laswarnforce("global encoding indicates file is not in Adjusted Standard GPS time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
              }
          }
          else if (verify_current_gps_time_type == gps_time_type::OFFSET_GPS_TIME) {
              if (((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE)) == 0) || ((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG)) == 0))
              {
                  lastool.laswarnforce("global encoding indicates file is not in Offset GPS time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
              }
          }
          else if (verify_current_gps_time_type == gps_time_type::ADJUSTED_OR_OFFSET_GPS_TIME) {
              if (((lasreader->header.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE)) == 0))
              {
                  lastool.laswarnforce("global encoding indicates file is neither in Adjusted Standard GPS nor in Offset GPS time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
              }
          }
      }
      else {
          // if we do not convert and still set options, we keep the old checks for consistency 
          if (set_global_encoding_gps_bit != -1)
          {
              if (set_global_encoding_gps_bit == 0)
              {
                  if ((lasreader->header.global_encoding & 1) == 0)
                  {
                      lastool.laswarnforce("global encoding indicates file already in GPS week time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
                  }
              }
              else if (set_global_encoding_gps_bit == 1)
              {
                  if ((lasreader->header.global_encoding & 1) == 1)
                  {
                      lastool.laswarnforce("global encoding indicates file already in Adjusted Standard GPS time. %s", (lastool.force ? "Forced conversion" : "Use '-force' to force conversion"));
                  }
              }
          }
      }

      // offset gps time?
      if (set_time_offset > -1) {
          if ((lasreader->header.version_minor >= 5) || (set_version_minor >= 5))
          {
              if (set_global_encoding_gps_bit > -1) {
                  laserror("cannot set global encoding gps bit and set Offset GPS Time at the same time.");
              }
              lasreader->header.global_encoding |= ((1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE) | (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG));
              lasreader->header.time_offset = set_time_offset;
          }
          else {
              laserror("LAS %d.%d does not support Offset GPS Time, require at least LAS 1.5.", lasreader->header.version_major, (set_version_minor >= 0 ? set_version_minor : lasreader->header.version_minor));
          }
      }
      else if (set_global_encoding_gps_bit != -1) {
          if (set_global_encoding_gps_bit == 0) {
              lasreader->header.global_encoding &= ~((1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE) | (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG));
              lasreader->header.time_offset = 0;
          }
          else if (set_global_encoding_gps_bit == 1) {
              lasreader->header.global_encoding |= (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE);
              lasreader->header.global_encoding &= ~(1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG);
              lasreader->header.time_offset = 0;
          }
          else
          {
              LASMessage(LAS_WARNING, "ignoring invalid option '-set_global_encoding_gps_bit %d'", set_global_encoding_gps_bit);
          }
      }

      if (set_attribute_scales)
      {
        for (i = 0; i < set_attribute_scales; i++)
        {
          if (set_attribute_scale_index[i] != -1)
          {
            if (set_attribute_scale_index[i] < lasreader->header.number_attributes)
            {
              lasreader->header.attributes[set_attribute_scale_index[i]].set_scale(set_attribute_scale_scale[i]);
            }
            else
            {
              laserror("attribute index %d out-of-range. only %d attributes in file. ignoring ... ", set_attribute_scale_index[i], lasreader->header.number_attributes);
            }
          }
        }
      }

      if (set_attribute_offsets)
      {
        for (i = 0; i < set_attribute_offsets; i++)
        {
          if (set_attribute_offset_index[i] != -1)
          {
            if (set_attribute_offset_index[i] < lasreader->header.number_attributes)
            {
              lasreader->header.attributes[set_attribute_offset_index[i]].set_offset(set_attribute_offset_offset[i]);
            }
            else
            {
              laserror("attribute index %d out-of-range. only %d attributes in file. ignoring ... ", set_attribute_offset_index[i], lasreader->header.number_attributes);
            }
          }
        }
      }

      if (unset_attribute_scales)
      {
        for (i = 0; i < unset_attribute_scales; i++)
        {
          if (unset_attribute_scale_index[i] != -1)
          {
            if (unset_attribute_scale_index[i] < lasreader->header.number_attributes)
            {
              lasreader->header.attributes[unset_attribute_scale_index[i]].unset_scale();
            }
            else
            {
              laserror("attribute index %d out-of-range. only %d attributes in file. ignoring ... ", unset_attribute_scale_index[i], lasreader->header.number_attributes);
            }
          }
        }
      }

      if (unset_attribute_offsets)
      {
        for (i = 0; i < unset_attribute_offsets; i++)
        {
          if (unset_attribute_offset_index[i] != -1)
          {
            if (unset_attribute_offset_index[i] < lasreader->header.number_attributes)
            {
              lasreader->header.attributes[unset_attribute_offset_index[i]].unset_offset();
            }
            else
            {
              laserror("attribute index %d out-of-range. only %d attributes in file. ignoring ... ", unset_attribute_offset_index[i], lasreader->header.number_attributes);
            }
          }
        }
      }

      if (set_attribute_scales || set_attribute_offsets || unset_attribute_scales || unset_attribute_offsets)
      {
        lasreader->header.update_extra_bytes_vlr();
      }

      if (set_point_data_format > 5)
      {
        if (set_version_minor == -1) 
        {
            if (lasreader->header.version_minor < 4) {
                set_version_minor = 4;
            }
        }
      }

      if (set_version_major != -1)
      {
        if (set_version_major != 1)
        {
          laserror("unknown version_major %d", set_version_major);
        }
        lasreader->header.version_major = (U8)set_version_major;
      }

      if (set_version_minor >= 0)
      {
        if (set_version_minor > 5)
        {
          laserror("unknown version_minor %d", set_version_minor);
        }
        if (set_version_minor < 3)
        {
          if (lasreader->header.version_minor == 3)
          {
            lasreader->header.header_size -= 8;
            lasreader->header.offset_to_point_data -= 8;
          }
          else if (lasreader->header.version_minor == 4)
          {
            lasreader->header.header_size -= (8 + 140);
            lasreader->header.offset_to_point_data -= (8 + 140);
          }
          else if (lasreader->header.version_minor >= 5)
          {
              lasreader->header.header_size -= (8 + 140 + 18);
              lasreader->header.offset_to_point_data -= (8 + 140 + 18);
          }
        }
        else if (set_version_minor == 3)
        {
          if (lasreader->header.version_minor < 3)
          {
            lasreader->header.header_size += 8;
            lasreader->header.offset_to_point_data += 8;
            lasreader->header.start_of_waveform_data_packet_record = 0;
          }
          else if (lasreader->header.version_minor == 4)
          {
            lasreader->header.header_size -= 140;
            lasreader->header.offset_to_point_data -= 140;
          }
          else if (lasreader->header.version_minor >= 5)
          {
              lasreader->header.header_size -= (140 + 18);
              lasreader->header.offset_to_point_data -= (140 + 18);
          }
        }
        else if (set_version_minor == 4)
        {
          if (lasreader->header.version_minor < 3)
          {
            lasreader->header.header_size += (8 + 140);
            lasreader->header.offset_to_point_data += (8 + 140);
            lasreader->header.start_of_waveform_data_packet_record = 0;
          }
          else if (lasreader->header.version_minor == 3)
          {
            lasreader->header.header_size += 140;
            lasreader->header.offset_to_point_data += 140;
          }
          else if (lasreader->header.version_minor >= 5)
          {
              lasreader->header.header_size -= 18;
              lasreader->header.offset_to_point_data -= 18;
          }

          if (lasreader->header.version_minor < 4)
          {
            if (set_point_data_format > 5)
            {
              lasreader->header.extended_number_of_point_records = lasreader->header.number_of_point_records;
              lasreader->header.number_of_point_records = 0;
              for (i = 0; i < 5; i++)
              {
                lasreader->header.extended_number_of_points_by_return[i] = lasreader->header.number_of_points_by_return[i];
                lasreader->header.number_of_points_by_return[i] = 0;
              }
            }
          }
        }
        else if (set_version_minor == 5)
        {
            if (lasreader->header.version_minor < 3)
            {
                lasreader->header.header_size += (8 + 140 + 18);
                lasreader->header.offset_to_point_data += (8 + 140 + 18);
                lasreader->header.start_of_waveform_data_packet_record = 0;
            }
            else if (lasreader->header.version_minor == 3)
            {
                lasreader->header.header_size += (140 + 18);
                lasreader->header.offset_to_point_data += (140 + 18);
            }
            else if (lasreader->header.version_minor == 4)
            {
                lasreader->header.header_size += 18;
                lasreader->header.offset_to_point_data += 18;
            }

            if (lasreader->header.version_minor < 4)
            {
                if (set_point_data_format > 5)
                {
                    lasreader->header.extended_number_of_point_records = lasreader->header.number_of_point_records;
                    lasreader->header.number_of_point_records = 0;
                    for (i = 0; i < 5; i++)
                    {
                        lasreader->header.extended_number_of_points_by_return[i] = lasreader->header.number_of_points_by_return[i];
                        lasreader->header.number_of_points_by_return[i] = 0;
                    }
                }
            }
        }

        if ((set_version_minor <= 3) && (lasreader->header.version_minor >= 4))
        {
          if (lasreader->header.point_data_format > 5)
          {
            switch (lasreader->header.point_data_format)
            {
            case 6:
              LASMessage(LAS_WARNING, "downgrading point_data_format from %d to 1 and point_data_record_length from %d to %d",
                lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
              lasreader->header.point_data_format = 1;
              lasreader->header.point_data_record_length -= 2;
              break;
            case 7:
              LASMessage(LAS_WARNING, "downgrading point_data_format from %d to 3 and point_data_record_length from %d to %d",
                lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
              lasreader->header.point_data_format = 3;
              lasreader->header.point_data_record_length -= 2;
              break;
            case 8:
              LASMessage(LAS_WARNING, "downgrading point_data_format from %d to 3 and point_data_record_length from %d to %d",
                lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
              lasreader->header.point_data_format = 3;
              lasreader->header.point_data_record_length -= 4;
              break;
            case 9:
              LASMessage(LAS_WARNING, "downgrading point_data_format from %d to 4 and point_data_record_length from %d to %d",
                lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
              lasreader->header.point_data_format = 4;
              lasreader->header.point_data_record_length -= 2;
              break;
            case 10:
              LASMessage(LAS_WARNING, "downgrading point_data_format from %d to 5 and point_data_record_length from %d to %d",
                lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
              lasreader->header.point_data_format = 5;
              lasreader->header.point_data_record_length -= 4;
              break;
            default:
              laserror("unknown point_data_format %d", lasreader->header.point_data_format);
            }
            point = new LASpoint;
            lasreader->header.clean_laszip();
          }
          if (lasreader->header.get_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS))
          {
            LASMessage(LAS_WARNING, "unsetting global encoding bit %d when downgrading from version 1.%d to version 1.%d",
              LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS, lasreader->header.version_minor, set_version_minor);
            lasreader->header.unset_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
          }
          if (lasreader->header.number_of_extended_variable_length_records)
          {
            LASMessage(LAS_WARNING, "loosing %d EVLR%s when downgrading from version 1.%d to version 1.%d. attempting to move %s to the VLR section ...",
              lasreader->header.number_of_extended_variable_length_records, (lasreader->header.number_of_extended_variable_length_records > 1 ? "s" : ""),
              lasreader->header.version_minor, set_version_minor, (lasreader->header.number_of_extended_variable_length_records > 1 ? "them" : "it"));
            U32 u;
            for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
            {
              if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
              {
                lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
                lasreader->header.evlrs[u].data = 0;
                LASMessage(LAS_INFO, "         moved EVLR %d with user ID '%s' and %lld bytes of payload", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
              }
              else
              {
                LASMessage(LAS_INFO, "         lost EVLR %d with user ID '%s' and %lld bytes of payload", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
              }
            }
          }
        }
       else if ((set_version_minor >= 5) && (lasreader->header.point_data_format <= 5)) {
            // LAS 1.5 does not support point data formats 0 - 5 anymore, upgrade them
            if (set_point_data_format == -1) // we accept unsupported point format if user requests it
            {
                switch (lasreader->header.point_data_format)
                {
                case 0:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 6 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 10);
                    lasreader->header.point_data_format = 6;
                    lasreader->header.point_data_record_length += 10;
                    break;
                case 1:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 6 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 2);
                    lasreader->header.point_data_format = 6;
                    lasreader->header.point_data_record_length += 2;
                    break;
                case 2:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 7 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 10);
                    lasreader->header.point_data_format = 7;
                    lasreader->header.point_data_record_length += 10;
                    break;
                case 3:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 7 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 2);
                    lasreader->header.point_data_format = 7;
                    lasreader->header.point_data_record_length += 2;
                    break;
                case 4:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 9 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 2);
                    lasreader->header.point_data_format = 9;
                    lasreader->header.point_data_record_length += 2;
                    break;
                case 5:
                    LASMessage(LAS_WARNING, "upgrading point_data_format from %d to 10 and point_data_record_length from %d to %d",
                        lasreader->header.point_data_format, lasreader->header.point_data_record_length, lasreader->header.point_data_record_length + 4);
                    lasreader->header.point_data_format = 10;
                    lasreader->header.point_data_record_length += 4;
                    break;
                default:
                    laserror("unknown point_data_format %d", lasreader->header.point_data_format);
                }
                point = new LASpoint;
                lasreader->header.clean_laszip();
            }
        }

        lasreader->header.version_minor = (U8)set_version_minor;
      }

      // are we supposed to change the point data format

      if (set_point_data_format != -1)
      {
        if (set_point_data_format < 0 || set_point_data_format > 10)
        {
          laserror("unknown point_data_format %d", set_point_data_format);
        }
        // depending on the conversion we may need to copy the point
        if (convert_point_type_from_to[lasreader->header.point_data_format][set_point_data_format])
        {
          if (point == 0) point = new LASpoint;
        }
        // were there extra bytes before
        I32 num_extra_bytes = 0;
        switch (lasreader->header.point_data_format)
        {
        case 0:
          num_extra_bytes = lasreader->header.point_data_record_length - 20;
          break;
        case 1:
          num_extra_bytes = lasreader->header.point_data_record_length - 28;
          break;
        case 2:
          num_extra_bytes = lasreader->header.point_data_record_length - 26;
          break;
        case 3:
          num_extra_bytes = lasreader->header.point_data_record_length - 34;
          break;
        case 4:
          num_extra_bytes = lasreader->header.point_data_record_length - 57;
          break;
        case 5:
          num_extra_bytes = lasreader->header.point_data_record_length - 63;
          break;
        case 6:
          num_extra_bytes = lasreader->header.point_data_record_length - 30;
          break;
        case 7:
          num_extra_bytes = lasreader->header.point_data_record_length - 36;
          break;
        case 8:
          num_extra_bytes = lasreader->header.point_data_record_length - 38;
          break;
        case 9:
          num_extra_bytes = lasreader->header.point_data_record_length - 59;
          break;
        case 10:
          num_extra_bytes = lasreader->header.point_data_record_length - 67;
          break;
        }
        if (num_extra_bytes < 0)
        {
          laserror("point record length has %d fewer bytes than needed", -num_extra_bytes);
        }
        lasreader->header.point_data_format = (U8)set_point_data_format;
        lasreader->header.clean_laszip();
        switch (lasreader->header.point_data_format)
        {
        case 0:
          lasreader->header.point_data_record_length = 20 + num_extra_bytes;
          break;
        case 1:
          lasreader->header.point_data_record_length = 28 + num_extra_bytes;
          break;
        case 2:
          lasreader->header.point_data_record_length = 26 + num_extra_bytes;
          break;
        case 3:
          lasreader->header.point_data_record_length = 34 + num_extra_bytes;
          break;
        case 4:
          lasreader->header.point_data_record_length = 57 + num_extra_bytes;
          break;
        case 5:
          lasreader->header.point_data_record_length = 63 + num_extra_bytes;
          break;
        case 6:
          lasreader->header.point_data_record_length = 30 + num_extra_bytes;
          break;
        case 7:
          lasreader->header.point_data_record_length = 36 + num_extra_bytes;
          break;
        case 8:
          lasreader->header.point_data_record_length = 38 + num_extra_bytes;
          break;
        case 9:
          lasreader->header.point_data_record_length = 59 + num_extra_bytes;
          break;
        case 10:
          lasreader->header.point_data_record_length = 67 + num_extra_bytes;
          break;
        }
      }

      // are we supposed to change the point data record length

      if (set_point_data_record_length != -1)
      {
        I32 num_extra_bytes = 0;
        switch (lasreader->header.point_data_format)
        {
        case 0:
          num_extra_bytes = set_point_data_record_length - 20;
          break;
        case 1:
          num_extra_bytes = set_point_data_record_length - 28;
          break;
        case 2:
          num_extra_bytes = set_point_data_record_length - 26;
          break;
        case 3:
          num_extra_bytes = set_point_data_record_length - 34;
          break;
        case 4:
          num_extra_bytes = set_point_data_record_length - 57;
          break;
        case 5:
          num_extra_bytes = set_point_data_record_length - 63;
          break;
        case 6:
          num_extra_bytes = set_point_data_record_length - 30;
          break;
        case 7:
          num_extra_bytes = set_point_data_record_length - 36;
          break;
        case 8:
          num_extra_bytes = set_point_data_record_length - 38;
          break;
        case 9:
          num_extra_bytes = set_point_data_record_length - 59;
          break;
        case 10:
          num_extra_bytes = set_point_data_record_length - 67;
          break;
        }
        if (num_extra_bytes < 0)
        {
          laserror("point_data_format %d needs record length of at least %d", lasreader->header.point_data_format, set_point_data_record_length - num_extra_bytes);
        }
        if (lasreader->header.point_data_record_length < set_point_data_record_length)
        {
          if (!point) point = new LASpoint;
        }
        lasreader->header.point_data_record_length = (U16)set_point_data_record_length;
        lasreader->header.clean_laszip();
      }

      // are we supposed to add attributes

      if (lasreadopener.get_number_attributes())
      {
        I32 attibutes_before_size = lasreader->header.get_attributes_size();
        for (i = 0; i < lasreadopener.get_number_attributes(); i++)
        {
          I32 type = (lasreadopener.get_attribute_data_type(i) - 1) % 10;
          try {
            LASattribute attribute(type, lasreadopener.get_attribute_name(i), lasreadopener.get_attribute_description(i));
            if (lasreadopener.get_attribute_scale(i) != 1.0 || lasreadopener.get_attribute_offset(i) != 0.0)
            {
              attribute.set_scale(lasreadopener.get_attribute_scale(i));
            }
            if (lasreadopener.get_attribute_offset(i) != 0.0)
            {
              attribute.set_offset(lasreadopener.get_attribute_offset(i));
            }
            if (lasreadopener.get_attribute_no_data(i) != F64_MAX)
            {
              attribute.set_no_data(lasreadopener.get_attribute_no_data(i));
            }
            lasreader->header.add_attribute(attribute);
          }
          catch (...) {
            laserror("initializing attribute %s", lasreadopener.get_attribute_name(i));
          }
        }
        I32 attibutes_after_size = lasreader->header.get_attributes_size();
        if (!point) point = new LASpoint;
        lasreader->header.update_extra_bytes_vlr();
        lasreader->header.point_data_record_length += (attibutes_after_size - attibutes_before_size);
        lasreader->header.clean_laszip();
      }

      if (set_lastiling_buffer_flag != -1)
      {
        if (lasreader->header.vlr_lastiling)
        {
          lasreader->header.vlr_lastiling->buffer = set_lastiling_buffer_flag;
        }
        else
        {
          LASMessage(LAS_WARNING, "file '%s' has no LAStiling VLR. cannot set buffer flag.", lasreadopener.get_file_name());
        }
      }

      if (move_evlrs_to_vlrs)
      {
        if (lasreader->header.number_of_extended_variable_length_records > 0)
        {
          U32 u;
          for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
          {
            if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
            {
              lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
              lasreader->header.evlrs[u].data = 0;
              LASMessage(LAS_VERY_VERBOSE, "moved EVLR %d with user ID '%s' and %lld bytes of payload", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
            }
          }
          U32 remaining = 0;
          for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
          {
            if (lasreader->header.evlrs[u].record_length_after_header > U16_MAX)
            {
              lasreader->header.evlrs[remaining] = lasreader->header.evlrs[u];
              remaining++;
            }
          }
          LASMessage(LAS_VERBOSE, "moved %u EVLRs to VLRs. %u EVLRs with large payload remain.", u - remaining, remaining);
          lasreader->header.number_of_extended_variable_length_records = remaining;
        }
      }

      // if the point needs to be copied set up the data fields

      if (point)
      {
        point->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);
      }
      
      // reproject or just set the projection?
      LASquantizer* reproject_quantizer = 0;
      bool set_projection_in_header = false;
      bool set_wkt_global_encoding_bit = false;

      if (geoprojectionconverter.has_projection(false)) // reproject because a target projection was provided in the command line
      {
        if (!geoprojectionconverter.has_projection(true))      // if no source projection was provided in the command line ...
        {
          if (lasreader->header.vlr_geo_ogc_wkt)               // ... try to get it from the OGC WKT string ...
          {
            geoprojectionconverter.set_projection_from_ogc_wkt(lasreader->header.vlr_geo_ogc_wkt);
          }
          if (!geoprojectionconverter.has_projection(true))    // ... nothing ... ? ...
          {
            if (lasreader->header.vlr_geo_keys)                // ... try to get it from the geo keys.
            {
              geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
            }
          }
        }
        if (!geoprojectionconverter.has_projection(true))
        {
          LASMessage(LAS_WARNING, "cannot determine source projection of '%s'. not reprojecting ... ", lasreadopener.get_file_name());

          set_projection_in_header = false;
        }
        else
        {
          geoprojectionconverter.check_horizontal_datum_before_reprojection();

          reproject_quantizer = new LASquantizer();
          double point[3];
          point[0] = (lasreader->header.min_x + lasreader->header.max_x) / 2;
          point[1] = (lasreader->header.min_y + lasreader->header.max_y) / 2;
          point[2] = (lasreader->header.min_z + lasreader->header.max_z) / 2;
          geoprojectionconverter.to_target(point);
          if (!geoprojectionconverter.has_target_precision() && lasreadopener.get_scale_factor() != nullptr &&
              lasreadopener.get_scale_factor()[0] > 0.0) {
            LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for x scaling", lasreadopener.get_scale_factor()[0]);
            reproject_quantizer->x_scale_factor = lasreadopener.get_scale_factor()[0];
          } else {
            LASMessage(LAS_VERBOSE, "target precision is used for x scaling");
            reproject_quantizer->x_scale_factor = geoprojectionconverter.get_target_precision(lasreader->header.x_scale_factor);
          }
          if (!geoprojectionconverter.has_target_precision() && lasreadopener.get_scale_factor() != nullptr &&
              lasreadopener.get_scale_factor()[1] > 0.0) {
            LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for y scaling", lasreadopener.get_scale_factor()[1]);
            reproject_quantizer->y_scale_factor = lasreadopener.get_scale_factor()[1];
          } else {
            LASMessage(LAS_VERBOSE, "target precision is used for y scaling");
            reproject_quantizer->y_scale_factor = geoprojectionconverter.get_target_precision(lasreader->header.y_scale_factor);
          }
          if (!geoprojectionconverter.has_target_elevation_precision() && lasreadopener.get_scale_factor() != nullptr &&
              lasreadopener.get_scale_factor()[2] > 0.0) {
            LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for z scaling", lasreadopener.get_scale_factor()[2]);
            reproject_quantizer->z_scale_factor = lasreadopener.get_scale_factor()[2];
          } else {
            LASMessage(LAS_VERBOSE, "target elevation precision is used for z scaling");
            reproject_quantizer->z_scale_factor = geoprojectionconverter.get_target_elevation_precision(lasreader->header.z_scale_factor);
          }      
          reproject_quantizer->x_offset = ((I64)((point[0] / reproject_quantizer->x_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->x_scale_factor;
          reproject_quantizer->y_offset = ((I64)((point[1] / reproject_quantizer->y_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->y_scale_factor;
          reproject_quantizer->z_offset = ((I64)((point[2] / reproject_quantizer->z_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->z_scale_factor;

          set_projection_in_header = true;
        }
      }
      else if (geoprojectionconverter.has_projection(true)) // set because only a source projection was provided in the command line
      {
        set_projection_in_header = true;
      }
      // PROJ transformation
      if (geoprojectionconverter.is_proj_request && !set_ogc_wkt) 
      {
        LASMessage(LAS_VERY_VERBOSE, "the PROJ transformation is prepared within las2las");
        // 1. If the source CRS is not specified as an argument (cmd line), try to generate it from the input file
        if (geoprojectionconverter.check_header_for_crs) 
        {      
          LASMessage(LAS_VERY_VERBOSE, "the header of the input file is checked for its CRS");
          // 2. try to get it the source CRS from the OGC header WKT string
          if (lasreader->header.vlr_geo_ogc_wkt) 
          {
            geoprojectionconverter.set_proj_crs_with_file_header_wkt(lasreader->header.vlr_geo_ogc_wkt, true);
          } 
          else if (lasreader->header.vlr_geo_keys) 
          {
            geoprojectionconverter.disable_messages = true;

            geoprojectionconverter.set_projection_from_geo_keys(
                lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries,
                lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);

            CHAR* ogc_wkt = nullptr;
            I32 len = 0;
            // 3. if no WKT exist in file header try to generate WKT from GeoTiff and lastool
            if (geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt))
            {
              geoprojectionconverter.set_proj_crs_with_file_header_wkt(ogc_wkt, true);
            }
            // 4. if no WKT can be generated from GeoTiff try to get die EPSG from GeoTiff
            else if (geoprojectionconverter.source_header_epsg)
            {
              LASMessage(LAS_WARNING, "No valid WKT could be generated from the GeoTiff of the source file. The EPSG code from the GeoTiff is used for the transformation, "
                             "this can lead to loss of GeoTiff arguments, which can lead to inaccuracies or data loss during the transformation.");
              geoprojectionconverter.set_proj_crs_with_epsg(geoprojectionconverter.source_header_epsg, true);
            }
            else 
            {
              laserror("No valid CRS could be extracted from the header information of the source file. Please specify the coordinate system (CRS) directly when calling the transformation tool.");
            }
            geoprojectionconverter.reset_projection();
          } 
          else 
          {
            laserror("No file header information could be found to identify the CRS.");
          }
          // Create the transformation PROJ object
          geoprojectionconverter.set_proj_crs_transform();
        }
        reproject_quantizer = new LASquantizer();
        double point[3];
        point[0] = (lasreader->header.min_x + lasreader->header.max_x) / 2;
        point[1] = (lasreader->header.min_y + lasreader->header.max_y) / 2;
        point[2] = (lasreader->header.min_z + lasreader->header.max_z) / 2;
        geoprojectionconverter.to_target(point);
        if (!geoprojectionconverter.has_target_precision() && lasreadopener.get_scale_factor() != nullptr &&
            lasreadopener.get_scale_factor()[0] > 0.0) {
          LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for x scaling", lasreadopener.get_scale_factor()[0]);
          reproject_quantizer->x_scale_factor = lasreadopener.get_scale_factor()[0];
        } else {
          LASMessage(LAS_VERBOSE, "target precision is used for x scaling");
          reproject_quantizer->x_scale_factor = geoprojectionconverter.get_target_precision(lasreader->header.x_scale_factor);
        }
        if (!geoprojectionconverter.has_target_precision() && lasreadopener.get_scale_factor() != nullptr &&
            lasreadopener.get_scale_factor()[1] > 0.0) {
          LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for y scaling", lasreadopener.get_scale_factor()[1]);
          reproject_quantizer->y_scale_factor = lasreadopener.get_scale_factor()[1];
        } else {
          LASMessage(LAS_VERBOSE, "target precision is used for y scaling");
          reproject_quantizer->y_scale_factor = geoprojectionconverter.get_target_precision(lasreader->header.y_scale_factor);
        }
        if (!geoprojectionconverter.has_target_elevation_precision() && lasreadopener.get_scale_factor() != nullptr &&
            lasreadopener.get_scale_factor()[2] > 0.0) {
          LASMessage(LAS_VERBOSE, "the scale factor '%g' from rescale argument is used for z scaling", lasreadopener.get_scale_factor()[2]);
          reproject_quantizer->z_scale_factor = lasreadopener.get_scale_factor()[2];
        } else {
          LASMessage(LAS_VERBOSE, "target elevation precision is used for z scaling");
          reproject_quantizer->z_scale_factor = geoprojectionconverter.get_target_elevation_precision(lasreader->header.z_scale_factor);
        }
        reproject_quantizer->x_offset = ((I64)((point[0] / reproject_quantizer->x_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->x_scale_factor;
        reproject_quantizer->y_offset = ((I64)((point[1] / reproject_quantizer->y_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->y_scale_factor;
        reproject_quantizer->z_offset = ((I64)((point[2] / reproject_quantizer->z_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->z_scale_factor;

        set_projection_in_header = true;
        ogc_wkt_in_header = true;
        set_wkt_global_encoding_bit = true;
        lasreader->header.clean_vlrs();
        lasreader->header.clean_evlrs();
        LASMessage(LAS_VERY_VERBOSE, "the original vlr and evlr header information from the source file are not transferred to the target file");
      }

      if (set_projection_in_header)
      {
        int number_of_keys;
        GeoProjectionGeoKeys* geo_keys = 0;
        int num_geo_double_params;
        double* geo_double_params = 0;

        if (geoprojectionconverter.get_geo_keys_from_projection(number_of_keys, &geo_keys, num_geo_double_params, &geo_double_params, !geoprojectionconverter.has_projection(false)))
        {
          lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
          free(geo_keys);
          if (geo_double_params)
          {
            lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
            free(geo_double_params);
          }
          else
          {
            lasreader->header.del_geo_double_params();
          }
          lasreader->header.del_geo_ascii_params();
          lasreader->header.del_geo_ogc_wkt();
        }

        if (set_ogc_wkt || ogc_wkt_in_header || (lasreader->header.point_data_format >= 6)) // maybe also set the OCG WKT
        {
          CHAR* ogc_wkt = set_ogc_wkt_string;
          // The WKT should be generated via the PROJ lib if: it is a PROJ transformation, a soure EPSG in the arguments is specified or a WKT is specified in the file header. 
          // For older file versions that only contain GeoKeys in file header, the WKT should still be generated via LAStool.
          if (ogc_wkt == 0 && geoprojectionconverter.is_proj_request) { 
            LASMessage(LAS_VERBOSE, "the WKT for the file header is created via the PROJ library");
            geoprojectionconverter.get_wkt_from_proj(ogc_wkt, geoprojectionconverter, lasreader);
          }

          I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);

          // If the WKT could not be created by the PROJ lib
          if (ogc_wkt == 0)
          {
            if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt, !geoprojectionconverter.has_projection(false)))
            {
              LASMessage(LAS_WARNING, "cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'", lasreadopener.get_file_name());
              if (ogc_wkt) free(ogc_wkt);
              ogc_wkt = 0;
            }
          }
          if (ogc_wkt)
          {
            if (set_ogc_wkt_in_evlr)
            {
              if (lasreader->header.version_minor >= 4)
              {
                lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
              }
              else
              {
                LASMessage(LAS_WARNING, "input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...", lasreader->header.version_minor);
                lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
              }
            }
            else
            {
              lasreader->header.set_geo_ogc_wkt(len, ogc_wkt);
            }
            if (!set_ogc_wkt_string) free(ogc_wkt);
            if (((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6)) || set_wkt_global_encoding_bit)
            {
              lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
            }
          }
        }
      } 
      else if (set_ogc_wkt) // maybe only set the OCG WKT
      {
        CHAR* ogc_wkt = set_ogc_wkt_string;
        // The WKT should be generated via the PROJ lib if: it is a PROJ transformation, a soure EPSG in the arguments is specified or a WKT is specified in the file header. 
        // For older file versions that only contain GeoKeys in file header, the WKT should still be generated via LAStool.
        if (ogc_wkt == 0 && geoprojectionconverter.is_proj_request) {
          LASMessage(LAS_VERBOSE, "the WKT for the file header is created via the PROJ library");
          geoprojectionconverter.get_wkt_from_proj(ogc_wkt, geoprojectionconverter, lasreader);
        }

        I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);
        //If the WKT could not be created by the PROJ lib
        if (ogc_wkt == 0) 
        { 
          if (lasreader->header.vlr_geo_keys) 
          {
            geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
            if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt)) 
            {
              LASMessage(LAS_WARNING, "cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'", lasreadopener.get_file_name());
              if (ogc_wkt) free(ogc_wkt);
              ogc_wkt = 0;
            }
          } 
          else 
          {
            LASMessage(LAS_WARNING, "no projection information. ignoring '-set_ogc_wkt' for '%s'", lasreadopener.get_file_name());
          }  
        }

        if (ogc_wkt)
        {
          if (set_ogc_wkt_in_evlr)
          {
            if (lasreader->header.version_minor >= 4)
            {
              lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
            }
            else
            {
              LASMessage(LAS_WARNING, "input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...", lasreader->header.version_minor);
              lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
            }
          }
          else
          {
            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
          }

          if (!set_ogc_wkt_string) free(ogc_wkt);

          if (((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6)) || set_wkt_global_encoding_bit)
          {
            lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
          }
        }
      }
      ogc_wkt_in_header = false;

      // maybe we should remove some stuff

      if (remove_header_padding)
      {
        lasreader->header.clean_user_data_in_header();
        lasreader->header.clean_user_data_after_header();
      }

      if (remove_all_variable_length_records)
      {
        lasreader->header.clean_vlrs();
      }
      else
      {
        if (remove_variable_length_record != -1)
        {
          lasreader->header.remove_vlr(remove_variable_length_record);
        }

        if (remove_variable_length_record_from != -1)
        {
          for (i = remove_variable_length_record_to; i >= remove_variable_length_record_from; i--)
          {
            lasreader->header.remove_vlr(i);
          }
        }
      }

      if (remove_all_extended_variable_length_records)
      {
        lasreader->header.clean_evlrs();
      }
      else
      {
        if (remove_extended_variable_length_record != -1)
        {
          lasreader->header.remove_evlr(remove_extended_variable_length_record);
        }

        if (remove_extended_variable_length_record_from != -1)
        {
          for (i = remove_extended_variable_length_record_to; i >= remove_extended_variable_length_record_from; i--)
          {
            lasreader->header.remove_evlr(i);
          }
        }
      }

      if (remove_tiling_vlr)
      {
        lasreader->header.clean_lastiling();
      }

      if (remove_original_vlr)
      {
        lasreader->header.clean_lasoriginal();
      }

      if (lasreader->header.vlr_lastiling || lasreader->header.vlr_lasoriginal)
      {
        if (lasreader->get_transform())
        {
          if (lasreader->get_transform()->transformed_fields & (LASTRANSFORM_X_COORDINATE | LASTRANSFORM_Y_COORDINATE))
          {
            LASMessage(LAS_WARNING, "transforming x or y coordinates of file with %s VLR invalidates this VLR", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
          }
        }
        if (reproject_quantizer)
        {
          LASMessage(LAS_WARNING, "reprojecting file with %s VLR invalidates this VLR", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
        }
      }

      if (save_vlrs)
      {
        save_vlrs_to_file(&lasreader->header);
        save_vlrs = false;
      }

      if (load_vlrs)
      {
        load_vlrs_from_file(&lasreader->header);
      }

      if (save_vlr)
      {
        save_single_vlr_to_file(&lasreader->header, vlr_index, vlr_user_id, vlr_record_id, vlr_filename);
      }

      if (load_vlr)
      {
        load_single_vlr_from_file(&lasreader->header, vlr_index, vlr_user_id, vlr_record_id, vlr_filename);
        load_vlr = false;
      }

      if (add_empty_vlr_user_ID != 0)
      {
        lasreader->header.add_vlr(add_empty_vlr_user_ID, add_empty_vlr_record_ID, 0, 0, (add_empty_vlr_description ? FALSE : TRUE), add_empty_vlr_description);
      }

      if (save_vlr == false)
      {
        // do we need an extra pass
        BOOL extra_pass = laswriteopener.is_piped();
        
        // we only really need an extra pass if the coordinates are altered or if points are filtered
        if (extra_pass)
        {
          if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (reproject_quantizer == 0) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
          {
            extra_pass = FALSE;
          }
        }

        bool doIntensityRangeGet = false;
        U16 intensityMin = UINT16_MAX;
        U16 intensityMax = 0;
        LASoperationMultiplyScaledIntensityRangeIntoRGB* op = nullptr;
        if ((lasreadopener.get_transform() != nullptr) && (lasreadopener.get_transform()->find_operation(op)))
        {
          doIntensityRangeGet = true;
          extra_pass = true;
        }

        LASheader* header_writer = new LASheader();
        *header_writer = lasreader->header;

        // for piped output we need an extra pass
        if (extra_pass)
        {
          if (lasreadopener.is_piped())
          {
            laserror("input and output cannot both be piped");
          }
          LASMessage(LAS_VERBOSE, "extra pass required: reading %lld points ...", lasreader->npoints);
          // maybe seek to start position
          if (subsequence_start) lasreader->seek(subsequence_start);
          while (lasreader->read_point())
          {
            if (lasreader->p_cnt > subsequence_stop) break;
        
            if (clip_to_bounding_box)
            {
              if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
              {
                continue;
              }
            }

            if (reproject_quantizer)
            {
                lasreader->point.compute_coordinates();
                geoprojectionconverter.to_target(lasreader->point.coordinates);
                lasreader->point.compute_XYZ(reproject_quantizer);
            }
            lasinventory.add(&lasreader->point);

            if (doIntensityRangeGet)
            {
              U16 ints = lasreader->point.get_intensity();
              intensityMin = MIN2(ints, intensityMin);
              intensityMax = MAX2(ints, intensityMax);
            }
          }
          lasreader->close();

          if (doIntensityRangeGet)
          {
            op->intensityMin = intensityMin;
            op->intensityRange = intensityMax - intensityMin;
            if (op->intensityRange == 0)
            {
              LASMessage(LAS_WARNING, "range of intensity values is zero. no intensity calculations will be done.");
            }
          }

          lasinventory.update_header(header_writer);
       
          LASMessage(LAS_VERBOSE, "extra pass took %g sec.", taketime() - start_time);
          start_time = taketime();
          LASMessage(LAS_VERBOSE, "piped output: reading %lld and writing %lld points ...", lasreader->npoints, lasinventory.extended_number_of_point_records);
        }
        else
        {
          LASMessage(LAS_VERBOSE, "reading %lld and writing all surviving points ...", lasreader->npoints);
        }

        // check output
        if (!laswriteopener.active())
        {
          // create name from input name
          laswriteopener.make_file_name(lasreadopener.get_file_name());
        }
        else
        {
          // make sure we do not corrupt the input file
          if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
          {
            laserror("input and output file name are identical: '%s'", lasreadopener.get_file_name());
          }
        }

        // prepare the header for the surviving points
        strncpy_las(lasreader->header.system_identifier, LAS_HEADER_CHAR_LEN, LAS_TOOLS_COPYRIGHT);
        char temp[64];
        snprintf(temp, sizeof(temp), "las2las%s (version %d)", (IS64 ? "64" : ""), LAS_TOOLS_VERSION);
        memset(lasreader->header.generating_software, 0, LAS_HEADER_CHAR_LEN);
        strncpy_las(lasreader->header.generating_software, LAS_HEADER_CHAR_LEN, temp);
        
        // open laswriter
        if (reproject_quantizer) {
          *header_writer = *reproject_quantizer;
        }
        LASwriter* laswriter = laswriteopener.open(header_writer);
        if (laswriter == 0)
        {
          laserror("could not open laswriter");
        }

        // for piped output we need to re-open the input file
        if (extra_pass)
        {
          if (!lasreadopener.reopen(lasreader))
          {
            laserror("could not re-open lasreader");
          }
        }

        // maybe seek to start position
        if (subsequence_start) lasreader->seek(subsequence_start);

        // loop over points
        if (point) // full rewrite: point copy
        {
          while (lasreader->read_point())
          {
            if (lasreader->p_cnt > subsequence_stop) break;

            if (clip_to_bounding_box)
            {
              if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
              {
                continue;
              }
            }

            if (reproject_quantizer)
            {
              lasreader->point.compute_coordinates();
              geoprojectionconverter.to_target(lasreader->point.coordinates);
              lasreader->point.compute_XYZ(reproject_quantizer);
            }
            *point = lasreader->point;
            laswriter->write_point(point);
            // without extra pass we need inventory of surviving points
            if (!extra_pass) laswriter->update_inventory(point);
          }
          delete point;
          point = 0;
        }
        else // direct copy from source point to target point
        {
          while (lasreader->read_point())
          {
            if (lasreader->p_cnt > subsequence_stop) break;

            if (clip_to_bounding_box)
            {
              if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
              {
                continue;
              }
            }

            if (reproject_quantizer)
            {
              lasreader->point.compute_coordinates();
              geoprojectionconverter.to_target(lasreader->point.coordinates);
              lasreader->point.compute_XYZ(reproject_quantizer);
            }
            laswriter->write_point(&lasreader->point);
            // without extra pass we need inventory of surviving points
            if (!extra_pass) laswriter->update_inventory(&lasreader->point);
          }
        }

        // without the extra pass we need to fix the header now
        if (!extra_pass)
        {
          laswriter->update_header(&lasreader->header, TRUE);
          LASMessage(LAS_VERBOSE, "total time: %g sec. written %u surviving points to '%s'.", taketime() - start_time, (U32)laswriter->p_count, laswriteopener.get_file_name());
        }
        else
        {
          LASMessage(LAS_VERBOSE, "main pass took %g sec.", taketime() - start_time);
        }

        laswriter->close();
        // delete empty output files
        if (remove_empty_files && (laswriter->npoints == 0) && laswriteopener.get_file_name())
        {
          remove(laswriteopener.get_file_name());
          LASMessage(LAS_VERBOSE, "removing empty output file '%s'", laswriteopener.get_file_name());
        }
        delete laswriter;
      }

      lasreader->close();
      delete lasreader;

      if (reproject_quantizer) delete reproject_quantizer;

      laswriteopener.set_file_name(0);
    }
    catch (...)
    {
      laswriteopener.set_file_name(0);
      laserror("processing file '%s'. maybe file is corrupt?", lasreadopener.get_file_name());
    }
  }
  byebye();
}

