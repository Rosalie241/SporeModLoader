/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGER_HPP
#define SPOREMODMANAGER_HPP

#include <filesystem>
#include <vector>

namespace SporeModManager
{
    /// <summary>
    ///  Lists Installed mods
    /// </summary>
    bool ListInstalledMods(void);

    /// <summary>
    ///  Installs mod
    /// </summary>
    bool InstallMods(std::vector<std::filesystem::path> paths);

    /// <summary>
    ///  Updates mod
    /// </summary>
    bool UpdateMods(std::vector<std::filesystem::path> paths, bool requiresInstalled = true);

    /// <summary>
    ///  Uninstalls mods with ids
    /// </summary>
    bool UninstallMods(std::vector<int> ids);
}

#endif // SPOREMODMANAGER_HPP