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

namespace SporeModManager
{
    /// <summary>
    ///  Lists Installed mods
    /// </summary>
    bool ListInstalledMods(void);

    /// <summary>
    ///  Installs mod
    /// </summary>
    bool InstallMod(std::filesystem::path path);

    /// <summary>
    ///  Updates mod
    /// </summary>
    bool UpdateMod(std::filesystem::path path);

    /// <summary>
    ///  Uninstalls mod with id
    /// </summary>
    bool UninstallMod(int id);
}

#endif // SPOREMODMANAGER_HPP