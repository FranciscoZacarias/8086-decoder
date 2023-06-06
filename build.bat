@echo off

if not defined VCVARS_RAN (
    set "VCVARS_RAN=true"
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

cl.exe /EHsc /I "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include" sim86.cpp
