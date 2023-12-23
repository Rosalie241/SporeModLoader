@echo off

set SPOREMODAPI_CUR_DIR=%~dp0

for /F "usebackq" %%F in (`git --no-pager --git-dir %SPOREMODAPI_CUR_DIR%/Spore-ModAPI/.git log --oneline ^| find "" /v /c`) do (
	set SPOREMODAPI_COMMITCOUNT=%%F
)

msbuild "%SPOREMODAPI_CUR_DIR%\Spore-ModAPI\Spore ModAPI" ^
	/t:BuildDlls ^
	/p:BuildVer=%SPOREMODAPI_COMMITCOUNT% ^
	/p:Config=Release ^
	/p:Platform=Win32 ^
	/m
