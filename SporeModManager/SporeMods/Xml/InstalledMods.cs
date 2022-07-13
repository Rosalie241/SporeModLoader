/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace SporeModManager.SporeMods.Xml
{
    public class InstalledMods
    {
        public List<InstalledMod> InstalledModList { get; set; }

        public static InstalledMods ReadFromXml()
        {
            var installedMods = new InstalledMods();

            if (File.Exists("InstalledMods.xml"))
            {
                using (var fileStream = File.OpenRead("InstalledMods.xml"))
                {
                    var xmlSerializer = new XmlSerializer(typeof(InstalledMods));
                    installedMods = (InstalledMods)xmlSerializer.Deserialize(fileStream);
                }
            }
            else
            {
                installedMods.InstalledModList = new List<InstalledMod>();
            }

            return installedMods;
        }

        public static void SaveToXml(InstalledMods installedMods)
        {
            using (var textWriter = new StreamWriter("InstalledMods.xml"))
            {
                var serializer = new XmlSerializer(typeof(InstalledMods));
                serializer.Serialize(textWriter, installedMods);
            }
        }
    }
}
