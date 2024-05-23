/*
===============================================================================

  FILE:  lasmessage.hpp

  CONTENTS:

    This file defines message and logging functionality.

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

    07 Februar 2024 -- initial version

===============================================================================
*/
#ifndef LAS_MESSAGE_HPP
#define LAS_MESSAGE_HPP

#include <sstream>
#include <string>
#include <iomanip>
#include "mydefs.hpp"
#include "laszip_common.h"

extern long lasmessage_cnt[LAS_QUIET];

// central las message function
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif
void LASLIB_DLL LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*), ...);

// special debug message function that will be removed in release mode
#if defined(_DEBUG)|| !defined(NDEBUG)
#define LASDebug(...) LASMessage(LAS_DEBUG, __VA_ARGS__)
#else
#define LASDebug(...)  /* skip debug message in release mode at compile time. */
#endif

// The default laslib message handler outputs message to stderr. Per default all message types below info
// will be skipped. Using set_default_message_log_level the log level can be changed/increased.

// set log level of default laslib message handler (default = LAS_INFO). This also affect a custom message
// handler which can be set by set_las_message_handler. Use LAS_QUIET to disable any output
void LASLIB_DLL set_message_log_level(LAS_MESSAGE_TYPE loglevel);
// get log level of default laslib message handler
LAS_MESSAGE_TYPE LASLIB_DLL get_message_log_level();

// callback function type for overwriting the default laslib message handler
typedef void  (*LASMessageHandler)(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

// set a new custom message handler (the user data pointer will be passed to the callback)
void LASLIB_DLL set_las_message_handler(LASMessageHandler callback, void* user_data = 0);
// restore the default laslib message handler
void LASLIB_DLL unset_las_message_handler();

// @brief Logger-Wrapper that allows to stream to the logger.
// @usage LASMessageStream(LAS_FATAL_ERROR) << "Pi is approximately: " << LASMessageStream().precision(2) << 3.14159 << std::endl;
class LASMessageStream
{
public:
    LASMessageStream(LAS_MESSAGE_TYPE type) : setType(type) {}
    ~LASMessageStream() { if (setType != LAS_QUIET) LASMessage(setType, usedStream.str().c_str()); }

    template <typename T>
    LASMessageStream& operator<<(const T& value)
    {
        usedStream << value;
        return *this;
    }

    // just in case someone streams std::endl ...
    LASMessageStream& operator<<(std::ostream& (*func)(std::ostream&));
    LASMessageStream& precision(int prec);

private:
    LAS_MESSAGE_TYPE setType;
    std::ostringstream usedStream;
};

#define logdebug LASMessageStream(LAS_DEBUG)
#define logveryverbose LASMessageStream(LAS_VERY_VERBOSE)
#define logverbose LASMessageStream(LAS_VERBOSE)
#define loginfo LASMessageStream(LAS_INFO)
#define logwarning LASMessageStream(LAS_WARNING)
#define logseriouswarning LASMessageStream(LAS_SERIOUS_WARNING)
#define logerror LASMessageStream(LAS_ERROR)
#define logfatalerror LASMessageStream(LAS_FATAL_ERROR)
#define logquiet LASMessageStream(LAS_QUIET)

#endif // LAS_MESSAGE_HPP
