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

#include "SporeModLoader.hpp"
#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

//
// Exported Functions
//

bool SporeModLoader::Initialize()
{
    try
    {
        Logger::Open();

        for (const auto& path : Path::GetCoreLibsPaths())
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
        if (!Library::LoadAll(Path::GetCoreLibsPaths()))
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
        if (!Library::LoadAll(Path::GetModLibsPaths()))
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
