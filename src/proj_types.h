/*
===============================================================================

  FILE:  proj_types.h

  CONTENTS:
    Defines all project-specific types, enums, and opaque structs for accessing the PROJ C API
    without directly including proj.h. Keeps the application ABI-stable and independent of PROJ versions.
    Contains no logic, only type definitions.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2025, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#ifndef PROJ_TYPES_H
#define PROJ_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------
// Opaque types (no proj.h dependency)
// ------------------------
struct PJ_CONTEXT;
struct PJ;
struct PJ_AREA;
struct PROJ_STRING_LIST;

typedef struct PJ_CONTEXT PJ_CONTEXT;
typedef struct PJ PJ;
typedef struct PJ_AREA PJ_AREA;
typedef struct PROJ_STRING_LIST PROJ_STRING_LIST;

typedef struct {
  double x, y, z, t;
} MyPJ_COORD;

typedef struct {
  int major;
  int minor;
  int patch;
  const char* release;
  const char* version;
  const char* searchpath;
} MyPJ_INFO;

enum MyPJ_TYPE_ENUM {
  MY_PJ_TYPE_UNKNOWN = 0,
  MY_PJ_TYPE_TRANSFORMATION = 1,
  MY_PJ_TYPE_CONVERSION = 2,
  MY_PJ_TYPE_CONCATENATED_OPERATION = 3,
  MY_PJ_TYPE_CRS = 4,
  MY_PJ_TYPE_ELLIPSOID = 5
};

typedef struct {
  int id;
  int type;
} MyPJ_TYPE;

 typedef enum {
  PJ_TYPE_UNKNOWN = 0,
  PJ_TYPE_TRANSFORMATION,
  PJ_TYPE_CONVERSION,
  PJ_TYPE_CONCATENATED_OPERATION,
  PJ_TYPE_CRS,
  PJ_TYPE_ELLIPSOID,
} PJ_TYPE;

typedef enum {
  PJ_CS_TYPE_UNKNOWN,
  PJ_CS_TYPE_CARTESIAN,
  PJ_CS_TYPE_ELLIPSOIDAL,
  PJ_CS_TYPE_VERTICAL,
  PJ_CS_TYPE_SPHERICAL,
  PJ_CS_TYPE_ORDINAL,
  PJ_CS_TYPE_PARAMETRIC,
  PJ_CS_TYPE_DATETIMETEMPORAL,
  PJ_CS_TYPE_TEMPORALCOUNT,
  PJ_CS_TYPE_TEMPORALMEASURE
} PJ_COORDINATE_SYSTEM_TYPE;

typedef enum { PJ_FWD = 1, PJ_IDENT = 0, PJ_INV = -1 } PJ_DIRECTION;

typedef enum { PJ_PROJ_5, PJ_PROJ_4 } PJ_PROJ_STRING_TYPE;

typedef enum {
  PJ_WKT2_2015,
  PJ_WKT2_2015_SIMPLIFIED,
  PJ_WKT2_2019,
  PJ_WKT2_2018 = PJ_WKT2_2019,
  PJ_WKT2_2019_SIMPLIFIED,
  PJ_WKT2_2018_SIMPLIFIED = PJ_WKT2_2019_SIMPLIFIED,
  PJ_WKT1_GDAL,
  PJ_WKT1_ESRI
} PJ_WKT_TYPE;

typedef struct {
  double v[4];
} PROJ_COORD_OPAQUE;

#ifdef __cplusplus
}
#endif

// PROJ log levels (ABI-safe)
constexpr int PJ_LOG_ERROR = 0;
constexpr int PJ_LOG_DEBUG = 1;
constexpr int PJ_LOG_TRACE = 2;

#endif
