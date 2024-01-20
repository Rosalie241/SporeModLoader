/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "String.hpp"

#include <algorithm>
#include <sstream>
#include <cerrno>

using namespace SporeModManagerHelpers;

std::vector<std::string> String::Split(const std::string& string, char delimiter)
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

std::vector<std::wstring> String::Split(const std::wstring& string, wchar_t delimiter)
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

std::string String::Join(const std::vector<int>& numbers, char delimiter)
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

std::string String::Replace(const std::string& string, const std::string& findString, const std::string& replacementString)
{
    std::string str = string;
    size_t pos = 0;
    while ((pos = str.find(findString, pos)) != std::string::npos)
    {
        str.replace(pos, findString.size(), replacementString);
        pos += replacementString.size();
    }
    return str;
}

std::string String::Lowercase(const std::string& string)
{
    std::string str = string;
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) 
    { 
        return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; 
    });
    return str;
}

bool String::ToInt(const std::string& string, int& number)
{
    char* endptr;
    number = strtol(string.c_str(), &endptr, 10);
    return errno != ERANGE && endptr == (string.c_str() + string.size());
}

bool String::ToInt(const std::wstring& string, int& number)
{
    wchar_t* endptr;
    number = wcstol(string.c_str(), &endptr, 10);
    return errno != ERANGE && endptr == (string.c_str() + string.size());
}
