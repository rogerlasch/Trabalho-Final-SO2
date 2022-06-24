

@echo off
mkdir build
cd build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
cl /nologo /EHsc /std:c++latest /MP /Gm /ZI /I ..\..\dependencies ..\*.cpp
link /nologo /INCREMENTAL:NO /out:..\main.exe /MACHINE:X86 /MANIFEST:NO  *.obj /LIBPATH:"..\..\dependencies\libs" biblioteca.lib
cd ..
