/*
===============================================================================

  FILE:  mydefs.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2022, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "mydefs.hpp"

#include "lasmessage.hpp"
#include "laszip_common.h"

#include <cstring>
#include <filesystem>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <iomanip>
#include <unistd.h>
#endif

#if defined(_MSC_VER)
#include <windows.h>
/// Converting UTF-8 to UTF-16
wchar_t* UTF8toUTF16(const char* utf8) {
  if (utf8 == nullptr) return nullptr;
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
  if (len <= 0) return nullptr;
  wchar_t* utf16 = new wchar_t[len];
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, len);
  return utf16;
}

/// Converting ANSI to UTF-16
wchar_t* ANSItoUTF16(const char* ansi) {
  if (ansi == nullptr) return nullptr;
  int len = MultiByteToWideChar(CP_ACP, 0, ansi, -1, nullptr, 0);
  if (len <= 0) return nullptr;
  wchar_t* utf16 = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, ansi, -1, utf16, len);
  return utf16;
}
#endif

bool wait_on_exit
#if defined(__GNUC__)
    __attribute__((unused))
#endif
    = false;
bool print_log_stats = false;
bool halt_on_error = true;

LAS_EXIT_CODE las_exit_code(bool error) {
  return (error ? LAS_EXIT_ERROR : LAS_EXIT_OK);
}

void byebye() {
  // optional: print stats
  if (print_log_stats) {
    LASMessage(
        LAS_INFO, "Log stats: FE=%lu,E=%lu,SW=%lu,W=%lu,I=%lu", lasmessage_cnt[LAS_FATAL_ERROR], lasmessage_cnt[LAS_ERROR],
        lasmessage_cnt[LAS_SERIOUS_WARNING], lasmessage_cnt[LAS_WARNING], lasmessage_cnt[LAS_INFO]);
  }
  if (wait_on_exit) {
    std::fprintf(stderr, "<press ENTER>\n");
    std::getc(stdin);
  }
  //
  int code = 0;
  if (lasmessage_cnt[LAS_FATAL_ERROR] > 0) {
    code = 4;
  } else if (lasmessage_cnt[LAS_ERROR] > 0) {
    code = 3;
  } else if (lasmessage_cnt[LAS_SERIOUS_WARNING] > 0) {
    code = 2;
  } else if (lasmessage_cnt[LAS_WARNING] > 0) {
    code = 1;
  }
  exit(code);
}

/// Validates whether a given string is UTF-8 encoded.
bool validate_utf8(const char* utf8) noexcept {
  if (utf8 == nullptr) return false;

  while (*utf8) {
    if ((*utf8 & 0b10000000) != 0) {
      if ((*utf8 & 0b01000000) == 0) return false;
      if ((*utf8 & 0b00100000) != 0) {
        if ((*utf8 & 0b00010000) != 0) {
          if ((*utf8 & 0b00001000) != 0) return false;
          if ((*++utf8 & 0b11000000) != 0b10000000) return false;
        }
        if ((*++utf8 & 0b11000000) != 0b10000000) return false;
      }
      if ((*++utf8 & 0b11000000) != 0b10000000) return false;
    }
    ++utf8;
  }
  return true;
}

// To re-enable, change #if 0 to #if 1.
#if 0
  /// Non Windows - specific conversion ANSI to UTF-8
  char* ANSItoUTF8(const char* ansi)
  {
    if (ansi == nullptr) return nullptr;
  
    // Non-Windows-specific conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring utf16 = converter.from_bytes(ansi);
    size_t len = utf16.length() + 1;
  
    char* utf8 = new char[len];
    std::wcstombs(utf8, utf16.c_str(), len);
  
    return utf8;
  }
#endif

/// Opens a file with the specified filename and mode, converting filename and mode to UTF-16 on Windows.
FILE* LASfopen(const char* const filename, const char* const mode) {
  if (filename == nullptr || mode == nullptr || mode[0] == '\0') {
    return nullptr;
  }
  FILE* file = nullptr;

#ifdef _MSC_VER
  wchar_t* utf16_file_name = nullptr;
  wchar_t* utf16_mode = nullptr;

  if (validate_utf8(filename)) {
    utf16_file_name = UTF8toUTF16(filename);
  } else {
    utf16_file_name = ANSItoUTF16(filename);
  }
  // mode can be converted with UTF8toUTF16, as it contains only ASCII characters
  utf16_mode = UTF8toUTF16(mode);

  if (utf16_file_name && utf16_mode) {
    file = _wfopen(utf16_file_name, utf16_mode);
  }
  delete[] utf16_file_name;
  delete[] utf16_mode;
#else
// The following block of code is deactivated as it is very unlikely to be needed under non-Windows systems.
// To re-enable, change #if 0 to #if 1.
#if 0
    char* utf8_file_name = nullptr;
  
    if (validate_utf8(filename)) {
      file = fopen(filename, mode);
    } else {
      utf8_file_name = ANSItoUTF8(filename);
      file = fopen(utf8_file_name, mode);
    }
    delete[] utf8_file_name;
#else
  file = fopen(filename, mode);
#endif
#endif

  return file;
}

/// !!The caller is responsible for managing the memory of the returned const char*
/// using 'delete[]' when done!!
/// Indents each line of content by the given 'indent'
const char* indent_text(const char* text, const char* indent) {
  if (text == nullptr || indent == nullptr) return nullptr;

  size_t indent_len = strlen(indent);
  size_t text_len = strlen(text);

  // Count the number of line breaks and calculate the length of the new buffer
  size_t num_lines = 1;
  for (size_t i = 0; i < text_len; ++i) {
    if (text[i] == '\n') ++num_lines;
  }
  size_t new_len = text_len + num_lines * indent_len;
  char* result = new char[new_len + 1];

  const char* src = text;
  char* dest = result;

  // Add indentation prefix before each line
  while (*src) {
    // Add the indentation prefix
    memcpy(dest, indent, indent_len);
    dest += indent_len;

    while (*src && *src != '\n') {
      *dest++ = *src++;
    }
    // Add the line break, if available
    if (*src == '\n') *dest++ = *src++;
  }
  *dest = '\0';

  return result;
}

/// <summary>
/// Trim path to part prior last directory separator
/// Sample: c:\temp\a.exe >> c:\temp || /temp/a >> /temp
/// </summary>
/// <param name="path_len">current len of path</param>
/// <param name="path">pointer to path char</param>
void ExeNameToPathWithoutTrailingDelimiter(int& path_len, char* path) {
  while ((path_len > 0) && (path[path_len] != DIRECTORY_SLASH) && (path[path_len] != ':')) path_len--;
  path[path_len] = 0;
}

/// get the path of the exe file (incl. trailing path delimiter)
std::string exe_path() {
  size_t len = 0;
#ifdef _WIN32
  TCHAR path[MAX_PATH];
  GetModuleFileName(NULL, path, MAX_PATH);
#ifdef UNICODE
  len = wcslen(path);
#else
  len = strlen(path);
#endif
#else
  char path[MAX_PATH];
  len = readlink("/proc/self/exe", path, MAX_PATH);
#endif
  while (len && (path[len] != '\\') && (path[len] != '/')) len--;
  path[len] = '\0';
  //
#if defined(_WIN32) && defined(UNICODE)
  return bufferWToString(path, len) + DIRECTORY_SLASH;
#else
  return std::string(path, len) + DIRECTORY_SLASH;
#endif
}

/// <summary>
/// get the current directory (exclude trailing delimiter)
/// </summary>
std::string dir_current() {
  char curr_directory[MAX_PATH];
#ifdef _MSC_VER
  GetCurrentDirectory(MAX_PATH, curr_directory);
#else
  getcwd(curr_directory, MAX_PATH);
#endif
  return std::string(curr_directory);
}

/// replace string and return new string
std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}

/// replace string in current string
void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

/// checks if a fullString ends with a certain ending
bool StringEndsWith(const std::string& fullString, const std::string& ending) {
  if (ending.size() > fullString.size()) return false;
  return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
}
