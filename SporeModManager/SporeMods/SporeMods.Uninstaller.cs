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
using System.IO;
using System.Linq;

namespace SporeModManager.SporeMods
{
    internal partial class SporeMods
    {
        public static void UninstallMod(int id)
        {
            var installedMods = InstalledMods.ReadFromXml();

            int installedModListCount = installedMods.InstalledModList.Count();
            if (id < 0 || id > installedModListCount || installedModListCount == 0)
            {
                throw new Exception("Invalid mod ID!");
            }

            var installedMod = installedMods.InstalledModList[id];

            Console.WriteLine($"> uninstalling {installedMod.Name}");

            foreach (var installedModFile in installedMod.Files)
            {
                Console.WriteLine($"-> removing {installedModFile.FileName}");
                string directory = Directory.GetDirectoryFromInstallLocation(installedModFile.InstallLocation);
                File.Delete(Path.Combine(directory, installedModFile.FileName));
            }

            installedMods.InstalledModList.Remove(installedMod);
            InstalledMods.SaveToXml(installedMods);

            Console.WriteLine($"> successfully uninstalled {installedMod.Name}");
        }
    }
}
