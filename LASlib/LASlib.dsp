# Microsoft Developer Studio Project File - Name="LASlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LASlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LASlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LASlib.mak" CFG="LASlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LASlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "LASlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LASlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\laszip\src" /I "inc" /I "..\laszip\inc" /I "..\laszip\stl" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\LASlib.lib lib\LASlib.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "LASlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\laszip\src" /I "inc" /I "..\laszip\inc" /I "..\laszip\stl" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\LASlib.lib lib\LASlibD.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "LASlib - Win32 Release"
# Name "LASlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\LASzip\src\arithmeticdecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\arithmeticencoder.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\arithmeticmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fopen_compressed.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\integercompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasignore.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasindex.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasinterval.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laskdtree.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasquadtree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_asc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_bil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_bin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_dtm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_las.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_ply.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_qfit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_shp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreader_txt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaderbuffered.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreadermerged.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaderpipeon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaderstored.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v3.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v4.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreadpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lastransform.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasutility.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswaveform13reader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswaveform13writer.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v3.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v4.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswritepoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter_bin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter_las.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter_qfit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter_txt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriter_wrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswritercompatible.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip.cpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\mydefs.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\LASzip\src\arithmeticdecoder.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\arithmeticencoder.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\arithmeticmodel.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamin.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamin_array.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamin_file.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamin_istream.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamout.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamout_array.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamout_file.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamout_nil.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\bytestreamout_ostream.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\integercompressor.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasattributer.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasdefinitions.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasfilter.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasignore.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasindex.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasinterval.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laskdtree.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laspoint.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasquadtree.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasquantizer.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_asc.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_bil.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_bin.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_dtm.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_las.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_ply.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_qfit.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_shp.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreader_txt.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreaderbuffered.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreadermerged.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreaderpipeon.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasreaderstored.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditem.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v3.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemcompressed_v4.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreaditemraw.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\lasreadpoint.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lastransform.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasutility.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasvlr.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\lasvlrpayload.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswaveform13reader.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswaveform13writer.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitem.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v3.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemcompressed_v4.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswriteitemraw.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laswritepoint.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter_bin.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter_las.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter_qfit.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter_txt.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswriter_wrl.hpp
# End Source File
# Begin Source File

SOURCE=.\inc\laswritercompatible.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip_common_v1.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip_common_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip_common_v3.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\laszip_decompress_selective_v3.hpp
# End Source File
# Begin Source File

SOURCE=..\LASzip\src\mydefs.hpp
# End Source File
# End Group
# End Target
# End Project
