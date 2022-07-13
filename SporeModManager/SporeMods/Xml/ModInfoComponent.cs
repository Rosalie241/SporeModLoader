/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System;
using System.Collections.Generic;
using System.Xml;

namespace SporeModManager.SporeMods.Xml
{
    internal class ModInfoComponent
    {
        public string DisplayName { get; set; }
        public string UniqueName { get; set; }
        public string Description { get; set; }
        public bool DefaultChecked { get; set; }
        public List<ModInfoInstallLocation> InstallLocations;
        public List<string> FileNames { get; set; }


        public static ModInfoComponent ParseXmlElement(XmlElement element)
        {
            var modInfoComponent = new ModInfoComponent();
            modInfoComponent.InstallLocations = new List<ModInfoInstallLocation>();
            modInfoComponent.FileNames = new List<string>();

            if (element.HasAttribute("displayName"))
            {
                modInfoComponent.DisplayName = element.GetAttribute("displayName");
            }

            if (element.HasAttribute("unique"))
            {
                modInfoComponent.UniqueName = element.GetAttribute("unique");
            }

            if (element.HasAttribute("description"))
            {
                modInfoComponent.Description = element.GetAttribute("description");
            }

            if (element.HasAttribute("defaultChecked"))
            {
                modInfoComponent.DefaultChecked = Boolean.Parse(element.GetAttribute("defaultChecked"));
            }

            if (element.HasAttribute("game"))
            {
                string gameElementString = element.GetAttribute("game").ToLower();

                foreach (var game in gameElementString.Split('?'))
                {
                    ModInfoInstallLocation installLocation = ModInfoInstallLocation.ModLibs;
                    if (game == "galacticadventures")
                    {
                        installLocation = ModInfoInstallLocation.GalacticAdventuresData;
                    }
                    else if (game == "spore")
                    {
                        installLocation = ModInfoInstallLocation.CoreSporeData;
                    }

                    modInfoComponent.InstallLocations.Add(installLocation);
                }
            }

            if (!String.IsNullOrEmpty(element.InnerText))
            {
                foreach (var fileName in element.InnerText.Split('?'))
                {
                    modInfoComponent.FileNames.Add(fileName);
                }
            }

            return modInfoComponent;
        }
    }
}
