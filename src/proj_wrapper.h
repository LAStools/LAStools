/*
===============================================================================

  FILE:  proj_wrapper.h

  CONTENTS:
    Provides a slim, ABI-stable wrapper layer over the PROJ C API.
    Encapsulates all PROJ calls and provides unified, secure functions to the application.
    Maintains stable usage even when PROJ is dynamically loaded or updated.

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

#ifndef PROJ_WRAPPER_H
#define PROJ_WRAPPER_H

#include "proj_types.h"
#include "proj_loader.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------
// PROJ Wrapper functions declarations for ABI Security
// ------------------------

/// ABI-safe transformation
MyPJ_COORD transform_point(PJ* P, PJ_DIRECTION direction, double x, double y, double z, double t);

/// Returns the coordinate operation used by the last proj_trans() call
PJ* my_proj_trans_get_last_used_operation(PJ* transform);

/// Create context
PJ_CONTEXT* my_proj_context_create();

/// Destroy context
void my_proj_context_destroy(PJ_CONTEXT* ctx);

/// Destroy crs
void my_proj_destroy(PJ* crs);

/// Create CRS object
PJ* my_proj_create(PJ_CONTEXT* ctx, const char* crs);

/// Query error code from context
int my_proj_context_errno(PJ_CONTEXT* ctx);

/// Get error string from context
const char* my_proj_context_errno_string(PJ_CONTEXT* ctx, int err_no);

/// Query projection type 
MyPJ_TYPE my_proj_get_type(PJ* P);

/// Read PROJ information 
MyPJ_INFO my_proj_info(void);

/// Get PROJ object name
const char* my_proj_get_name(const PJ* obj);

int my_proj_coordoperation_is_instantiable(PJ_CONTEXT* ctx, const PJ* operation);

int my_proj_coordoperation_has_ballpark_transformation(PJ_CONTEXT* ctx, const PJ* operation);

double my_proj_coordoperation_get_accuracy(PJ_CONTEXT* ctx, const PJ* operation);

int my_proj_coordoperation_get_grid_used_count(PJ_CONTEXT* ctx, const PJ* operation);

int my_proj_coordoperation_get_grid_used( PJ_CONTEXT* ctx, const PJ* operation, int index, const char** short_name, const char** full_name, const char** package_name, const char** url,
    int* direct_download, int* open_license, int* available);

PJ_OPERATION_FACTORY_CONTEXT* my_proj_create_operation_factory_context(PJ_CONTEXT* ctx, const char* authority);

void my_proj_operation_factory_context_destroy(PJ_OPERATION_FACTORY_CONTEXT* ctx);

void my_proj_operation_factory_context_set_grid_availability_use(
    PJ_CONTEXT* ctx, PJ_OPERATION_FACTORY_CONTEXT* factory_ctx, PROJ_GRID_AVAILABILITY_USE use);

PJ_OBJ_LIST* my_proj_create_operations(PJ_CONTEXT* ctx, const PJ* source, const PJ* target, const PJ_OPERATION_FACTORY_CONTEXT* operation_context);

int my_proj_list_get_count(const PJ_OBJ_LIST* list);

PJ* my_proj_list_get(PJ_CONTEXT* ctx, const PJ_OBJ_LIST* list, int index);

void my_proj_list_destroy(PJ_OBJ_LIST* list);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // PROJ_WRAPPER_H
