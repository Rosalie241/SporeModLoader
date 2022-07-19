/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeModManagerHelpers.hpp"

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

using namespace SporeModManagerHelpers;

//
// Local Variables
//

std::filesystem::path l_ModLibsPath;
std::filesystem::path l_GalacticAdventuresDataPath;
std::filesystem::path l_CoreSporeDataPath;

//
// Helper Functions
//

std::filesystem::path MakeAbsolutePath(std::filesystem::path& path)
{
    std::filesystem::path currentExecutablePath;
    std::filesystem::path fullPath;

    if (path.is_absolute())
    {
        return path;
    }

    currentExecutablePath = Path::GetCurrentExecutablePath();
    fullPath = currentExecutablePath;
    fullPath += "\\";
    fullPath += path;

    try
    {
        fullPath = std::filesystem::canonical(fullPath);
    }
    catch (...)
    {
        // just return the original path,
        // because the directory probably doesn't exist
        return path;
    }

    return fullPath;
}

//
// Exported Functions
//

bool Path::CheckIfPathsExist(void)
{
    if (!SporeMod::Xml::GetDirectories(l_ModLibsPath, l_GalacticAdventuresDataPath, l_CoreSporeDataPath))
    {
        std::cerr << "SporeMod::Xml::GetDirectories() Failed!" << std::endl;
    }

    l_ModLibsPath = MakeAbsolutePath(l_ModLibsPath);
    l_GalacticAdventuresDataPath = MakeAbsolutePath(l_GalacticAdventuresDataPath);
    l_CoreSporeDataPath = MakeAbsolutePath(l_CoreSporeDataPath);

    for (const auto& path : { l_ModLibsPath, l_GalacticAdventuresDataPath, l_CoreSporeDataPath })
    {
        if (!std::filesystem::is_directory(path))
        {
            std::cerr << "\"" << path.string() << "\" is not a directory or doesn't exist!" << std::endl
                      << "If you want to change the configured directories, see SporeModManager.xml" << std::endl;
            return false;
        }
    }

    return true;
}

std::filesystem::path Path::GetCurrentExecutablePath(void)
{
#ifdef _WIN32
    static std::filesystem::path currentExecutablePath;
    wchar_t currentExecutablePathBuf[MAX_PATH];

    if (!currentExecutablePath.empty())
    {
        return currentExecutablePath;
    }

    if (GetModuleFileNameW(nullptr, currentExecutablePathBuf, MAX_PATH) == 0)
    {
        std::cerr << "GetModuleFileNameW() Failed!" << std::endl;
        return currentExecutablePath;
    }

    currentExecutablePath = currentExecutablePathBuf;
    currentExecutablePath = currentExecutablePath.parent_path();

    return currentExecutablePath;
#else // _WIN32
    return std::filesystem::current_path()
#endif // _WIN32
}

std::filesystem::path Path::GetFullInstallPath(SporeMod::InstallLocation installLocation, std::filesystem::path path)
{
    std::filesystem::path fullPath;

    switch (installLocation)
    {
    default:
    case SporeMod::InstallLocation::ModLibs:
        fullPath = l_ModLibsPath;
        break;
    case SporeMod::InstallLocation::GalacticAdventuresData:
        fullPath = l_GalacticAdventuresDataPath;
        break;
    case SporeMod::InstallLocation::CoreSporeData:
        fullPath = l_CoreSporeDataPath;
        break;
    }

    fullPath += "\\";
    fullPath += path;

    return fullPath;
}

std::filesystem::path Path::GetConfigFilePath(void)
{
    std::filesystem::path configFileName;
    static std::filesystem::path configFilePath;

    configFileName = "SporeModManager.xml";

    if (!configFilePath.empty())
    {
        return configFilePath;
    }

    configFilePath = Path::GetCurrentExecutablePath();
    if (!configFilePath.empty())
    {
        configFilePath += "\\";
    }
    configFilePath += configFileName;

    return configFilePath;
}

