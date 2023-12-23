/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <cstdio>
#include <algorithm>

#include "SporeModManager.hpp"
#include "SporeModManagerHelpers.hpp"

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

static bool GetInstalledSporeModList(void)
{
    if (!l_HasInstalledSporeMods)
    {
        if (!SporeMod::Xml::GetInstalledModList(l_InstalledSporeMods))
        {
            fprintf(stderr, "SporeMod::Xml::GetInstalledMostList() Failed!\n");
            return false;
        }
        l_HasInstalledSporeMods = true;
    }
    return true;
}

static bool SaveInstalledSporeModList(void)
{
    if (l_SaveInstalledSporeMods)
    {
        if (!SporeMod::Xml::SaveInstalledModList(l_InstalledSporeMods))
        {
            fprintf(stderr, "SporeMod::Xml::SaveInstalledModList() Failed!\n");
            return false;
        }
    }
    return true;
}

static bool GetSporeModInfo(const std::filesystem::path& path, const std::string& extension, SporeMod::Xml::SporeModInfo& sporeModInfo)
{
    Zip::ZipFile zipFile = nullptr;
    std::vector<char> modInfoFileBuffer;

    if (extension == ".sporemod")
    {
        if (!Zip::OpenFile(zipFile, path))
        {
            fprintf(stderr, "Zip::OpenFile() Failed!\n");
            return false;
        }

        if (!Zip::ExtractFile(zipFile, "modinfo.xml", modInfoFileBuffer))
        {
            fprintf(stderr, "Zip::ExtractFile() Failed!\n");
            Zip::CloseFile(zipFile);
            return false;
        }

        if (!SporeMod::Xml::ParseSporeModInfo(modInfoFileBuffer, sporeModInfo))
        {
            fprintf(stderr, "SporeMod::Xml::ParseSporeModInfo() Failed!\n");
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
    else if (extension == ".package")
    {
        sporeModInfo.UniqueName = path.stem().string();
    }

    l_SporeModInfos.push_back(sporeModInfo);
    l_ZipFiles.push_back(zipFile);
    return true;
}

static void CloseZipFiles(void)
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

    if (!GetInstalledSporeModList())
    {
        return false;
    }

    for (size_t i = 0; i < l_InstalledSporeMods.size(); i++)
    {
        installedSporeMod = l_InstalledSporeMods.at(i);

        fprintf(stdout, "[%u] %s\n",
                         (unsigned int)i, installedSporeMod.Name.c_str());
        if (!installedSporeMod.Description.empty())
        {
            fprintf(stdout, "  %s\n", String::Replace(installedSporeMod.Description, "\n", "\n  ").c_str());
        }
    }

    return true;
}

bool SporeModManager::InstallMods(std::vector<std::filesystem::path> paths, bool skipValidation, bool skipInstalled)
{
    std::vector<std::string> uniqueNames;
    std::string              extension;
    bool                     returnValue = true;
    int                              installedSporeModId;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    SporeMod::Xml::SporeModInfo      sporeModInfo;

    if (!GetInstalledSporeModList())
    {
        return false;
    }

    // do some basic validation before attempting
    // to install the given mods
    if (!skipValidation)
    {
        for (size_t i = 0; i < paths.size(); i++)
        {
            const std::filesystem::path& path = paths.at(i);

            if (!std::filesystem::is_regular_file(path))
            {
                fprintf(stderr, "\"%s\" is not a regular file or doesn't exist!\n", path.string().c_str());
                CloseZipFiles();
                return false;
            }

            sporeModInfo = {};
            extension = String::Lowercase(path.extension().string());
            if (extension == ".sporemod" || extension == ".package")
            {
                if (!GetSporeModInfo(path, extension, sporeModInfo))
                {
                    CloseZipFiles();
                    return false;
                }
            }
            else
            {
                fprintf(stderr, "\"%s\" is an invalid extension!\n", extension.c_str());
                CloseZipFiles();
                return false;
            }

            // check if we need to skip the file due to
            // either already having a mod in the install list
            // with the same unique name or a mod with the same
            // unique name already being installed
            bool hasUniqueName = std::find(uniqueNames.begin(), uniqueNames.end(), sporeModInfo.UniqueName) != uniqueNames.end();
            bool skipInstall   = skipInstalled && SporeMod::FindInstalledMod(sporeModInfo.UniqueName, installedSporeModId, l_InstalledSporeMods);
            if (hasUniqueName || skipInstall)
            {
                if (hasUniqueName)
                {
                    fprintf(stderr, "Skipping \"%s\" due to another mod having the same unique name!\n", path.string().c_str());
                }
                else
                {
                    fprintf(stdout, "Skipping \"%s\" as it's already installed!\n", path.string().c_str());
                }
                paths.erase(paths.begin() + i);
                l_ZipFiles.erase(l_ZipFiles.begin() + i);
                l_SporeModInfos.erase(l_SporeModInfos.begin() + i);
                i -= 1;
                continue;
            }
            uniqueNames.push_back(sporeModInfo.UniqueName);
        }
    }

    // install given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths.at(i);

        installedSporeMod = {};
        extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod")
        {
            if (!SporeMod::InstallSporeMod(l_ZipFiles.at(i), l_SporeModInfos.at(i), installedSporeMod, l_InstalledSporeMods))
            {
                returnValue = false;
                break;
            }
        }
        else if (extension == ".package")
        {
            if (!SporeMod::InstallPackage(path, installedSporeMod, l_InstalledSporeMods))
            {
                returnValue = false;
                break;
            }
        }
        l_InstalledSporeMods.push_back(installedSporeMod);
    }

    if (!SaveInstalledSporeModList())
    {
        CloseZipFiles();
        return false;
    }

    CloseZipFiles();
    return returnValue;
}

bool SporeModManager::UpdateMods(std::vector<std::filesystem::path> paths, bool requiresInstalled)
{
    int                      installedSporeModId = 0;
    std::vector<int>         installedSporeModIds;
    std::vector<std::string> uniqueNames;
    std::string              extension;
    SporeMod::Xml::SporeModInfo sporeModInfo;

    if (!GetInstalledSporeModList())
    {
        return false;
    }

    // do validation before attempting
    // to update the given mods
    for (size_t i = 0; i < paths.size(); i++)
    {
        const std::filesystem::path& path = paths.at(i);

        if (!std::filesystem::is_regular_file(path))
        {
            fprintf(stderr, "\"%s\" is not a regular file or doesn't exist!\n", path.string().c_str());
            return false;
        }

        sporeModInfo = {};
        extension = String::Lowercase(path.extension().string());
        if (extension == ".sporemod" || extension == ".package")
        {
            if (!GetSporeModInfo(path, extension, sporeModInfo))
            {
                CloseZipFiles();
                return false;
            }
        }
        else
        {
            fprintf(stderr, "\"%s\" is an invalid extension!\n", extension.c_str());
            return false;
        }

        // ensure we only have unique mod names
        if (std::find(uniqueNames.begin(), uniqueNames.end(), sporeModInfo.UniqueName) != uniqueNames.end())
        {
            fprintf(stderr, "Skipping \"%s\" due to another mod having the same unique name!\n", path.string().c_str());
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
                CloseZipFiles();
                fprintf(stderr, "No mod found with the same unique name (%s)\n"
                                "Did you mean install?\n",
                                sporeModInfo.UniqueName.c_str());
                return false;
            }
        }
        else
        {
            installedSporeModIds.push_back(installedSporeModId);
        }
    }

    // disable saving the installed mod list when uninstalling mods
    l_SaveInstalledSporeMods = false;

    // apply changes
    if (!installedSporeModIds.empty() && !UninstallMods(installedSporeModIds))
    {
        CloseZipFiles();
        return false;
    }

    // save installed mod list when installing mods
    l_SaveInstalledSporeMods = true;

    return InstallMods(paths, true);
}

bool SporeModManager::UninstallMods(std::vector<int> ids)
{
    std::vector<SporeMod::Xml::InstalledSporeMod> removedSporeMods;
    SporeMod::Xml::InstalledSporeMod installedSporeMod;
    std::filesystem::path fullInstallPath;

    if (!GetInstalledSporeModList())
    {
        return false;
    }

    for (const auto& id : ids)
    {
        if (id < 0 || (size_t)id >= l_InstalledSporeMods.size() || l_InstalledSporeMods.empty())
        {
            fprintf(stderr, "ID(s) must be valid!\n");
            return false;
        }
    }

    for (const auto& id : ids)
    {
        installedSporeMod = l_InstalledSporeMods.at(id);

        fprintf(stdout, "-> Removing %s\n", installedSporeMod.Name.c_str());

        for (const auto& installedFile : installedSporeMod.InstalledFiles)
        {
            try
            {
                fullInstallPath = Path::GetFullInstallPath(installedFile.InstallLocation, installedFile.FileName);

                if (UI::GetVerboseMode())
                {
                    fprintf(stdout, "--> Removing %s\n", fullInstallPath.string().c_str());
                }

                std::filesystem::remove(fullInstallPath);
            }
            catch (...)
            {
                fprintf(stderr, "std::filesystem::remove(\"%s\") Failed!\n", fullInstallPath.string().c_str());
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

    if (!SaveInstalledSporeModList())
    {
        return false;
    }

    return true;
}
