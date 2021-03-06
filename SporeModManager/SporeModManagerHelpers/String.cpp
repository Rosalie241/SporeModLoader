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

#include <iostream>
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

std::string String::Lowercase(std::string string)
{
    std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) 
    { 
        return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; 
    });
    return string;
}

