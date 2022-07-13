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
using System.IO;
using System.Xml;

namespace SporeModManager.SporeMods.Xml
{
    internal class ModInfo
    {
        public string DisplayName { get; set; }
        public string UniqueName { get; set; }
        public string Description { get; set; }
        public bool IsExperimental { get; set; }
        public bool RequiresGalaxyReset { get; set; }
        public bool CausesSaveDependency { get; set; }

        public Version InstallerSystemVersion { get; set; }
        public Version ModAPIDllsBuildVersion { get; set; }

        public List<ModInfoComponentGroup> ComponentGroups { get; set; }
        public List<ModInfoComponent> Components { get; set; }
        public List<ModInfoPrerequisite> Prerequisites { get; set; }
        public static ModInfo DeserializeFromStream(Stream stream)
        {
            var modInfo = new ModInfo();

            modInfo.ComponentGroups = new List<ModInfoComponentGroup>();
            modInfo.Components = new List<ModInfoComponent>();
            modInfo.Prerequisites = new List<ModInfoPrerequisite>();

            var xmlDocument = new XmlDocument();

            xmlDocument.Load(stream);

            var modElement = xmlDocument.DocumentElement;

            if (modElement.HasAttribute("displayName"))
            {
                modInfo.DisplayName = modElement.GetAttribute("displayName");
            }

            if (modElement.HasAttribute("unique"))
            {
                modInfo.UniqueName = modElement.GetAttribute("unique");
            }

            if (modElement.HasAttribute("description"))
            {
                modInfo.Description = modElement.GetAttribute("description");
            }

            if (modElement.HasAttribute("isExperimental"))
            {
                modInfo.IsExperimental = Boolean.Parse(modElement.GetAttribute("isExperimental"));
            }

            if (modElement.HasAttribute("requiresGalaxyReset"))
            {
                modInfo.RequiresGalaxyReset = Boolean.Parse(modElement.GetAttribute("requiresGalaxyReset"));
            }

            if (modElement.HasAttribute("causesSaveDataDependency"))
            {
                modInfo.CausesSaveDependency = Boolean.Parse(modElement.GetAttribute("causesSaveDataDependency"));
            }

            if (modElement.HasAttribute("installerSystemVersion"))
            {
                modInfo.InstallerSystemVersion = new Version(modElement.GetAttribute("installerSystemVersion"));
            }

            if (modElement.HasAttribute("dllsBuild"))
            {
                modInfo.ModAPIDllsBuildVersion = new Version(modElement.GetAttribute("dllsBuild"));
            }

            foreach (XmlElement componentGroupElement in modElement.SelectNodes("componentGroup"))
            {
                var componentGroup = new ModInfoComponentGroup();
                componentGroup.Components = new List<ModInfoComponent>();

                if (componentGroupElement.HasAttribute("displayName"))
                {
                    componentGroup.DisplayName = componentGroupElement.GetAttribute("displayName");
                }

                if (componentGroupElement.HasAttribute("unique"))
                {
                    componentGroup.UniqueName = componentGroupElement.GetAttribute("unique");
                }

                foreach (XmlElement componentElement in componentGroupElement.SelectNodes("component"))
                {
                    componentGroup.Components.Add(ModInfoComponent.ParseXmlElement(componentElement));
                }

                modInfo.ComponentGroups.Add(componentGroup);
            }

            foreach (XmlElement componentElement in modElement.SelectNodes("component"))
            {
                modInfo.Components.Add(ModInfoComponent.ParseXmlElement(componentElement));
            }

            foreach (XmlElement prerequisiteElement in modElement.SelectNodes("prerequisite"))
            {
                modInfo.Prerequisites.Add(ModInfoPrerequisite.ParseXmlElement(prerequisiteElement));
            }

            return modInfo;
        }

    }
}
