/*
===============================================================================

    FILE:  lasformula_api.h

    CONTENTS:

        Header file defines the public Formula API and provides  a uniform 
        interface for header binding, point binding, filter evaluation 
        and transformations. 

    PROGRAMMERS:

        info@rapidlasso.de  -  https://rapidlasso.de

    COPYRIGHT:

        (c) 2007-2026, rapidlasso GmbH - fast tools to catch reality

        This is free software; you can redistribute and/or modify it under the
        terms of the GNU Lesser General Licence as published by the Free Software
        Foundation. See the LICENSE.txt file for more information.

        This software is distributed WITHOUT ANY WARRANTY and without even the
        implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    CHANGE HISTORY:


===============================================================================
*/
#ifndef FORMULA_API_H
#define FORMULA_API_H

class LASheader;
class LASpoint;
class LASfilter;
class LAStransform;

enum LASFormulaMode { LASFORMULA_POINT = 0, LASFORMULA_FILE = 1 };

bool lasformula_bind_file_header_and_eval(LASheader* header);
void lasformula_bind_point(LASpoint* point);
void lasformula_eval_transforms();
bool lasformula_eval_filters();
bool lasformula_has_filters();
bool lasformula_has_transforms();
void lasformula_parse_formula_string(const char* expr, LASFormulaMode mode);

#endif