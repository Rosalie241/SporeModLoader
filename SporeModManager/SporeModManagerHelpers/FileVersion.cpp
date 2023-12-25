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
#include <fstream>
#include <algorithm>
#include <array>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#define MAX_FILE_READ_SIZE 67108860 /* 64 MiB */

//
// Exported Functions
//

bool FileVersion::GetCoreLibFileVersionInfo(FileVersionInfo& fileVersionInfo)
{
    std::filesystem::path coreLibPath;
    static FileVersionInfo cachedFileVersionInfo;
    static bool hasCachedFileVersionInfo = false;

    if (hasCachedFileVersionInfo)
    {
        fileVersionInfo = cachedFileVersionInfo;
        return true;
    }

    coreLibPath = Path::Combine({ Path::GetCoreLibsPath(), "SporeModAPI.dll" });

    if (!std::filesystem::is_regular_file(coreLibPath))
    {
        std::cerr << "\"" << coreLibPath.string() << "\" doesn't exist!" << std::endl;
        return false;
    }

    if (!FileVersion::ParseFile(coreLibPath, fileVersionInfo))
    {
        std::cerr << "FileVersion::ParseFile() Failed!" << std::endl;
        return false;
    }

    cachedFileVersionInfo    = fileVersionInfo;
    hasCachedFileVersionInfo = true;
    return true;
}

bool FileVersion::CheckIfCoreLibMatchesVersion(FileVersionInfo& modFileVersionInfo, std::string modName)
{
    FileVersionInfo coreLibFileVersionInfo;

    if (!GetCoreLibFileVersionInfo(coreLibFileVersionInfo))
    {
        std::cerr << "FileVersion::GetCoreLibFileVersionInfo() Failed!" << std::endl;
        return false;
    }

    if (modFileVersionInfo > coreLibFileVersionInfo)
    {
        std::cerr << "\"" << modName << "\" requires newer modapi dll (\"" << modFileVersionInfo.to_string() <<
            "\") than what's currently installed (\"" << coreLibFileVersionInfo.to_string() << "\")" << std::endl;
        return false;
    }

    return true;
}

bool FileVersion::ParseString(std::string string, FileVersionInfo& fileVersionInfo)
{
    std::vector<std::string> splitString;

    splitString = String::Split(string, '.');

    if (splitString.size() > 4)
    {
        std::cerr << "\"" << string << "\" is not a valid version string!" << std::endl;
        return false;
    }

    // reset values to 0
    fileVersionInfo = { 0, 0, 0, 0 };

    const std::array<std::reference_wrapper<int>, 4> numbers
    {
        fileVersionInfo.Major,
        fileVersionInfo.Minor,
        fileVersionInfo.Build,
        fileVersionInfo.Revision
    };

    for (size_t i = 0; i < std::min(splitString.size(), numbers.size()); i++)
    {
        try
        {
            numbers.at(i).get() = std::stoi(splitString.at(i));
        }
        catch (...)
        {
            std::cerr << "\"" << string << "\" is not a valid version string!" << std::endl;
            return false;
        }
    }

    return true;
}

bool FileVersion::ParseFile(std::filesystem::path path, FileVersionInfo& fileVersionInfo)
{
    std::ifstream  fileStream;
    std::string    versionString;
    std::streamoff fileStreamLength;

    fileStream.open(path, std::ios_base::in | std::ios_base::binary);
    if (!fileStream.is_open())
    {
        std::cerr << "fileStream.open() Failed!" << std::endl;
        return false;
    }

    // retrieve filestream length
    fileStream.seekg(0, std::ios_base::end);
    fileStreamLength = fileStream.tellg();
    fileStream.seekg(0, std::ios_base::beg);

    // make sure it doesn't go over the hard-limit
    if (fileStreamLength > MAX_FILE_READ_SIZE)
    {
        std::cerr << "refusing to read file bigger than 64MiB!" << std::endl;
        return false;
    }

    std::vector<char> buffer((size_t)fileStreamLength);
    fileStream.read(buffer.data(), fileStreamLength);

    if (fileStream.fail())
    {
        std::cerr << "fileStream.fail()!" << std::endl;
        return false;
    }

    std::vector<int> searchBytes =
    { // F\0i\0l\0e\0V\0e\0r\0s\0i\0o\0n\0\0\0\0\0
        0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x56, 0x00, 0x65,
        0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00,
        0x00, 0x00, 0x00
    };

    auto bufferIter = std::search(buffer.begin(), buffer.end(), searchBytes.begin(), searchBytes.end());
    if (bufferIter == buffer.end())
    {
        std::cout << "std::search() didn't find the required bytes!" << std::endl;
        return false;
    }

    // move to after the found bytes
    bufferIter = bufferIter + searchBytes.size() + 1;

    // construct version string
    while (bufferIter != buffer.end())
    {
        char byte     = *bufferIter;
        char nextByte = *(bufferIter + 1);

        if (byte == '\0' && nextByte == '\0')
        {
            break;
        }

        if (byte != '\0')
        {
            versionString += byte;
        }

        bufferIter++;
    }

    return ParseString(versionString, fileVersionInfo);
}
