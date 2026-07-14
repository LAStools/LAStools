/*
===============================================================================

  FILE:  lasformula_api_stub.cpp

  CONTENTS:

  File contains the open-source stub implementation of the Formula API. 
  It ensures that LASlib remains API-compatible, even though formulas are not 
  supported in open build

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2026, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#include "lasformula_api.h"
#include "lasmessage.hpp"

bool lasformula_bind_file_header_and_eval(LASheader* header) {
  return false;
}

void lasformula_bind_point(LASpoint* point) {
  // no-op
}

void lasformula_eval_transforms() {
  // no-op
}

bool lasformula_eval_filters() {
  return false;
}

bool lasformula_has_filters() {
  return false;
}

bool lasformula_has_transforms() {
  return false;
}

void lasformula_parse_formula_string(const char* expr, LASFormulaMode mode) {
  LASMessage(LAS_WARNING, "the options '-formula' and '-fileformula' require a licensed version of LAStools");
}
