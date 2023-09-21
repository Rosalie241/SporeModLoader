@echo off

set SPOREMODAPI_CUR_DIR=%~dp0

msbuild "%SPOREMODAPI_CUR_DIR%\Spore-ModAPI\Spore ModAPI" ^
	/t:BuildDlls ^
	/p:BuildVer=280 ^
	/p:Config=Release ^
	/p:Platform=Win32 ^
	/m