/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <windows.h>
#include <algorithm>

#include "SporeModLoader.hpp"
#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

//
// Local Variables
//

static std::vector<std::filesystem::path> l_CoreLibsPaths;
static std::vector<std::filesystem::path> l_ModLibsPaths;

static std::vector<HMODULE> l_LoadedCoreLibs;
static std::vector<HMODULE> l_LoadedModLibs;

//
// Exported Functions
//

bool SporeModLoader::Initialize()
{
    if (!Logger::Open())
    {
        UI::ShowErrorMessage("SporeModLoader::Initialize() Failed!");
        return false;
    }

    Logger::AddMessage("SporeModLoader::Initialize()");

    if (!Path::GetCoreLibsPaths(l_CoreLibsPaths) ||
        !Path::GetModLibsPaths(l_ModLibsPaths))
    {
        Logger::AddMessage("SporeModLoader::Initialize() == 0");
        UI::ShowErrorMessage("SporeModLoader::Initialize() Failed!");
        return false;
    }

    // allocate early
    l_LoadedCoreLibs.reserve(l_CoreLibsPaths.size());
    l_LoadedModLibs.reserve(l_ModLibsPaths.size());

    Logger::AddMessage("SporeModLoader::Initialize() == 1");
    return true;
}

bool SporeModLoader::LoadCoreLibs()
{
    Logger::AddMessage("SporeModLoader::LoadCoreLibs()");
   
    if (!Library::LoadAll(l_CoreLibsPaths, l_LoadedCoreLibs))
    {
        Logger::AddMessage("SporeModLoader::LoadCoreLibs() == 0");
        UI::ShowErrorMessage("SporeModLoader::LoadCoreLibs() Failed!");
        return false;
    }
    
    Logger::AddMessage("SporeModLoader::LoadCoreLibs() == 1");
    return true;
}

bool SporeModLoader::LoadModLibs()
{
    Logger::AddMessage("SporeModLoader::LoadModLibs()");
   
    if (!Library::LoadAll(l_ModLibsPaths, l_LoadedModLibs))
    {
        Logger::AddMessage("SporeModLoader::LoadModLibs() == 0");
        UI::ShowErrorMessage("SporeModLoader::LoadModLibs() Failed!");
        return false;
    }
    
    Logger::AddMessage("SporeModLoader::LoadModLibs() == 1");
    return true;
}

bool SporeModLoader::UnloadCoreLibs(void)
{
    // reverse the order of the core libraries,
    // because the legacy library depends on the new one
    std::reverse(l_LoadedCoreLibs.begin(), l_LoadedCoreLibs.end());
    std::reverse(l_CoreLibsPaths.begin(), l_CoreLibsPaths.end());

    Logger::AddMessage("SporeModLoader::UnloadCoreLibs()");
    
    if (!Library::UnloadAll(l_LoadedCoreLibs, l_CoreLibsPaths))
    {
        Logger::AddMessage("SporeModLoader::UnloadCoreLibs() == 0");
        UI::ShowErrorMessage("SporeModLoader::UnloadCoreLibs() Failed!");
        return false;
    }

    Logger::AddMessage("SporeModLoader::UnloadCoreLibs() == 1");
    return true;
}

bool SporeModLoader::UnloadModLibs(void)
{
    Logger::AddMessage("SporeModLoader::UnloadModLibs()");

    if (!Library::UnloadAll(l_LoadedModLibs, l_ModLibsPaths))
    {
        Logger::AddMessage("SporeModLoader::UnloadModLibs() == 0");
        UI::ShowErrorMessage("SporeModLoader::UnloadModLibs() Failed!");
        return false;
    }

    Logger::AddMessage("SporeModLoader::UnloadModLibs() == 1");
    return true;
}
