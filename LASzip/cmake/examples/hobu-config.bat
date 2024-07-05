@echo off

set G="NMake Makefiles"
REM set G="Visual Studio 9 2008"
REM set G="Visual Studio 10"
REM set BUILD_TYPE=RelWithDebInfo
REM set BUILD_TYPE=Debug
set BUILD_TYPE=Release
    
cmake -G %G% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_VERBOSE_MAKEFILE=OFF ^
    -DBUILD_STATIC=FALSE ^
    .
