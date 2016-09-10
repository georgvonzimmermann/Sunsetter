# Microsoft Developer Studio Project File - Name="msvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=msvc - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "msvc.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "msvc.mak" CFG="msvc - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "msvc - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "msvc - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "msvc - Win32 Profile" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/msvc", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msvc - Win32 Release"

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
# ADD CPP /G6 /W4 /O2 /Ob2 /D "_win32_" /D "_CONSOLE" /D "NDEBUG"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_win32_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"Sunsetter.exe"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Zi /Od /D "_win32_" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"Sunsetter.exe"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "msvc___Win32_Profile"
# PROP BASE Intermediate_Dir "msvc___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /G6 /W4 /O2 /Ob2 /D "_win32_" /D "_CONSOLE" /D "NDEBUG"
# ADD CPP /G6 /W4 /Zi /O2 /Ob2 /D "_win32_" /D "_CONSOLE" /D "NDEBUG"
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_win32_"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_win32_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"Sunsetter.exe"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386 /out:"Sunsetter.exe"

!ENDIF 

# Begin Target

# Name "msvc - Win32 Release"
# Name "msvc - Win32 Debug"
# Name "msvc - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=aimoves.cpp
DEP_CPP_AIMOV=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=bitboard.cpp
DEP_CPP_BITBO=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=board.cpp
DEP_CPP_BOARD=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=bughouse.cpp
DEP_CPP_BUGHO=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=capture_moves.cpp
DEP_CPP_CAPTU=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=check_moves.cpp
DEP_CPP_CHECK=\
	".\board.h"\
	".\definitions.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=evaluate.cpp
DEP_CPP_EVALU=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\evaluate.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=interface.cpp
DEP_CPP_INTER=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=moves.cpp
DEP_CPP_MOVES=\
	".\board.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=notation.cpp
DEP_CPP_NOTAT=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=order_moves.cpp
DEP_CPP_ORDER=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=partner.cpp
DEP_CPP_PARTN=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=quiescense.cpp
DEP_CPP_QUIES=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=search.cpp
DEP_CPP_SEARC=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tests.cpp
DEP_CPP_TESTS=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=transposition.cpp
DEP_CPP_TRANS=\
	".\board.h"\
	".\brain.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\notation.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=validate.cpp
DEP_CPP_VALID=\
	".\board.h"\
	".\bughouse.h"\
	".\definitions.h"\
	".\interface.h"\
	".\variables.h"\
	

!IF  "$(CFG)" == "msvc - Win32 Release"

!ELSEIF  "$(CFG)" == "msvc - Win32 Debug"

!ELSEIF  "$(CFG)" == "msvc - Win32 Profile"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=board.h
# End Source File
# Begin Source File

SOURCE=brain.h
# End Source File
# Begin Source File

SOURCE=bughouse.h
# End Source File
# Begin Source File

SOURCE=definitions.h
# End Source File
# Begin Source File

SOURCE=interface.h
# End Source File
# Begin Source File

SOURCE=notation.h
# End Source File
# Begin Source File

SOURCE=variables.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
