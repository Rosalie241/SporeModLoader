/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODLOADER_HPP
#define SPOREMODLOADER_HPP

namespace SporeModLoader
{
    /// <summary>
    ///  Initializes SporeModLoader
    /// </summary>
    bool Initialize(void);

    /// <summary>
    ///  Loads Core Library, required to call before LoadMods
    /// </summary>
    bool LoadCoreLibs(void);

    /// <summary>
    ///  Loads Mod Libraryes
    /// </summary>
    bool LoadModLibs(void);
}

#endif // SPOREMODLOADER_HPP
