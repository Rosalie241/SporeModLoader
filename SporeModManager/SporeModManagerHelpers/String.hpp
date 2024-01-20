/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_STRING_HPP
#define SPOREMODMANAGERHELPERS_STRING_HPP

#include <vector>
#include <string>

namespace SporeModManagerHelpers
{
    namespace String
    {
        /// <summary>
        ///     Splits the given string by the delimiter
        /// </summary>
        std::vector<std::string>  Split(const std::string& string, char delimiter);
        std::vector<std::wstring> Split(const std::wstring& string, wchar_t delimiter);

        /// <summary>
        ///     Joins numbers to string with delimiter
        /// </summary>
        std::string Join(const std::vector<int>& numbers, char delimiter);

        /// <summary>
        ///     Replaces findString with replacementString in string
        /// </summary>
        std::string Replace(const std::string& string, const std::string& findString, const std::string& replacementString);

        /// <summary>
        ///     Returns the lowercase version of the given string
        /// </summary>
        std::string Lowercase(const std::string& string);

        /// <summary>
        ///     Returns whether converting the string to number succeeds
        /// </summary>
        bool ToInt(const std::string& string, int& number);
        bool ToInt(const std::wstring& string, int& number);
    }
}

#endif // SPOREMODMANAGERHELPERS_STRING_HPP