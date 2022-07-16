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

#include "SporeModManager.hpp"
#include "SporeModManagerHelpers.hpp"

using namespace SporeModManagerHelpers;

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
            std::cout << "  " << installedSporeMod.Description << std::endl;
        }
    }

    return true;
}

bool SporeModManager::InstallMod(std::filesystem::path path)
{
    if (!std::filesystem::is_regular_file(path))
    {
        std::cerr << "\"" << path.string() << "\" is not a regular file or doesn't exist!" << std::endl;
        return false;
    }

    std::string extension = path.extension().string();
    if (extension == ".sporemod")
    {
        return SporeMod::InstallSporeMod(path);
    }
    else if (extension == ".package")
    {
        return SporeMod::InstallPackage(path);
    }
    else
    {
        std::cerr << "\"" << extension << "\" is an invalid extension!" << std::endl;
        return false;
    }

    return true;
}

bool SporeModManager::UpdateMod(std::filesystem::path path)
{
    Zip::ZipFile zipFile;
    char* modInfoFileBuffer;
    size_t modInfoFileBufferSize;
    SporeMod::Xml::SporeModInfo sporeModInfo;
    int installedSporeModId = 0;
    std::string installedSporeModUniqueName;

    if (!std::filesystem::is_regular_file(path))
    {
        std::cerr << "\"" << path.string() << "\" is not a regular file or doesn't exist!" << std::endl;
        return false;
    }

    std::string extension = path.extension().string();
    if (extension == ".sporemod")
    {
        if (!Zip::OpenFile(zipFile, path))
        {
            std::cerr << "Zip::OpenFile() Failed!" << std::endl;
            return false;
        }

        if (!Zip::ExtractFile(zipFile, "modinfo.xml", &modInfoFileBuffer, &modInfoFileBufferSize))
        {
            std::cerr << "Zip::ExtractFile() Failed!" << std::endl;
            Zip::CloseFile(zipFile);
            return false;
        }

        if (!SporeMod::Xml::ParseSporeModInfo(modInfoFileBuffer, modInfoFileBufferSize, sporeModInfo))
        {
            std::cerr << "SporeMod::Xml::ParseSporeModInfo() Failed!" << std::endl;
            free(modInfoFileBuffer);
            Zip::CloseFile(zipFile);
            return false;
        }

        installedSporeModUniqueName = sporeModInfo.UniqueName;

        free(modInfoFileBuffer);
        Zip::CloseFile(zipFile);
    }
    else if (extension == ".package")
    {
        installedSporeModUniqueName = path.stem().string();
    }
    else
    {
        std::cerr << "\"" << extension << "\" is an invalid extension!" << std::endl;
        return false;
    }

    if (!SporeMod::FindInstalledMod(installedSporeModUniqueName, installedSporeModId))
    {
        std::cerr << "No mod found with the same unique name" << std::endl
                  << "Did you mean install?" << std::endl;
        return false;
    }

    return UninstallMods({ installedSporeModId }) && InstallMod(path);
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
