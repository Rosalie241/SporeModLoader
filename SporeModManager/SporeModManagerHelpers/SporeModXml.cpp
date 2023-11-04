/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeModManagerHelpers.hpp"

#include <iostream>
#include <algorithm>
#include <tinyxml2.h>

using namespace SporeModManagerHelpers;

//
// Helper Functions
//

static std::string InstallLocationToString(SporeMod::InstallLocation installLocation)
{
    switch (installLocation)
    {
    default:
    case SporeMod::InstallLocation::ModLibs:
        return "ModLibs";
    case SporeMod::InstallLocation::GalacticAdventuresData:
        return "GalacticAdventuresData";
    case SporeMod::InstallLocation::CoreSporeData:
        return "CoreSporeData";
    }
}

static SporeMod::InstallLocation ParseInstallLocation(std::string text)
{
    SporeMod::InstallLocation installLocation;

    if (text == "GalacticAdventures" || text == "GalacticAdventuresData")
    {
        installLocation = SporeMod::InstallLocation::GalacticAdventuresData;
    }
    else if (text == "CoreSpore" || text == "CoreSporeData")
    {
        installLocation = SporeMod::InstallLocation::CoreSporeData;
    }
    else
    {
        installLocation = SporeMod::InstallLocation::ModLibs;
    }

    return installLocation;
}

static std::string GetAttributeText(tinyxml2::XMLElement* element, std::string attributeName)
{
    const tinyxml2::XMLAttribute* xmlAttribute;

    xmlAttribute = element->FindAttribute(attributeName.c_str());
    if (xmlAttribute != nullptr)
    {
        if (xmlAttribute->Value() != nullptr)
        {
            return xmlAttribute->Value();
        }
    }

    return "";
}

static bool GetAttributeBool(tinyxml2::XMLElement* element, std::string attributeName)
{
    std::string boolString;

    boolString = GetAttributeText(element, attributeName);

    return String::Lowercase(boolString) == "true";
}

static std::string GetElementText(tinyxml2::XMLElement* element)
{
    const char* text;

    if (element == nullptr)
    {
        return "";
    }

    text = element->GetText();
    if (text != nullptr)
    {
        return text;
    }

    return "";
}

static std::string GetElementName(tinyxml2::XMLElement* element)
{
    if (element->Name() != nullptr)
    {
        return element->Name();
    }

    return "";
}

static tinyxml2::XMLElement* FindElement(tinyxml2::XMLElement* rootElement, std::string name)
{
    tinyxml2::XMLElement* xmlElement;
    std::string xmlElementName;

    xmlElement = rootElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);
        if (xmlElementName == name)
        {
            return xmlElement;
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return nullptr;
}

static std::vector<SporeMod::Xml::SporeModFile> ParseFiles(tinyxml2::XMLElement* element, bool parseCompatFile = false)
{
    std::vector<SporeMod::Xml::SporeModFile> files;

    std::vector<std::string> installLocations;
    std::vector<std::string> installFiles;

    installLocations = String::Split(GetAttributeText(element, (parseCompatFile ? "compatTargetGame" : "game")), '?');
    installFiles     = String::Split((parseCompatFile ? 
                                        GetAttributeText(element, "compatTargetFileName") : 
                                        GetElementText(element)), 
                                     '?');

    for (size_t i = 0; i < installFiles.size(); i++)
    {
        SporeMod::Xml::SporeModFile file;

        if (installLocations.size() > i)
        {
            file.InstallLocation = ParseInstallLocation(installLocations[i]);
        }
        else
        {
            file.InstallLocation = SporeMod::InstallLocation::ModLibs;
        }
        file.FileName = installFiles[i];

        files.push_back(file);
    }

    return files;
}

static SporeMod::Xml::SporeModInfoComponent ParseComponentElement(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoComponent component;

    component.Name           = GetAttributeText(element, "displayName");
    component.UniqueName     = GetAttributeText(element, "unique");
    component.Description    = GetAttributeText(element, "description");
    component.DefaultChecked = GetAttributeBool(element, "defaultChecked");
    component.Files          = ParseFiles(element);

    return component;
}


static SporeMod::Xml::SporeModInfoComponentGroup ParseComponentGroupElement(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoComponentGroup componentGroup;
    tinyxml2::XMLElement* xmlElement;
    std::string xmlElementName;

    componentGroup.Name       = GetAttributeText(element, "displayName");
    componentGroup.UniqueName = GetAttributeText(element, "unique");
    
    xmlElement = element->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);

        if (xmlElementName == "component")
        {
            componentGroup.Components.push_back(ParseComponentElement(xmlElement));
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return componentGroup;
}

static SporeMod::Xml::SporeModInfoPrerequisite ParsePrerequisiteElement(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoPrerequisite prerequisite;

    prerequisite.Files = ParseFiles(element);

    return prerequisite;
}

static SporeMod::Xml::SporeModInfoCompatFile ParseCompatFileElement(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoCompatFile compatFile;

    compatFile.RequiredFiles = ParseFiles(element, true);
    compatFile.Files         = ParseFiles(element);

    return compatFile;
}

static std::vector<SporeMod::Xml::SporeModFile> ParseInstalledSporeModFilesElement(tinyxml2::XMLElement* element)
{
    std::vector<SporeMod::Xml::SporeModFile> sporeModFiles;
    tinyxml2::XMLElement* xmlElement;
    std::string xmlElementName;

    if (element == nullptr)
    {
        return sporeModFiles;
    }

    xmlElement = element->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);

        if (xmlElementName == "InstalledModFile")
        {
            SporeMod::Xml::SporeModFile sporeModFile;

            sporeModFile.FileName        = GetElementText(FindElement(xmlElement, "FileName"));
            sporeModFile.InstallLocation = ParseInstallLocation(GetElementText(FindElement(xmlElement, "InstallLocation")));

            sporeModFiles.push_back(sporeModFile);
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return sporeModFiles;
}

static SporeMod::Xml::InstalledSporeMod ParseInstalledSporeModElement(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::InstalledSporeMod installedSporeMod;

    installedSporeMod.Name           = GetElementText(FindElement(element, "Name"));
    installedSporeMod.UniqueName     = GetElementText(FindElement(element, "UniqueName"));
    installedSporeMod.Description    = GetElementText(FindElement(element, "Description"));
    installedSporeMod.InstalledFiles = ParseInstalledSporeModFilesElement(FindElement(element, "Files"));

    return installedSporeMod;
}

//
// Exported Functions
//

bool SporeMod::Xml::ParseSporeModInfo(const std::vector<char>& buffer, SporeModInfo& sporeModInfo)
{
    tinyxml2::XMLDocument xmlDocument;
    tinyxml2::XMLElement* xmlElement;
    tinyxml2::XMLError    error;
    std::string xmlElementName;
    std::string xmlAttributeText;
    bool ret;

    error = xmlDocument.Parse(buffer.data(), buffer.size());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    {
        std::cerr << "XmlDocument.Parse() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "XDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    sporeModInfo.Name        = GetAttributeText(xmlElement, "displayName");
    sporeModInfo.UniqueName  = GetAttributeText(xmlElement, "unique");
    sporeModInfo.Description = GetAttributeText(xmlElement, "description");

    sporeModInfo.IsExperimental           = GetAttributeBool(xmlElement, "isExperimental");
    sporeModInfo.RequiresGalaxyReset      = GetAttributeBool(xmlElement, "requiresGalaxyReset");
    sporeModInfo.CausesSaveDataDependency = GetAttributeBool(xmlElement, "causesSaveDataDependency");
    sporeModInfo.HasCustomInstaller       = GetAttributeBool(xmlElement, "hasCustomInstaller");

    xmlAttributeText = GetAttributeText(xmlElement, "installerSystemVersion");
    if (!xmlAttributeText.empty())
    {
        ret = FileVersion::ParseString(xmlAttributeText, sporeModInfo.InstallerVersion);
        if (!ret)
        {
            std::cerr << "FileVersion::ParseString() Failed!" << std::endl;
            return false;
        }
        else
        {
            FileVersion::FileVersionInfo maxSupportedInstallerVersion;
            FileVersion::ParseString("1.0.1.2", maxSupportedInstallerVersion);

            if (sporeModInfo.InstallerVersion > maxSupportedInstallerVersion)
            {
                std::cerr << "installerSystemVersion \"" << sporeModInfo.InstallerVersion.to_string() << "\" is unsupported!" << std::endl;
                return false;
            }
        }
    }

    xmlAttributeText = GetAttributeText(xmlElement, "dllsBuild");
    if (!xmlAttributeText.empty())
    {
        ret = FileVersion::ParseString(xmlAttributeText, sporeModInfo.MinimumModAPILibVersion);
        if (!ret)
        {
            std::cerr << "FileVersion::ParseString() Failed!" << std::endl;
            return false;
        }
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);

        if (xmlElementName == "componentGroup")
        {
            sporeModInfo.ComponentGroups.push_back(ParseComponentGroupElement(xmlElement));
        }
        else if (xmlElementName == "component")
        {
            sporeModInfo.Components.push_back(ParseComponentElement(xmlElement));
        }
        else if (xmlElementName == "prerequisite")
        {
            sporeModInfo.Prerequisites.push_back(ParsePrerequisiteElement(xmlElement));
        }
        else if (xmlElementName == "compatFile")
        {
            sporeModInfo.CompatFiles.push_back(ParseCompatFileElement(xmlElement));
        }

        xmlElement = xmlElement->NextSiblingElement();
    }
    return true;
}

bool SporeMod::Xml::GetDirectories(std::filesystem::path& coreLibsPath, std::filesystem::path& modLibsPath, std::filesystem::path& galacticAdventuresDataPath, std::filesystem::path& coreSporeDataPath)
{
    std::filesystem::path configFilePath;
    tinyxml2::XMLDocument xmlDocument;
    tinyxml2::XMLElement* xmlElement;
    tinyxml2::XMLElement* childXmlElement;
    tinyxml2::XMLError    error;
    std::string xmlElementName;

    configFilePath = Path::GetConfigFilePath();

    if (!std::filesystem::is_regular_file(configFilePath))
    {
        if (!Xml::SaveDirectories(Path::Combine({ "..", "CoreLibs" }),
                                  Path::Combine({ "..", "ModLibs" }),
                                  Path::Combine({ "..", "..", "DataEP1" }),
                                  Path::Combine({ "..", "..", "Data" })))
        {
            std::cerr << "Xml::SaveDirectories() Failed!" << std::endl;
            return false;
        }
    }

    error = xmlDocument.LoadFile(configFilePath.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    {
        std::cerr << "XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "XDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);
        if (xmlElementName == "Directories")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = GetElementName(childXmlElement);
                if (xmlElementName == "CoreLibsDirectory")
                {
                    coreLibsPath = GetElementText(childXmlElement);
                }
                else if (xmlElementName == "ModLibsDirectory")
                {
                    modLibsPath = GetElementText(childXmlElement);
                }
                else if (xmlElementName == "GalacticAdventuresDataDirectory")
                {
                    galacticAdventuresDataPath = GetElementText(childXmlElement);
                }
                else if (xmlElementName == "CoreSporeDataDirectory")
                {
                    coreSporeDataPath = GetElementText(childXmlElement);
                }

                childXmlElement = childXmlElement->NextSiblingElement();
            }
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return true;
}

bool SporeMod::Xml::SaveDirectories(std::filesystem::path coreLibsPath, std::filesystem::path modLibsPath, std::filesystem::path galacticAdventuresDataPath, std::filesystem::path coreSporeDataPath)
{
    std::filesystem::path configFilePath;
    tinyxml2::XMLDocument xmlDocument;
    tinyxml2::XMLElement* rootXmlElement;
    tinyxml2::XMLElement* xmlElement;
    tinyxml2::XMLElement* childXmlElement;
    tinyxml2::XMLError    error;
    std::string xmlElementName;

    // do nothing when we've been given empty paths
    if (coreLibsPath.empty() && modLibsPath.empty() &&
        galacticAdventuresDataPath.empty() && coreSporeDataPath.empty())
    {
        return true;
    }

    configFilePath = Path::GetConfigFilePath();

    error = xmlDocument.LoadFile(configFilePath.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    { // config file doesn't exist
        rootXmlElement = xmlDocument.NewElement("SporeModManager");
        xmlDocument.InsertFirstChild(rootXmlElement);

        xmlElement = rootXmlElement->InsertNewChildElement("Directories");
        xmlElement->InsertNewChildElement("CoreLibsDirectory")->SetText(coreLibsPath.string().c_str());
        xmlElement->InsertNewChildElement("ModLibsDirectory")->SetText(modLibsPath.string().c_str());
        xmlElement->InsertNewChildElement("GalacticAdventuresDataDirectory")->SetText(galacticAdventuresDataPath.string().c_str());
        xmlElement->InsertNewChildElement("CoreSporeDataDirectory")->SetText(coreSporeDataPath.string().c_str());

        xmlDocument.SaveFile(configFilePath.string().c_str());
        return true;
    }
    
    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "XDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);
        if (xmlElementName == "Directories")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = GetElementName(childXmlElement);
                if (xmlElementName == "CoreLibsDirectory" && !coreLibsPath.empty())
                {
                    childXmlElement->SetText(Path::GetAbsolutePath(coreLibsPath).string().c_str());
                }
                else if (xmlElementName == "ModLibsDirectory" && !modLibsPath.empty())
                {
                    childXmlElement->SetText(Path::GetAbsolutePath(modLibsPath).string().c_str());
                }
                else if (xmlElementName == "GalacticAdventuresDataDirectory" && !galacticAdventuresDataPath.empty())
                {
                    childXmlElement->SetText(Path::GetAbsolutePath(galacticAdventuresDataPath).string().c_str());
                }
                else if (xmlElementName == "CoreSporeDataDirectory" && !coreSporeDataPath.empty())
                {
                    childXmlElement->SetText(Path::GetAbsolutePath(coreSporeDataPath).string().c_str());
                }

                childXmlElement = childXmlElement->NextSiblingElement();
            }
        }

        xmlElement = xmlElement->NextSiblingElement();
    }
    
    xmlDocument.SaveFile(configFilePath.string().c_str());
    return true;
}

bool SporeMod::Xml::GetInstalledModList(std::vector<InstalledSporeMod>& installedSporeModList)
{
    std::filesystem::path configFilePath;
    tinyxml2::XMLDocument xmlDocument;
    tinyxml2::XMLElement* xmlElement;
    tinyxml2::XMLElement* childXmlElement;
    tinyxml2::XMLError    error;
    std::string xmlElementName;

    configFilePath = Path::GetConfigFilePath();

    if (!std::filesystem::is_regular_file(configFilePath))
    { 
        return true;
    }

    error = xmlDocument.LoadFile(configFilePath.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    {
        std::cerr << "XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "XDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = GetElementName(xmlElement);
        if (xmlElementName == "InstalledSporeMods")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = GetElementName(childXmlElement);
                if (xmlElementName == "InstalledSporeMod")
                {
                    installedSporeModList.push_back(ParseInstalledSporeModElement(childXmlElement));
                }

                childXmlElement = childXmlElement->NextSiblingElement();
            }
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    // sort list by alphabet
    std::sort(installedSporeModList.begin(), installedSporeModList.end(),
        [](const InstalledSporeMod& a, const InstalledSporeMod& b) 
        {
            return a.Name < b.Name;
        }
    );

    return true;
}

bool SporeMod::Xml::SaveInstalledModList(const std::vector<InstalledSporeMod>& installedSporeModList)
{
    std::filesystem::path configFilePath;
    tinyxml2::XMLDocument xmlDocument;
    tinyxml2::XMLElement* rootXmlElement;
    tinyxml2::XMLElement* installedSporeModsElement;
    tinyxml2::XMLElement* xmlElement;
    tinyxml2::XMLElement* filesXmlElement;
    tinyxml2::XMLElement* installedModFileElement;
    tinyxml2::XMLError    error;
    std::string xmlElementName;

    configFilePath = Path::GetConfigFilePath();

    error = xmlDocument.LoadFile(configFilePath.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    {
        std::cerr << "XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    rootXmlElement = xmlDocument.RootElement();
    xmlDocument.InsertFirstChild(rootXmlElement);

    installedSporeModsElement = FindElement(rootXmlElement, "InstalledSporeMods");
    if (installedSporeModsElement != nullptr)
    { // element exists, so remove all children
        installedSporeModsElement->DeleteChildren();
    }
    else
    { // element doesn't exist, so insert it
        installedSporeModsElement = rootXmlElement->InsertNewChildElement("InstalledSporeMods");
    }

    for (const auto& installedSporeMod : installedSporeModList)
    {
        xmlElement = installedSporeModsElement->InsertNewChildElement("InstalledSporeMod");
        xmlElement->InsertNewChildElement("Name")->SetText(installedSporeMod.Name.c_str());
        xmlElement->InsertNewChildElement("UniqueName")->SetText(installedSporeMod.UniqueName.c_str());
        xmlElement->InsertNewChildElement("Description")->SetText(installedSporeMod.Description.c_str());

        filesXmlElement = xmlElement->InsertNewChildElement("Files");

        for (const auto& installedFile : installedSporeMod.InstalledFiles)
        {
            std::string fileName = installedFile.FileName.string();
            std::string installLocation = InstallLocationToString(installedFile.InstallLocation);

            installedModFileElement = filesXmlElement->InsertNewChildElement("InstalledModFile");
            installedModFileElement->InsertNewChildElement("FileName")->SetText(fileName.c_str());
            installedModFileElement->InsertNewChildElement("InstallLocation")->SetText(installLocation.c_str());
        }
    }

    xmlDocument.SaveFile(configFilePath.string().c_str());
    return true;
}

