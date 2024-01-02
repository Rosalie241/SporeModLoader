@echo off

set SPOREMODAPI_CUR_DIR=%~dp0

msbuild "%SPOREMODAPI_CUR_DIR%\Spore-ModAPI\Spore ModAPI" ^
	/p:Configuration="Release DLL" ^
	/p:SDK_BUILD_VER=298 ^
	/p:EXECUTABLE_TYPE=10 ^
	/p:Platform=Win32 ^
	/m
