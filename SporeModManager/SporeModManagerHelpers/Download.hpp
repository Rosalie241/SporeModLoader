/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_DOWNLOAD_HPP
#define SPOREMODMANAGERHELPERS_DOWNLOAD_HPP

#include <string>
#include <filesystem>

namespace SporeModManagerHelpers
{
    namespace Download
    {
        /// <summary>
        ///     Downloads url to path
        /// </summary>
        bool DownloadFile(std::string url, std::filesystem::path path);
    }
}

#endif // SPOREMODMANAGERHELPERS_DOWNLOAD_HPP