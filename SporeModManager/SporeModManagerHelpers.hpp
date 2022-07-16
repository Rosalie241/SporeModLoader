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
#include <vector>

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
                InstallLocation InstallLocation;
                std::filesystem::path FileName;
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

            struct SporeModInfo
            {
                std::string Name;
                std::string UniqueName;
                std::string Description;

                std::vector<SporeModInfoComponentGroup> ComponentGroups;
                std::vector<SporeModInfoComponent> Components;
                std::vector<SporeModInfoPrerequisite> Prerequisites;
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
                        InstalledFiles.size() == other.InstalledFiles.size();
                }
            };

            /// <summary>
            ///     Parses SporeMod.xml buffer into a SporeModInfo
            /// </summary>
            bool ParseSporeModInfo(char* buffer, size_t bufferSize, SporeModInfo& sporeModInfo);

            /// <summary>
            ///     Retrieves directories 
            /// </summary>>
            bool GetDirectories(std::filesystem::path& modLibsPath, std::filesystem::path& galacticAdventuresDataPath, std::filesystem::path& coreSporeDataPath);

            /// <summary>
            ///     Saves directories
            /// </summary>
            bool SaveDirectories(std::filesystem::path modLibsPath, std::filesystem::path galacticAdventuresDataPath, std::filesystem::path coreSporeDataPath);

            /// <summary>
            ///     Retrieves installed mod list
            /// </summary>
            bool GetInstalledModList(std::vector<InstalledSporeMod>& installedSporeModList);

            /// <summary>
            ///     Saves installed mod list
            /// </summary>
            bool SaveInstalledModList(std::vector<InstalledSporeMod> installedSporeModList);
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
        ///     Retrieves full installation path
        /// </summary>
        std::filesystem::path GetFullInstallPath(SporeMod::InstallLocation installLocation, std::filesystem::path path);

        /// <summary>
        ///     Returns mod libs path
        /// </summary>
        std::filesystem::path GetModLibsPath(void);

        /// <summary>
        ///     Returns Galactic Adventures data path
        /// </summary>
        std::filesystem::path GetGalacticAdventuresDataPath(void);

        /// <summary>
        ///     Returns Core Spore data path
        /// </summary>
        std::filesystem::path GetCoreSporeDataPath(void);
    }

    namespace UI
    {
        /// <summary>
        ///     Asks user for an integer
        /// </summary>
        void AskUserInput(std::string text, int& number, int min, int max);

        /// <summary>
        ///     Asks user for an integers
        /// </summary>
        void AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, int min, int max);
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
        bool ExtractFile(ZipFile zipFile, std::filesystem::path file, char** buffer, size_t* bufferSize);
    }
}

#endif // SPOREMODMANAGERHELPERS_HPP
