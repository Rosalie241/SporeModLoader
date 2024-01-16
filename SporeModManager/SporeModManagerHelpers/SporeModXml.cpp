/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeModXml.hpp"
#include "String.hpp"
#include "Path.hpp"

#include <iostream>
#include <algorithm>
#include <tinyxml2.h>

using namespace SporeModManagerHelpers;

//
// Helper Functions
//

static std::string install_location_to_string(SporeMod::InstallLocation installLocation)
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

static SporeMod::InstallLocation parse_install_location(std::string text, bool configuration)
{
    SporeMod::InstallLocation installLocation;

    // lowercase text to make the 
    // value case insensitive
    text = String::Lowercase(text);

    if ((!configuration && text == "galacticadventures") || 
        (configuration  && text == "galacticadventuresdata"))
    {
        installLocation = SporeMod::InstallLocation::GalacticAdventuresData;
    }
    else if ((!configuration && text == "spore") || 
             (configuration  && text == "coresporedata"))
    {
        installLocation = SporeMod::InstallLocation::CoreSporeData;
    }
    else
    {
        installLocation = SporeMod::InstallLocation::ModLibs;
    }

    return installLocation;
}

static std::string get_attribute_text(tinyxml2::XMLElement* element, std::string attributeName)
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

static bool get_attribute_bool(tinyxml2::XMLElement* element, std::string attributeName)
{
    std::string boolString;

    boolString = get_attribute_text(element, attributeName);

    return String::Lowercase(boolString) == "true";
}

static std::string get_element_text(tinyxml2::XMLElement* element)
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

static std::string get_element_name(tinyxml2::XMLElement* element)
{
    if (element->Name() != nullptr)
    {
        return element->Name();
    }

    return "";
}

static tinyxml2::XMLElement* find_element(tinyxml2::XMLElement* rootElement, std::string name)
{
    tinyxml2::XMLElement* xmlElement;
    std::string xmlElementName;

    xmlElement = rootElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);
        if (xmlElementName == name)
        {
            return xmlElement;
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return nullptr;
}

static std::vector<SporeMod::Xml::SporeModFile> parse_files_element(tinyxml2::XMLElement* element, bool parseCompatFile = false)
{
    std::vector<SporeMod::Xml::SporeModFile> files;

    std::vector<std::string> installLocations;
    std::vector<std::string> installFiles;

    installLocations = String::Split(get_attribute_text(element, (parseCompatFile ? "compatTargetGame" : "game")), '?');
    installFiles     = String::Split((parseCompatFile ? 
                                        get_attribute_text(element, "compatTargetFileName") : 
                                        get_element_text(element)), 
                                     '?');

    for (size_t i = 0; i < installFiles.size(); i++)
    {
        SporeMod::Xml::SporeModFile file;

        if (installLocations.size() > i)
        {
            file.InstallLocation = parse_install_location(installLocations[i], false);
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

static SporeMod::Xml::SporeModInfoComponent parse_component_element(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoComponent component;

    component.Name           = get_attribute_text(element, "displayName");
    component.UniqueName     = get_attribute_text(element, "unique");
    component.Description    = get_attribute_text(element, "description");
    component.DefaultChecked = get_attribute_bool(element, "defaultChecked");
    component.Files          = parse_files_element(element);

    return component;
}


static SporeMod::Xml::SporeModInfoComponentGroup parse_componentgroup_element(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoComponentGroup componentGroup;
    tinyxml2::XMLElement* xmlElement;
    std::string xmlElementName;

    componentGroup.Name       = get_attribute_text(element, "displayName");
    componentGroup.UniqueName = get_attribute_text(element, "unique");
    
    xmlElement = element->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);

        if (xmlElementName == "component")
        {
            componentGroup.Components.push_back(parse_component_element(xmlElement));
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return componentGroup;
}

static SporeMod::Xml::SporeModInfoPrerequisite parse_prerequisite_element(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoPrerequisite prerequisite;

    prerequisite.Files = parse_files_element(element);

    return prerequisite;
}

static SporeMod::Xml::SporeModInfoCompatFile parse_compatfile_element(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::SporeModInfoCompatFile compatFile;

    compatFile.RequiredFiles = parse_files_element(element, true);
    compatFile.Files         = parse_files_element(element);

    return compatFile;
}

static std::vector<SporeMod::Xml::SporeModFile> parse_installedsporemodfiles_element(tinyxml2::XMLElement* element)
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
        xmlElementName = get_element_name(xmlElement);

        if (xmlElementName == "InstalledModFile")
        {
            SporeMod::Xml::SporeModFile sporeModFile;

            sporeModFile.FileName        = get_element_text(find_element(xmlElement, "FileName"));
            sporeModFile.InstallLocation = parse_install_location(get_element_text(find_element(xmlElement, "InstallLocation")), true);

            sporeModFiles.push_back(sporeModFile);
        }

        xmlElement = xmlElement->NextSiblingElement();
    }

    return sporeModFiles;
}

static SporeMod::Xml::InstalledSporeMod parse_installedsporemod_element(tinyxml2::XMLElement* element)
{
    SporeMod::Xml::InstalledSporeMod installedSporeMod;

    installedSporeMod.Name           = get_element_text(find_element(element, "Name"));
    installedSporeMod.UniqueName     = get_element_text(find_element(element, "UniqueName"));
    installedSporeMod.Description    = get_element_text(find_element(element, "Description"));
    installedSporeMod.InstalledFiles = parse_installedsporemodfiles_element(find_element(element, "Files"));

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
        std::cerr << "Error: XmlDocument.Parse() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "Error: XMLDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    sporeModInfo.Name        = get_attribute_text(xmlElement, "displayName");
    sporeModInfo.UniqueName  = get_attribute_text(xmlElement, "unique");
    sporeModInfo.Description = get_attribute_text(xmlElement, "description");

    sporeModInfo.IsExperimental           = get_attribute_bool(xmlElement, "isExperimental");
    sporeModInfo.RequiresGalaxyReset      = get_attribute_bool(xmlElement, "requiresGalaxyReset");
    sporeModInfo.CausesSaveDataDependency = get_attribute_bool(xmlElement, "causesSaveDataDependency");
    sporeModInfo.HasCustomInstaller       = get_attribute_bool(xmlElement, "hasCustomInstaller");
    sporeModInfo.CompatOnly               = get_attribute_bool(xmlElement, "compatOnly");

    xmlAttributeText = get_attribute_text(xmlElement, "installerSystemVersion");
    if (!xmlAttributeText.empty())
    {
        ret = FileVersion::ParseString(xmlAttributeText, sporeModInfo.InstallerVersion);
        if (!ret)
        {
            std::cerr << "Error: FileVersion::ParseString() Failed!" << std::endl;
            return false;
        }
        else
        {
            FileVersion::FileVersionInfo maxSupportedInstallerVersion = {1, 0, 1, 2};
            if (sporeModInfo.InstallerVersion > maxSupportedInstallerVersion)
            {
                std::cerr << "Error: installerSystemVersion \"" << sporeModInfo.InstallerVersion.to_string() << "\" is unsupported!" << std::endl;
                return false;
            }
        }
    }

    // installer version 1.0.0.0 doesn't have the hasCustomInstaller attribute,
    // so keep backwards compatibility by enabling hasCustomInstaller when
    // the compatOnly attribute is false, see the code in Spore-Mod-Manager:
    // https://github.com/Splitwirez/Spore-Mod-Manager/blob/b07dabf53716fe2ea455d6dad21b213720b5ad91/SporeMods.Core/Mods/XmlModIdentityV1.cs#L175
    FileVersion::FileVersionInfo installerVersion = {1, 0, 0, 0};
    if (sporeModInfo.InstallerVersion == installerVersion && !sporeModInfo.HasCustomInstaller)
    {
        sporeModInfo.HasCustomInstaller = !sporeModInfo.CompatOnly;
    }

    xmlAttributeText = get_attribute_text(xmlElement, "dllsBuild");
    if (!xmlAttributeText.empty())
    {
        ret = FileVersion::ParseString(xmlAttributeText, sporeModInfo.MinimumModAPILibVersion);
        if (!ret)
        {
            std::cerr << "Error: FileVersion::ParseString() Failed!" << std::endl;
            return false;
        }
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);

        if (xmlElementName == "componentGroup")
        {
            sporeModInfo.ComponentGroups.push_back(parse_componentgroup_element(xmlElement));
        }
        else if (xmlElementName == "component")
        {
            sporeModInfo.Components.push_back(parse_component_element(xmlElement));
        }
        else if (xmlElementName == "prerequisite")
        {
            sporeModInfo.Prerequisites.push_back(parse_prerequisite_element(xmlElement));
        }
        else if (xmlElementName == "compatFile")
        {
            sporeModInfo.CompatFiles.push_back(parse_compatfile_element(xmlElement));
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
            std::cerr << "Error: Xml::SaveDirectories() Failed!" << std::endl;
            return false;
        }
    }

    error = xmlDocument.LoadFile(configFilePath.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS)
    {
        std::cerr << "Error: XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "Error: XMLDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);
        if (xmlElementName == "Directories")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = get_element_name(childXmlElement);
                if (xmlElementName == "CoreLibsDirectory")
                {
                    coreLibsPath = get_element_text(childXmlElement);
                }
                else if (xmlElementName == "ModLibsDirectory")
                {
                    modLibsPath = get_element_text(childXmlElement);
                }
                else if (xmlElementName == "GalacticAdventuresDataDirectory")
                {
                    galacticAdventuresDataPath = get_element_text(childXmlElement);
                }
                else if (xmlElementName == "CoreSporeDataDirectory")
                {
                    coreSporeDataPath = get_element_text(childXmlElement);
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
        std::cerr << "Error: XMLDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);
        if (xmlElementName == "Directories")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = get_element_name(childXmlElement);
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
        std::cerr << "Error: XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlDocument.RootElement();
    if (xmlElement == nullptr)
    {
        std::cerr << "Error: XMLDocument.RootElement() Failed!" << std::endl;
        return false;
    }

    xmlElement = xmlElement->FirstChildElement();
    while (xmlElement != nullptr)
    {
        xmlElementName = get_element_name(xmlElement);
        if (xmlElementName == "InstalledSporeMods")
        {
            childXmlElement = xmlElement->FirstChildElement();
            while (childXmlElement != nullptr)
            {
                xmlElementName = get_element_name(childXmlElement);
                if (xmlElementName == "InstalledSporeMod")
                {
                    installedSporeModList.push_back(parse_installedsporemod_element(childXmlElement));
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
        std::cerr << "Error: XmlDocument.LoadFile() Failed!" << std::endl;
        return false;
    }

    rootXmlElement = xmlDocument.RootElement();
    xmlDocument.InsertFirstChild(rootXmlElement);

    installedSporeModsElement = find_element(rootXmlElement, "InstalledSporeMods");
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
            std::string installLocation = install_location_to_string(installedFile.InstallLocation);

            installedModFileElement = filesXmlElement->InsertNewChildElement("InstalledModFile");
            installedModFileElement->InsertNewChildElement("FileName")->SetText(fileName.c_str());
            installedModFileElement->InsertNewChildElement("InstallLocation")->SetText(installLocation.c_str());
        }
    }

    xmlDocument.SaveFile(configFilePath.string().c_str());
    return true;
}

