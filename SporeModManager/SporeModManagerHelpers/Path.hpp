/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_PATH_HPP
#define SPOREMODMANAGERHELPERS_PATH_HPP

#include <filesystem>

#include "SporeModXml.hpp"

namespace SporeModManagerHelpers
{
    namespace Path
    {
        /// <summary>
        ///     Sets directories for current session
        /// </summary>>
        void SetDirectories(const std::filesystem::path& coreLibsPath, const std::filesystem::path& modLibsPath, 
                            const std::filesystem::path& galacticAdventuresDataPath, const std::filesystem::path& coreSporeDataPath);

        /// <summary>
        ///     Returns wether all required paths exist
        /// </summary>
        bool CheckIfPathsExist(void);

        /// <summary>
        ///     Returns the combined path of paths
        /// </summary>
        std::filesystem::path Combine(const std::vector<std::filesystem::path>& paths);

        /// <summary>
        ///     Returns the absolute path of path
        /// </summary>
        std::filesystem::path GetAbsolutePath(const std::filesystem::path& path);

        /// <summary>
        ///     Retrieves full installation path
        /// </summary>
        std::filesystem::path GetFullInstallPath(SporeMod::InstallLocation installLocation, const std::filesystem::path& path);

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
}

#endif // SPOREMODMANAGERHELPERS_PATH_HPP