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

static bool check_other_mod_files(const SporeMod::Xml::InstalledSporeMod& installedSporeMod,
                                         const std::vector<SporeMod::Xml::InstalledSporeMod>& installedSporeMods)
{
    for (const auto& installedSporeModIter : installedSporeMods)
    {
        for (const auto& sporeModFile : installedSporeMod.InstalledFiles)
        {
            auto predicate = [installedSporeMod, installedSporeModIter, sporeModFile](const SporeMod::Xml::SporeModFile& file)
            {
                return installedSporeMod.UniqueName != installedSporeModIter.UniqueName && file == sporeModFile;
            };
            auto installedFileIter = std::find_if(installedSporeModIter.InstalledFiles.begin(), installedSporeModIter.InstalledFiles.end(), predicate);
            if (installedFileIter != installedSporeModIter.InstalledFiles.end())
            {
                std::cerr << "Error: an already installed mod (" << installedSporeModIter.Name
                          << ") contains a file (" << (*installedFileIter).FileName << ") that this mod wants to install!" << std::endl;
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
        installedSporeMod = installedSporeMods[i];
        if (installedSporeMod.UniqueName == uniqueName)
        {
            installedSporeModId = i;
            return true;
        }
    }

    return false;
}

bool SporeMod::ConfigureSporeMod(const Xml::SporeModInfo& sporeModInfo, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod> &installedSporeMods)
{
    Xml::SporeModInfoComponent component;
    size_t             componentsSize;
    std::optional<int> defaultComponentId;
    int                componentId;
    std::vector<int>   defaultComponentIds;
    std::vector<int>   componentIds;
    std::string        uiText;
    bool               configurationRequired = false;

    installedSporeMod.Name        = sporeModInfo.Name;
    installedSporeMod.UniqueName  = sporeModInfo.UniqueName;
    installedSporeMod.Description = sporeModInfo.Description;

    configurationRequired = sporeModInfo.IsExperimental           || 
                            sporeModInfo.RequiresGalaxyReset      ||
                            sporeModInfo.CausesSaveDataDependency ||
                            (sporeModInfo.HasCustomInstaller &&
                            (!sporeModInfo.ComponentGroups.empty() ||
                             !sporeModInfo.Components.empty()));

    if (configurationRequired)
    {
        std::cout << "-> Configuring " << sporeModInfo.Name << std::endl;
    }

    struct
    {
        bool NeedsWarn;
        std::string Text;
    } warningOptions[] =
    {
        { sporeModInfo.IsExperimental,           "--> This mod is experimental" },
        { sporeModInfo.RequiresGalaxyReset,      "--> This mod requires a galaxy reset" },
        { sporeModInfo.CausesSaveDataDependency, "--> This mod causes save data dependency" }
    };

    for (const auto& warnOption : warningOptions)
    {
        if (warnOption.NeedsWarn)
        {
            std::string inputText = warnOption.Text;
            bool userAnswer = false;
            inputText += "\n--> Are you sure you want to continue? [y/N]: ";
            UI::AskUserInput(inputText, userAnswer, false);
            if (!userAnswer)
            {
                return false;
            }
        }
    }

    if (sporeModInfo.HasCustomInstaller)
    {
        for (const auto& componentGroup : sporeModInfo.ComponentGroups)
        {
            std::cout << "--> " << componentGroup.Name << std::endl;

            componentsSize     = componentGroup.Components.size();
            componentId        = 0;
            defaultComponentId = std::nullopt;

            for (size_t i = 0; i < componentsSize; i++)
            {
                component = componentGroup.Components[i];
                std::cout << "[" << i << "] " << component.Name << std::endl;

                // set default checked component id
                if (component.DefaultChecked &&
                    !defaultComponentId.has_value())
                {
                    defaultComponentId = i;
                }
            }

            uiText = "--> select which component you want";
            if (defaultComponentId.has_value())
            {
                uiText += " [";
                uiText += std::to_string(defaultComponentId.value());
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
                component = sporeModInfo.Components[i];
                std::cout << "[" << i << "] " << component.Name << std::endl
                          << "  " << component.Description << std::endl;

                // set default checked component ids
                if (component.DefaultChecked)
                {
                    defaultComponentIds.push_back(i);
                }
            }

            uiText = "--> select which components you want to install (comma seperated";
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
    if (check_other_mod_files(installedSporeMod, installedSporeMods))
    {
        return false;
    }

    return true;
}

bool SporeMod::ConfigurePackage(const std::filesystem::path& path, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    Xml::SporeModFile installedModFile;

    std::string baseName = path.stem().string();

    installedModFile.FileName = path.filename();
    installedModFile.InstallLocation = InstallLocation::GalacticAdventuresData;

    installedSporeMod.Name       = baseName;
    installedSporeMod.UniqueName = baseName;
    installedSporeMod.InstalledFiles.push_back(installedModFile);

    // file collision detection
    if (check_other_mod_files(installedSporeMod, installedSporeMods))
    {
        return false;
    }

    return true;
}

bool SporeMod::InstallSporeMod(void* zipFile, const Xml::SporeModInfo& sporeModInfo, const Xml::InstalledSporeMod& installedSporeMod)
{
    std::cout << "-> Installing " << sporeModInfo.Name << std::endl;

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::filesystem::path sourcePath  = installedFile.FileName;
        std::filesystem::path installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (UI::GetVerboseMode())
        {
            std::cout << "--> Installing " << installedFile.FileName  << " to " << installPath << std::endl;
        }

        if (!Zip::ExtractFile(zipFile, sourcePath, installPath))
        {
            std::cerr << "Error: Zip::ExtractFile() Failed!" << std::endl;
            // cleanup installed files that were left over
            for (const auto& installedFileToRemove : installedSporeMod.InstalledFiles)
            {
                installPath = Path::GetFullInstallPath(installedFileToRemove.InstallLocation, installedFileToRemove.FileName);
                if (std::filesystem::is_regular_file(installPath))
                {
                    try
                    {
                        if (UI::GetVerboseMode())
                        {
                            std::cout << "--> Removing " << installPath << std::endl;
                        }
                        std::filesystem::remove(installPath);
                    }
                    catch(...)
                    {
                        std::cerr << "Error: std::filesystem::remove(" << installPath << ") Failed!" << std::endl;
                    }
                }
            }
            return false;
        }
    }

    return true;
}

bool SporeMod::InstallPackage(const std::filesystem::path& path, const Xml::InstalledSporeMod& installedSporeMod)
{
    std::cout << "-> Installing " << installedSporeMod.Name << std::endl;
    
    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {

        const std::filesystem::path& sourcePath  = path;
        const std::filesystem::path& installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (UI::GetVerboseMode())
        {
            std::cout << "--> Installing " << installedFile.FileName << " to " << installPath << std::endl;
        }

        try
        {
            std::filesystem::copy_file(sourcePath, installPath, std::filesystem::copy_options::overwrite_existing);
        }
        catch (...)
        {
            std::cerr << "Error: std::filesystem::copy_file(" << sourcePath << "," << installPath << ") Failed!" << std::endl;
            return false;
        }
    }

    return true;
}

