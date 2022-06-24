

@echo off
mkdir build
cd build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
cl /nologo /EHsc /std:c++latest /MP /Gm /ZI /I ..\..\dependencies /c ..\*.cpp
link /nologo /INCREMENTAL:NO /out:..\main.exe /MACHINE:X86 /MANIFEST:NO  *.obj /LIBPATH:"..\..\dependencies\libs" bass.lib bass_fx.lib Tolk.lib dlb_utils.lib
cd ..
call "main.exe"
