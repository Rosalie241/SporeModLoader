/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <algorithm>

#include "SporeModManager.hpp"
#include "SporeModManagerHelpers.hpp"

using namespace SporeModManagerHelpers;

//
// Helper Functions
//

static bool GetUniqueName(const std::filesystem::path& path, const std::string& extension, std::string& uniqueName)
{
    Zip::ZipFile zipFile;
    std::vector<char> modInfoFileBuffer;
    SporeMod::Xml::SporeModInfo sporeModInfo;

    if (extension == ".sporemod")
    {
        modInfoFileBuffer.clear();
        if (!Zip::OpenFile(zipFile, path))
        {
            std::cerr << "Zip::OpenFile() Failed!" << std::endl;
            return false;
        }

        if (!Zip::ExtractFile(zipFile, "modinfo.xml", modInfoFileBuffer))
        {
            std::cerr << "Zip::ExtractFile() Failed!" << std::endl;
            Zip::CloseFile(zipFile);
            return false;
        }

        if (!SporeMod::Xml::ParseSporeModInfo(modInfoFileBuffer, sporeModInfo))
        {
            std::cerr << "SporeMod::Xml::ParseSporeModInfo() Failed!" << std::endl;
            Zip::CloseFile(zipFile);
            return false;
        }

        // make sure we have the modapi dll that the mod requires
        if (!FileVersion::CheckIfCoreLibMatchesVersion(sporeModInfo.MinimumModAPILibVersion, sporeModInfo.Name))
        {
            Zip::CloseFile(zipFile);
            return false;
        }

        uniqueName = sporeModInfo.UniqueName;
        Zip::CloseFile(zipFile);
    }
    else if (extension == ".package")
    {
        uniqueName = path.stem().string();
    }

    return true;
}

//
// Exported Functions
//

bool SporeModManager::ListInstalledMods(void)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> installedSporeMods;

    if (!SporeMod::Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "SporeMod::Xml::GetInstalledMostList() Failed!" << std::endl;
        return false;
    }

    for (size_t i = 0; i < installedSporeMods.size(); i++)
    {
        SporeMod::Xml::InstalledSporeMod installedSporeMod;
        installedSporeMod = installedSporeMods[i];

        std::cout << "[" << i << "] " << installedSporeMod.Name << std::endl;
        if (!installedSporeMod.Description.empty())
        {
            std::cout << "  " << String::Replace(installedSporeMod.Description, "\n", "\n  ") << std::endl;
        }
    }

    return true;
}

bool SporeModManager::InstallMods(std::vector<std::filesystem::path> paths)
{
    std::string uniqueName;
    std::vector<std::string> uniqueNames;

    // do some basic validation before attempting
    // to install the given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths.at(i);

        if (!std::filesystem::is_regular_file(path))
        {
            std::cerr << "\"" << path.string() << "\" is not a regular file or doesn't exist!" << std::endl;
            return false;
        }

        std::string extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod" || extension == ".package")
        {
            if (!GetUniqueName(path, extension, uniqueName))
            {
                return false;
            }
        }
        else
        {
            std::cerr << "\"" << extension << "\" is an invalid extension!" << std::endl;
            return false;
        }

        // ensure we only have unique mod names
        if (std::find(uniqueNames.begin(), uniqueNames.end(), uniqueName) != uniqueNames.end())
        {
            std::cerr << "Removing \"" << path.string() << "\" from the installation list due to another mod having the same unique name!" << std::endl;
            paths.erase(paths.begin() + i);
            i -= 1;
            continue;
        }
        uniqueNames.push_back(uniqueName);
    }

    // install given mods
    for (const auto& path : paths)
    {
        std::string extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod")
        {
            if (!SporeMod::InstallSporeMod(path))
            {
                return false;
            }
        }
        else if (extension == ".package")
        {
            if (!SporeMod::InstallPackage(path))
            {
                return false;
            }
        }
    }

    return true;
}

bool SporeModManager::UpdateMods(std::vector<std::filesystem::path> paths, bool requiresInstalled)
{
    int                      installedSporeModId = 0;
    std::vector<int>         installedSporeModIds;
    std::string              uniqueName;
    std::vector<std::string> uniqueNames;

    // do validation before attempting
    // to update the given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths.at(i);

        if (!std::filesystem::is_regular_file(path))
        {
            std::cerr << "\"" << path.string() << "\" is not a regular file or doesn't exist!" << std::endl;
            return false;
        }

        std::string extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod" || extension == ".package")
        {
            if (!GetUniqueName(path, extension, uniqueName))
            {
                return false;
            }
        }
        else
        {
            std::cerr << "\"" << extension << "\" is an invalid extension!" << std::endl;
            return false;
        }

        // ensure we only have unique mod names
        if (std::find(uniqueNames.begin(), uniqueNames.end(), uniqueName) !=
            uniqueNames.end())
        {
            std::cerr << "Removing \"" << path.string() << "\" from the installation/update list due to another mod having the same unique name!" << std::endl;
            paths.erase(paths.begin() + i);
            i -= 1;
            continue;
        }
        uniqueNames.push_back(uniqueName);

        if (!SporeMod::FindInstalledMod(uniqueName, installedSporeModId))
        {
            if (requiresInstalled)
            {
                std::cerr << "No mod found with the same unique name (" << uniqueName << ")" << std::endl
                          << "Did you mean install?" << std::endl;
                return false;
            }
        }
        else
        {
            installedSporeModIds.push_back(installedSporeModId);
        }
    }

    // apply changes
    if (!installedSporeModIds.empty() && !UninstallMods(installedSporeModIds))
    {
        return false;
    }

    return InstallMods(paths);
}

bool SporeModManager::UninstallMods(std::vector<int> ids)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> installedSporeMods;
    std::vector<SporeMod::Xml::InstalledSporeMod> removedSporeMods;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    std::filesystem::path fullInstallPath;

    if (!SporeMod::Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "SporeMod::Xml::GetInstalledModList() Failed!" << std::endl;
        return false;
    }

    for (const auto& id : ids)
    {
        if (id < 0 || (size_t)id >= installedSporeMods.size() || installedSporeMods.empty())
        {
            std::cerr << "ID(s) must be valid!" << std::endl;
            return false;
        }
    }

    for (const auto& id : ids)
    {
        installedSporeMod = installedSporeMods.at(id);

        for (const auto& installedFile : installedSporeMod.InstalledFiles)
        {
            try
            {
                fullInstallPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

                std::cout << "-> Removing " << installedFile.FileName.string() << std::endl;

                std::filesystem::remove(fullInstallPath);
            }
            catch (...)
            {
                std::cerr << "std::filesystem::remove(" << fullInstallPath << "\") Failed!" << std::endl;
                return false;
            }
        }

        removedSporeMods.push_back(installedSporeMod);
    }

    for (const auto& removedSporeMod : removedSporeMods)
    {
        auto installedSporeModIter = std::find(installedSporeMods.begin(), installedSporeMods.end(), removedSporeMod);
        if (installedSporeModIter != installedSporeMods.end())
        {
            installedSporeMods.erase(installedSporeModIter);
        }
    }

    if (!SporeMod::Xml::SaveInstalledModList(installedSporeMods))
    {
        std::cerr << "SporeMod::Xml::SaveInstalledModList() Failed!" << std::endl;
        return false;
    }

    return true;
}
