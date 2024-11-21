/*
===============================================================================

  FILE:  lasmessage.cpp

  CONTENTS:

    see corresponding header file

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

    see corresponding header file

===============================================================================
*/
#include "lasmessage.hpp"

#include <cassert>
#include <stdarg.h>
#include <stdio.h>
#include <string>

long lasmessage_cnt[LAS_QUIET];

void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

static LASMessageHandler message_handler = &las_default_message_handler;
static void* message_user_data = 0;
static LAS_MESSAGE_TYPE las_message_level = LAS_INFO;

std::string las_message_type_string(LAS_MESSAGE_TYPE in) {
  switch (in) {
    case LAS_DEBUG:
      return "DEBUG";
    case LAS_VERY_VERBOSE:
      return "VERY_VERBOSE";
    case LAS_VERBOSE:
      return "VERBOSE";
    case LAS_INFO:
      return "INFO";
    case LAS_WARNING:
      return "WARNING";
    case LAS_SERIOUS_WARNING:
      return "SERIOUS_WARNING";
    case LAS_ERROR:
      return "ERROR";
    case LAS_FATAL_ERROR:
      return "FATAL_ERROR";
    case LAS_QUIET:
      return "QUIET";
    default:
      return "?";
  }
}

void LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*) fmt, ...) {
  assert(type <= LAS_FATAL_ERROR);  // message type must be less than or equal to LAS_FATAL_ERROR (LAS_QUIET must not be used in LASMessage calls)

  lasmessage_cnt[type]++;

  if (type < las_message_level) return;

  char buffer[LAS_MAX_MESSAGE_LENGTH];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buffer, LAS_MAX_MESSAGE_LENGTH, fmt, args);
  if (len > LAS_MAX_MESSAGE_LENGTH) {  // avoid buffer overflow. messages longer than LAS_MAX_MESSAGE_LENGTH should not occur!
    fprintf(stderr, "INTERNAL: message cropped from %d to %d chars", len, LAS_MAX_MESSAGE_LENGTH);
    len = LAS_MAX_MESSAGE_LENGTH;
  }
  va_end(args);

  // remove trailing line feed
  while (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
    --len;
  }

  (*message_handler)(type, buffer, message_user_data);
}

void LASLIB_DLL set_message_log_level(LAS_MESSAGE_TYPE loglevel) {
  if (las_message_level != loglevel) {
    las_message_level = loglevel;
    LASMessage(LAS_VERY_VERBOSE, "Log level [%s]", las_message_type_string(loglevel).c_str());
  }
}

LAS_MESSAGE_TYPE LASLIB_DLL get_message_log_level() {
  return las_message_level;
}

void LASLIB_DLL set_las_message_handler(LASMessageHandler callback, void* user_data /*= 0*/) {
  message_handler = callback;
  message_user_data = user_data;
}

void LASLIB_DLL unset_las_message_handler() {
  message_handler = las_default_message_handler;
  message_user_data = 0;
}

void format_message(std::string& messsage, unsigned multiline_ident, bool append_trailing_lf = true) {
  size_t lines = messsage.find('\n');
  if (lines == std::string::npos) {
    lines = 1;
  }
  std::string result;
  result.reserve(messsage.size() + (lines - 1) * multiline_ident + 1);

  const std::string find_str = "\n\t";
  std::string replace_str((size_t)multiline_ident + 1, ' ');
  replace_str[0] = '\n';
  size_t start_pos = 0, pos = messsage.find(find_str, start_pos);
  while (pos != std::string::npos) {
    result += messsage.substr(start_pos, pos - start_pos);
    result += replace_str;
    start_pos = pos + find_str.size();
    pos = messsage.find(find_str, start_pos);
  }
  result += messsage.substr(start_pos, pos - start_pos);

  if (append_trailing_lf) result += "\n";

  messsage = result;
}

void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data) {
  std::string prefix;
  std::string message(msg);
  switch (type) {
    case LAS_DEBUG:
      // prefix = "";	//add possible prefix
      break;
    case LAS_VERY_VERBOSE:
      // prefix = "";	//add possible prefix
      break;
    case LAS_VERBOSE:
      // prefix = "";	//add possible prefix
      break;
    case LAS_INFO:
      // prefix = "";	//add possible prefix
      break;
    case LAS_WARNING:
      prefix = "WARNING: ";
      break;
    case LAS_SERIOUS_WARNING:
      prefix = "SERIOUS WARNING: ";
      break;
    case LAS_ERROR:
      prefix = "ERROR: ";
      break;
    case LAS_FATAL_ERROR:
      prefix = "FATAL ERROR: ";
      break;
    case LAS_QUIET:
      break;
  }

  if (!prefix.empty()) {
    format_message(message, (unsigned)prefix.size());
    fprintf(stderr, "%s", prefix.c_str());
    fprintf(stderr, "%s", message.c_str());
  } else {
    fprintf(stderr, "%s\n", message.c_str());
  }
}

LASMessageStream& LASMessageStream::operator<<(std::ostream& (*func)(std::ostream&)) {
  usedStream << func;
  return *this;
}

LASMessageStream& LASMessageStream::precision(int prec) {
  usedStream << std::setprecision(prec);
  return *this;
}
