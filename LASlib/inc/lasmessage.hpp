/*
===============================================================================

  FILE:  lasmessage.hpp

  CONTENTS:

    This file defines message and logging functionality.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    07 Februar 2024 -- intial version

===============================================================================
*/
#ifndef LAS_MESSAGE_HPP
#define LAS_MESSAGE_HPP


#include "mydefs.hpp"

/// maximum length of
#define LAS_MAX_MESSAGE_LENGTH 4096

enum LAS_MESSAGE_TYPE {
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

#if defined(_MSC_VER)
#include <sal.h>
/** activate printf-like format check for logging */
#  define LAS_FORMAT_STRING(arg) _Printf_format_string_ arg
#else
/** wrap the format argument of a printf-like function */
# define LAS_FORMAT_STRING(arg) arg
#endif 


// central las message function 
void LASLIB_DLL LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*), ...);

// special debug message function that will be removed in release mode
#if defined(_DEBUG)|| !defined(NDEBUG)
#define LASDebug(...) LASMessage(LAS_DEBUG, __VA_ARGS__)
#else
#define LASDebug(...)  /* skip debug message in release mode at compile time. */
#endif

// The default laslib messsage handler outputs message to stderr. Per default all message types below info
// will be skipped. Using set_default_message_log_level the log level can be changed/increased. 

// set log level of default laslib message handler (default = LAS_INFO). This also affect a costum message
// handler which can be set by set_las_message_handler. Use LAS_QUIET to disable any output
void LASLIB_DLL set_message_log_level(LAS_MESSAGE_TYPE loglevel);
// get log level of default laslib message handler
LAS_MESSAGE_TYPE LASLIB_DLL get_message_log_level();


// callback function type for overwritting the default laslib message handler
typedef void  (*LASMessageHandler)(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

// set a new costum message handler (the user data pointer will be passed to the callback)
void LASLIB_DLL set_las_message_handler(LASMessageHandler callback, void* user_data = 0);
// restore the default laslib message handler
void LASLIB_DLL unset_las_message_handler();



#endif // LAS_MESSAGE_HPP
