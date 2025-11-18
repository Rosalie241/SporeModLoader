/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_ZIP_HPP
#define SPOREMODMANAGERHELPERS_ZIP_HPP

#include <vector>
#include <filesystem>

namespace SporeModManagerHelpers
{
    namespace Zip
    {
        typedef void* ZipFile;

        /// <summary>
        ///     Opens the given zip file
        /// </summary>
        bool OpenFile(ZipFile& zipFile, const std::filesystem::path& path);

        /// <summary>
        ///     Closes the given zip
        /// </summary>
        bool CloseFile(ZipFile zipFile);

        /// <summary>
        ///     Retrieves file list of the given zip
        /// </summary>
        bool GetFileList(ZipFile zipFile, std::vector<std::filesystem::path>& fileList);

        /// <summary>
        ///     Locates file in given zip
        /// </summary>
        bool LocateFile(ZipFile zipFile, const std::filesystem::path& file);

        /// <summary>
        ///     Extracts file to outputFile
        /// </summary>
        bool ExtractFile(ZipFile zipFile, const std::filesystem::path& file, const std::filesystem::path& outputFile);

        /// <summary>
        ///     Extracts file to buffer
        /// </summary>
        bool ExtractFile(ZipFile zipFile, const std::filesystem::path& file, std::vector<char>& buffer);
    }
}

#endif // SPOREMODMANAGERHELPERS_ZIP_HPP