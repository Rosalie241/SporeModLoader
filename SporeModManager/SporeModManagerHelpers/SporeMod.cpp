/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeMod.hpp"
#include "String.hpp"
#include "Path.hpp"
#include "UI.hpp"

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

bool SporeMod::ResolveInstallDependencies(const std::vector<Xml::SporeModInfo>& pendingSporeMods,
                                          const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    Xml::SporeModDependency dependency;
    std::vector<Xml::SporeModDependency> dependencies;
    std::vector<Xml::SporeModInfo> dependants;

    // check if dependencies are installed
    for (const auto& pendingSporeMod : pendingSporeMods)
    {
        for (size_t i = 0; i < pendingSporeMod.Dependencies.size(); i++)
        {
            dependency = pendingSporeMod.Dependencies[i];

            auto predicate = [dependency](const SporeMod::Xml::InstalledSporeMod& installedSporeMod)
            {
                return installedSporeMod.UniqueName == dependency.UniqueName &&
                        installedSporeMod.Version >= dependency.Version;
            };
            auto dependencyIter = std::find_if(installedSporeMods.begin(), installedSporeMods.end(), predicate);
            if (dependencyIter == installedSporeMods.end())
            {
                dependencies.push_back(dependency);
                dependants.push_back(pendingSporeMod);
            }
        }
    }

    // check if dependencies will be installed
    if (!dependencies.empty())
    {
        for (const auto& pendingSporeMod : pendingSporeMods)
        {
            for (size_t i = 0; i < dependencies.size(); i++)
            {
                if (pendingSporeMod.UniqueName == dependencies[i].UniqueName &&
                    pendingSporeMod.Version >= dependencies[i].Version)
                {
                    dependencies.erase(dependencies.begin() + i);
                    dependants.erase(dependants.begin() + i);
                    i--;
                }
            }
        }
    }

    // show an error when we still have dependencies
    for (size_t i = 0; i < dependencies.size(); i++)
    {
        std::cerr << "Error: required dependency for \"" 
                            << dependants[i].Name << "\" is missing: \""
                            << dependencies[i].UniqueName << "\" "
                            << dependencies[i].Version.to_string(true) 
                            << "!" << std::endl;
    }

    return dependencies.empty();
}

bool SporeMod::ResolveUpdateDependencies(const std::vector<Xml::SporeModInfo>& pendingSporeMods,
                                         const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> dependants;
    std::vector<Xml::SporeModInfo> dependencies;
    FileVersion::FileVersionInfo dependencyVersion;

    // check if dependencies are being updated
    for (const auto& pendingSporeMod : pendingSporeMods)
    {
        for (const auto& installedSporeMod : installedSporeMods)
        {
            auto predicate = [pendingSporeMod](const SporeMod::Xml::SporeModDependency& dependency)
            {
                return pendingSporeMod.UniqueName == dependency.UniqueName &&
                        pendingSporeMod.Version < dependency.Version;
            };
            auto dependencyIter = std::find_if(installedSporeMod.Dependencies.begin(), installedSporeMod.Dependencies.end(), predicate);
            if (dependencyIter != installedSporeMod.Dependencies.end())
            {
                dependants.push_back(installedSporeMod);
                dependencies.push_back(pendingSporeMod);
            }
        }
    }

    // check if dependant mods are being updated 
    if (!dependants.empty())
    {
        for (const auto& pendingSporeMod : pendingSporeMods)
        {
            for (size_t i = 0; i < dependants.size(); i++)
            {
                if (pendingSporeMod.UniqueName == dependants[i].UniqueName &&
                    pendingSporeMod.Version >= dependants[i].Version)
                {
                    dependants.erase(dependants.begin() + i);
                    dependencies.erase(dependencies.begin() + i);
                    i--;
                }
            }
        }
    }
  
    // show an error when we still have dependencies that
    // don't fullfill the version requirement of the dependant
    for (size_t i = 0; i < dependants.size(); i++)
    {
        // try to find the dependency version that the  dependant requires
        dependencyVersion = {};
        for (const auto& dependency : dependants[i].Dependencies)
        {
            if (dependency.UniqueName == dependencies[i].UniqueName)
            {
                dependencyVersion = dependency.Version;
            }
        }

        // we're in a broken state if we get here, so error out immediately
        if (!dependencyVersion.is_valid())
        {
            std::cerr << "Error: failed to retrieve dependency version for \"" 
                        << dependencies[i].Name << "\"!" << std::endl;
            return false;
        }

        std::cerr << "Error: cannot update \"" << dependencies[i].Name << "\" " 
                    << dependencies[i].Version.to_string(true)
                    << " because \"" << dependants[i].Name << "\" depends on \"" << dependencies[i].Name << "\" "
                    << dependencyVersion.to_string(true) 
                    << "!" << std::endl;
    }

    return dependants.empty();
}

bool SporeMod::ResolveUninstallDependencies(const std::vector<Xml::InstalledSporeMod>& installedSporeMods,
                                            const std::vector<int>& ids)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> dependants;
    std::vector<std::string> dependenciesNames;
    Xml::InstalledSporeMod installedSporeMod;

    // check if dependencies are being uninstalled
    for (const auto& id : ids)
    {
        const auto& uninstallSporeMod = installedSporeMods[id];

        for (const auto& installedSporeMod : installedSporeMods)
        {
            auto predicate = [uninstallSporeMod](const SporeMod::Xml::SporeModDependency& dependency)
            {
                return uninstallSporeMod.UniqueName == dependency.UniqueName &&
                        uninstallSporeMod.Version >= dependency.Version;
            };
            auto dependencyIter = std::find_if(installedSporeMod.Dependencies.begin(), installedSporeMod.Dependencies.end(), predicate);
            if (dependencyIter != installedSporeMod.Dependencies.end())
            {
                dependants.push_back(installedSporeMod);
                dependenciesNames.push_back(installedSporeMods[id].Name);
            }
        }
    }

    // check if dependant mods are being uninstalled 
    if (!dependants.empty())
    {
        for (const auto& id : ids)
        {
            installedSporeMod = installedSporeMods[id];

            for (size_t i = 0; i < dependants.size(); i++)
            {
                if (installedSporeMod.UniqueName == dependants[i].UniqueName)
                {
                    dependants.erase(dependants.begin() + i);
                    dependenciesNames.erase(dependenciesNames.begin() + i);
                    i--;
                }
            }
        }
    }
  
    // show an error when we still have dependants
    for (size_t i = 0; i < dependants.size(); i++)
    {
        std::cerr << "Error: cannot uninstall \"" << dependenciesNames[i] << "\" because it's a dependency of \"" << dependants[i].Name << "\"!" << std::endl;
    }

    return dependants.empty();
}

bool SporeMod::FindInstalledMod(const std::string& uniqueName, int& installedSporeModId, 
                                const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
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

bool SporeMod::ConfigureSporeMod(Zip::ZipFile zipFile, const Xml::SporeModInfo& sporeModInfo, 
                                 Xml::InstalledSporeMod& installedSporeMod,
                                 const std::vector<Xml::InstalledSporeMod> &installedSporeMods)
{
    Xml::SporeModInfoComponent component;
    size_t             componentsSize;
    std::optional<int> defaultComponentId;
    int                componentId;
    std::vector<int>   defaultComponentIds;
    std::vector<int>   componentIds;
    std::string        uiText;
    bool               configurationRequired = false;
    std::vector<std::filesystem::path> zipFileList;

    // support for pre-modinfo.xml mods
    if (!sporeModInfo.HasModInfoXml)
    {
        installedSporeMod.UniqueName = sporeModInfo.UniqueName;
        installedSporeMod.Name       = sporeModInfo.UniqueName;

        if (!Zip::GetFileList(zipFile, zipFileList))
        {
            std::cerr << "Error: failed to retrieve file list from zip file!" << std::endl;
            return false;
        }

        for (const auto& path : zipFileList)
        {
            const std::string extension = String::Lowercase(path.extension().string());
            if (extension == ".dll")
            {
                installedSporeMod.InstalledFiles.push_back({InstallLocation::ModLibs, path.filename(), path});
            }
            else if (extension == ".package")
            {
                installedSporeMod.InstalledFiles.push_back({InstallLocation::GalacticAdventuresData, path.filename(), path});
            }
        }
        return true;
    }

    installedSporeMod.Name        = sporeModInfo.Name;
    installedSporeMod.UniqueName  = sporeModInfo.UniqueName;
    installedSporeMod.Description = sporeModInfo.Description;
    installedSporeMod.Version     = sporeModInfo.Version;
    installedSporeMod.Dependencies = sporeModInfo.Dependencies;

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

    const struct
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

bool SporeMod::ConfigurePackage(const std::filesystem::path& path, Xml::InstalledSporeMod& installedSporeMod,
                                const std::vector<Xml::InstalledSporeMod>& installedSporeMods)
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

bool SporeMod::InstallSporeMod(Zip::ZipFile zipFile, const Xml::InstalledSporeMod& installedSporeMod)
{
    std::error_code error;

    std::cout << "-> Installing " << installedSporeMod.Name << std::endl;

    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {
        std::filesystem::path sourcePath  = installedFile.FullPath.empty() ? 
                                                installedFile.FileName :
                                                installedFile.FullPath;
        std::filesystem::path installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (UI::GetVerboseMode())
        {
            std::cout << "--> Installing " << installedFile.FileName  << " to " << installPath << std::endl;
        }

        if (!Zip::ExtractFile(zipFile, sourcePath, installPath))
        {
            std::cerr << "Error: failed to extract file from zip file!" << std::endl;
            // cleanup installed files that were left over
            for (const auto& installedFileToRemove : installedSporeMod.InstalledFiles)
            {
                installPath = Path::GetFullInstallPath(installedFileToRemove.InstallLocation, installedFileToRemove.FileName);
                if (std::filesystem::is_regular_file(installPath))
                {
                    if (UI::GetVerboseMode())
                    {
                        std::cout << "--> Removing " << installPath << std::endl;
                    }
                    std::filesystem::remove(installPath, error);
                    if (error)
                    {
                        std::cerr << "Error: failed to remove " << installPath << ": " << error.message() << std::endl;
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
    std::error_code error;

    std::cout << "-> Installing " << installedSporeMod.Name << std::endl;
    
    for (const auto& installedFile : installedSporeMod.InstalledFiles)
    {

        const std::filesystem::path& sourcePath  = path;
        const std::filesystem::path& installPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

        if (UI::GetVerboseMode())
        {
            std::cout << "--> Installing " << installedFile.FileName << " to " << installPath << std::endl;
        }

        // mingw doesn't respect copy_options::overwrite_existing in
        // std::filesystem::copy_file(), so just delete the target file
        // first before trying to copy the package file over
#ifdef __MINGW32__
        std::filesystem::remove(installPath, error);
        if (error)
        {
            std::cerr << "Error: failed to remove " << installPath << ": " << error.message() << std::endl;
            return false;
        }
#endif

        std::filesystem::copy_file(sourcePath, installPath, std::filesystem::copy_options::overwrite_existing, error);
        if (error)
        {
            std::cerr << "Error: failed to copy " << sourcePath << " to " << installPath << ": " << error.message() << std::endl;
            return false;
        }
    }

    return true;
}

