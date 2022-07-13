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
using System.IO;

namespace SporeModManager.SporeMods
{
    internal class Directory
    {
        private static DirectoryConfig directoryConfig;

        public static string GetModLibsDirectory()
        {
            return directoryConfig.ModLibsDirectory;
        }

        public static string GetGalacticAdventuresDataDirectory()
        {
            return directoryConfig.GalacticAdventuresDataDirectory;
        }

        public static string GetCoreSporeDataDirectory()
        {
            return directoryConfig.CoreSporeDataDirectory;
        }

        public static string GetDirectoryFromInstallLocation(ModInfoInstallLocation modInfoInstallLocation)
        {
            switch (modInfoInstallLocation)
            {
                default:
                case ModInfoInstallLocation.ModLibs:
                    return GetModLibsDirectory();

                case ModInfoInstallLocation.GalacticAdventuresData:
                    return GetGalacticAdventuresDataDirectory();

                case ModInfoInstallLocation.CoreSporeData:
                    return GetCoreSporeDataDirectory();
            }
        }

        public static void ReadDirectoryConfig()
        {
            directoryConfig = DirectoryConfig.ReadFromXml();
        }

        public static void EnsureDirectoriesExist()
        {
            foreach (var directory in new string[] { Directory.GetModLibsDirectory(), 
                                                        Directory.GetGalacticAdventuresDataDirectory(), 
                                                        Directory.GetCoreSporeDataDirectory()})
            {
                string fullPath = Path.GetFullPath(directory);
                if (!System.IO.Directory.Exists(fullPath))
                {
                    throw new DirectoryNotFoundException(fullPath);
                }
            }
        }

    }
}
