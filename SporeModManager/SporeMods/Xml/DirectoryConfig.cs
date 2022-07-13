/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.IO;
using System.Xml.Serialization;

namespace SporeModManager.SporeMods.Xml
{
    public class DirectoryConfig
    {
        public string ModLibsDirectory { get; set; }
        public string GalacticAdventuresDataDirectory { get; set; }
        public string CoreSporeDataDirectory { get; set; }

        public static DirectoryConfig ReadFromXml()
        {
            var directoryConfig = new DirectoryConfig();

            if (File.Exists("DirectoryConfig.xml"))
            {
                using (var fileStream = File.OpenRead("DirectoryConfig.xml"))
                {
                    var xmlSerializer = new XmlSerializer(typeof(DirectoryConfig));
                    directoryConfig = (DirectoryConfig)xmlSerializer.Deserialize(fileStream);
                    fileStream.Close();
                }
            }
            else
            {
                directoryConfig.ModLibsDirectory = @"..\ModLibs";
                directoryConfig.GalacticAdventuresDataDirectory = @"..\..\DataEP1";
                directoryConfig.CoreSporeDataDirectory = @"..\..\Data";

                using (var textWriter = new StreamWriter("DirectoryConfig.xml"))
                {
                    var serializer = new XmlSerializer(typeof(DirectoryConfig));
                    serializer.Serialize(textWriter, directoryConfig);
                }
            }

            return directoryConfig;
        }
    }
}
