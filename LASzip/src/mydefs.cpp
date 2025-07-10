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

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <filesystem>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <math.h>
#include <algorithm>
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

static bool wait_on_exit_ = false;
static bool print_log_stats_ = false;
static bool halt_on_error_ = true;

extern void LASLIB_DLL wait_on_exit(bool woe) {
  wait_on_exit_ = woe;
}

extern void LASLIB_DLL print_log_stats(bool pls) {
  print_log_stats_ = pls;
}

extern void LASLIB_DLL halt_on_error(bool hoe) {
  halt_on_error_ = hoe;
}

extern bool LASLIB_DLL do_halt_on_error() {
  return halt_on_error_;
}

LAS_EXIT_CODE las_exit_code(bool error) {
  return (error ? LAS_EXIT_ERROR : LAS_EXIT_OK);
}

extern void LASLIB_DLL byebye() {
  // optional: print stats
  if (print_log_stats_) {
    LASMessage(
        LAS_INFO, "Log stats: FE=%lu,E=%lu,SW=%lu,W=%lu,I=%lu", lasmessage_cnt[LAS_FATAL_ERROR], lasmessage_cnt[LAS_ERROR],
        lasmessage_cnt[LAS_SERIOUS_WARNING], lasmessage_cnt[LAS_WARNING], lasmessage_cnt[LAS_INFO]);
  }
  if (wait_on_exit_) {
    std::fprintf(stderr, "<press ENTER>\n");
    (void)std::getc(stdin);
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

/// replace all occurrences of search in subject with replace and return new string
std::string ReplaceString(const std::string& subject, const std::string& search, const std::string& replace) {
  // 'abc','a','ab' -> 'abbc'
  // 'abc','ab','a' -> 'ac'
  // 'aaa','a','aa' -> 'aaaaaa'
  // '1|   2','| ','|' -> '1,2'
  std::string result = subject;
  size_t pos = 0;
  size_t add = 0;
  // avoid endless replace if replace contains search token
  if (replace.find(search) != std::string::npos) {
    add = replace.length();
  }
  // replace loop
  while ((pos = result.find(search, pos)) != std::string::npos) {
    result.replace(pos, search.length(), replace);
    pos += add;  // set startpoint for next loop
  }
  return result;
}

/// replace all occurrences of search in subject with replace
void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace) {
  subject = ReplaceString(subject, search, replace);
}

/// checks if a fullString ends with a certain ending
bool StringEndsWith(const std::string& fullString, const std::string& ending) {
  if (ending.size() > fullString.size()) return false;
  return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
}

/// <summary>
/// Check if file has a certain file extension (case insensitive)
/// </summary>
/// <param name="fn">filename</param>
/// <param name="ext">extension, like ".laz" or "laz"</param>
/// <returns>TRUE if filename has the given extension</returns>
bool HasFileExt(std::string fn, std::string ext) {
  if (fn.empty()) return false;
  if (ext.empty()) return false;
  // if fn does not have a ext: it is maybe just an ext - compare those
  if ((fn.length() == ext.length()) || (fn.find_last_of(".") == std::string::npos)) {
    return fn.compare(ext) == 0;
  }
  if (ext[0] != '.') ext = '.' + ext;
  to_lower(ext);
  to_lower(fn);
  return (fn.substr(fn.find_last_of(".")) == ext);
}

// replace file extension of input_file with new extension (with or without leading '.')
std::string FileExtSet(std::string fn_in, std::string ext_new) {
  if (!ext_new.empty() && (ext_new[0] != '.')) ext_new = '.' + ext_new;
  size_t pos = fn_in.find_last_of('.');
  if (pos == std::string::npos) {
    return fn_in  + ext_new;
  } else {
    return fn_in.substr(0, pos) + ext_new;
  }
}

// checks if given file is a las/laz file
bool IsLasLazFile(std::string fn) {
  return HasFileExt(fn, "las") || HasFileExt(fn, "laz");
}

/// returns TRUE if 'val' is found in 'vec'
bool StringInVector(const std::string& val, const std::vector<std::string>& vec, bool casesense) {
  if (casesense) {
    return std::find(vec.begin(), vec.end(), val) != vec.end();
  } else {
    auto iterator = std::find_if(vec.begin(), vec.end(), [&](std::string s) { return (to_lower_copy(s).compare(to_lower_copy(val)) == 0); });
    return iterator != vec.end();
  }
}

/// extension of the realloc function to check memory allocation errors
void* realloc_las(void* ptr, size_t size) {
  void* temp = realloc(ptr, size);
  if (temp == NULL) {
    LASMessage(LAS_WARNING, "realloc_las: memory allocation failed\n");
    return ptr;
  } else {
    return temp;
  }
}

/// Wrapper for `vsscanf`
int sscanf_las(const char* buffer, const char* format, ...) {
  va_list args;
  va_start(args, format);

  int result = vsscanf(buffer, format, args);

  va_end(args);
  return result;
}

/// <summary>
/// secure wrapper for `strncpy`
/// target size defined, source size detected or defined.
/// target size needs to be 1 larger than bytes to copy to ensure trailing 0.
/// WARNING if target size is smaller than source data or requested size to copy.
/// ERROR (and halt by default) if strncopy failed.
/// </summary>
/// <param name="dest">target buffer</param>
/// <param name="destsz">size of target buffer include trailing 0</param>
/// <param name="src">source buffer</param>
/// <param name="count">number of bytes to copy without trailing 0; 0=detect number of bytes by src length</param>
void strncpy_las(char* dest, size_t destsz, const char* src, size_t count /*=0*/) {
  // source is empty -> set target empty and return
  if (src == nullptr) {
    if (destsz > 0 && dest != nullptr) {
      dest[0] = '\0';
    } 
    return;
  }
  // target NULL -> nothing to do
  if (dest == nullptr) {
    return;
  }
  // calculate src len if not given; crop len if src is shorter than defined len
  if (count == 0) {
    count = strlen(src);
  }
  // if target is smaller than source: copy as much as possible
  bool free = false;
  char* source;
  if (destsz <= count) {
    source = new char[destsz];
    memcpy(source, src, destsz - 1);
    source[destsz-1] = '\0';
    free = true;
    LASMessage(LAS_WARNING, "target buffer too small [%llu < %llu] for \"%s\"", destsz-1, count, src);
    count = destsz-1;
  } else {
    source = const_cast<char*>(src);
  }
#ifdef _MSC_VER
  errno_t err = strncpy_s(dest, destsz, source, count);
  if (err != 0) {
    laserror("strncpy_s failed: %d", err);
  }
#else
  strncpy(dest, source, count);
#endif
  if (free) {
    delete[] source;
  }
  // ensure string termination
  dest[count] = '\0';
}

#ifdef BOOST_USE
#else
// simple NON BOOST implementations
void to_lower(std::string& in) {
  for (size_t i = 0; i < in.size(); i++) in[i] = std::tolower(in[i]);
  return;
}

void to_upper(std::string& in) {
  for (size_t i = 0; i < in.size(); i++) in[i] = std::toupper(in[i]);
  return;
}

std::string to_lower_copy(const std::string& in) {
  std::string result = in;
  to_lower(result);
  return result;
}

std::string to_upper_copy(const std::string& in) {
  std::string result = in;
  to_lower(result);
  return result;
}

std::string trim(const std::string& in) {
  if (in.empty()) return "";
  size_t i = 0;
  while (i < in.length() && (in[i] == ' ' || in[i] == '\n' || in[i] == '\t' || in[i] == '\r' || in[i] == '\f' || in[i] == '\v')) i++;
  size_t j = in.length() - 1;
  while (j > 0 && (in[j] == ' ' || in[j] == '\n' || in[j] == '\t' || in[i] == '\r' || in[i] == '\f' || in[i] == '\v')) j--;
  if (j - i + 1 > 0) {
    return in.substr(i, j - i + 1);
  } else {
    return "";
  }
}
#endif

/// <summary>
/// Get next token till end of input
/// </summary>
/// <param name="in"></param>
/// <param name="delim"></param>
/// <param name="out"></param>
/// <returns></returns>
bool GetTokenNext(std::string& in, std::string delim, std::string& out) {
  size_t pos = in.find(delim);
  if (pos != std::string::npos) {
    out = in.substr(0, pos);
    in = in.substr(pos + delim.length(), in.length());
  } else {
    out = in;
    in = "";
  }
  return !(in.empty() && out.empty());
}

/// returns next token, "" if done or first empty token
std::string TokenNext(std::string& in, std::string delim) {
  std::string out;
  if (GetTokenNext(in, delim, out)) {
    return out;
  } else {
    return "";
  }
}

/// output all vector values separated by delimiter
std::string VectorDelimited(const std::vector<std::string>& items, const std::string& delimiter) {
  if (items.empty()) {
    return "";
  }
  std::string result = items[0];
  for (size_t i = 1; i < items.size(); ++i) {
    result += delimiter + items[i];
  }
  return result;
}

int stoidefault(const std::string& val, int def) {
  try {
    return std::stoi(val);
  } catch (const std::exception&) {
    return def;
  }
}

double stoddefault(const std::string& val, double def) {
  try {
    return std::stod(val);
  } catch (const std::exception&) {
    return def;
  }
}

double DoubleRound(double value, int decimals) {
  double scale = pow(10.0, decimals);  // e.g. 10^10 for 10 decimal places
  return std::round(value * scale) / scale;
}

std::string DoubleToString(double dd, short decimals, bool trim_right_zeros) {
  char xx[44];
  int cnt = snprintf(xx, 44, "%.*f", decimals, dd);
  if (trim_right_zeros) {
    while ((cnt > 0) && ((xx[cnt - 1] == '0') || (xx[cnt - 1] == '.'))) {
      xx[cnt - 1] = '\0';
      cnt--;
    }
  }
  return xx;
}

std::string CcToUnderline(const std::string& in) {
  std::string res = "";
  for (size_t ii = 0; ii < in.size(); ii++) {
    if (isupper(in[ii])) {
      if (ii > 0) {
        res = '_' + res;
      }
      res = tolower(in[ii]);
    } else {
      res = in[ii];
    }
  }
  return res;
}

/// returns the occurency count of 'toCount' in 'in'
size_t StringCountChar(const std::string& in, const char toCount) {
  int count = 0;
  for (size_t i = 0; i < in.size(); i++)
    if (in[i] == toCount) count++;
  return count;
}
