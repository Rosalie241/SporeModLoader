/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "FileVersion.hpp"
#include "String.hpp"
#include "Path.hpp"

#include <algorithm>
#include <optional>
#include <iostream>
#include <fstream>
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
    static std::optional<FileVersionInfo> cachedFileVersionInfo;

    if (cachedFileVersionInfo.has_value())
    {
        fileVersionInfo = cachedFileVersionInfo.value();
        return true;
    }

    coreLibPath = Path::Combine({ Path::GetCoreLibsPath(), "SporeModAPI.dll" });

    if (!std::filesystem::is_regular_file(coreLibPath))
    {
        std::cerr << "Error: " << coreLibPath << " doesn't exist!" << std::endl;
        return false;
    }

    if (!FileVersion::ParseFile(coreLibPath, fileVersionInfo))
    {
        std::cerr << "Error: failed to parse file version!" << std::endl;
        return false;
    }

    cachedFileVersionInfo = fileVersionInfo;
    return true;
}

bool FileVersion::CheckIfCoreLibMatchesVersion(FileVersionInfo& modFileVersionInfo, std::string modName)
{
    FileVersionInfo coreLibFileVersionInfo;

    if (!GetCoreLibFileVersionInfo(coreLibFileVersionInfo))
    {
        std::cerr << "Error: failed to retrieve SporeModAPI.dll version!" << std::endl;
        return false;
    }

    if (modFileVersionInfo > coreLibFileVersionInfo)
    {
        std::cerr << "Error: \"" << modName << "\" requires newer SporeModAPI.dll (\"" << modFileVersionInfo.to_string() <<
            "\") than what's currently installed (\"" << coreLibFileVersionInfo.to_string() << "\")" << std::endl;
        return false;
    }

    return true;
}

bool FileVersion::ParseString(const std::string& string, FileVersionInfo& fileVersionInfo)
{
    std::vector<std::string> splitString;

    splitString = String::Split(string, '.');

    if (splitString.size() > 4)
    {
        std::cerr << "Error: \"" << string << "\" is not a valid version string!" << std::endl;
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
        if (!String::ToInt(splitString[i], numbers[i].get()))
        {
            std::cerr << "Error: \"" << string << "\" is not a valid version string!" << std::endl;
            return false;
        }
    }

    return true;
}

bool FileVersion::ParseFile(const std::filesystem::path& path, FileVersionInfo& fileVersionInfo)
{
    std::ifstream  fileStream;
    std::streamoff fileStreamLength;

    fileStream.open(path, std::ios_base::in | std::ios_base::binary);
    if (!fileStream.is_open())
    {
        std::cerr << "Error: failed to open " << path << std::endl;
        return false;
    }

    // retrieve filestream length
    fileStream.seekg(0, std::ios_base::end);
    fileStreamLength = fileStream.tellg();
    fileStream.seekg(0, std::ios_base::beg);

    // make sure it doesn't go over the hard-limit
    if (fileStreamLength > MAX_FILE_READ_SIZE)
    {
        std::cerr << "Error: refusing to read file bigger than 64MiB!" << std::endl;
        return false;
    }

    std::vector<char> buffer(static_cast<size_t>(fileStreamLength));
    fileStream.read(buffer.data(), fileStreamLength);

    if (fileStream.fail())
    {
        std::cerr << "Error: failed to read data from " << path << std::endl;
        return false;
    }

    return ParseBuffer(buffer, fileVersionInfo);    
}

bool FileVersion::ParseBuffer(const std::vector<char>& buffer, FileVersionInfo& fileVersionInfo)
{
    std::string versionString;
    const std::array<char, 25> fileVersionBytes =
    { // F\0i\0l\0e\0V\0e\0r\0s\0i\0o\0n\0\0\0\0\0
        0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x56, 0x00, 
        0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x00, 0x00, 0x00
    };

    auto bufferIter = std::search(buffer.begin(), buffer.end(), fileVersionBytes.begin(), fileVersionBytes.end());
    if (bufferIter == buffer.end())
    {
        std::cerr << "Error: failed to find file version bytes!" << std::endl;
        return false;
    }

    // ensure we have enough bytes available
    const auto availableBytes = std::distance(bufferIter + fileVersionBytes.size(), buffer.end());
    if (availableBytes <= 1)
    {
        std::cerr << "Error: failed to find file version bytes!" << std::endl;
        return false;
    }

    // move to after the found bytes
    bufferIter = bufferIter + fileVersionBytes.size() + 1;

    // construct version string
    while (bufferIter != buffer.end())
    {
        char byte     = *bufferIter;
        char nextByte = (std::distance(bufferIter, buffer.end()) <= 1) ? 0 : *(bufferIter + 1);

        if (byte == '\0' && nextByte == '\0')
        {
            break;
        }

        if (byte != '\0')
        {
            versionString.push_back(byte);
        }

        bufferIter++;
    }

    return ParseString(versionString, fileVersionInfo);
}
