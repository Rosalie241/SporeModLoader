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
        ///     Returns ModLoader path
        /// </summary>
        std::filesystem::path GetModLoaderPath(void);

        /// <summary>
        ///     Returns path to the log file
        /// </summary>
        std::filesystem::path GetLogFilePath(void);

        /// <summary>
        ///     Returns path to the core libs
        /// </summary>
        std::filesystem::path GetCoreLibsPath(void);

        /// <summary>
        ///     Returns path to the mod libs
        /// </summary>
        std::filesystem::path GetModLibsPath(void);
    }

    namespace Logger
    {
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
        ///     Loads all DLL files that are in path
        /// </summary>
        bool LoadAllInPath(std::filesystem::path path);
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
}

#endif // SPOREMODLOADERHELPERS_HPP
