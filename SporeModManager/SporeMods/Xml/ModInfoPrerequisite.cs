/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.Xml;

namespace SporeModManager.SporeMods.Xml
{
    internal class ModInfoPrerequisite
    {
        public ModInfoInstallLocation InstallLocation { get; set; }
        public string FileName { get; set; }

        public static ModInfoPrerequisite ParseXmlElement(XmlElement element)
        {
            var modInfoPrerequisite = new ModInfoPrerequisite();

            modInfoPrerequisite.InstallLocation = ModInfoInstallLocation.ModLibs;
            if (element.HasAttribute("game"))
            {
                string game = element.GetAttribute("game").ToLower();

                if (game == "galacticadventures")
                {
                    modInfoPrerequisite.InstallLocation = ModInfoInstallLocation.GalacticAdventuresData;
                }
                else if (game == "spore")
                {
                    modInfoPrerequisite.InstallLocation = ModInfoInstallLocation.CoreSporeData;
                }
            }

            modInfoPrerequisite.FileName = element.InnerText;

            return modInfoPrerequisite;
        }
    }
}
