/*
===============================================================================

  FILE:  proj_wrapper.cpp

  CONTENTS:

    see corresponding header file

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

    see corresponding header file

===============================================================================
*/

#include "proj_wrapper.h"

// -----------------------
// ABI-safe wrappers
// -----------------------

/// Performs an ABI-safe PROJ transformation of (x, y, z, t) via PJ in the PJ_DIRECTION direction and returns the result as MyPJ_COORD.
MyPJ_COORD transform_point(PJ* P, PJ_DIRECTION direction, double x, double y, double z, double t) {
  if (!proj_coord_ptr || !proj_trans_ptr) {
    MyPJ_COORD err = {0, 0, 0, 0};
    return err;
  }

  PROJ_COORD_OPAQUE in_coord = proj_coord_ptr(x, y, z, t);
  PROJ_COORD_OPAQUE out_coord = proj_trans_ptr(P, direction, in_coord);

  MyPJ_COORD result;
  result.x = out_coord.v[0];
  result.y = out_coord.v[1];
  result.z = out_coord.v[2];
  result.t = out_coord.v[3];

  return result;
}

/// Returns the coordinate operation used during the last transformation
PJ* my_proj_trans_get_last_used_operation(PJ* transform) {
  if (!transform || !proj_trans_get_last_used_operation_ptr) return nullptr;

  return proj_trans_get_last_used_operation_ptr(transform);
}

/// Wrapper for proj_context_create
PJ_CONTEXT* my_proj_context_create() {
  return proj_context_create_ptr ? proj_context_create_ptr() : nullptr;
}

/// Wrapper for proj_context_destroy
void my_proj_context_destroy(PJ_CONTEXT* ctx) {
  if (proj_context_destroy_ptr) proj_context_destroy_ptr(ctx);
}

/// Wrapper for proj_context_destroy
void my_proj_destroy(PJ* crs) {
  if (proj_destroy_ptr) proj_destroy_ptr(crs);
}

/// Wrapper for proj_create
PJ* my_proj_create(PJ_CONTEXT* ctx, const char* crs) {
  return proj_create_ptr ? proj_create_ptr(ctx, crs) : nullptr;
}

/// Wrapper for proj_context_errno
int my_proj_context_errno(PJ_CONTEXT* ctx) {
  return proj_context_errno_ptr ? proj_context_errno_ptr(ctx) : 0;
}

/// Wrapper for proj_context_errno_string
const char* my_proj_context_errno_string(PJ_CONTEXT* ctx, int err_no) {
  return proj_context_errno_string_ptr ? proj_context_errno_string_ptr(ctx, err_no) : nullptr;
}

/// Wrapper for proj_get_type
MyPJ_TYPE my_proj_get_type(PJ* P) {
  MyPJ_TYPE out = {0, MY_PJ_TYPE_UNKNOWN};
  if (!proj_get_type_ptr) return out;

  // proj_get_type_ptr returns the real PROJ type internally
  int internal_type = proj_get_type_ptr(P);
  switch (internal_type) {
    case 1:
      out.type = MY_PJ_TYPE_TRANSFORMATION;
      break;
    case 2:
      out.type = MY_PJ_TYPE_CONVERSION;
      break;
    case 3:
      out.type = MY_PJ_TYPE_CONCATENATED_OPERATION;
      break;
    case 4:
      out.type = MY_PJ_TYPE_CRS;
      break;
    case 5:
      out.type = MY_PJ_TYPE_ELLIPSOID;
      break;
    default:
      out.type = MY_PJ_TYPE_UNKNOWN;
      break;
  }
  return out;
}

/// Wrapper for proj_info – provides the global PROJ version and build information.
MyPJ_INFO my_proj_info(void) {
  if (!proj_info_ptr) {
    MyPJ_INFO info = {0, 0, 0, nullptr, nullptr, nullptr};
    return info;
  }
  return proj_info_ptr();
}

/// Wrapper for proj_get_name
const char* my_proj_get_name(const PJ* obj) {
  return proj_get_name_ptr ? proj_get_name_ptr(obj) : nullptr;
}

/// Wrapper for proj_coordoperation_is_instantiable
int my_proj_coordoperation_is_instantiable(PJ_CONTEXT* ctx, const PJ* operation) {
  return proj_coordoperation_is_instantiable_ptr ? proj_coordoperation_is_instantiable_ptr(ctx, operation) : -1;
}

/// Wrapper for proj_coordoperation_has_ballpark_transformation
int my_proj_coordoperation_has_ballpark_transformation(PJ_CONTEXT* ctx, const PJ* operation) {
  return proj_coordoperation_has_ballpark_transformation_ptr ? proj_coordoperation_has_ballpark_transformation_ptr(ctx, operation) : -1;
}

/// Wrapper for proj_coordoperation_get_accuracy
double my_proj_coordoperation_get_accuracy(PJ_CONTEXT* ctx, const PJ* operation) {
  return proj_coordoperation_get_accuracy_ptr ? proj_coordoperation_get_accuracy_ptr(ctx, operation) : -1.0;
}

/// Wrapper for proj_coordoperation_get_grid_used_count
int my_proj_coordoperation_get_grid_used_count(PJ_CONTEXT* ctx, const PJ* operation) {
  return proj_coordoperation_get_grid_used_count_ptr ? proj_coordoperation_get_grid_used_count_ptr(ctx, operation) : -1;
}

/// Wrapper for proj_coordoperation_get_grid_used
int my_proj_coordoperation_get_grid_used(
    PJ_CONTEXT* ctx, const PJ* operation, int index, const char** short_name, const char** full_name, const char** package_name, const char** url,
    int* direct_download, int* open_license, int* available) {
  return proj_coordoperation_get_grid_used_ptr
             ? proj_coordoperation_get_grid_used_ptr(
                   ctx, operation, index, short_name, full_name, package_name, url, direct_download, open_license, available)
             : 0;
}


/// Wrapper for proj_create_operation_factory_context_ptr
/// Creates a PROJ operation factory context used to discover candidate coordinate operations
PJ_OPERATION_FACTORY_CONTEXT* my_proj_create_operation_factory_context(PJ_CONTEXT* ctx, const char* authority) {
  if (!proj_create_operation_factory_context_ptr) {
    return nullptr;
  }

  return proj_create_operation_factory_context_ptr(ctx, authority);
}

/// Wrapper for proj_operation_factory_context_destroy_ptr
/// Destroys a PROJ operation factory context created with my_proj_create_operation_factory_context()
void my_proj_operation_factory_context_destroy(PJ_OPERATION_FACTORY_CONTEXT* ctx) {
  if (proj_operation_factory_context_destroy_ptr) {
    proj_operation_factory_context_destroy_ptr(ctx);
  }
}

/// Wrapper for proj_operation_factory_context_set_grid_availability_use_ptr
/// Configures how PROJ should handle grid availability when discovering candidate coordinate operations
void my_proj_operation_factory_context_set_grid_availability_use(
    PJ_CONTEXT* ctx, PJ_OPERATION_FACTORY_CONTEXT* factory_ctx, PROJ_GRID_AVAILABILITY_USE use) {
  if (proj_operation_factory_context_set_grid_availability_use_ptr) {
    proj_operation_factory_context_set_grid_availability_use_ptr(ctx, factory_ctx, use);
  }
}

/// Wrapper for proj_create_operations_ptr
/// Creates a list of candidate coordinate operations between the given source and target CRS or CoordinateMetadata objects
PJ_OBJ_LIST* my_proj_create_operations(PJ_CONTEXT* ctx, const PJ* source, const PJ* target, const PJ_OPERATION_FACTORY_CONTEXT* operation_context) {
  if (!proj_create_operations_ptr) {
    return nullptr;
  }

  return proj_create_operations_ptr(ctx, source, target, operation_context);
}

/// Wrapper for proj_list_get_count_ptr
/// Returns the number of coordinate operations contained in a PROJ object list
int my_proj_list_get_count(const PJ_OBJ_LIST* list) {
  if (!proj_list_get_count_ptr) {
    return -1;
  }

  return proj_list_get_count_ptr(list);
}

/// Wrapper for proj_list_get_ptr
/// Returns the coordinate operation at the given index from a PROJ object list
PJ* my_proj_list_get(PJ_CONTEXT* ctx, const PJ_OBJ_LIST* list, int index) {
  if (!proj_list_get_ptr) {
    return nullptr;
  }

  return proj_list_get_ptr(ctx, list, index);
}

/// Wrapper for proj_list_destroy_ptr
/// Creates a PROJ operation factory context used to discover candidate coordinate operations
void my_proj_list_destroy(PJ_OBJ_LIST* list) {
  if (proj_list_destroy_ptr) {
    proj_list_destroy_ptr(list);
  }
}