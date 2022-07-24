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
#include <algorithm>

using namespace SporeModManagerHelpers;

//
// Helper Functions
//

static bool IsModAlreadyInstalled(std::string uniqueName)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> installedSporeMods;

    if (!SporeMod::Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "SporeMod::Xml::GetInstalledModList() Failed!" << std::endl;
        return true;
    }

    for (const auto& installedSporeMod : installedSporeMods)
    {
        if (installedSporeMod.UniqueName == uniqueName)
        {
            std::cerr << "A mod with the same unique name (" << installedSporeMod.Name << ") has already been installed" << std::endl
                      << "Did you mean update?" << std::endl;
            return true;
        }
    }

    return false;
}

static bool CheckIfOtherModContainsFiles(std::vector<SporeMod::Xml::SporeModFile> sporeModFiles)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> installedSporeMods;

    if (!SporeMod::Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "SporeMod::Xml::GetInstalledModList() Failed!" << std::endl;
        return true;
    }

    for (const auto& installedSporeMod : installedSporeMods)
    {
        for (const auto& sporeModFile : sporeModFiles)
        {
            auto installedFileIter = std::find(installedSporeMod.InstalledFiles.begin(), installedSporeMod.InstalledFiles.end(), sporeModFile);
            if (installedFileIter != installedSporeMod.InstalledFiles.end())
            {
                std::cerr << "An already installed mod (" << installedSporeMod.Name
                    << ") contains a file (\"" << sporeModFile.FileName.string() << "\") that this mod wants to install!" << std::endl;
                return true;
            }
        }
    }

    return false;
}

//
// Exported Functions
//

bool SporeMod::FindInstalledMod(std::string uniqueName, int& installedSporeModId)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> installedSporeMods;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;

    if (!Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "Xml::GetInstalledModList() Failed!" << std::endl;
        return true;
    }

    for (size_t i = 0; i < installedSporeMods.size(); i++)
    {
        installedSporeMod = installedSporeMods[i];
        if (installedSporeMod.UniqueName == uniqueName)
        {
            installedSporeModId = i;
            return true;
        }
    }

    return false;
}

bool SporeMod::InstallSporeMod(std::filesystem::path path)
{
    Zip::ZipFile zipFile;
    char*  modInfoFileBuffer;
    size_t modInfoFileBufferSize = 0;
    Xml::SporeModInfo sporeModInfo;
    Xml::InstalledSporeMod installedSporeMod;
    Xml::SporeModInfoComponent component;
    size_t componentsSize;

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

    if (!Xml::ParseSporeModInfo(modInfoFileBuffer, modInfoFileBufferSize, sporeModInfo))
    {
        std::cerr << "Xml::ParseSporeModInfo() Failed!" << std::endl;
        free(modInfoFileBuffer);
        Zip::CloseFile(zipFile);
        return false;
    }

    installedSporeMod.Name = sporeModInfo.Name;
    installedSporeMod.UniqueName = sporeModInfo.UniqueName;
    installedSporeMod.Description = sporeModInfo.Description;

    // check if mod with the same unique name is 
    // already installed
    if (IsModAlreadyInstalled(sporeModInfo.UniqueName))
    {
        return false;
    }

    // make sure we have the modapi dll that the mod requires
    if (!FileVersion::CheckIfCoreLibMatchesVersion(sporeModInfo.MinimumModAPILibVersion, sporeModInfo.Name))
    {
        return false;
    }

    struct
    {
        bool NeedsWarn;
        std::string Text;
    } warningOptions[] =
    {
        { sporeModInfo.IsExperimental, "This mod is experimental" },
        { sporeModInfo.RequiresGalaxyReset, "This mod requires a galaxy reset" },
        { sporeModInfo.CausesSaveDataDependency, "This mod causes save data dependency" }
    };

    for (const auto& warnOption : warningOptions)
    {
        if (warnOption.NeedsWarn)
        {
            std::string inputText = warnOption.Text;
            bool userAnswer = false;
            inputText += "\n-> Are you sure you want to continue? [y/N]: ";
            UI::AskUserInput(inputText, userAnswer, false);
            if (!userAnswer)
            {
                return false;
            }
        }
    }

    for (const auto& componentGroup : sporeModInfo.ComponentGroups)
    {
        std::cout << "-> " << componentGroup.Name << std::endl;

        componentsSize = componentGroup.Components.size();

        for (size_t i = 0; i < componentsSize; i++)
        {
            component = componentGroup.Components[i];
            std::cout << "[" << i << "] " << component.Name << std::endl;
        }

        int componentId = 0;
        UI::AskUserInput("-> select which component you want: ", componentId, 0, componentsSize - 1);

        component = componentGroup.Components[componentId];

        for (const auto& file : component.Files)
        {
            installedSporeMod.InstalledFiles.push_back(file);
        }
    }

    if (!sporeModInfo.Components.empty())
    {
        componentsSize = sporeModInfo.Components.size();

        for (size_t i = 0; i < componentsSize; i++)
        {
            component = sporeModInfo.Components[i];
            std::cout << "[" << i << "] " << component.Name << std::endl
                      << "  " << component.Description << std::endl;
        }

        std::vector<int> componentIds;

        UI::AskUserInput("-> select which components you want to install (comma seperated): ", ',', componentIds, 0, componentsSize);

        for (const auto& componentId : componentIds)
        {
            component = sporeModInfo.Components[componentId];

            for (const auto& file : component.Files)
            {
                installedSporeMod.InstalledFiles.push_back(file);
            }
        }
    }

    for (const auto& prerequisite : sporeModInfo.Prerequisites)
    {
        for (const auto& file : prerequisite.Files)
        {
            installedSporeMod.InstalledFiles.push_back(file);
        }
    }

    // file collision detection
    if (CheckIfOtherModContainsFiles(installedSporeMod.InstalledFiles))
    {
        return false;
    }

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::cout << "-> Installing " << installedFile.FileName.string() << std::endl;

        std::filesystem::path sourcePath = installedFile.FileName;
        std::filesystem::path installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (!Zip::ExtractFile(zipFile, sourcePath, installPath))
        {
            std::cerr << "Zip::ExtractFile() Failed!" << std::endl;
            free(modInfoFileBuffer);
            Zip::CloseFile(zipFile);
            return false;
        }
    }

    std::vector<Xml::InstalledSporeMod> installedSporeMods;
    if (!Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "Xml::GetInstalledModList() Failed!" << std::endl;
        free(modInfoFileBuffer);
        Zip::CloseFile(zipFile);
        return false;
    }

    installedSporeMods.push_back(installedSporeMod);

    Xml::SaveInstalledModList(installedSporeMods);

    free(modInfoFileBuffer);
    Zip::CloseFile(zipFile);
    return true;
}

bool SporeMod::InstallPackage(std::filesystem::path path)
{
    Xml::InstalledSporeMod installedSporeMod;
    Xml::SporeModFile installedFile;

    std::string baseName = path.stem().string();

    installedFile.FileName = path.filename();
    installedFile.InstallLocation = InstallLocation::GalacticAdventuresData;

    installedSporeMod.Name = baseName;
    installedSporeMod.UniqueName = baseName;
    installedSporeMod.InstalledFiles.push_back(installedFile);

    // check if mod with the same unique name is 
    // already installed
    if (IsModAlreadyInstalled(baseName))
    {
        return false;
    }

    // file collision detection
    if (CheckIfOtherModContainsFiles(installedSporeMod.InstalledFiles))
    {
        return false;
    }

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::cout << "-> Installing " << installedFile.FileName.string() << std::endl;

        std::filesystem::path sourcePath = path;
        std::filesystem::path installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        try
        {
            std::filesystem::copy_file(sourcePath, installPath, std::filesystem::copy_options::overwrite_existing);
        }
        catch (...)
        {
            std::cerr << "std::filesystem::copy_file(\"" << sourcePath.string() << "\",\"" << installPath.string() << "\") Failed!" << std::endl;
            return false;
        }
    }

    std::vector<Xml::InstalledSporeMod> installedSporeMods;
    if (!Xml::GetInstalledModList(installedSporeMods))
    {
        std::cerr << "Xml::GetInstalledModList() Failed!" << std::endl;
        return false;
    }

    installedSporeMods.push_back(installedSporeMod);

    Xml::SaveInstalledModList(installedSporeMods);
    return true;
}

