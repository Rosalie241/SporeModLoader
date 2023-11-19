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

static bool IsModAlreadyInstalled(std::string uniqueName, 
                                  const std::vector<SporeMod::Xml::InstalledSporeMod>& installedSporeMods)
{
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

static bool CheckIfOtherModContainsFiles(std::vector<SporeMod::Xml::SporeModFile> sporeModFiles,
                                         const std::vector<SporeMod::Xml::InstalledSporeMod>& installedSporeMods)
{
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

bool SporeMod::FindInstalledMod(std::string uniqueName, int& installedSporeModId, const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    Xml::InstalledSporeMod installedSporeMod;

    for (size_t i = 0; i < installedSporeMods.size(); i++)
    {
        installedSporeMod = installedSporeMods.at(i);
        if (installedSporeMod.UniqueName == uniqueName)
        {
            installedSporeModId = i;
            return true;
        }
    }

    return false;
}

bool SporeMod::InstallSporeMod(void* zipFile, const Xml::SporeModInfo& sporeModInfo, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    Xml::SporeModInfoComponent component;
    size_t             componentsSize;
    std::optional<int> defaultComponentId;
    int                componentId;
    std::vector<int>   defaultComponentIds;
    std::vector<int>   componentIds;
    std::string        uiText;

    installedSporeMod.Name        = sporeModInfo.Name;
    installedSporeMod.UniqueName  = sporeModInfo.UniqueName;
    installedSporeMod.Description = sporeModInfo.Description;

    // check if mod with the same unique name is 
    // already installed
    if (IsModAlreadyInstalled(sporeModInfo.UniqueName, installedSporeMods))
    {
        return false;
    }

    struct
    {
        bool NeedsWarn;
        std::string Text;
    } warningOptions[] =
    {
        { sporeModInfo.IsExperimental,           "This mod is experimental" },
        { sporeModInfo.RequiresGalaxyReset,      "This mod requires a galaxy reset" },
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

        componentsSize     = componentGroup.Components.size();
        componentId        = 0;
        defaultComponentId = std::nullopt;

        for (size_t i = 0; i < componentsSize; i++)
        {
            component = componentGroup.Components.at(i);
            std::cout << "[" << i << "] " << component.Name << std::endl;

            // set default checked component id
            if (component.DefaultChecked &&
                !defaultComponentId.has_value())
            {
                defaultComponentId = i;
            }
        }

        uiText = "-> select which component you want";
        if (defaultComponentId.has_value())
        {
            uiText += " [";
            uiText += std::to_string(componentId);
            uiText += "]: ";
        }
        else
        {
            uiText += ": ";
        }

        UI::AskUserInput(uiText, componentId, defaultComponentId, 0, componentsSize - 1);

        component = componentGroup.Components[componentId];

        for (const auto& file : component.Files)
        {
            installedSporeMod.InstalledFiles.push_back(file);
        }
    }

    if (!sporeModInfo.Components.empty())
    {
        componentsSize = sporeModInfo.Components.size();
        defaultComponentIds.clear();
        componentIds.clear();

        for (size_t i = 0; i < componentsSize; i++)
        {
            component = sporeModInfo.Components.at(i);
            std::cout << "[" << i << "] " << component.Name << std::endl
                << "  " << component.Description << std::endl;

            // set default checked component ids
            if (component.DefaultChecked)
            {
                defaultComponentIds.push_back(i);
            }
        }

        uiText = "-> select which components you want to install (comma seperated";
        if (!defaultComponentIds.empty())
        {
            uiText += " or N for none) [";
            uiText += String::Join(defaultComponentIds, ',');
            uiText += "]: ";
        }
        else
        {
            uiText += "): ";
        }

        UI::AskUserInput(uiText, ',', componentIds, defaultComponentIds, 0, componentsSize - 1);

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

    for (const auto& compatFile : sporeModInfo.CompatFiles)
    {
        bool installFiles = true;

        for (const auto& requiredFile : compatFile.RequiredFiles)
        {
            std::filesystem::path requiredFilePath = Path::GetFullInstallPath(requiredFile.InstallLocation, requiredFile.FileName);
            if (!std::filesystem::is_regular_file(requiredFilePath))
            {
                installFiles = false;
                break;
            }
        }

        if (installFiles)
        {
            for (const auto& file : compatFile.Files)
            {
                installedSporeMod.InstalledFiles.push_back(file);
            }
        }
    }

    // file collision detection
    if (CheckIfOtherModContainsFiles(installedSporeMod.InstalledFiles, installedSporeMods))
    {
        return false;
    }

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::cout << "-> Installing " << installedFile.FileName.string() << std::endl;

        std::filesystem::path sourcePath  = installedFile.FileName;
        std::filesystem::path installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (!Zip::ExtractFile(zipFile, sourcePath, installPath))
        {
            std::cerr << "Zip::ExtractFile() Failed!" << std::endl;
            // cleanup installed files that were left over
            for (const auto& installedFileToRemove : installedSporeMod.InstalledFiles)
            {
                installPath = Path::GetFullInstallPath(installedFileToRemove.InstallLocation, installedFileToRemove.FileName);
                if (std::filesystem::is_regular_file(installPath))
                {
                    try
                    {
                        std::cout << "-> Removing " << installPath.filename().string() << std::endl;
                        std::filesystem::remove(installPath);
                    }
                    catch(...)
                    {
                        std::cerr << "std::filesystem::remove(" << installPath << "\") Failed!" << std::endl;
                    }
                }
            }
            return false;
        }
    }

    return true;
}

bool SporeMod::InstallPackage(std::filesystem::path path, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    Xml::SporeModFile installedModFile;

    std::string baseName = path.stem().string();

    installedModFile.FileName = path.filename();
    installedModFile.InstallLocation = InstallLocation::GalacticAdventuresData;

    installedSporeMod.Name       = baseName;
    installedSporeMod.UniqueName = baseName;
    installedSporeMod.InstalledFiles.push_back(installedModFile);

    // check if mod with the same unique name is 
    // already installed
    if (IsModAlreadyInstalled(baseName, installedSporeMods))
    {
        return false;
    }

    // file collision detection
    if (CheckIfOtherModContainsFiles(installedSporeMod.InstalledFiles, installedSporeMods))
    {
        return false;
    }

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::cout << "-> Installing " << installedFile.FileName.string() << std::endl;

        std::filesystem::path sourcePath  = path;
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

    return true;
}

