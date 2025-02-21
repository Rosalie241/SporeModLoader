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
    try
    {
        Logger::Open();

        l_CoreLibsPaths = Path::GetCoreLibsPaths();
        l_ModLibsPaths  = Path::GetModLibsPaths();

        // allocate early
        l_LoadedCoreLibs.reserve(l_CoreLibsPaths.size());
        l_LoadedModLibs.reserve(l_ModLibsPaths.size());

        for (const auto& path : l_CoreLibsPaths)
        {
            if (!std::filesystem::exists(path))
            {
                std::wstring errorMessage;
                errorMessage = L"\"";
                errorMessage += path.wstring();
                errorMessage += L"\" doesn't exist!";
                UI::ShowErrorMessage(errorMessage);
                throw std::exception();
            }
        }

        Logger::AddMessage(L"SporeModLoader::Initialize() == 1");
        return true;
    }
    catch (...)
    {
        Logger::AddMessage(L"SporeModLoader::Initialize() == 0");
        UI::ShowErrorMessage(L"SporeModLoader::Initialize() Failed!");
        return false;
    }
}

bool SporeModLoader::LoadCoreLibs()
{
    try
    {
        Logger::AddMessage(L"SporeModLoader::LoadCoreLibs()");
        if (!Library::LoadAll(l_CoreLibsPaths, l_LoadedCoreLibs))
        {
            throw std::exception();
        }
        Logger::AddMessage(L"SporeModLoader::LoadCoreLibs() == 1");
        return true;
    }
    catch (...)
    {
        Logger::AddMessage(L"SporeModLoader::LoadCoreLibs() == 0");
        UI::ShowErrorMessage(L"SporeModLoader::LoadCoreLibs() Failed!");
        return false;
    }
}

bool SporeModLoader::LoadModLibs()
{
    try
    {
        Logger::AddMessage(L"SporeModLoader::LoadModLibs()");
        if (!Library::LoadAll(l_ModLibsPaths, l_LoadedModLibs))
        {
            throw std::exception();
        }
        Logger::AddMessage(L"SporeModLoader::LoadModLibs() == 1");
        return true;
    }
    catch (...)
    {
        Logger::AddMessage(L"SporeModLoader::LoadModLibs() == 0");
        UI::ShowErrorMessage(L"SporeModLoader::LoadModLibs() Failed!");
        return false;
    }
}

bool SporeModLoader::UnloadCoreLibs(void)
{
    try
    {
        // reverse the order of the core libraries,
        // because the legacy library depends on the new one
        std::reverse(l_LoadedCoreLibs.begin(), l_LoadedCoreLibs.end());
        std::reverse(l_CoreLibsPaths.begin(), l_CoreLibsPaths.end());

        Logger::AddMessage(L"SporeModLoader::UnloadCoreLibs()");
        if (!Library::UnloadAll(l_LoadedCoreLibs, l_CoreLibsPaths))
        {
            throw std::exception();
        }
        Logger::AddMessage(L"SporeModLoader::UnloadCoreLibs() == 1");
        return true;
    }
    catch (...)
    {
        Logger::AddMessage(L"SporeModLoader::UnloadCoreLibs() == 0");
        UI::ShowErrorMessage(L"SporeModLoader::UnloadCoreLibs() Failed!");
        return false;
    }
}

bool SporeModLoader::UnloadModLibs(void)
{
    try
    {
        Logger::AddMessage(L"SporeModLoader::UnloadModLibs()");
        if (!Library::UnloadAll(l_LoadedModLibs, l_ModLibsPaths))
        {
            throw std::exception();
        }
        Logger::AddMessage(L"SporeModLoader::UnloadModLibs() == 1");
        return true;
    }
    catch (...)
    {
        Logger::AddMessage(L"SporeModLoader::UnloadModLibs() == 0");
        UI::ShowErrorMessage(L"SporeModLoader::UnloadModLibs() Failed!");
        return false;
    }
}
