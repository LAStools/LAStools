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

/// Create context
PJ_CONTEXT* my_proj_context_create();

/// Destroy context
void my_proj_context_destroy(PJ_CONTEXT* ctx);

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

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // PROJ_WRAPPER_H
