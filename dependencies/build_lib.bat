@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
mkdir build
cd build
cl /nologo /EHsc /c /MP "..\*.cpp" "..\fmt\*.cpp"
lib /nologo /out:"..\libs\biblioteca.lib" *.obj
cd ..
