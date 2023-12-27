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
#include <vector>

namespace SporeModLoaderHelpers
{
    namespace Path
    {
        /// <summary>
        ///     Returns current executable path
        /// </summary>
        std::filesystem::path GetCurrentExecutablePath(void);

        /// <summary>
        ///     Returns ModLoader path
        /// </summary>
        std::filesystem::path GetModLoaderPath(void);

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
        ///     Opens log file
        /// </summary>
        /// <param name=""></param>
        void Open(void);

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
        ///     Loads all specified DLL files
        /// </summary>
        bool LoadAll(std::vector<std::filesystem::path> paths);
    }

    namespace Game
    {
        enum class GameVersion
        {
            Disk_1_5_1 = 0,
            GogOrSteam_March2017,
            Unknown
        };

        /// <summary>
        ///     Returns current game version
        /// </summary>
        GameVersion GetCurrentVersion(void);
    }
}

#endif // SPOREMODLOADERHELPERS_HPP
