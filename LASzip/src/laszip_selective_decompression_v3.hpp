/*
===============================================================================

  FILE:  laszip_selective_decompression_v3.hpp

  CONTENTS:

    Contains LASitem and LASchunk structs as well as the IDs of the currently
    supported entropy coding scheme

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
   14 April 2017 -- created at Lo Que Hay where Gui was having birthday dinner

===============================================================================
*/
#ifndef LASZIP_SELECTIVE_DECOMPRESSION_V3_HPP
#define LASZIP_SELECTIVE_DECOMPRESSION_V3_HPP

#define LASZIP_SELECTIVE_DECOMPRESSION_ALL                0xFFFFFFFF
#define LASZIP_SELECTIVE_DECOMPRESSION_CHANNEL_RETURNS_XY 0x00000000
#define LASZIP_SELECTIVE_DECOMPRESSION_CLASSIFICATIONS    0x00000001
#define LASZIP_SELECTIVE_DECOMPRESSION_FLAGS              0x00000002
#define LASZIP_SELECTIVE_DECOMPRESSION_INTENSITY          0x00000004
#define LASZIP_SELECTIVE_DECOMPRESSION_SCAN_ANGLE         0x00000008
#define LASZIP_SELECTIVE_DECOMPRESSION_USER_DATA          0x00000010
#define LASZIP_SELECTIVE_DECOMPRESSION_POINT_SOURCE       0x00000020
#define LASZIP_SELECTIVE_DECOMPRESSION_GPS_TIME           0x00000040
#define LASZIP_SELECTIVE_DECOMPRESSION_RGB                0x00000080
#define LASZIP_SELECTIVE_DECOMPRESSION_NIR                0x00000100
#define LASZIP_SELECTIVE_DECOMPRESSION_WAVEPACKET         0x00000200
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE0              0x00010000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE1              0x00020000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE2              0x00040000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE3              0x00080000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE4              0x00100000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE5              0x00200000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE6              0x00400000
#define LASZIP_SELECTIVE_DECOMPRESSION_BYTE7              0x00800000

#endif // LASZIP_SELECTIVE_DECOMPRESSION_V3_HPP
