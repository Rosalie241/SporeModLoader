/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_SPOREMODXML_HPP
#define SPOREMODMANAGERHELPERS_SPOREMODXML_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "FileVersion.hpp"

namespace SporeModManagerHelpers
{
    namespace SporeMod
    {
        enum class InstallLocation
        {
            ModLibs = 0,
            GalacticAdventuresData = 1,
            CoreSporeData = 2
        };

        namespace Xml
        {
            struct SporeModFile
            {
                SporeMod::InstallLocation InstallLocation;
                std::filesystem::path     FileName;
                std::filesystem::path     FullPath;

                bool operator==(const SporeModFile& other) const
                {
                    return InstallLocation == other.InstallLocation &&
                        FileName == other.FileName &&
                        FullPath == other.FullPath;
                }
            };

            struct SporeModInfoComponent
            {
                std::string Name;
                std::string UniqueName;
                std::string Description;
                bool        DefaultChecked;
                std::vector<SporeModFile> Files;
            };

            struct SporeModInfoComponentGroup
            {
                std::string Name;
                std::string UniqueName;
                std::vector<SporeModInfoComponent> Components;
            };

            struct SporeModInfoPrerequisite
            {
                std::vector<SporeModFile> Files;
            };

            struct SporeModInfoCompatFile
            {
                std::vector<SporeModFile> RequiredFiles;
                std::vector<SporeModFile> Files;
            };

            struct SporeModInfo
            {
                std::string Name;
                std::string UniqueName;
                std::string Description;

                bool HasModInfoXml = false;
                bool IsExperimental = false;
                bool RequiresGalaxyReset = false;
                bool CausesSaveDataDependency = false;
                bool HasCustomInstaller = false;
                bool CompatOnly = false;

                FileVersion::FileVersionInfo InstallerVersion;
                FileVersion::FileVersionInfo MinimumModAPILibVersion;

                std::vector<SporeModInfoComponentGroup> ComponentGroups;
                std::vector<SporeModInfoComponent>      Components;
                std::vector<SporeModInfoPrerequisite>   Prerequisites;
                std::vector<SporeModInfoCompatFile>     CompatFiles;
            };

            struct InstalledSporeMod
            {
                std::string Name;
                std::string UniqueName;
                std::string Description;

                std::vector<SporeModFile> InstalledFiles;

                bool operator==(const InstalledSporeMod& other) const
                {
                    return Name == other.Name &&
                        UniqueName == other.UniqueName &&
                        Description == other.Description &&
                        InstalledFiles == other.InstalledFiles;
                }
            };

            /// <summary>
            ///     Parses SporeMod.xml buffer into a SporeModInfo
            /// </summary>
            bool ParseSporeModInfo(const std::vector<char>& buffer, SporeModInfo& sporeModInfo);

            /// <summary>
            ///     Retrieves directories 
            /// </summary>>
            bool GetDirectories(std::filesystem::path& coreLibsPath, std::filesystem::path& modLibsPath, std::filesystem::path& galacticAdventuresDataPath, std::filesystem::path& coreSporeDataPath);

            /// <summary>
            ///     Saves directories
            /// </summary>
            bool SaveDirectories(std::filesystem::path coreLibsPath, std::filesystem::path modLibsPath, std::filesystem::path galacticAdventuresDataPath, std::filesystem::path coreSporeDataPath);

            /// <summary>
            ///     Retrieves installed mod list
            /// </summary>
            bool GetInstalledModList(std::vector<InstalledSporeMod>& installedSporeModList);

            /// <summary>
            ///     Saves installed mod list
            /// </summary>
            bool SaveInstalledModList(const std::vector<InstalledSporeMod>& installedSporeModList);
        }
    }
}

#endif // SPOREMODMANAGERHELPERS_SPOREMODXML_HPP