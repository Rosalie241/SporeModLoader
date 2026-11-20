## SporeModLoader

This is a simple mod loader for Spore,
it loads DLLs at runtime after Spore sets
the custom memory allocator and unloads the DLLs
before the custom memory allocator is unset

## SporeModManager

This is a simple commandline mod manager for Spore,
it allows you to install, update and remove mods 
easily using the commandline

It's inside the `SporeModLoader/SporeModManager`
directory

Example usage can be found when running the executable
with the `help` argument

## Installation

Download the latest release and extract the
archive into the game directory

If you're using Linux or MacOS with WINE, ensure you
set a DLL override for `dinput8` to native,
this can be achieved with `WINEDLLOVERRIDES="dinput8=n"`
or by using `winecfg`

## Building

#### Linux/MacOS

* Install `make`, `gcc` and `g++`
* Execute `make all`

#### Windows
* Open `SporeModLoader.sln` with Visual Studio or execute `msbuild` manually

