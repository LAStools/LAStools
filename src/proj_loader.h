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

#include "proj_types.h"

// ---------------------------------------------------------------------------
// Modern using function pointers
// ---------------------------------------------------------------------------
using proj_as_wkt_t = const char* (*)(PJ_CONTEXT*, const PJ*, PJ_WKT_TYPE, const char* const*);
using proj_as_proj_string_t = const char* (*)(PJ_CONTEXT*, const PJ*, PJ_PROJ_STRING_TYPE, const char* const*);
using proj_as_projjson_t = const char* (*)(PJ_CONTEXT*, const PJ*, const char* const*);
using proj_get_source_crs_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_get_target_crs_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_destroy_t = void (*)(PJ*);
using proj_get_id_code_t = const char* (*)(const PJ*, int);
using proj_get_ellipsoid_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_ellipsoid_get_parameters_t = int (*)(PJ_CONTEXT*, const PJ*, double*, double*, int*, double*);
using proj_crs_get_datum_ensemble_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_get_name_t = const char* (*)(const PJ*);
using proj_datum_ensemble_get_member_count_t = int (*)(PJ_CONTEXT*, const PJ*);
using proj_datum_ensemble_get_accuracy_t = double (*)(PJ_CONTEXT*, const PJ*);
using proj_datum_ensemble_get_member_t = PJ* (*)(PJ_CONTEXT*, const PJ*, int);
using proj_crs_get_datum_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_crs_get_coordinate_system_t = PJ* (*)(PJ_CONTEXT*, const PJ*);
using proj_cs_get_type_t = PJ_COORDINATE_SYSTEM_TYPE (*)(PJ_CONTEXT*, const PJ*);
using proj_cs_get_axis_count_t = int (*)(PJ_CONTEXT*, const PJ*);
using proj_cs_get_axis_info_t =
    int (*)(PJ_CONTEXT*, const PJ*, int, const char**, const char**, const char**, double*, const char**, const char**, const char**);

using proj_create_t = PJ* (*)(PJ_CONTEXT*, const char*);
using proj_create_argv_t = PJ* (*)(PJ_CONTEXT*, int, char**);
using proj_create_crs_to_crs_t = PJ* (*)(PJ_CONTEXT*, const char*, const char*, PJ_AREA*);
using proj_create_crs_to_crs_from_pj_t = PJ* (*)(PJ_CONTEXT*, const PJ*, const PJ*, PJ_AREA*, const char* const*);
using proj_create_from_wkt_t = PJ* (*)(PJ_CONTEXT*, const char*, const char* const*, PROJ_STRING_LIST*, PROJ_STRING_LIST*);

using proj_context_create_t = PJ_CONTEXT* (*)(void);
using proj_context_destroy_t = void (*)(PJ_CONTEXT*);
using proj_context_errno_t = int (*)(PJ_CONTEXT*);
using proj_context_errno_string_t = const char* (*)(PJ_CONTEXT*, int);

using proj_coord_t = PROJ_COORD_OPAQUE (*)(double, double, double, double);
using proj_trans_t = PROJ_COORD_OPAQUE (*)(PJ*, int, PROJ_COORD_OPAQUE);
using proj_get_type_t = int (*)(PJ*);
using proj_is_crs_t = int (*)(const PJ*);
using proj_log_func_t = void (*)(PJ_CONTEXT*, void*, void(*)(void*, int, const char*));

// MyPJ_INFO - own compatible type
using proj_info_t = MyPJ_INFO (*)(void);

// ------------------------
// Function pointer (external)
// ------------------------
extern proj_as_wkt_t proj_as_wkt_ptr;
extern proj_as_proj_string_t proj_as_proj_string_ptr;
extern proj_as_projjson_t proj_as_projjson_ptr;
extern proj_get_source_crs_t proj_get_source_crs_ptr;
extern proj_get_target_crs_t proj_get_target_crs_ptr;
extern proj_destroy_t proj_destroy_ptr;
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
extern proj_cs_get_axis_count_t proj_cs_get_axis_count_ptr;
extern proj_cs_get_axis_info_t proj_cs_get_axis_info_ptr;
extern proj_cs_get_type_t proj_cs_get_type_ptr;
extern proj_is_crs_t proj_is_crs_ptr;
extern proj_create_t proj_create_ptr;
extern proj_create_argv_t proj_create_argv_ptr;
extern proj_create_crs_to_crs_t proj_create_crs_to_crs_ptr;
extern proj_create_crs_to_crs_from_pj_t proj_create_crs_to_crs_from_pj_ptr;
extern proj_create_from_wkt_t proj_create_from_wkt_ptr;
extern proj_context_create_t proj_context_create_ptr;
extern proj_context_destroy_t proj_context_destroy_ptr;
extern proj_context_errno_t proj_context_errno_ptr;
extern proj_context_errno_string_t proj_context_errno_string_ptr;
extern proj_coord_t proj_coord_ptr;
extern proj_trans_t proj_trans_ptr;
extern proj_get_type_t proj_get_type_ptr;
extern proj_info_t proj_info_ptr;
extern proj_log_func_t proj_log_func_ptr;

// ------------------------
// Library loader
// ------------------------
bool load_proj_library(const char* path, bool isNecessary = true);
void unload_proj_library();

extern "C" void myCustomProjErrorHandler(void* app_data, int level, const char* msg);

#endif  // PROJ_LOADER_H
