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
#include <cstring>

#include <unzip.h>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#define UNZIP_READ_SIZE 67108860 /* 64 MiB */

//
// Exported Functions
//

bool Zip::OpenFile(ZipFile& zipFile, std::filesystem::path path)
{
    zipFile = unzOpen(path.string().c_str());
    return zipFile != nullptr;
}

bool Zip::CloseFile(ZipFile zipFile)
{
    return unzClose(zipFile) == UNZ_OK;
}

bool Zip::GetFileList(ZipFile zipFile, std::vector<std::filesystem::path>& fileList)
{
    unz_global_info zipInfo;

    if (unzGetGlobalInfo(zipFile, &zipInfo) != UNZ_OK)
    {
        std::cerr << "unzGetGlobalInfo() Failed!" << std::endl;
        return false;
    }

    for (uint32_t i = 0; i < zipInfo.number_entry; i++)
    {
        unz_file_info fileInfo;
        char          fileName[2048];

        // skip the file when we can't retrieve file info
        if (unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, 2048, nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            continue;
        }

        fileList.push_back(fileName);

        // break when we've iterated over all entries
        if ((uint32_t)(i + 1) >= zipInfo.number_entry)
        {
            break;
        }

        // move to next file
        if (unzGoToNextFile(zipFile) != UNZ_OK)
        {
            unzClose(zipFile);
            std::cerr << "unzGoToNextFile() Failed!" << std::endl;
            return false;
        }
    }

    return true;
}

bool Zip::ExtractFile(ZipFile zipFile, std::filesystem::path file, std::filesystem::path outputFile)
{
    char*  buffer;
    size_t dataSize = UNZIP_READ_SIZE;
    size_t bytesRead = 0;
    std::ofstream outputFileStream;

    // try to find file in zip
    if (unzLocateFile(zipFile, file.string().c_str(), 2) != UNZ_OK)
    {
        std::cerr << "Failed to find \"" << file << "\" in zip file!" << std::endl;
        return false;
    }

    buffer = (char*)malloc(UNZIP_READ_SIZE);
    if (buffer == nullptr)
    {
        std::cerr << "malloc() Failed!" << std::endl;
        return false;
    }

    outputFileStream.open(outputFile, std::ofstream::trunc | std::ofstream::binary);
    if (!outputFileStream.is_open())
    {
        free(buffer);
        std::cerr << "ofstream.open() Failed!" << std::endl;
        return false;
    }

    if (unzOpenCurrentFile(zipFile) != UNZ_OK)
    {
        free(buffer);
        std::cerr << "unzOpenCurrentFile() Failed!" << std::endl;
        return false;
    }

    do
    {
        bytesRead = unzReadCurrentFile(zipFile, buffer, UNZIP_READ_SIZE);
        if (bytesRead < 0)
        {
            unzCloseCurrentFile(zipFile);
            free(buffer);
            std::cerr << "unzReadCurrentFile() Failed: " << std::to_string(bytesRead) << std::endl;
            return false;
        }

        outputFileStream.write(buffer, bytesRead);
    } while (bytesRead > 0);

    unzCloseCurrentFile(zipFile);
    outputFileStream.flush();
    outputFileStream.close();
    free(buffer);
    return true;
}

bool Zip::ExtractFile(ZipFile zipFile, std::filesystem::path file, std::vector<char>& outBuffer)
{
    std::vector<char> buffer(UNZIP_READ_SIZE);
    size_t dataSize = UNZIP_READ_SIZE;
    size_t totalBytesRead = 0;
    size_t bytesRead = 0;

    // try to find file in zip
    if (unzLocateFile(zipFile, file.string().c_str(), 2) != UNZ_OK)
    {
        std::cerr << "Failed to find \"" << file << "\" in zip file!" << std::endl;
        return false;
    }

    outBuffer.reserve(UNZIP_READ_SIZE);

    if (unzOpenCurrentFile(zipFile) != UNZ_OK)
    {
        std::cerr << "unzOpenCurrentFile() Failed!" << std::endl;
        return false;
    }

    do
    {
        bytesRead = unzReadCurrentFile(zipFile, buffer.data(), UNZIP_READ_SIZE);
        if (bytesRead < 0)
        {
            unzCloseCurrentFile(zipFile);
            std::cerr << "unzReadCurrentFile() Failed: " << std::to_string(bytesRead) << std::endl;
            return false;
        }
        else if (bytesRead > 0)
        {
            outBuffer.insert(outBuffer.end(), buffer.begin(), std::next(buffer.begin(), bytesRead));
        }
    } while (bytesRead > 0);

    unzCloseCurrentFile(zipFile);
    return true;


}

