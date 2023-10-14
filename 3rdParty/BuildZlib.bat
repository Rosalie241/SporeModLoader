@echo off
set ZLIB_CUR_DIR=%~dp0

set ZLIB_DIR=%ZLIB_CUR_DIR%\zlib

mkdir "%ZLIB_DIR%\build"

set PATH=%PATH%;C:\Program Files\CMake\bin

cmake -S "%ZLIB_DIR%" -B "%ZLIB_DIR%\build" -A Win32 ^
	-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded

cmake --build "%ZLIB_DIR%\build" --config Release --target zlibstatic