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
#include <unordered_map>
#include <regex>
#include <cstring>

long lasmessage_cnt[LAS_QUIET];

void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

static LASMessageHandler message_handler = &las_default_message_handler;
static void* message_user_data = 0;
static LAS_MESSAGE_TYPE las_message_level = LAS_INFO;

struct LogData {
  LAS_MESSAGE_TYPE type;
  U32 count = 0;
  U32 rep_times = 5;
};

static std::unordered_map<std::string, LogData> ext_log_entrys;
static std::string current_msg;
static U32 rep_log_msg = 0;
static U8 max_rep_log_msg = 15;  // Maximum number of repeated log messages

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

void buildMessage(char *buffer, LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*) fmt, va_list args) {
  int len = vsnprintf(buffer, LAS_MAX_MESSAGE_LENGTH, fmt, args);
  if (len > LAS_MAX_MESSAGE_LENGTH) {  // avoid buffer overflow. messages longer than LAS_MAX_MESSAGE_LENGTH should not occur!
    fprintf(stderr, "INTERNAL: message cropped from %d to %d chars", len, LAS_MAX_MESSAGE_LENGTH);
    len = LAS_MAX_MESSAGE_LENGTH;
  }
  // remove trailing line feed
  while (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
    --len;
  }
}

void LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*) fmt, ...) {
  assert(type <= LAS_FATAL_ERROR);  // message type must be less than or equal to LAS_FATAL_ERROR (LAS_QUIET must not be used in LASMessage calls)

  lasmessage_cnt[type]++;

  if (type < las_message_level) return;

  bool clear_log_entry = false;
  va_list args;
  va_start(args, fmt);
  char buffer[LAS_MAX_MESSAGE_LENGTH];
  buildMessage(buffer, type, fmt, args);
  va_end(args);

  if (current_msg != fmt) {
    if (rep_log_msg > max_rep_log_msg) {
        std::string rep_msg = "(log message repeated " + std::to_string(rep_log_msg) + " times)";
        (*message_handler)(LAS_INFO, rep_msg.c_str(), message_user_data);
    }
    // message template (unformatted message)
    current_msg = fmt;
    rep_log_msg = 1; 
  } else {
    ++rep_log_msg;
  }

  if (rep_log_msg <= max_rep_log_msg) {
    (*message_handler)(type, buffer, message_user_data);
  }
}

/// Reduces repetitive log messages and summarises them.
/// On the one hand, both formatted and unformatted messages (arguments inserted and not) are collected.
/// ext_cnt specifies whether the summary should be displayed at the end of the log.
/// rep_times specifies how often the message should be repeated in the log before it is summarised
void LASMessageExt(LAS_MESSAGE_TYPE type, unsigned int rep_times, LAS_FORMAT_STRING(const char*) fmt, ...) {
  assert(type <= LAS_FATAL_ERROR);  // message type must be less than or equal to LAS_FATAL_ERROR (LAS_QUIET must not be used in LASMessage calls)

  lasmessage_cnt[type]++;

  if (type < las_message_level) return;

  // Check whether summarisation is still required for normal logging.
  if (current_msg != fmt) {
    if (rep_log_msg > max_rep_log_msg) {
      std::string rep_msg = "(log message repeated " + std::to_string(rep_log_msg) + " times)";
      (*message_handler)(LAS_INFO, rep_msg.c_str(), message_user_data);
    }
    rep_log_msg = 0;
  }

  va_list args;
  va_start(args, fmt);
  char buffer[LAS_MAX_MESSAGE_LENGTH];
  buildMessage(buffer, type, fmt, args);
  va_end(args);

  // formatted message only fetch if fmt == buffer
  LogData& data = ext_log_entrys[buffer];
  // message template (unformatted message)
  LogData& data_unform = ext_log_entrys[fmt];

  if (data.count < rep_times && data_unform.count < rep_times) {
    std::string msg = buffer;

    if (data.count == 0) {
      data.type = type;
      data.rep_times = rep_times;
    }
    (*message_handler)(type, msg.c_str(), message_user_data);
    if ((data.count == rep_times - 1 || data_unform.count == rep_times - 1)) {
      const char *rep_msg = "(This message hit the repetition limit - summary provided at the end of the log)";
      (*message_handler)(LAS_INFO, rep_msg, message_user_data);
    }
  }

  if (strcmp(fmt, buffer) != 0) {
    if (data_unform.count == 0) {
      data_unform.type = type;
      data_unform.rep_times = rep_times;
    } else if (data.count == rep_times && data_unform.count >= rep_times) {
      ++data_unform.count;
      // formatted log 'data.count' DO NOT increase
      return;
    }
    ++data_unform.count;
  }
  ++data.count;
}

/// Writes the summarised repeated messages at the end of the log. 
/// Replaces the placeholders in unformatted messages '%...' with # using a regex.
void flush_repeated_logs() {
  bool printedHeader = false;

  for (const auto& [msg, data] : ext_log_entrys) {
    if (data.count > data.rep_times) {
      std::string repeated_msg = msg + " (repeated times: " + std::to_string(data.count) + ")";
      // replace all placeholder '%...' with '#'
      if (repeated_msg.find('%') != std::string::npos) {
        std::regex fmt_regex(R"(%([-+0 #]*)?(\d+|\*)?(\.\d+)?(hh|h|ll|l|j|z|t|L)?([diuoxXfFeEgGaAcspn%]))");
        repeated_msg = std::regex_replace(repeated_msg, fmt_regex, "#");  // Replaces every placeholder with #
      }
      if (!repeated_msg.empty() && repeated_msg != "#") {
        if (!printedHeader) {
          // Output heading once
          (*message_handler)(LAS_INFO, "Summary of repeated logs:", message_user_data);
          printedHeader = true;
        }
        (*message_handler)(data.type, repeated_msg.c_str(), message_user_data);
      }
    }
  }
  ext_log_entrys.clear();
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
