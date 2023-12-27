## SporeModLoader

This is a simple mod loader for Spore,
it injects DLLs at runtime before executing
the entry point of the Spore executable  

## SporeModManager

This is a simple commandline mod manager for Spore,
it allows you to install, update and remove mods 
easily using the commandline
  
It's inside the `SporeModLoader/SporeModManager`
directory, with both a windows and linux executable  

Example usage can be found when running the executable
with the `help` argument

## Installation

Download the latest release and extract the
archive into the game directory

If you're using Linux with WINE, ensure you
set a DLL override for `dinput8` to native,
this can be achieved with `WINEDLLOVERRIDES="dinput8=n"`
or by using `winecfg`

## Building

#### Linux

* Install `make`, `gcc`, `g++` and optionally `i686-w64-mingw32-gcc` and `i686-w64-mingw32-g++`
* Execute `make`, if you don't have mingw installed, execute `make SporeModManager` instead

#### Windows
* Open `SporeModLoader.sln` with Visual Studio or execute `msbuild` manually

