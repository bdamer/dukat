@echo off

set VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
set DEVENV=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe

echo Setting up environment
call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x86 10.0.19041.0 -vcvars_ver=14.29

echo Cleaning up last build
"%DEVENV%" .\vs2015\dukat.sln /clean "Release"
if NOT ERRORLEVEL 0 GOTO ERROR

echo Building dukat
"%DEVENV%" .\vs2015\dukat.sln /build "Release"
if NOT ERRORLEVEL 0 GOTO ERROR

echo Build complete
GOTO DONE

:ERROR
echo Build failed

:DONE