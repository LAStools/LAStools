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
#include <string>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(_WIN32)
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
  //duplicate log messages with the number of repeated times.
  flush_repeated_logs();
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
bool validate_utf8(const char* str, bool restrict_to_two_bytes) noexcept {
  if (!str) {
    return false;
  }

  constexpr uint8_t UTF8_2BYTE_MASK = 0xE0;
  constexpr uint8_t UTF8_2BYTE_PREFIX = 0xC0;
  constexpr uint8_t UTF8_3BYTE_MASK = 0xF0;
  constexpr uint8_t UTF8_3BYTE_PREFIX = 0xE0;
  constexpr uint8_t UTF8_4BYTE_MASK = 0xF8;
  constexpr uint8_t UTF8_4BYTE_PREFIX = 0xF0;
  constexpr uint8_t UTF8_CONTINUATION_MASK = 0xC0;
  constexpr uint8_t UTF8_CONTINUATION_PREFIX = 0x80;

  // bool has_two_byte = false;
  const auto* p = reinterpret_cast<const unsigned char*>(str);

  while (*p) {
    uint8_t c = *p;

    // Single-byte ASCII (U+0000 - U+007F)
    if (c <= 0x7F) {
      ++p;
      continue;
    }

    // Detect standalone high bytes (0x80-0xFF), common in Windows-1252
    if (c >= 0x80 && (c < 0xC2 || (c & UTF8_2BYTE_MASK) != UTF8_2BYTE_PREFIX) && (c & UTF8_3BYTE_MASK) != UTF8_3BYTE_PREFIX &&
        (c & UTF8_4BYTE_MASK) != UTF8_4BYTE_PREFIX) {
      return false;  // Invalid UTF-8 start byte, likely ANSI
    }

    // 2-byte sequence (U+0080 - U+07FF)
    if ((c & UTF8_2BYTE_MASK) == UTF8_2BYTE_PREFIX) {
      if (c < 0xC2 || !p[1]) {
        return false;  // Overlong encoding or missing continuation byte
      }
      uint8_t c1 = p[1];
      if ((c1 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX) {
        // Check for ANSI-like second byte (0x40-0x7F), common in GBK/Shift-JIS
        if (c1 >= 0x40 && c1 <= 0x7F) {
          return false;  // Likely GBK or Shift-JIS, not UTF-8
        }
        return false;  // Invalid continuation byte
      }
      // Decode and check code point to exclude rare ranges
      uint32_t code_point = ((c & 0x1F) << 6) | (c1 & 0x3F);
      if (code_point < 0x80 || code_point > 0x7FF || (code_point >= 0x0080 && code_point <= 0x05FF)) {
        return false;  // Invalid or rare code point, likely ANSI
      }
      // has_two_byte = true;
      p += 2;
      continue;
    }

    // Restrict to 2-byte sequences if specified
    if (restrict_to_two_bytes) {
      return false;
    }

    // 3-byte sequence (U+0800 - U+FFFF)
    if ((c & UTF8_3BYTE_MASK) == UTF8_3BYTE_PREFIX) {
      if (!p[1] || !p[2]) {
        return false;  // Missing continuation bytes
      }
      uint8_t c1 = p[1], c2 = p[2];
      if ((c1 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX || (c2 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX) {
        return false;  // Invalid continuation bytes
      }
      // Check for overlong encoding or surrogate pairs (U+D800-U+DFFF)
      if ((c == 0xE0 && c1 < 0xA0) || (c == 0xED && c1 >= 0xA0)) {
        return false;
      }
      p += 3;
      continue;
    }

    // 4-byte sequence (U+10000 - U+10FFFF)
    if ((c & UTF8_4BYTE_MASK) == UTF8_4BYTE_PREFIX) {
      if (!p[1] || !p[2] || !p[3]) {
        return false;  // Missing continuation bytes
      }
      uint8_t c1 = p[1], c2 = p[2], c3 = p[3];
      if ((c1 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX || (c2 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX ||
          (c3 & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PREFIX) {
        return false;  // Invalid continuation bytes
      }
      // Check valid code point range
      if ((c == 0xF0 && c1 < 0x90) || (c == 0xF4 && c1 > 0x8F) || (c > 0xF4)) {
        return false;
      }
      p += 4;
      continue;
    }

    return false;  // Invalid start byte
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

#ifdef _WIN32
  wchar_t* utf16_file_name = nullptr;
  wchar_t* utf16_mode = nullptr;

#ifdef FORCE_UTF8_PATH
  utf16_file_name = UTF8toUTF16(filename);
#else
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
#endif
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

/// <summary>
/// delete a file on common OS and report failure
/// </summary>
/// <param name="filename">absolute filename to delete</param>
/// <param name="onFailMsg">message type on failure</param>
void FileDelete(std::string filename, LAS_MESSAGE_TYPE onFailMsg) {
  if (filename.empty()) return;
  std::string cmd;
#ifdef _WIN32
  cmd = "del ";
#else
  cmd = "rm ";
#endif
  cmd += filename;
  LASMessage(LAS_VERY_VERBOSE, "deleting '%s'", filename.c_str());
  int res = system(cmd.c_str());
  if (res != 0) {
    LASMessage(onFailMsg, "delete of file '%s' failed [%d]", filename.c_str(), res);
  }
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
  TCHAR path[MAX_PATH_LAS];
  GetModuleFileName(NULL, path, MAX_PATH_LAS);
#ifdef UNICODE
  len = wcslen(path);
#else
  len = strlen(path);
#endif
#else
  char path[MAX_PATH_LAS];
  len = readlink("/proc/self/exe", path, MAX_PATH_LAS);
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
  char curr_directory[MAX_PATH_LAS];
#ifdef _WIN32
  GetCurrentDirectory(MAX_PATH_LAS, curr_directory);
#else
  getcwd(curr_directory, MAX_PATH_LAS);
#endif
  return std::string(curr_directory);
}

/// <summary>
/// get the temp path include trailing delimiter. system default or temp_user.
/// responsitility of the user if temp_user is a valid and terminated path.
/// </summary>
/// <param name="temp_user">optional user defined temp path</param>
/// <returns>temp path to use</returns>
std::string temp_path(std::string temp_user) {
  std::string td;
  if (temp_user.empty()) {
    td = std::filesystem::temp_directory_path().string();
    if (td.empty()) {
      td = ".";
    }
  } else {
    td = temp_user;
  }
  if (td.back() != DIRECTORY_SLASH) {
    td += DIRECTORY_SLASH;
  }
  return td;
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

/// Returns the extension of the file
std::string getFileExtension(const char* filepath) {
  if (!filepath) return "";

  const char* dot = strrchr(filepath, '.');
  if (!dot || *(dot + 1) == '\0') return "";  // No point or nothing after that

  std::string ext(dot + 1);
  // Optional: in lowercase letters
  for (char& c : ext) c = std::tolower(static_cast<unsigned char>(c));
  return ext;
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

/// Converts a byte count into a human-readable value and unit
void bytes_to_readable(size_t bytes, double* value_out, const char** unit_out) {
  double value = (double)bytes;
  const char* unit = "Bytes";

  if (bytes >= (1ULL << 30)) {  // 1 GB
    value = value / (1ULL << 30);
    unit = "GB";
  } else if (bytes >= (1ULL << 20)) {  // 1 MB
    value = value / (1ULL << 20);
    unit = "MB";
  } else if (bytes >= (1ULL << 10)) {  // 1 KB
    value = value / (1ULL << 10);
    unit = "KB";
  }

  if (value_out) *value_out = value;
  if (unit_out) *unit_out = unit;
}

/// Returns the amount of currently available physical RAM in bytes
/// Note that this value is only a snapshot.
#ifdef _WIN32
size_t get_available_RAM() {
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    return statex.ullAvailPhys;
  }
  return 0;
}
#else
size_t get_available_RAM() {
  FILE* f = fopen("/proc/meminfo", "r");
  if (!f) return 0;
  size_t avail = 0;
  char line[256];
  while (fgets(line, sizeof(line), f)) {
    if (sscanf(line, "MemAvailable: %zu kB", &avail) == 1) {
      avail *= 1024;
      break;
    }
  }
  fclose(f);
  return avail;
}
#endif

/// Checks whether the available RAM is larger than the requested allocation. 
/// The check is only performed for requests of at least 1 MB to avoid overhead for small allocations.
bool check_available_RAM(size_t size) {
  const size_t MIN_MB = 1024 * 1024;  // 1 MB in Bytes

  if (size > MIN_MB) {
    size_t ram_size = get_available_RAM();

    if (size > ram_size) {
      double value_1, value_2;
      const char *unit_1, *unit_2;
    
      bytes_to_readable(size, &value_1, &unit_1);
      bytes_to_readable(ram_size, &value_2, &unit_2);
      LASMessage(LAS_WARNING, "failed to allocate %.0f%s memory as only approx. %.0f%s free reported. An error may occur", value_1, unit_1, value_2, unit_2);
      return true;
    }
  }
  return false;
}

/// extension of the realloc function to check memory allocation errors
void* realloc_las(void* ptr, size_t size) {
  bool warning = check_available_RAM(size);

  void* temp = realloc(ptr, size);
  if (!temp && !warning) {
    LASMessage(LAS_WARNING, "memory reallocation failed");
    return ptr;
  } else {
    return temp;
  }
}

/// extension of the malloc function to check memory allocation errors
void* malloc_las(size_t size) {
  bool warning = check_available_RAM(size);

  void* ptr = malloc(size);
  if (!ptr && !warning) {
    LASMessage(LAS_WARNING, "memory allocation failed");
  }
  return ptr;
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

/// Function for determining the standard programme paths
const char** getDefaultProgramPaths(size_t& numPaths) {
#ifdef _WIN32
  // Windows: Use environment variables or API for Program Files directories
  static const char* defaultPaths[] = {getenv("ProgramFiles"), getenv("ProgramFiles(x86)"), "C:\\", nullptr};
  // Count valid paths
  numPaths = 0;
  while (defaultPaths[numPaths] != nullptr) {
    ++numPaths;
  }
  return defaultPaths;
#elif __APPLE__
  // macOS: Standard directories
  static const char* defaultPaths[] = {"/Applications/", "/usr/local/", nullptr};
  numPaths = sizeof(defaultPaths) / sizeof(defaultPaths[0]) - 1;
  return defaultPaths;
#else
  // Linux/Unix: Standard directories
  static const char* defaultPaths[] = {"/usr/local/", "/opt/", "/usr/share/", nullptr};
  numPaths = sizeof(defaultPaths) / sizeof(defaultPaths[0]) - 1;
  return defaultPaths;
#endif
}

/// Function to get the home directory of the current user
const char* getHomeDirectory() {
#ifdef _WIN32
  return getenv("USERPROFILE");
#else
  return getenv("HOME");
#endif
}

/// Does the file exist
BOOL file_exists(const std::string& path) {
  FILE* file = std::fopen(path.c_str(), "r");
  if (file) {
    fclose(file);
    return TRUE;
  }
  return FALSE;
}

/// Get the digits
I32 get_digits(F64 scale_factor) {
  if (fp_equal(scale_factor, 0.01)) {
    return 2;
  } else if (fp_equal(scale_factor, 0.001)) {
    return 3;
  } else if (fp_equal(scale_factor, 0.0025) || fp_equal(scale_factor, 0.0001)) {
    return 4;
  } else if (fp_equal(scale_factor, 0.00025) || fp_equal(scale_factor, 0.00001)) {
    return 5;
  } else if (fp_equal(scale_factor, 0.000001)) {
    return 6;
  } else if (fp_equal(scale_factor, 0.0000001)) {
    return 7;
  } else if (fp_equal(scale_factor, 0.00000001)) {
    return 8;
  }
  return -1;
}

/// endians 
namespace Endian {
/// Checks at runtime whether the system stores its multi-byte numbers in little-endian format in memory
/// Initialisation when the programme starts, once only
const bool IS_LITTLE_ENDIAN = []() {
  uint16_t test = 1;
  return *reinterpret_cast<uint8_t*>(&test) == 1;
}();

void to_big_endian(int* value) {
  if (Endian::IS_LITTLE_ENDIAN) {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[3];
    field[3] = help;
    help = field[1];
    field[1] = field[2];
    field[2] = help;
  }
}

void to_little_endian(int* value) {
  if (!Endian::IS_LITTLE_ENDIAN) {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[3];
    field[3] = help;
    help = field[1];
    field[1] = field[2];
    field[2] = help;
  }
}

void to_big_endian(double* value) {
  if (Endian::IS_LITTLE_ENDIAN) {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[7];
    field[7] = help;
    help = field[1];
    field[1] = field[6];
    field[6] = help;
    help = field[2];
    field[2] = field[5];
    field[5] = help;
    help = field[3];
    field[3] = field[4];
    field[4] = help;
  }
}

void to_little_endian(double* value) {
  if (!Endian::IS_LITTLE_ENDIAN) {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[7];
    field[7] = help;
    help = field[1];
    field[1] = field[6];
    field[6] = help;
    help = field[2];
    field[2] = field[5];
    field[5] = help;
    help = field[3];
    field[3] = field[4];
    field[4] = help;
  }
}
}  // namespace Endian
