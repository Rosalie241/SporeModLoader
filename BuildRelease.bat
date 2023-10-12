@echo off

set CUR_DIR=%~dp0
set OBJ_DIR=%CUR_DIR%\obj
set BIN_DIR=%CUR_DIR%\bin
set THIRDPARTY_DIR=%CUR_DIR%\3rdParty

rmdir /S /Q "%OBJ_DIR%" "%BIN_DIR%"

mkdir "%CUR_DIR%\obj"
mkdir "%CUR_DIR%\bin"

mkdir "%OBJ_DIR%\SporebinEP1"
mkdir "%OBJ_DIR%\SporeModLoader"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs\disk"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs\march2017"
mkdir "%OBJ_DIR%\SporeModLoader\ModLibs"
mkdir "%OBJ_DIR%\SporeModLoader\SporeModManager"

CALL "%THIRDPARTY_DIR%\BuildDetours.bat"
CALL "%THIRDPARTY_DIR%\BuildZlib.bat"
CALL "%THIRDPARTY_DIR%\BuildSporeModAPI.bat"

msbuild "%CUR_DIR%\SporeModLoader" ^
    /p:Configuration=Release ^
    /p:Platform=Win32 ^
    /m

msbuild "%CUR_DIR%\SporeModManager" ^
    /p:Configuration=Release ^
    /p:Platform=Win32 ^
    /m

copy "%THIRDPARTY_DIR%\Spore-ModAPI\dll\Release\SporeModAPI.disk.dll" "%OBJ_DIR%\SporeModLoader\CoreLibs\disk\SporeModAPI.dll"
copy "%THIRDPARTY_DIR%\Spore-ModAPI\dll\Release\SporeModAPI.march2017.dll" "%OBJ_DIR%\SporeModLoader\CoreLibs\march2017\SporeModAPI.dll"
copy "%THIRDPARTY_DIR%\Spore-ModAPI\dll\Release\SporeModAPI.lib" "%OBJ_DIR%\SporeModLoader\CoreLibs\SporeModAPI.lib"
copy "%CUR_DIR%\SporeModLoader\Bin\Release\dinput8.dll" "%OBJ_DIR%\SporebinEP1\dinput8.dll"
copy "%CUR_DIR%\SporeModManager\Bin\Release\SporeModManager.exe" "%OBJ_DIR%\SporeModLoader\SporeModManager"

cd "%OBJ_DIR%"

"C:\Program Files\7-Zip\7z" a -tzip "%BIN_DIR%\SporeModLoader.zip" *

cd "%CUR_DIR%"

rmdir /S /Q "%OBJ_DIR%"
