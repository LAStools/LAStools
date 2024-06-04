/*
===============================================================================

  FILE:  laszip_common.h

  CONTENTS:

     LASzip common types

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

    30 April 2024 -- Created

===============================================================================
*/

#ifndef LASZIP_COMMON_H
#define LASZIP_COMMON_H

/// maximum length of
#define LAS_MAX_MESSAGE_LENGTH 8192

#if defined(_MSC_VER)
#include <sal.h>
/** activate printf-like format check for logging */
#  define LAS_FORMAT_STRING(arg) _Printf_format_string_ arg
#else
/** wrap the format argument of a printf-like function */
# define LAS_FORMAT_STRING(arg) arg
#endif

enum LAS_MESSAGE_TYPE
{
  LAS_DEBUG = 0,
  LAS_VERY_VERBOSE,
  LAS_VERBOSE,
  LAS_INFO,
  LAS_WARNING,
  LAS_SERIOUS_WARNING,
  LAS_ERROR,
  LAS_FATAL_ERROR,
  LAS_QUIET             //this is not supposed to be used in LASMessage, but in set_message_log_level for disabling any message
};

#endif /* LASZIP_COMMON_H */
