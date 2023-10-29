/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_HPP
#define SPOREMODMANAGERHELPERS_HPP

#include <filesystem>
#include <optional>
#include <vector>

namespace SporeModManagerHelpers
{
    namespace FileVersion
    {
        struct FileVersionInfo
        {
            int Major    = 0;
            int Minor    = 0;
            int Build    = 0;
            int Revision = 0;

            bool operator>(const FileVersionInfo& other) const
            {
                return (Major > other.Major) ||
                    (Major == other.Major && Minor > other.Minor) ||
                    (Major == other.Major && Minor == other.Minor &&
                        Build > other.Build) ||
                    (Major == other.Major && Minor == other.Minor &&
                        Build == other.Build && Revision > other.Revision);
            }

            std::string to_string(void) const
            { 
                std::string string;
                string =  std::to_string(Major) + ".";
                string += std::to_string(Minor) + ".";
                string += std::to_string(Build) + ".";
                string += std::to_string(Revision);
                return string;
            }
        };

        /// <summary>
        ///     Retrieves file version info for the core lib
        /// </summary>
        bool GetCoreLibFileVersionInfo(FileVersionInfo& fileVersionInfo);

        /// <summary>
        ///     Returns whether the core lib matches the version required by the mod
        /// </summary>
        bool CheckIfCoreLibMatchesVersion(FileVersionInfo& modFileVersionInfo, std::string modName);

        /// <summary>
        ///     Parses FileVersionInfo from string
        /// </summary>
        bool ParseString(std::string string, FileVersionInfo& fileVersionInfo);

        /// <summary>
        ///     Parses FileVersionInfo from a file
        /// </summary>
        bool ParseFile(std::filesystem::path path, FileVersionInfo& fileVersionInfo);
    }

    namespace SporeMod
    {
        enum class InstallLocation
        {
            ModLibs                = 0,
            GalacticAdventuresData = 1,
            CoreSporeData          = 2
        };

        namespace Xml
        {
            struct SporeModFile
            {
                SporeMod::InstallLocation InstallLocation;
                std::filesystem::path FileName;

                bool operator==(const SporeModFile& other) const
                {
                    return InstallLocation == other.InstallLocation &&
                        FileName == other.FileName;
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

                bool IsExperimental           = false;
                bool RequiresGalaxyReset      = false;
                bool CausesSaveDataDependency = false;
                bool HasCustomInstaller       = false;

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

        /// <summary>
        ///     Tries to find installed mod with uniqueName
        /// </summary>
        bool FindInstalledMod(std::string uniqueName, int& installedSporeModId);

        /// <summary>
        ///     Installs sporemod file
        /// </summary>
        bool InstallSporeMod(std::filesystem::path path);

        /// <summary>
        ///    Installs package file
        /// </summary>
        bool InstallPackage(std::filesystem::path path);
    }

    namespace Path
    {
        /// <summary>
        ///     Returns wether all required paths exist
        /// </summary>
        bool CheckIfPathsExist(void);

        /// <summary>
        ///     Returns the combined path of paths
        /// </summary>
        std::filesystem::path Combine(std::vector<std::filesystem::path> paths);

        /// <summary>
        ///     Retrieves full installation path
        /// </summary>
        std::filesystem::path GetFullInstallPath(SporeMod::InstallLocation installLocation, std::filesystem::path path);

        /// <summary>
        ///     Returns full path from the executable (without the executable filename)
        /// </summary>
        std::filesystem::path GetCurrentExecutablePath(void);

        /// <summary>
        ///     Returns full path to the config file
        /// </summary>
        std::filesystem::path GetConfigFilePath(void);

        /// <summary>
        ///     Returns the CoreLibs path
        /// </summary>
        std::filesystem::path GetCoreLibsPath(void);
    }

    namespace UI
    {
        /// <summary>
        ///     Asks user for an integer
        /// </summary>
        void AskUserInput(std::string text, int& number, std::optional<int> defaultNumber, int min, int max);

        /// <summary>
        ///     Asks user for an integers
        /// </summary>
        void AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, std::vector<int> defaultNumbers, int min, int max);

        /// <summary>
        ///     Asks user for a boolean
        /// </summary>
        void AskUserInput(std::string text, bool& boolValue, bool defaultValue);
    }

    namespace Zip
    {
        typedef void* ZipFile;

        /// <summary>
        ///     Opens the given zip file
        /// </summary>
        bool OpenFile(ZipFile& zipFile, std::filesystem::path path);

        /// <summary>
        ///     Closes the given zip
        /// </summary>
        bool CloseFile(ZipFile zipFile);

        /// <summary>
        ///     Retrieves the file list of a zip file
        /// </summary>
        bool GetFileList(ZipFile zipFile, std::vector<std::filesystem::path>& fileList);

        /// <summary>
        ///     Extracts file to outputFile
        /// </summary>
        bool ExtractFile(ZipFile zipFile, std::filesystem::path file, std::filesystem::path outputFile);

        /// <summary>
        ///     Extracts file to buffer
        /// </summary>
        bool ExtractFile(ZipFile zipFile, std::filesystem::path file, std::vector<char>& buffer);
    }

    namespace String
    {
        /// <summary>
        ///     Splits the given string by the delimiter
        /// </summary>
        std::vector<std::string> Split(std::string string, char delimiter);

        /// <summary>
        ///     Joins numbers to string with delimiter
        /// </summary>
        std::string Join(std::vector<int> numbers, char delimiter);

        /// <summary>
        ///     Returns the lowercase version of the given string
        /// </summary>
        std::string Lowercase(std::string string);
    }
}

#endif // SPOREMODMANAGERHELPERS_HPP
