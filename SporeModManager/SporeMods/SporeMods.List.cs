/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using SporeModManager.SporeMods.Xml;
using System;
using System.Linq;

namespace SporeModManager.SporeMods
{
    internal partial class SporeMods
    {
        public static void ListInstalledMods()
        {
            var installedMods = InstalledMods.ReadFromXml();

            for (int i = 0; i < installedMods.InstalledModList.Count(); i++)
            {
                var installedMod = installedMods.InstalledModList[i];

                Console.WriteLine($"[{i}] {installedMod.Name}");
                Console.WriteLine($"  {installedMod.Description}");
            }
        }
    }
}
