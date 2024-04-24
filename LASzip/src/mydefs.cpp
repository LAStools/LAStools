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
#include <stdio.h>
#include <stdarg.h>
#include "laszip_common.h"
#include "lasmessage.hpp"

#if defined(_MSC_VER)
#include <windows.h>
wchar_t* UTF8toUTF16(const char* utf8)
{
  wchar_t* utf16 = 0;
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, 0, 0);
  if (len > 0)
  {
    utf16 = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, len);
  }
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

LAS_EXIT_CODE las_exit_code(bool error)
{
  return (error ? LAS_EXIT_ERROR : LAS_EXIT_OK);
}

void byebye()
{
  // optional: print stats
  if (print_log_stats)
  {
    LASMessage(LAS_INFO, "Log stats: FE=%lu,E=%lu,SW=%lu,W=%lu,I=%lu",
      lasmessage_cnt[LAS_FATAL_ERROR],
      lasmessage_cnt[LAS_ERROR],
      lasmessage_cnt[LAS_SERIOUS_WARNING],
      lasmessage_cnt[LAS_WARNING],
      lasmessage_cnt[LAS_INFO]);
  }
  if (wait_on_exit)
  {
    std::fprintf(stderr, "<press ENTER>\n");
    std::getc(stdin);
  }
  //
  int code = 0;
  if (lasmessage_cnt[LAS_FATAL_ERROR] > 0)
  {
    code = 4;
  }
  else if (lasmessage_cnt[LAS_ERROR] > 0)
  {
    code = 3;
  }
  else if (lasmessage_cnt[LAS_SERIOUS_WARNING] > 0)
  {
    code = 2;
  }
  else if (lasmessage_cnt[LAS_WARNING] > 0)
  {
    code = 1;
  }
  exit(code);
}
