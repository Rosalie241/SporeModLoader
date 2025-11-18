@echo off

set SPOREMODAPI_CUR_DIR=%~dp0
set VERSION=402

msbuild "%SPOREMODAPI_CUR_DIR%\Spore-ModAPI\Spore ModAPI" ^
	/p:Configuration="Release DLL" ^
	/p:SDK_BUILD_VER=%VERSION% ^
	/p:EXECUTABLE_TYPE=10 ^
	/p:Platform=Win32 ^
	/m

msbuild "%SPOREMODAPI_CUR_DIR%\Spore-ModAPI\Spore ModAPI Legacy" ^
	/p:Config=Release ^
	/t:BuildLegacyDlls ^
	/p:BuildVer=%VERSION% ^
	/p:Platform=Win32 ^
	/m