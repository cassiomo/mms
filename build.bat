@SET VSINSTALLDIR=c:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE
@SET VCINSTALLDIR=c:\Program Files\Microsoft Visual Studio .NET 2003
@SET FrameworkDir=c:\WINDOWS\Microsoft.NET\Framework
@SET FrameworkVersion=v1.1.4322
@SET FrameworkSDKDir=c:\Program Files\Microsoft Visual Studio .NET 2003\SDK\v1.1
@rem Root of Visual Studio common files.

@if "%VSINSTALLDIR%"=="" goto Usage
@if "%VCINSTALLDIR%"=="" set VCINSTALLDIR=%VSINSTALLDIR%

CALL vcvars32.bat

@cd mms

@mkdir Debug

cl /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "MMS_EXPORTS" /D "_WINDLL" /D "_MBCS" /Gm /EHsc /RTC1 /MTd /Yc"stdafx.h" /Fp"Debug/mms.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\stdafx.cpp" 
cl /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "MMS_EXPORTS" /D "_WINDLL" /D "_MBCS" /Gm /EHsc /RTC1 /MTd /Yu"stdafx.h" /Fp"Debug/mms.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP  ".\mms.cpp"
link /OUT:"Debug/mms.dll" /INCREMENTAL /NOLOGO /DLL /DEBUG /PDB:"Debug/mms.pdb" /SUBSYSTEM:WINDOWS /IMPLIB:"Debug/mms.lib" /MACHINE:X86 Psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ".\Debug\mms.obj"  ".\Debug\stdafx.obj" 

copy Debug\mms.dll ..
copy Debug\mms.lib ..
copy Debug\mms.h ..

@cd ..

@cd mmdump

@mkdir Debug

cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yc"stdafx.h" /Fp"Debug/mmdump.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\stdafx.cpp"
cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yu"stdafx.h" /Fp"Debug/mmdump.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\mmdump.cpp"
link /OUT:"Debug/mmdump.exe" /INCREMENTAL /NOLOGO /DEBUG /PDB:"Debug/mmdump.pdb" /SUBSYSTEM:CONSOLE /MACHINE:X86 ../mms/Debug/mms.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "..\mms\Debug\mms.lib" ".\Debug\mmdump.obj" ".\Debug\stdafx.obj"

copy Debug\mmdump.exe ..


@cd ..



@cd mmstart

@mkdir Debug

cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yc"stdafx.h" /Fp"Debug/mmstart.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\stdafx.cpp"
cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yu"stdafx.h" /Fp"Debug/mmstart.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\mmstart.cpp"
link /OUT:"Debug/mmstart.exe" /INCREMENTAL /NOLOGO /DEBUG /PDB:"Debug/mmstart.pdb" /SUBSYSTEM:CONSOLE /MACHINE:X86 ../mms/Debug/mms.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "..\mms\Debug\mms.lib" ".\Debug\mmstart.obj" ".\Debug\stdafx.obj"

copy Debug\mmstart.exe ..

@cd ..



@cd mmtest

@mkdir Debug

cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yc"stdafx.h" /Fp"Debug/mmtest.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\stdafx.cpp"
cl /Od /I "../mms" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Gm /EHsc /RTC1 /MLd /Yu"stdafx.h" /Fp"Debug/mmtest.pch" /Fo"Debug/" /Fd"Debug/vc70.pdb" /W3 /c /Wp64 /ZI /TP ".\mmtest.cpp"
link /OUT:"Debug/mmtest.exe" /INCREMENTAL /NOLOGO /DEBUG /PDB:"Debug/mmtest.pdb" /SUBSYSTEM:CONSOLE /MACHINE:X86 ../mms/Debug/mms.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "..\mms\Debug\mms.lib" ".\Debug\mmtest.obj" ".\Debug\stdafx.obj"

copy Debug\mmtest.exe ..

@cd ..

@echo Done building mms project.