/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_SPOREMOD_HPP
#define SPOREMODMANAGERHELPERS_SPOREMOD_HPP

#include <string>
#include <vector>
#include <filesystem>

#include "SporeModXml.hpp"
#include "Zip.hpp"

namespace SporeModManagerHelpers
{
    namespace SporeMod
    {
        /// <summary>
        ///     Tries to find installed mod with uniqueName
        /// </summary>
        bool FindInstalledMod(std::string uniqueName, int& installedSporeModId, const std::vector<Xml::InstalledSporeMod>& installedSporeMods);

        /// <summary>
        ///     Configures sporemod file
        /// </summary>
        bool ConfigureSporeMod(Zip::ZipFile zipFile, const Xml::SporeModInfo& sporeModInfo, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod>& installedSporeMods);

        /// <summary>
        ///     Configures package file
        /// </summary>
        bool ConfigurePackage(const std::filesystem::path& path, Xml::InstalledSporeMod& installedSporeMod, const std::vector<Xml::InstalledSporeMod>& installedSporeMods);

        /// <summary>
        ///     Installs sporemod file
        /// </summary>
        bool InstallSporeMod(Zip::ZipFile zipFile, const Xml::InstalledSporeMod& installedSporeMod);

        /// <summary>
        ///    Installs package file
        /// </summary>
        bool InstallPackage(const std::filesystem::path& path, const Xml::InstalledSporeMod& installedSporeMod);
    }
}

#endif // SPOREMODMANAGERHELPERS_SPOREMOD_HPP
