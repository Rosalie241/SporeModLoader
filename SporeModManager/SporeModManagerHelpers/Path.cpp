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

#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

using namespace SporeModManagerHelpers;

//
// Local Variables
//

static std::filesystem::path l_CoreLibsPath;
static std::filesystem::path l_ModLibsPath;
static std::filesystem::path l_GalacticAdventuresDataPath;
static std::filesystem::path l_CoreSporeDataPath;

//
// Exported Functions
//

void Path::SetDirectories(std::filesystem::path coreLibsPath, std::filesystem::path modLibsPath, std::filesystem::path galacticAdventuresDataPath, std::filesystem::path coreSporeDataPath)
{
    l_CoreLibsPath               = coreLibsPath;
    l_ModLibsPath                = modLibsPath;
    l_GalacticAdventuresDataPath = galacticAdventuresDataPath;
    l_CoreSporeDataPath          = coreSporeDataPath;
}

bool Path::CheckIfPathsExist(void)
{
    std::filesystem::path coreLibsPath;
    std::filesystem::path modLibsPath;
    std::filesystem::path galacticAdventuresDataPath;
    std::filesystem::path coreSporeDataPath;

    if (!SporeMod::Xml::GetDirectories(coreLibsPath, modLibsPath, galacticAdventuresDataPath, coreSporeDataPath))
    {
        std::cerr << "SporeMod::Xml::GetDirectories() Failed!" << std::endl;
        return false;
    }

    if (l_CoreLibsPath.empty())
    {
        l_CoreLibsPath = GetAbsolutePath(coreLibsPath);
    }
    if (l_ModLibsPath.empty())
    {
        l_ModLibsPath = GetAbsolutePath(modLibsPath);
    }
    if (l_GalacticAdventuresDataPath.empty())
    {
        l_GalacticAdventuresDataPath = GetAbsolutePath(galacticAdventuresDataPath);
    }
    if (l_CoreSporeDataPath.empty())
    {
        l_CoreSporeDataPath = GetAbsolutePath(coreSporeDataPath);
    }


    for (const auto& path : { l_CoreLibsPath, l_ModLibsPath, l_GalacticAdventuresDataPath, l_CoreSporeDataPath })
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

std::filesystem::path Path::Combine(std::vector<std::filesystem::path> paths)
{
    std::filesystem::path combinedPath;

    for (size_t i = 0; i < paths.size(); i++)
    {
        if (paths[i].empty())
        { // skip empty paths
            continue; 
        }
        combinedPath += paths[i];
        if (i == (paths.size() - 1))
        { // break when at end
            break;
        }
#ifdef _WIN32
        combinedPath += "\\";
#else
        combinedPath += "/";
#endif // _WIN32
    }

    return combinedPath;
}

std::filesystem::path Path::GetAbsolutePath(std::filesystem::path path)
{
    std::filesystem::path fullPath = path;

    if (fullPath.is_absolute())
    {
        return fullPath;
    }

    if (!std::filesystem::is_directory(fullPath))
    {
        fullPath = Path::Combine({ Path::GetCurrentExecutablePath(), path });
    }

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
    { // fallback to current path
        std::cerr << "GetModuleFileNameW() Failed!" << std::endl;
        return std::filesystem::current_path();
    }

    currentExecutablePath = currentExecutablePathBuf;
    currentExecutablePath = currentExecutablePath.parent_path();
    return currentExecutablePath;
#else // _WIN32
    try
    {
        return std::filesystem::canonical("/proc/self/exe").parent_path();
    }
    catch (...)
    { // fallback to current path
        std::cerr << "std::filesystem::canonical(\"/proc/self/exe\") Failed!" << std::endl;
        return std::filesystem::current_path();
    }
#endif // _WIN32
}

std::filesystem::path Path::GetFullInstallPath(SporeMod::InstallLocation installLocation, std::filesystem::path path)
{
    std::filesystem::path installPath;

    switch (installLocation)
    {
    default:
    case SporeMod::InstallLocation::ModLibs:
        installPath = l_ModLibsPath;
        break;
    case SporeMod::InstallLocation::GalacticAdventuresData:
        installPath = l_GalacticAdventuresDataPath;
        break;
    case SporeMod::InstallLocation::CoreSporeData:
        installPath = l_CoreSporeDataPath;
        break;
    }

    return Path::Combine({ installPath, path });
}

std::filesystem::path Path::GetConfigFilePath(void)
{
    return Path::Combine({ Path::GetCurrentExecutablePath(), "SporeModManager.xml" });;
}

std::filesystem::path Path::GetCoreLibsPath(void)
{
    return l_CoreLibsPath;
}

