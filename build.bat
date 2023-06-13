@echo off

if not defined VCVARS_RAN (
    set "VCVARS_RAN=true"
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

@echo off
IF NOT EXIST build mkdir build
pushd build
call cl -nologo     -std:c++17 -Zi -FC ..\sim86.cpp -Fesim86_msvc_debug.exe
call cl -O2 -nologo -std:c++17 -Zi -FC ..\sim86.cpp -Fesim86_msvc_release.exe
popd

