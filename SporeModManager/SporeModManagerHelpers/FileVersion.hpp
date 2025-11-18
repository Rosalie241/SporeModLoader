/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_FILEVERSION_HPP
#define SPOREMODMANAGERHELPERS_FILEVERSION_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace SporeModManagerHelpers
{
    namespace FileVersion
    {
        struct FileVersionInfo
        {
            int Major = 0;
            int Minor = 0;
            int Build = 0;
            int Revision = 0;

            bool operator>(const FileVersionInfo& other) const
            {
                return (Major > other.Major) ||
                    (Major == other.Major && Minor > other.Minor) ||
                    (Major == other.Major && Minor == other.Minor &&
                        Build > other.Build) ||
                    (Major == other.Major && Minor == other.Minor &&
                        Build == other.Build && Revision > other.Revision);
            }

            bool operator==(const FileVersionInfo& other) const
            {
                return Major == other.Major &&
                    Minor == other.Minor &&
                    Build == other.Build &&
                    Revision == other.Revision;
            }

            std::string to_string(void) const
            {
                std::string string;
                string = std::to_string(Major) + ".";
                string += std::to_string(Minor) + ".";
                string += std::to_string(Build) + ".";
                string += std::to_string(Revision);
                return string;
            }
        };

        /// <summary>
        ///     Retrieves file version info for the core lib
        /// </summary>
        bool GetCoreLibFileVersionInfo(FileVersionInfo& fileVersionInfo);

        /// <summary>
        ///     Returns whether the core lib matches the version required by the mod
        /// </summary>
        bool CheckIfCoreLibMatchesVersion(FileVersionInfo& modFileVersionInfo, std::string modName);

        /// <summary>
        ///     Parses FileVersionInfo from string
        /// </summary>
        bool ParseString(const std::string& string, FileVersionInfo& fileVersionInfo);

        /// <summary>
        ///     Parses FileVersionInfo from a file
        /// </summary>
        bool ParseFile(const std::filesystem::path& path, FileVersionInfo& fileVersionInfo);

        /// <summary>
        ///     Parses FileVersionInfo from a buffer
        /// </summary>
        bool ParseBuffer(const std::vector<char>& buffer, FileVersionInfo& fileVersionInfo);
    }
}

#endif // SPOREMODMANAGERHELPERS_FILEVERSION_HPP