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

using namespace SporeModManagerHelpers;

//
// Local Variables
//

std::filesystem::path l_ModLibsPath;
std::filesystem::path l_GalacticAdventuresDataPath;
std::filesystem::path l_CoreSporeDataPath;

//
// Exported Functions
//

bool Path::CheckIfPathsExist(void)
{
    if (!SporeMod::Xml::GetDirectories(l_ModLibsPath, l_GalacticAdventuresDataPath, l_CoreSporeDataPath))
    {
        std::cerr << "SporeMod::Xml::GetDirectories() Failed!" << std::endl;
    }

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

std::filesystem::path Path::GetFullInstallPath(SporeMod::InstallLocation installLocation, std::filesystem::path path)
{
    std::filesystem::path fullPath;

    switch (installLocation)
    {
    default:
    case SporeMod::InstallLocation::ModLibs:
        fullPath = Path::GetModLibsPath();
        break;
    case SporeMod::InstallLocation::GalacticAdventuresData:
        fullPath = Path::GetGalacticAdventuresDataPath();
        break;
    case SporeMod::InstallLocation::CoreSporeData:
        fullPath = Path::GetCoreSporeDataPath();
        break;
    }

    fullPath += "\\";
    fullPath += path;

    return fullPath;
}

std::filesystem::path Path::GetModLibsPath(void)
{
    return l_ModLibsPath;
}

std::filesystem::path Path::GetGalacticAdventuresDataPath(void)
{
    return l_GalacticAdventuresDataPath;
}

std::filesystem::path Path::GetCoreSporeDataPath(void)
{
    return l_CoreSporeDataPath;
}

