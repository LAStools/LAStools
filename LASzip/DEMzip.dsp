# Microsoft Developer Studio Project File - Name="DEMzip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DEMzip - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DEMzip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DEMzip.mak" CFG="DEMzip - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DEMzip - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DEMzip - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DEMzip - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LASZIPDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "inc" /I "dll" /I "stl" /D "WIN32" /D "DUNORDERED" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LASZIPDLL_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\DEMzip.dll dll\DEMzip.dll	copy  Release\DEMzip.dll example\DEMzip.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DEMzip - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LASZIPDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /ML /W3 /GX /Od /I "inc" /I "dll" /I "stl" /D "WIN32" /D "DUNORDERED" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LASZIPDLL_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /force /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\DEMzip.dll dll\DEMzip.dll	copy Debug\DEMzip.dll example\DEMzip.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DEMzip - Win32 Release"
# Name "DEMzip - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\arithmeticdecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arithmeticencoder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arithmeticmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\demzip_dll.cpp
# SUBTRACT CPP /I "stl"
# End Source File
# Begin Source File

SOURCE=.\src\integercompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasindex.cpp

!IF  "$(CFG)" == "DEMzip - Win32 Release"

# SUBTRACT CPP /D "DUNORDERED"

!ELSEIF  "$(CFG)" == "DEMzip - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\lasinterval.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasquadtree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v3.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v4.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreadpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v3.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v4.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laswritepoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\laszip.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\arithmeticdecoder.hpp
# End Source File
# Begin Source File

SOURCE=.\src\arithmeticencoder.hpp
# End Source File
# Begin Source File

SOURCE=.\src\arithmeticmodel.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamin.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamin_array.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamin_file.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamin_istream.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamout.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamout_array.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamout_file.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamout_nil.hpp
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamout_ostream.hpp
# End Source File
# Begin Source File

SOURCE=.\dll\demzip_api.h
# End Source File
# Begin Source File

SOURCE=.\src\laszip.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laszip_common_v1.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laszip_common_v2.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laszip_common_v3.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laszip_decompress_selective_v3.hpp
# End Source File
# Begin Source File

SOURCE=.\src\integercompressor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasattributer.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasindex.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasinterval.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laspoint.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasquadtree.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasquantizer.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditem.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v3.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemcompressed_v4.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreaditemraw.hpp
# End Source File
# Begin Source File

SOURCE=.\src\lasreadpoint.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitem.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v3.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemcompressed_v4.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswriteitemraw.hpp
# End Source File
# Begin Source File

SOURCE=.\src\laswritepoint.hpp
# End Source File
# Begin Source File

SOURCE=.\src\mydefs.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
