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
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
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
        std::cerr << "Error: SporeMod::Xml::GetDirectories() Failed!" << std::endl;
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
            std::cerr << "Error: " << path << " is not a directory or doesn't exist!" << std::endl;
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
        // add seperator
        combinedPath += std::filesystem::path::preferred_separator;
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
        // mingw has weird behavior with std::filesystem::canonical(),
        // it throws an exception even though the relative path exists,
        // so to workaround that, we'll check if the fullPath exists and
        // return that
#ifdef __MINGW32__
        if (std::filesystem::is_directory(fullPath))
        {
            return fullPath;
        }
#endif
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
        std::cerr << "Error: GetModuleFileNameW() Failed!" << std::endl;
        std::exit(1);
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
        std::cerr << "Error: std::filesystem::canonical(\"/proc/self/exe\") Failed!" << std::endl;
        std::exit(1);
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
    static std::filesystem::path cachedConfigFilePath;
    if (!cachedConfigFilePath.empty())
    {
        return cachedConfigFilePath;
    }

#ifdef _WIN32
    wchar_t envBuffer[MAX_PATH] = {0};
    if (GetEnvironmentVariableW(L"SPOREMODMANAGER_CONFIGFILE", envBuffer, MAX_PATH) != 0)
    {
        cachedConfigFilePath = envBuffer;
    }
#else
    const char* configFile = std::getenv("SPOREMODMANAGER_CONFIGFILE");
    if (configFile != nullptr)
    {
        cachedConfigFilePath = configFile;
    }
#endif
    if (cachedConfigFilePath.empty())
    {
        cachedConfigFilePath = Path::Combine({ Path::GetCurrentExecutablePath(), "SporeModManager.xml" });
    }
    return cachedConfigFilePath;
}

std::filesystem::path Path::GetCoreLibsPath(void)
{
    return l_CoreLibsPath;
}

