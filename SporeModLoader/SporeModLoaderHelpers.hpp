/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODLOADERHELPERS_HPP
#define SPOREMODLOADERHELPERS_HPP

#include <string>
#include <filesystem>

namespace SporeModLoaderHelpers
{
    namespace Path
    {
        /// <summary>
        ///     Returns current executable path
        /// </summary>
        std::filesystem::path GetCurrentExecutablePath(void);

        /// <summary>
        ///     Returns the path to the SporeModManagerStorage path
        /// </summary>
        std::filesystem::path GetSporeModManagerStoragePath(void);

        /// <summary>
        ///     Returns the path to the Spore Mod Manager installation
        /// </summary>
        std::filesystem::path GetSporeModManagerInstallationPath(void);

        /// <summary>
        ///     Returns path to the log file
        /// </summary>
        std::filesystem::path GetLogFilePath(void);

        /// <summary>
        ///     Returns paths to the core libs
        /// </summary>
        std::vector<std::filesystem::path> GetCoreLibsPaths(void);

        /// <summary>
        ///     Returns paths to the mod libs
        /// </summary>
        std::vector<std::filesystem::path> GetModLibsPaths(void);
    }

    namespace Logger
    {
        /// <summary>
        ///     Clears log file
        /// </summary>
        void Clear(void);

        /// <summary>
        ///     Adds message to the log file
        /// </summary>
        void AddMessage(std::wstring message);
    }

    namespace UI
    {
        /// <summary>
        ///     Displays error message
        /// </summary>
        void ShowErrorMessage(std::wstring message);
    }

    namespace Library
    {
        /// <summary>
        ///     Loads all DLL files
        /// </summary>
        bool LoadAll(std::vector<std::filesystem::path> paths);
    }

    namespace Game
    {
        enum class GameVersion
        {
            Disk_1_5_1,
            Origin_1_5_1,
            Origin_March2017,
            GogOrSteam_1_5_1,
            GogOrSteam_March2017,
            Unknown
        };

        /// <summary>
        ///     Returns current game version
        /// </summary>
        GameVersion GetCurrentVersion(void);
    }

    namespace SporeModManager
    {
        /// <summary>
        ///     Checks whether Spore Mod Manager is running
        /// </summary>
        bool IsRunning();

        /// <summary>
        ///     Launches Spore Mod Manager
        /// </summary>
        void Launch();
    }
}

#endif // SPOREMODLOADERHELPERS_HPP
