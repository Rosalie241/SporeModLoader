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

#include <sstream>
#include <algorithm>

using namespace SporeModManagerHelpers;

std::vector<std::string> String::Split(std::string string, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream stringStream(string);
    std::string item;

    while (std::getline(stringStream, item, delimiter))
    {
        result.push_back(item);
    }

    return result;
}

std::vector<std::wstring> String::Split(std::wstring string, wchar_t delimiter)
{
    std::vector<std::wstring> result;
    std::wstringstream stringStream(string);
    std::wstring item;

    while (std::getline(stringStream, item, delimiter))
    {
        result.push_back(item);
    }

    return result;
}

std::string String::Join(std::vector<int> numbers, char delimiter)
{
    std::string result;

    for (size_t i = 0; i < numbers.size(); i++)
    {
        result += std::to_string(numbers[i]);
        if (i < (numbers.size() - 1))
        {
            result += delimiter;
        }
    }

    return result;
}

std::string String::Replace(std::string string, std::string findString, std::string replacementString)
{
    size_t pos = 0;
    while ((pos = string.find(findString, pos)) != std::string::npos)
    {
        string.replace(pos, findString.size(), replacementString);
        pos += replacementString.size();
    }
    return string;
}

std::string String::Lowercase(std::string string)
{
    std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) 
    { 
        return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; 
    });
    return string;
}

