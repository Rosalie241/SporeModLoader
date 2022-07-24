@echo off
set CUR_DIR=%~dp0

set MINIZIPNG_DIR=%CUR_DIR%\minizip-ng

rmdir /S /Q "%MINIZIPNG_DIR%\build"
mkdir "%MINIZIPNG_DIR%\build"

set PATH=%PATH%;C:\Program Files\CMake\bin

cmake -S "%MINIZIPNG_DIR%" -B "%MINIZIPNG_DIR%\build" -A Win32 ^
	-DMZ_LZMA=OFF -DMZ_ZSTD=OFF ^
	-DMZ_BZIP2=OFF -DMZ_PKCRYPT=OFF ^
	-DMZ_WZAES=OFF -DMZ_SIGNING=OFF ^
	-DMZ_DECOMPRESS_ONLY=ON ^
	-DCMAKE_POLICY_DEFAULT_CMP0091=NEW ^
	-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
	-DCMAKE_BUILD_TYPE=Release

cmake --build "%MINIZIPNG_DIR%\build" --config Release