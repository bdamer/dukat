@echo off

set VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community
set DEVENV=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe

echo Setting up environment
call "%VSPATH%\VC\vcvarsall.bat"

echo Cleaning up last build
"%DEVENV%" .\vs2015\dukat.sln /clean "Release|x86"
if NOT ERRORLEVEL 0 GOTO ERROR

echo Building dukat
"%DEVENV%" .\vs2015\dukat.sln /build "Release|x86"
if NOT ERRORLEVEL 0 GOTO ERROR

echo Build complete
GOTO DONE

:ERROR
echo Build failed

:DONE