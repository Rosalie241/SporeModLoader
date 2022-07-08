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
    /// <summary>
    ///     Displays error message
    /// </summary>
    void ShowErrorMessage(std::wstring message);

    /// <summary>
    ///     Checks whether Spore Mod Manager is running
    /// </summary>
    bool IsSporeModManagerRunning(void);

    /// <summary>
    ///     Launches Spore Mod Manager
    /// </summary>
    void LaunchSporeModManager(void);

    /// <summary>
    ///     Returns the path to the SporeModManagerStorage path
    /// </summary>
    std::filesystem::path GetSporeModManagerStoragePath(void);

    /// <summary>
    ///     Returns the path to the Spore Mod Manager installation
    /// </summary>
    std::filesystem::path GetSporeModManagerInstallationPath(void);
}

#endif // SPOREMODLOADERHELPERS_HPP
