@echo off

set DETOURS_CUR_DIR=%~dp0

cd "%DETOURS_CUR_DIR%\Detours\src"

set CL=/MP
set DETOURS_TARGET_PROCESSOR=x86

nmake clean
nmake