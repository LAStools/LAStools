# Microsoft Developer Studio Project File - Name="laszippertest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=laszippertest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "laszippertest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "laszippertest.mak" CFG="laszippertest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "laszippertest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "laszippertest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "laszippertest - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /w /W0 /GX /O2 /I "..\src" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "../src" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\laszippertest.exe laszippertest.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "laszippertest - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Gm /GX /ZI /Od /I "..\src" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\src" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\laszippertest.exe laszippertest.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "laszippertest - Win32 Release"
# Name "laszippertest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\arithmeticdecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\arithmeticencoder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\arithmeticmodel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\integercompressor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreadpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lasunzipper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitemcompressed_v1.cpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitemcompressed_v2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\laswritepoint.cpp
# End Source File
# Begin Source File

SOURCE=..\src\laszip.cpp
# End Source File
# Begin Source File

SOURCE=..\src\laszipper.cpp
# End Source File
# Begin Source File

SOURCE=.\laszippertest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\arithmeticdecoder.hpp
# End Source File
# Begin Source File

SOURCE=..\src\arithmeticencoder.hpp
# End Source File
# Begin Source File

SOURCE=..\src\arithmeticmodel.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamin.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamin_file.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamin_istream.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamout.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamout_file.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamout_nil.hpp
# End Source File
# Begin Source File

SOURCE=..\src\bytestreamout_ostream.hpp
# End Source File
# Begin Source File

SOURCE=..\src\entropydecoder.hpp
# End Source File
# Begin Source File

SOURCE=..\src\entropyencoder.hpp
# End Source File
# Begin Source File

SOURCE=..\src\integercompressor.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditem.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreaditemraw.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasreadpoint.hpp
# End Source File
# Begin Source File

SOURCE=..\src\lasunzipper.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitem.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitemcompressed_v1.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitemcompressed_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laswriteitemraw.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laswritepoint.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laszip.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laszip_common_v2.hpp
# End Source File
# Begin Source File

SOURCE=..\src\laszipper.hpp
# End Source File
# Begin Source File

SOURCE=..\src\mydefs.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
