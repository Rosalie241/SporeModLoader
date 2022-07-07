@echo off

del /s /q "bin\*"
mkdir bin

mkdir bin\SporebinEP1

msbuild SporeModLoader ^
    /p:Configuration=Release ^
    /p:Platform=Win32 ^
    /m

copy SporeModLoader\Bin\Release\*.dll bin\SporebinEP1\
