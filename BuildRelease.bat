@echo off

set CUR_DIR="%~dp0"
set OBJ_DIR="%CUR_DIR%\obj"
set BIN_DIR="%CUR_DIR%\bin"

rmdir /S /Q "%OBJ_DIR%" "%BIN_DIR%"

mkdir "%CUR_DIR%\obj"
mkdir "%CUR_DIR%\bin"

mkdir "%OBJ_DIR%\SporebinEP1"
mkdir "%OBJ_DIR%\SporeModLoader"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs\disk"
mkdir "%OBJ_DIR%\SporeModLoader\CoreLibs\march2017"
mkdir "%OBJ_DIR%\SporeModLoader\ModLibs"

msbuild "Spore-ModAPI\Spore ModAPI" ^
    /t:BuildDlls ^
    /p:BuildVer=83 ^
    /p:Config=Release ^
    /p:Platform=Win32 ^
    /m

msbuild SporeModLoader ^
    /p:Configuration=Release ^
    /p:Platform=Win32 ^
    /m

copy Spore-ModAPI\dll\Release\SporeModAPI.disk.dll "%OBJ_DIR%\SporeModLoader\CoreLibs\disk\SporeModAPI.dll"
copy Spore-ModAPI\dll\Release\SporeModAPI.march2017.dll "%OBJ_DIR%\SporeModLoader\CoreLibs\march2017\SporeModAPI.dll"
copy SporeModLoader\Bin\Release\dinput8.dll "%OBJ_DIR%\SporebinEP1\dinput8.dll"

cd "%OBJ_DIR%"

"C:\Program Files\7-Zip\7z" a -tzip "%BIN_DIR%\SporeModLoader.zip" *

cd "%CUR_DIR%"

rmdir /S /Q "%OBJ_DIR%"
