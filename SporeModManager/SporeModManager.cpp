/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include <iostream>
#include <fstream>

#include "SporeModManagerHelpers/Download.hpp"
#include "SporeModManagerHelpers/SporeMod.hpp"
#include "SporeModManagerHelpers/String.hpp"
#include "SporeModManagerHelpers/Path.hpp"
#include "SporeModManagerHelpers/Zip.hpp"
#include "SporeModManagerHelpers/UI.hpp"
#include "SporeModManager.hpp"

using namespace SporeModManagerHelpers;

//
// Local Variables
//

static bool                                          l_HasInstalledSporeMods = false;
static std::vector<SporeMod::Xml::InstalledSporeMod> l_InstalledSporeMods;
static bool                                          l_SaveInstalledSporeMods = true;
static std::vector<SporeMod::Xml::SporeModInfo>      l_SporeModInfos;
static std::vector<Zip::ZipFile>                     l_ZipFiles;

//
// Helper Functions
//

static bool get_installedsporemodlist(void)
{
    if (!l_HasInstalledSporeMods)
    {
        if (!SporeMod::Xml::GetInstalledModList(l_InstalledSporeMods))
        {
            std::cerr << "Error: failed to retrieve installed mod list!" << std::endl;
            return false;
        }
        l_HasInstalledSporeMods = true;
    }
    return true;
}

static bool save_installedsporemodlist(void)
{
    if (l_SaveInstalledSporeMods)
    {
        if (!SporeMod::Xml::SaveInstalledModList(l_InstalledSporeMods))
        {
            std::cerr << "Error: failed to save installed mod list!" << std::endl;
            return false;
        }
    }
    return true;
}

static bool get_sporemodinfo(const std::filesystem::path& path, const std::string& extension, SporeMod::Xml::SporeModInfo& sporeModInfo)
{
    Zip::ZipFile zipFile = nullptr;
    std::vector<char> modInfoFileBuffer;

    if (extension == ".sporemod")
    {
        if (!Zip::OpenFile(zipFile, path))
        {
            std::cerr << "Error: failed to open zip file!" << std::endl;
            return false;
        }

        sporeModInfo.HasModInfoXml = Zip::LocateFile(zipFile, "modinfo.xml");
        if (!sporeModInfo.HasModInfoXml)
        { // no modinfo.xml
            sporeModInfo.UniqueName = path.stem().string();
            sporeModInfo.Name       = sporeModInfo.UniqueName;
        }
        else
        { // has modinfo.xml
            if (!Zip::ExtractFile(zipFile, "", modInfoFileBuffer))
            {
                std::cerr << "Error: failed to extract ModInfo.xml from zip file!" << std::endl;
                Zip::CloseFile(zipFile);
                return false;
            }

            if (!SporeMod::Xml::ParseSporeModInfo(modInfoFileBuffer, sporeModInfo))
            {
                std::cerr << "Error: failed to parse ModInfo.xml!" << std::endl;
                Zip::CloseFile(zipFile);
                return false;
            }

            // make sure we have the modapi dll that the mod requires
            if (!FileVersion::CheckIfCoreLibMatchesVersion(sporeModInfo.MinimumModAPILibVersion, sporeModInfo.Name))
            {
                Zip::CloseFile(zipFile);
                return false;
            }
        }
    }
    else if (extension == ".package")
    {
        sporeModInfo.UniqueName = path.stem().string();
        sporeModInfo.Name       = sporeModInfo.UniqueName;
    }

    l_SporeModInfos.push_back(sporeModInfo);
    l_ZipFiles.push_back(zipFile);
    return true;
}

static void reserve_list_items(size_t size)
{
    l_SporeModInfos.reserve(size);
    l_ZipFiles.reserve(size);
}

static void close_zipfiles(void)
{
    for (const Zip::ZipFile& zipFile : l_ZipFiles)
    {
        Zip::CloseFile(zipFile);
    }
}

//
// Exported Functions
//

bool SporeModManager::ListInstalledMods(void)
{
    SporeMod::Xml::InstalledSporeMod installedSporeMod;

    if (!get_installedsporemodlist())
    {
        return false;
    }

    for (size_t i = 0; i < l_InstalledSporeMods.size(); i++)
    {
        installedSporeMod = l_InstalledSporeMods[i];

        std::cout << "[" << i << "] " << installedSporeMod.Name << std::endl;
        if (!installedSporeMod.Description.empty())
        {
            std::cout << "  " << String::Replace(installedSporeMod.Description, "\n", "\n  ") << std::endl;
        }
    }

    return true;
}

bool SporeModManager::InstallMods(std::vector<std::filesystem::path> paths, bool skipValidation, bool skipInstalled, bool skipConfiguration)
{
    std::vector<std::string> uniqueNames;
    std::string              extension;
    bool                     returnValue = true;
    int                              installedSporeModId;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    SporeMod::Xml::SporeModInfo      sporeModInfo;

    if (!get_installedsporemodlist())
    {
        return false;
    }

    // do some basic validation before attempting
    // to install the given mods
    if (!skipValidation)
    {
        // reserve list items
        reserve_list_items(paths.size());

        for (size_t i = 0; i < paths.size(); i++)
        {
            const std::filesystem::path& path = paths[i];

            if (!std::filesystem::is_regular_file(path))
            {
                std::cerr << "Error: " << path << " is not a regular file or doesn't exist!" << std::endl;
                close_zipfiles();
                return false;
            }

            sporeModInfo = {};
            extension = String::Lowercase(path.extension().string());
            if (extension == ".sporemod" || extension == ".package")
            {
                if (!get_sporemodinfo(path, extension, sporeModInfo))
                {
                    close_zipfiles();
                    return false;
                }
            }
            else
            {
                std::cerr << "Error: \"" << extension << "\" is an invalid extension!" << std::endl;
                close_zipfiles();
                return false;
            }

            // ensure the mod isn't already installed
            const bool hasInstalled = SporeMod::FindInstalledMod(sporeModInfo.UniqueName, installedSporeModId, l_InstalledSporeMods);
            if (!skipInstalled && hasInstalled)
            {
                std::cerr << "Error: a mod with the same unique name (" << sporeModInfo.Name << ") has already been installed" << std::endl;
                close_zipfiles();
                return false;
            }

            // check if we need to skip the file due to
            // either already having a mod in the install list
            // with the same unique name or a mod with the same
            // unique name already being installed
            const bool hasUniqueName = std::find(uniqueNames.begin(), uniqueNames.end(), sporeModInfo.UniqueName) != uniqueNames.end();
            const bool skipInstall   = skipInstalled && hasInstalled;
            if (hasUniqueName || skipInstall)
            {
                std::cout << "Skipping " << path << (hasUniqueName ?
                                " as it's already being installed!" :
                                " as it's already installed!") << std::endl;
                Zip::CloseFile(l_ZipFiles[i]);
                paths.erase(paths.begin() + i);
                l_ZipFiles.erase(l_ZipFiles.begin() + i);
                l_SporeModInfos.erase(l_SporeModInfos.begin() + i);
                i -= 1;
                continue;
            }
            uniqueNames.push_back(sporeModInfo.UniqueName);
        }
    }

    // configure given mods
    if (!skipConfiguration)
    {
        for (size_t i = 0; i < paths.size(); i++)
        {
            const std::filesystem::path& path = paths[i];

            installedSporeMod = {};
            extension = String::Lowercase(path.extension().string());
            if (extension == ".sporemod")
            {
                if (!SporeMod::ConfigureSporeMod(l_ZipFiles[i], l_SporeModInfos[i], installedSporeMod, l_InstalledSporeMods))
                {
                    close_zipfiles();
                    return false;
                }
            }
            else if (extension == ".package")
            {
                if (!SporeMod::ConfigurePackage(path, installedSporeMod, l_InstalledSporeMods))
                {
                    close_zipfiles();
                    return false;
                }
            }
            l_InstalledSporeMods.push_back(installedSporeMod);
        }
    }

    // install given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths[i];
        const size_t installedSporeModIndex = l_InstalledSporeMods.size() - paths.size() + i;
        const SporeMod::Xml::InstalledSporeMod& installedSporeMod = l_InstalledSporeMods[installedSporeModIndex];

        extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod")
        {
            if (!SporeMod::InstallSporeMod(l_ZipFiles[i], installedSporeMod))
            {
                returnValue = false;
            }
        }
        else if (extension == ".package")
        {
            if (!SporeMod::InstallPackage(path, installedSporeMod))
            {
                returnValue = false;
            }
        }

        if (!returnValue)
        {
            l_InstalledSporeMods.erase(l_InstalledSporeMods.begin() + installedSporeModIndex, l_InstalledSporeMods.end());
            break;
        }
    }

    if (!save_installedsporemodlist())
    {
        close_zipfiles();
        return false;
    }

    close_zipfiles();
    return returnValue;
}

bool SporeModManager::UpdateMods(std::vector<std::filesystem::path> paths, bool requiresInstalled)
{
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    int                      installedSporeModId = 0;
    std::vector<int>         installedSporeModIds;
    std::vector<std::string> uniqueNames;
    std::string              extension;
    SporeMod::Xml::SporeModInfo sporeModInfo;

    if (!get_installedsporemodlist())
    {
        return false;
    }

    // reserve list items
    reserve_list_items(paths.size());

    // do validation before attempting
    // to update the given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths[i];

        if (!std::filesystem::is_regular_file(path))
        {
            std::cerr << "Error: " << path << " is not a regular file or doesn't exist!" << std::endl;
            return false;
        }

        sporeModInfo = {};
        extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod" || extension == ".package")
        {
            if (!get_sporemodinfo(path, extension, sporeModInfo))
            {
                close_zipfiles();
                return false;
            }
        }
        else
        {
            std::cerr << "Error: \"" << extension << "\" is an invalid extension!" << std::endl;
            return false;
        }

        // ensure we only have unique mod names
        if (std::find(uniqueNames.begin(), uniqueNames.end(), sporeModInfo.UniqueName) != uniqueNames.end())
        {
            std::cout << "Skipping " << path << " as it's already being installed!" << std::endl;
            paths.erase(paths.begin() + i);
            l_ZipFiles.erase(l_ZipFiles.begin() + i);
            l_SporeModInfos.erase(l_SporeModInfos.begin() + i);
            i -= 1;
            continue;
        }
        uniqueNames.push_back(sporeModInfo.UniqueName);

        if (!SporeMod::FindInstalledMod(sporeModInfo.UniqueName, installedSporeModId, l_InstalledSporeMods))
        {
            if (requiresInstalled)
            {
                close_zipfiles();
                std::cerr << "Error: no mod found with the same unique name (" << sporeModInfo.UniqueName << ")" << std::endl;
                return false;
            }
        }
        else
        {
            installedSporeModIds.push_back(installedSporeModId);
        }
    }

    // configure mods before attempting
    // to updating the given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths[i];

        installedSporeMod = {};
        extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod")
        {
            if (!SporeMod::ConfigureSporeMod(l_ZipFiles[i], l_SporeModInfos[i], installedSporeMod, l_InstalledSporeMods))
            {
                close_zipfiles();
                return false;
            }
        }
        else if (extension == ".package")
        {
            if (!SporeMod::ConfigurePackage(path, installedSporeMod, l_InstalledSporeMods))
            {
                close_zipfiles();
                return false;
            }
        }
        l_InstalledSporeMods.push_back(installedSporeMod);
    }

    // disable saving the installed mod list when uninstalling mods
    l_SaveInstalledSporeMods = false;

    // apply changes
    if (!installedSporeModIds.empty() && !UninstallMods(installedSporeModIds))
    {
        close_zipfiles();
        return false;
    }

    // save installed mod list when installing mods
    l_SaveInstalledSporeMods = true;

    return InstallMods(paths, true, false, true);
}

bool SporeModManager::UninstallMods(std::vector<int> ids)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> removedSporeMods;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    std::filesystem::path fullInstallPath;
    std::error_code error;

    if (!get_installedsporemodlist())
    {
        return false;
    }

    for (const auto& id : ids)
    {
        if (id < 0 || (size_t)id >= l_InstalledSporeMods.size() || l_InstalledSporeMods.empty())
        {
            std::cerr << "Error: ID(s) must be valid!" << std::endl;
            return false;
        }
    }

    for (const auto& id : ids)
    {
        installedSporeMod = l_InstalledSporeMods[id];

        std::cout << "-> Removing " << installedSporeMod.Name << std::endl;

        for (const auto& installedFile : installedSporeMod.InstalledFiles)
        {
            fullInstallPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);
            if (UI::GetVerboseMode())
            {
                std::cout << "--> Removing " << fullInstallPath << std::endl;
            }
            std::filesystem::remove(fullInstallPath, error);
            if (error)
            {
                std::cerr << "Error: failed to remove " << fullInstallPath << ": " << error.message() << std::endl;
                return false;
            }
        }

        removedSporeMods.push_back(installedSporeMod);
    }

    for (const auto& removedSporeMod : removedSporeMods)
    {
        auto installedSporeModIter = std::find(l_InstalledSporeMods.begin(), l_InstalledSporeMods.end(), removedSporeMod);
        if (installedSporeModIter != l_InstalledSporeMods.end())
        {
            l_InstalledSporeMods.erase(installedSporeModIter);
        }
    }

    if (!save_installedsporemodlist())
    {
        return false;
    }

    return true;
}

bool SporeModManager::DownloadSporeModAPI(void)
{
    const std::string url = "https://github.com/emd4600/Spore-ModAPI/releases/latest/download/SporeModAPIdlls.zip";
    const std::filesystem::path updatePath = Path::Combine({ Path::GetCoreLibsPath(), "update.zip" });
    const std::filesystem::path coreLibPath = Path::Combine({ Path::GetCoreLibsPath(), "SporeModAPI.dll" });
    std::error_code error;
    Zip::ZipFile zipFile;
    std::vector<char> extractedFileBuffer;
    std::ofstream outputFileStream;
    FileVersion::FileVersionInfo currentVersionInfo;
    FileVersion::FileVersionInfo downloadedVersionInfo;

    if (!FileVersion::ParseFile(coreLibPath, currentVersionInfo))
    {
        std::cerr << "Warning: failed to retrieve version from " << coreLibPath << std::endl;
    }

    if (!Download::DownloadFile(url, updatePath))
    {
        return false;
    }

    if (!Zip::OpenFile(zipFile, updatePath))
    {
        return false;
    }

    std::cout << "-> Extracting SporeModAPI.dll" << std::endl;

    if (!Zip::ExtractFile(zipFile, "SporeModAPI.combined.dll", extractedFileBuffer))
    {
        Zip::CloseFile(zipFile);
        return false;
    }

    Zip::CloseFile(zipFile);

    if (!FileVersion::ParseBuffer(extractedFileBuffer, downloadedVersionInfo))
    {
        std::cerr << "Error: failed to retrieve version from downloaded SporeModAPI.dll" << std::endl;
        return false;
    }

    if (UI::GetVerboseMode())
    {
        std::cout << "--> Installed SporeModAPI.dll version: v" << currentVersionInfo.to_string() << std::endl
                  << "--> Downloaded SporeModAPI.dll version: v" << downloadedVersionInfo.to_string() << std::endl; 
    }

    if (downloadedVersionInfo > currentVersionInfo)
    {
        outputFileStream.open(coreLibPath, std::ios::trunc | std::ios::binary);
        if (!outputFileStream.is_open())
        {
            std::cerr << "Error: failed to open " << coreLibPath << std::endl;
            return false;
        }

        outputFileStream.write(extractedFileBuffer.data(), extractedFileBuffer.size());
        outputFileStream.flush();
        outputFileStream.close();

        std::cout << "-> Installed SporeModAPI.dll v" << downloadedVersionInfo.to_string() << std::endl;
    }
    else
    {
        std::cout << "-> Installed SporeModAPI.dll is already up-to-date" << std::endl;
    }

    std::filesystem::remove(updatePath, error);
    if (error)
    {
        std::cerr << "Error: failed to remove " << updatePath << ": " << error.message() << std::endl;
        return false;
    }

    return true;
}
