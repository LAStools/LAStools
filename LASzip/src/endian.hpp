/******************************************************************************
 * $Id$
 *
 * Project:  liblas - http://liblas.org - A BSD library for LAS format data.
 * Purpose:  Endian macros
 * Author:   Mateusz Loskot, mateusz@loskot.net
 *
 * This file has been stolen from <boost/endian.hpp> and
 * modified for libLAS purposes.
 * 
 * (C) Copyright Mateusz Loskot 2007, mateusz@loskot.net
 * (C) Copyright Caleb Epstein 2005
 * (C) Copyright John Maddock 2006
 * Distributed under the Boost  Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 * 
 * Revision History
 * 06 Feb 2006 - Initial Revision
 * 09 Nov 2006 - fixed variant and version bits for v4 guids
 * 13 Nov 2006 - added serialization
 * 17 Nov 2006 - added name-based guid creation
 * 20 Nov 2006 - add fixes for gcc (from Tim Blechmann)
 * 07 Mar 2007 - converted to header only
 * 20 Jan 2008 - removed dependency of Boost and modified for LASZIP (by Mateusz Loskot)
 ******************************************************************************
 *
 * Copyright (c) 1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * 
 *
 * Copyright notice reproduced from <boost/detail/limits.hpp>, from
 * which this code was originally taken.
 *
 * Modified by Caleb Epstein to use <endian.h> with GNU libc and to
 * defined the BOOST_ENDIAN macro.
 ****************************************************************************/

#ifndef LASZIP_DETAIL_ENDIAN_HPP_INCLUDED
#define LASZIP_DETAIL_ENDIAN_HPP_INCLUDED

// GNU libc offers the helpful header <endian.h> which defines
// __BYTE_ORDER

#if defined (__GLIBC__)
# include <endian.h>
# if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define LASZIP_LITTLE_ENDIAN
# elif (__BYTE_ORDER == __BIG_ENDIAN)
#  define LASZIP_BIG_ENDIAN
# elif (__BYTE_ORDER == __PDP_ENDIAN)
#  define LASZIP_PDP_ENDIAN
# else
#  error Unknown machine endianness detected.
# endif
# define LASZIP_BYTE_ORDER __BYTE_ORDER
#elif defined(_BIG_ENDIAN)
# define LASZIP_BIG_ENDIAN
# define LASZIP_BYTE_ORDER 4321
#elif defined(_LITTLE_ENDIAN)
# define LASZIP_LITTLE_ENDIAN
# define LASZIP_BYTE_ORDER 1234

#elif defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN) 
# define LASZIP_LITTLE_ENDIAN
# define LASZIP_BYTE_ORDER 1234

#elif defined(__sparc) || defined(__sparc__) \
   || defined(_POWER) || defined(__powerpc__) \
   || defined(__ppc__) || defined(__hpux) \
   || defined(_MIPSEB) || defined(_POWER) \
   || defined(__s390__)
# define LASZIP_BIG_ENDIAN
# define LASZIP_BYTE_ORDER 4321
#elif defined(__i386__) || defined(__alpha__) \
   || defined(__ia64) || defined(__ia64__) \
   || defined(_M_IX86) || defined(_M_IA64) \
   || defined(_M_ALPHA) || defined(__amd64) \
   || defined(__amd64__) || defined(_M_AMD64) \
   || defined(__x86_64) || defined(__x86_64__) \
   || defined(_M_X64)

# define LASZIP_LITTLE_ENDIAN
# define LASZIP_BYTE_ORDER 1234
#else
# error The file src/endian.hpp needs to be set up for your CPU type.
#endif


#if defined(LASZIP_BIG_ENDIAN)
# error LASzip is not currently big endian safe.  Please contact the authors on the liblas-devel mailing list for more information
#endif


#endif // LASZIP_DETAIL_ENDIAN_HPP_INCLUDED

