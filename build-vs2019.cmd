@echo off

set TARGET=%1
set VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
set DEVENV=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe

if "%TARGET%"=="" GOTO ERROR

echo Setting up environment
call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x86 10.0.19041.0 -vcvars_ver=14.29

echo Cleaning up last %TARGET% build
"%DEVENV%" .\vs2015\dukat.sln /clean %TARGET%
if NOT ERRORLEVEL 0 GOTO ERROR

echo Building dukat %TARGET%
"%DEVENV%" .\vs2015\dukat.sln /build %TARGET%
if NOT ERRORLEVEL 0 GOTO ERROR

echo Build complete
GOTO DONE

:ERROR
echo Build failed

:DONE