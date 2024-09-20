/*
===============================================================================

  FILE:  mydefs.hpp
  
  CONTENTS:

    Basic data type definitions and operations to be robust across platforms.
 
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
  
    28 October 2015 -- adding DLL bindings via 'COMPILE_AS_DLL' and 'USE_AS_DLL'
    10 January 2011 -- licensing change for LGPL release and libLAS integration
    13 July 2005 -- created after returning with many mosquito bites from OBX
  
===============================================================================
*/
#ifndef MYDEFS_HPP
#define MYDEFS_HPP

#ifndef _WIN32
#define LASLIB_DLL
#else  // _WIN32
#ifdef COMPILE_AS_DLL
#define LASLIB_DLL __declspec(dllexport)
#elif USE_AS_DLL
#define LASLIB_DLL __declspec(dllimport)
#else
#define LASLIB_DLL
#endif
#endif // _WIN32

#include <stdlib.h>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include "laszip_common.h"
#include "lasmessage.hpp"

extern void LASLIB_DLL LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*), ...);

typedef char               CHAR;

typedef int                I32;
typedef short              I16;
typedef char               I8;

typedef unsigned int       U32;
typedef unsigned short     U16;
typedef unsigned char      U8;

#if defined(_WIN32) && ! defined (__MINGW32__) // 64 byte integer under Windows 
typedef unsigned __int64   U64;
typedef __int64            I64;
#else                                          // 64 byte integer elsewhere ... 
typedef unsigned long long U64;
typedef long long          I64;
#endif

typedef float              F32;
typedef double             F64;

#if defined(_MSC_VER) || defined (__MINGW32__)
typedef int                BOOL;
#else
typedef bool               BOOL;
#endif

typedef union U32I32F32 { U32 u32; I32 i32; F32 f32; } U32I32F32;
typedef union U64I64F64 { U64 u64; I64 i64; F64 f64; } U64I64F64;
typedef union I64U32I32F32 { I64 i64; U32 u32[2]; I32 i32[2]; F32 f32[2]; } I64U32I32F32;

#define F32_MAX            +2.0e+37f
#define F32_MIN            -2.0e+37f

#define F64_MAX            +2.0e+307
#define F64_MIN            -2.0e+307

#define U8_MIN             ((U8)0x0)  // 0
#define U8_MAX             ((U8)0xFF) // 255
#define U8_MAX_MINUS_ONE   ((U8)0xFE) // 254
#define U8_MAX_PLUS_ONE    0x0100     // 256

#define U16_MIN            ((U16)0x0)    // 0
#define U16_MAX            ((U16)0xFFFF) // 65535
#define U16_MAX_MINUS_ONE  ((U16)0xFFFE) // 65534
#define U16_MAX_PLUS_ONE   0x00010000    // 65536

#define U32_MIN            ((U32)0x0)            // 0
#define U32_MAX            ((U32)0xFFFFFFFF)     // 4294967295
#define U32_MAX_MINUS_ONE  ((U32)0xFFFFFFFE)     // 4294967294
#if defined(WIN32)            // 64 byte unsigned int constant under Windows 
#define U32_MAX_PLUS_ONE   0x0000000100000000    // 4294967296
#else                         // 64 byte unsigned int constant elsewhere ... 
#define U32_MAX_PLUS_ONE   0x0000000100000000ull // 4294967296
#endif

#define I8_MIN             ((I8)0x80) // -128
#define I8_MAX             ((I8)0x7F) // 127

#define I16_MIN            ((I16)0x8000) // -32768
#define I16_MAX            ((I16)0x7FFF) // 32767

#define I32_MIN            ((I32)0x80000000) // -2147483648
#define I32_MAX            ((I32)0x7FFFFFFF) //  2147483647

#define I64_MIN            ((I64)0x8000000000000000)
#define I64_MAX            ((I64)0x7FFFFFFFFFFFFFFF)

#define U8_FOLD(n)      (((n) < U8_MIN) ? (n+U8_MAX_PLUS_ONE) : (((n) > U8_MAX) ? (n-U8_MAX_PLUS_ONE) : (n)))

#define I8_CLAMP(n)     (((n) <= I8_MIN) ? I8_MIN : (((n) >= I8_MAX) ? I8_MAX : ((I8)(n))))
#define U8_CLAMP(n)     (((n) <= U8_MIN) ? U8_MIN : (((n) >= U8_MAX) ? U8_MAX : ((U8)(n))))

#define I16_CLAMP(n)    (((n) <= I16_MIN) ? I16_MIN : (((n) >= I16_MAX) ? I16_MAX : ((I16)(n))))
#define U16_CLAMP(n)    (((n) <= U16_MIN) ? U16_MIN : (((n) >= U16_MAX) ? U16_MAX : ((U16)(n))))

#define I32_CLAMP(n)    (((n) <= I32_MIN) ? I32_MIN : (((n) >= I32_MAX) ? I32_MAX : ((I32)(n))))
#define U32_CLAMP(n)    (((n) <= U32_MIN) ? U32_MIN : (((n) >= U32_MAX) ? U32_MAX : ((U32)(n))))

#define I8_QUANTIZE(n) (((n) >= 0) ? (I8)((n)+0.5) : (I8)((n)-0.5))
#define U8_QUANTIZE(n) (((n) >= 0) ? (U8)((n)+0.5) : (U8)(0))

#define I16_QUANTIZE(n) (((n) >= 0) ? (I16)((n)+0.5) : (I16)((n)-0.5))
#define U16_QUANTIZE(n) (((n) >= 0) ? (U16)((n)+0.5) : (U16)(0))

#define I32_QUANTIZE(n) (((n) >= 0) ? (I32)((n)+0.5) : (I32)((n)-0.5))
#define U32_QUANTIZE(n) (((n) >= 0) ? (U32)((n)+0.5) : (U32)(0))

#define I64_QUANTIZE(n) (((n) >= 0) ? (I64)((n)+0.5) : (I64)((n)-0.5))
#define U64_QUANTIZE(n) (((n) >= 0) ? (U64)((n)+0.5) : (U64)(0))

#define I8_CLAMP_QUANTIZE(n)     (((n) <= I8_MIN) ? I8_MIN : (((n) >= I8_MAX) ? I8_MAX : (I8_QUANTIZE(n))))
#define U8_CLAMP_QUANTIZE(n)     (((n) <= U8_MIN) ? U8_MIN : (((n) >= U8_MAX) ? U8_MAX : (U8_QUANTIZE(n))))

#define I16_CLAMP_QUANTIZE(n)    (((n) <= I16_MIN) ? I16_MIN : (((n) >= I16_MAX) ? I16_MAX : (I16_QUANTIZE(n))))
#define U16_CLAMP_QUANTIZE(n)    (((n) <= U16_MIN) ? U16_MIN : (((n) >= U16_MAX) ? U16_MAX : (U16_QUANTIZE(n))))

#define I32_CLAMP_QUANTIZE(n)    (((n) <= I32_MIN) ? I32_MIN : (((n) >= I32_MAX) ? I32_MAX : (I32_QUANTIZE(n))))
#define U32_CLAMP_QUANTIZE(n)    (((n) <= U32_MIN) ? U32_MIN : (((n) >= U32_MAX) ? U32_MAX : (U32_QUANTIZE(n))))


#define I16_FLOOR(n) ((((I16)(n)) > (n)) ? (((I16)(n))-1) : ((I16)(n)))
#define I32_FLOOR(n) ((((I32)(n)) > (n)) ? (((I32)(n))-1) : ((I32)(n)))
#define I64_FLOOR(n) ((((I64)(n)) > (n)) ? (((I64)(n))-1) : ((I64)(n)))

#define I16_CEIL(n) ((((I16)(n)) < (n)) ? (((I16)(n))+1) : ((I16)(n)))
#define I32_CEIL(n) ((((I32)(n)) < (n)) ? (((I32)(n))+1) : ((I32)(n)))
#define I64_CEIL(n) ((((I64)(n)) < (n)) ? (((I64)(n))+1) : ((I64)(n)))

#define I8_FITS_IN_RANGE(n) (((n) >= I8_MIN) && ((n) <= I8_MAX) ? TRUE : FALSE)
#define U8_FITS_IN_RANGE(n) (((n) >= U8_MIN) && ((n) <= U8_MAX) ? TRUE : FALSE)
#define I16_FITS_IN_RANGE(n) (((n) >= I16_MIN) && ((n) <= I16_MAX) ? TRUE : FALSE)
#define U16_FITS_IN_RANGE(n) (((n) >= U16_MIN) && ((n) <= U16_MAX) ? TRUE : FALSE)
#define I32_FITS_IN_RANGE(n) (((n) >= I32_MIN) && ((n) <= I32_MAX) ? TRUE : FALSE)
#define U32_FITS_IN_RANGE(n) (((n) >= U32_MIN) && ((n) <= U32_MAX) ? TRUE : FALSE)

#define F32_IS_FINITE(n) ((F32_MIN < (n)) && ((n) < F32_MAX))
#define F64_IS_FINITE(n) ((F64_MIN < (n)) && ((n) < F64_MAX))

#define U32_ZERO_BIT_0(n) (((n)&(U32)0xFFFFFFFE))
#define U32_ZERO_BIT_0_1(n) (((n)&(U32)0xFFFFFFFC))

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef NULL
#define NULL    0
#endif

#ifdef _MSC_VER
#define strncpy_las(dest, destsz, src, count) strncpy_s((dest), (destsz), (src), (count))
#else
#define strncpy_las(dest, destsz, src, count) strncpy((dest), (src), (count)) 
#endif

#ifdef _MSC_VER
#define strcpy_las(dest, destsz, src) strcpy_s((dest), (destsz), (src))
#else
#define strcpy_las(dest, destsz, src) strcpy((dest), (src))
#endif

#ifdef _MSC_VER
#define sscanf_las(buf, format, ...) sscanf_s((buf), (format), __VA_ARGS__)
#else
#define sscanf_las(buf, format, ...) sscanf((buf), (format), __VA_ARGS__)
#endif

#ifdef _MSC_VER
#define strcat_las(dest, destsz, src) strcat_s((dest), (destsz), (src))
#else
#define strcat_las(dest, destsz, src) strcat((dest), (src))
#endif

#ifdef _MSC_VER
#define strdup_las(string) _strdup(string);
#else
#define strdup_las(string) strdup(string);
#endif

inline BOOL IS_LITTLE_ENDIAN()
{
  const U32 i = 1;
  return (*((const U8*)&i) == 1);
}

#define ENDIANSWAP16(n) \
	( ((((U16) n) << 8) & 0xFF00) | \
	  ((((U16) n) >> 8) & 0x00FF) )

#define ENDIANSWAP32(n) \
	( ((((U32) n) << 24) & 0xFF000000) |	\
	  ((((U32) n) <<  8) & 0x00FF0000) |	\
	  ((((U32) n) >>  8) & 0x0000FF00) |	\
	  ((((U32) n) >> 24) & 0x000000FF) )

inline void ENDIAN_SWAP_16_(U8* field)
{
  U8 help = field[0];
  field[0] = field[1];
  field[1] = help;
}

inline void ENDIAN_SWAP_32_(U8* field)
{
  U8 help;
  help = field[0];
  field[0] = field[3];
  field[3] = help;
  help = field[1];
  field[1] = field[2];
  field[2] = help;
}

inline void ENDIAN_SWAP_64_(U8* field)
{
  U8 help;
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

inline void ENDIAN_SWAP_16(const U8* from, U8* to)
{
  to[0] = from[1];
  to[1] = from[0];
}

inline void ENDIAN_SWAP_32(const U8* from, U8* to)
{
  to[0] = from[3];
  to[1] = from[2];
  to[2] = from[1];
  to[3] = from[0];
}

inline void ENDIAN_SWAP_64(const U8* from, U8* to)
{
  to[0] = from[7];
  to[1] = from[6];
  to[2] = from[5];
  to[3] = from[4];
  to[4] = from[3];
  to[5] = from[2];
  to[6] = from[1];
  to[7] = from[0];
}

#if defined(_MSC_VER)
#include <windows.h>
wchar_t* UTF8toUTF16(const char* utf8);
wchar_t* ANSItoUTF16(const char* ansi);
#endif

/// <summary>
/// exception within a file loop to proceed next file
/// </summary>
class exception_file_loop : public std::runtime_error
{
public:
  explicit exception_file_loop(char const* const _Message) noexcept : std::runtime_error(_Message) {};
};

/// <summary>
/// file loop exception on too less points to proceed
/// </summary>
class pnt_cnt_error : public exception_file_loop
{
public:
  explicit pnt_cnt_error() noexcept : exception_file_loop("too less points") {};
};

extern bool wait_on_exit;
extern bool halt_on_error;
extern bool print_log_stats;

enum LAS_EXIT_CODE { LAS_EXIT_OK = 0, LAS_EXIT_ERROR, LAS_EXIT_WARNING };

LAS_EXIT_CODE las_exit_code(bool error);

// void byebye(LAS_EXIT_CODE code = LAS_EXIT_OK);
void byebye();

#if 0
  // Non Windows - specific conversion ANSI to UTF-8
  char* ANSItoUTF8(const char* ansi);
#endif
// Validates whether a given string is UTF-8 encoded.
bool validate_utf8(const char* utf8) noexcept;
// Opens a file with the specified filename and mode, converting filename and mode to UTF-16 on Windows.
FILE* LASfopen(const char* const filename, const char* const mode);
const char* indent_text(const char* text, const char* indent);

// las error message function which leads to an immediate program stop by default
template<typename... Args>
void laserror(LAS_FORMAT_STRING(const char*) fmt, Args... args)
{
  LASMessage(LAS_ERROR, fmt, args...);
  if (halt_on_error)
  {
    byebye();
  }
  return;
};

// extended message with additional user info in console mode
template<typename... Args>
void laserrorm(LAS_FORMAT_STRING(const char*) fmt, Args... args)
{
  LASMessage(LAS_ERROR, fmt, args...);
  LASMessage(LAS_INFO, "\tcontact info@rapidlasso.de for support\n");
  if (halt_on_error)
  {
    byebye();
  }
  return;
};

// 32bit/64bit detection
#ifdef _WIN64
#define IS64 true
#elif _WIN32
#define IS64 false
#else
#define IS64 true
#endif

#ifdef _WIN32
#define DIRECTORY_SLASH '\\'
#else
#define DIRECTORY_SLASH '/'
#endif

#endif
