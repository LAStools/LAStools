/*
===============================================================================

  FILE:  proj_loader.h

  CONTENTS:
    This header file provides declarations and macros for dynamically loading
    and interfacing with the PROJ library. It includes function pointer type
    definitions for various PROJ functions, macros for simplified function
    calls, and functions for loading and unloading the PROJ library dynamically.
    It also defines platform-specific macros for handling dynamic libraries on
    different systems. 
    
  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2024, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/
#ifndef PROJ_LOADER_H
#define PROJ_LOADER_H

#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibraryEx(name, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#define GET_PROC_ADDRESS(lib, name) GetProcAddress((HMODULE)lib, name)
#define FREE_LIBRARY(lib) FreeLibrary((HMODULE)lib)
#define PROJ_LIB_HANDLE HMODULE
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define GET_PROC_ADDRESS(lib, name) dlsym(lib, name)
#define FREE_LIBRARY(lib) dlclose(lib)
#define PROJ_LIB_HANDLE void*
#endif

// Placeholder for compiling without proj.h
typedef void* PJ;
typedef void* PJ_CONTEXT;
typedef void* PJ_AREA;
typedef void* PROJ_STRING_LIST;

typedef struct {
  double x, y, z, t;
} PJ_XYZT;

typedef struct {
  PJ_XYZT xyzt;
} PJ_COORD;

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

typedef enum {
  PJ_FWD = 1,
  PJ_IDENT = 0,
  PJ_INV = -1
} PJ_DIRECTION;

typedef enum {
  PJ_TYPE_UNKNOWN = 0,
  PJ_TYPE_TRANSFORMATION,
  PJ_TYPE_CONVERSION,
  PJ_TYPE_CONCATENATED_OPERATION,
  PJ_TYPE_CRS,
  PJ_TYPE_ELLIPSOID,
} PJ_TYPE;

typedef enum {
  PJ_PROJ_5,
  PJ_PROJ_4
} PJ_PROJ_STRING_TYPE;

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

// Function pointer type definitions
typedef const char* (*proj_as_wkt_t)(PJ_CONTEXT*, const PJ*, PJ_WKT_TYPE, const char* const*);
typedef const char* (*proj_as_proj_string_t)(PJ_CONTEXT*, const PJ*, PJ_PROJ_STRING_TYPE, const char* const*);
typedef const char* (*proj_as_projjson_t)(PJ_CONTEXT*, const PJ*, const char* const*);
typedef PJ* (*proj_get_source_crs_t)(PJ_CONTEXT*, const PJ*);
typedef PJ* (*proj_get_target_crs_t)(PJ_CONTEXT*, const PJ*);
typedef void (*proj_destroy_t)(PJ*);
typedef PJ_CONTEXT* (*proj_context_create_t)(void);
typedef void (*proj_context_destroy_t)(PJ_CONTEXT*);
typedef const char* (*proj_get_id_code_t)(const PJ*, int);
typedef PJ* (*proj_get_ellipsoid_t)(PJ_CONTEXT*, const PJ*);
typedef int (*proj_ellipsoid_get_parameters_t)(PJ_CONTEXT*, const PJ*, double*, double*, int*, double*);
typedef PJ* (*proj_crs_get_datum_ensemble_t)(PJ_CONTEXT*, const PJ*);
typedef const char* (*proj_get_name_t)(const PJ*);
typedef int (*proj_datum_ensemble_get_member_count_t)(PJ_CONTEXT*, const PJ*);
typedef double (*proj_datum_ensemble_get_accuracy_t)(PJ_CONTEXT*, const PJ*);
typedef PJ* (*proj_datum_ensemble_get_member_t)(PJ_CONTEXT*, const PJ*, int);
typedef PJ* (*proj_crs_get_datum_t)(PJ_CONTEXT*, const PJ*);
typedef PJ* (*proj_crs_get_coordinate_system_t)(PJ_CONTEXT*, const PJ*);
typedef PJ_COORDINATE_SYSTEM_TYPE (*proj_cs_get_type_t)(PJ_CONTEXT*, const PJ*);
typedef int (*proj_cs_get_axis_count_t)(PJ_CONTEXT*, const PJ*);
typedef int (*proj_cs_get_axis_info_t)(PJ_CONTEXT*, const PJ*, int, const char**, const char**, const char**, double*, const char**, const char**, const char**);
typedef PJ* (*proj_create_t)(PJ_CONTEXT*, const char*);
typedef PJ* (*proj_create_argv_t)(PJ_CONTEXT*, int, char**);
typedef PJ* (*proj_create_crs_to_crs_t)(PJ_CONTEXT*, const char*, const char*, PJ_AREA*);
typedef PJ* (*proj_create_crs_to_crs_from_pj_t)(PJ_CONTEXT*, const PJ*, const PJ*, PJ_AREA*, const char* const*);
typedef PJ* (*proj_create_from_wkt_t)(PJ_CONTEXT*, const char*, const char* const*, PROJ_STRING_LIST*, PROJ_STRING_LIST*);
typedef int (*proj_context_errno_t)(PJ_CONTEXT*);
typedef const char* (*proj_context_errno_string_t)(PJ_CONTEXT*, int);
typedef PJ_COORD (*proj_coord_t)(double, double, double, double);
typedef PJ_COORD (*proj_trans_t)(PJ*, PJ_DIRECTION, PJ_COORD);
typedef PJ_TYPE (*proj_get_type_t)(const PJ*);
typedef int (*proj_is_crs_t)(const PJ*);

// External variables for function pointers
extern proj_as_wkt_t proj_as_wkt_ptr;
extern proj_as_proj_string_t proj_as_proj_string_ptr;
extern proj_as_projjson_t proj_as_projjson_ptr;
extern proj_get_source_crs_t proj_get_source_crs_ptr;
extern proj_get_target_crs_t proj_get_target_crs_ptr;
extern proj_destroy_t proj_destroy_ptr;
extern proj_context_create_t proj_context_create_ptr;
extern proj_context_destroy_t proj_context_destroy_ptr;
extern proj_get_id_code_t proj_get_id_code_ptr;
extern proj_get_ellipsoid_t proj_get_ellipsoid_ptr;
extern proj_ellipsoid_get_parameters_t proj_ellipsoid_get_parameters_ptr;
extern proj_crs_get_datum_ensemble_t proj_crs_get_datum_ensemble_ptr;
extern proj_get_name_t proj_get_name_ptr;
extern proj_datum_ensemble_get_member_count_t proj_datum_ensemble_get_member_count_ptr;
extern proj_datum_ensemble_get_accuracy_t proj_datum_ensemble_get_accuracy_ptr;
extern proj_datum_ensemble_get_member_t proj_datum_ensemble_get_member_ptr;
extern proj_crs_get_datum_t proj_crs_get_datum_ptr;
extern proj_crs_get_coordinate_system_t proj_crs_get_coordinate_system_ptr;
extern proj_cs_get_type_t proj_cs_get_type_ptr;
extern proj_cs_get_axis_count_t proj_cs_get_axis_count_ptr;
extern proj_cs_get_axis_info_t proj_cs_get_axis_info_ptr;
extern proj_create_t proj_create_ptr;
extern proj_create_argv_t proj_create_argv_ptr;
extern proj_create_crs_to_crs_t proj_create_crs_to_crs_ptr;
extern proj_create_crs_to_crs_from_pj_t proj_create_crs_to_crs_from_pj_ptr;
extern proj_create_from_wkt_t proj_create_from_wkt_ptr;
extern proj_context_errno_t proj_context_errno_ptr;
extern proj_context_errno_string_t proj_context_errno_string_ptr;
extern proj_coord_t proj_coord_ptr;
extern proj_trans_t proj_trans_ptr;
extern proj_get_type_t proj_get_type_ptr;
extern proj_is_crs_t proj_is_crs_ptr;

// Function for dynamic loading of the PROJ library
bool load_proj_library(const char* path, bool isNecessary = true);
// Function for unloading the PROJ library
void unload_proj_library();

// Macros for function calls
#define proj_as_wkt(ctx, obj, type, options) (proj_as_wkt_ptr ? proj_as_wkt_ptr(ctx, obj, type, options) : nullptr)

#define proj_as_proj_string(ctx, obj, type, options) (proj_as_proj_string_ptr ? proj_as_proj_string_ptr(ctx, obj, type, options) : nullptr)

#define proj_as_projjson(ctx, obj, options) (proj_as_projjson_ptr ? proj_as_projjson_ptr(ctx, obj, options) : nullptr)

#define proj_get_source_crs(ctx, obj) (proj_get_source_crs_ptr ? proj_get_source_crs_ptr(ctx, obj) : nullptr)

#define proj_get_target_crs(ctx, obj) (proj_get_target_crs_ptr ? proj_get_target_crs_ptr(ctx, obj) : nullptr)

#define proj_destroy(P)                                                                                                                              \
  if (proj_destroy_ptr) proj_destroy_ptr(P)

#define proj_context_create() (proj_context_create_ptr ? proj_context_create_ptr() : nullptr)

#define proj_context_destroy(ctx)                                                                                                                    \
  if (proj_context_destroy_ptr) proj_context_destroy_ptr(ctx)

#define proj_get_id_code(obj, index) (proj_get_id_code_ptr ? proj_get_id_code_ptr(obj, index) : nullptr)

#define proj_get_ellipsoid(ctx, obj) (proj_get_ellipsoid_ptr ? proj_get_ellipsoid_ptr(ctx, obj) : nullptr)

#define proj_ellipsoid_get_parameters(ctx, ellipsoid, out_semi_major_metre, out_semi_minor_metre, out_is_semi_minor_computed, out_inv_flattening)    \
  (proj_ellipsoid_get_parameters_ptr                                                                                                                 \
       ? proj_ellipsoid_get_parameters_ptr(                                                                                                          \
             ctx, ellipsoid, out_semi_major_metre, out_semi_minor_metre, out_is_semi_minor_computed, out_inv_flattening)                             \
       : 0)

#define proj_crs_get_datum_ensemble(ctx, crs) (proj_crs_get_datum_ensemble_ptr ? proj_crs_get_datum_ensemble_ptr(ctx, crs) : nullptr)

#define proj_get_name(obj) (proj_get_name_ptr ? proj_get_name_ptr(obj) : nullptr)

#define proj_datum_ensemble_get_member_count(ctx, datum_ensemble)                                                                                    \
  (proj_datum_ensemble_get_member_count_ptr ? proj_datum_ensemble_get_member_count_ptr(ctx, datum_ensemble) : 0)

#define proj_datum_ensemble_get_accuracy(ctx, datum_ensemble)                                                                                        \
  (proj_datum_ensemble_get_accuracy_ptr ? proj_datum_ensemble_get_accuracy_ptr(ctx, datum_ensemble) : 0)

#define proj_datum_ensemble_get_member(ctx, datum_ensemble, member_index)                                                                            \
  (proj_datum_ensemble_get_member_ptr ? proj_datum_ensemble_get_member_ptr(ctx, datum_ensemble, member_index) : nullptr)

#define proj_crs_get_datum(ctx, crs) (proj_crs_get_datum_ptr ? proj_crs_get_datum_ptr(ctx, crs) : nullptr)

#define proj_crs_get_coordinate_system(ctx, crs) (proj_crs_get_coordinate_system_ptr ? proj_crs_get_coordinate_system_ptr(ctx, crs) : nullptr)

#define proj_cs_get_type(ctx, cs) (proj_cs_get_type_ptr ? proj_cs_get_type_ptr(ctx, cs) : PJ_CS_TYPE_UNKNOWN)

#define proj_cs_get_axis_count(ctx, cs) (proj_cs_get_axis_count_ptr ? proj_cs_get_axis_count_ptr(ctx, cs) : 0)

#define proj_cs_get_axis_info(                                                                                                                       \
    ctx, cs, index, out_name, out_abbrev, out_direction, out_unit_conv_factor, out_unit_name, out_unit_auth_name, out_unit_code)                     \
  (proj_cs_get_axis_info_ptr                                                                                                                         \
       ? proj_cs_get_axis_info_ptr(                                                                                                                  \
             ctx, cs, index, out_name, out_abbrev, out_direction, out_unit_conv_factor, out_unit_name, out_unit_auth_name, out_unit_code)            \
       : 0)

#define proj_create(ctx, definition) (proj_create_ptr ? proj_create_ptr(ctx, definition) : nullptr)

#define proj_create_argv(ctx, argc, argv) (proj_create_argv_ptr ? proj_create_argv_ptr(ctx, argc, argv) : nullptr)

#define proj_create_crs_to_crs(ctx, source_crs, target_crs, area_of_use)                                                                             \
  (proj_create_crs_to_crs_ptr ? proj_create_crs_to_crs_ptr(ctx, source_crs, target_crs, area_of_use) : nullptr)

#define proj_create_crs_to_crs_from_pj(ctx, source_crs, target_crs, area_of_use, options)                                                            \
  (proj_create_crs_to_crs_from_pj_ptr ? proj_create_crs_to_crs_from_pj_ptr(ctx, source_crs, target_crs, area_of_use, options) : nullptr)

#define proj_create_from_wkt(ctx, wkt, options, proj_string_list, proj_string_list_out)                                                              \
  (proj_create_from_wkt_ptr ? proj_create_from_wkt_ptr(ctx, wkt, options, proj_string_list, proj_string_list_out) : nullptr)

#define proj_context_errno(ctx) (proj_context_errno_ptr ? proj_context_errno_ptr(ctx) : 0)

#define proj_context_errno_string(ctx, err) (proj_context_errno_string_ptr ? proj_context_errno_string_ptr(ctx, err) : nullptr)

#define proj_coord(x, y, z, t) (proj_coord_ptr ? proj_coord_ptr(x, y, z, t) : PJ_COORD{})

#define proj_trans(P, direction, coord) (proj_trans_ptr ? proj_trans_ptr(P, direction, coord) : PJ_COORD{})

#define proj_get_type(P) (proj_get_type_ptr ? proj_get_type_ptr(P) : PJ_TYPE{})

#define proj_is_crs(P) (proj_is_crs_ptr ? proj_is_crs_ptr(P) : 0)

#endif  // PROJ_LOADER_H
