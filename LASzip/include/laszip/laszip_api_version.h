/*
===============================================================================

  FILE:  laszip_api_version.h

  CONTENTS:

    Version information for LASzip API interface
    
  NOTE:
    laszip_api_version.h will be generated dynamic out of laszip_api_version.h.in by cmake build.
    The file laszip_api_version.h is static in the repository to allow also a *.sln build.  

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

    22 August 2017 -- Created

===============================================================================
*/

#ifndef LASZIP_API_VERSION_H
#define LASZIP_API_VERSION_H

/*
 * version settings
 */
#define LASZIP_API_VERSION_MAJOR 3
#define LASZIP_API_VERSION_MINOR 5
#define LASZIP_API_VERSION_PATCH 1

#define HAVE_UNORDERED_MAP 1

#endif /* LASZIP_API_VERSION_H */
