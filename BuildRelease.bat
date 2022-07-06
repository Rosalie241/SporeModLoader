@echo off

del /s /q "bin\*"
mkdir bin

mkdir bin\SporebinEP1
mkdir bin\SporeModLoader
mkdir bin\SporeModLoader\CoreLibs
mkdir bin\SporeModLoader\CoreLibs\disk
mkdir bin\SporeModLoader\CoreLibs\march2017
mkdir bin\SporeModLoader\ModLibs

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

copy Spore-ModAPI\dll\Release\SporeModAPI.disk.dll bin\SporeModLoader\CoreLibs\disk\
copy Spore-ModAPI\dll\Release\SporeModAPI.disk.dll bin\SporeModLoader\CoreLibs\disk\SporeModAPI.dll
copy Spore-ModAPI\dll\Release\SporeModAPI.march2017.dll bin\SporeModLoader\CoreLibs\march2017\
copy Spore-ModAPI\dll\Release\SporeModAPI.march2017.dll bin\SporeModLoader\CoreLibs\march2017\SporeModAPI.dll

copy SporeModLoader\Bin\Release\*.dll bin\SporebinEP1\
