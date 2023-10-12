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
// Local Functions
//

static voidpf zlib_filefunc_open(voidpf opaque, const void* filename, int mode)
{
    std::filesystem::path path = *(std::filesystem::path*)filename;

    // we need a static filestream
    static std::ifstream fileStream;

    // attempt to open file
    fileStream.open(path, std::ios::binary);
    if (!fileStream.is_open())
    {
        return nullptr;
    }

    return (voidpf)&fileStream;
}

static uLong zlib_filefunc_read(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    std::ifstream* fileStream = (std::ifstream*)stream;
    fileStream->read((char*)buf, size);
    return fileStream->fail() ? 0 : size;
}

static ZPOS64_T zlib_filefunc_tell(voidpf opaque, voidpf stream)
{
    std::ifstream* fileStream = (std::ifstream*)stream;
    return fileStream->tellg();
}

static long zlib_filefunc_seek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
    std::ifstream* fileStream = (std::ifstream*)stream;
    std::ios_base::seekdir seekOrigin;

    switch (origin)
    {
    default:
        return 0;
    case ZLIB_FILEFUNC_SEEK_CUR:
        seekOrigin = std::ios::cur;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        seekOrigin = std::ios::end;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        seekOrigin = std::ios::beg;
        break;
    }

    fileStream->seekg(offset, seekOrigin);
    return fileStream->fail() ? -1 : 0;
}

static int zlib_filefunc_close(voidpf opaque, voidpf stream)
{
    std::ifstream* fileStream = (std::ifstream*)stream;
    fileStream->close();
    return fileStream->fail() ? -1 : 0;
}

static int zlib_filefunc_testerror(voidpf opaque, voidpf stream)
{
    return errno;
}

//
// Exported Functions
//

bool Zip::OpenFile(ZipFile& zipFile, std::filesystem::path path)
{
    zlib_filefunc64_def filefuncs;
    filefuncs.zopen64_file = zlib_filefunc_open;
    filefuncs.zread_file   = zlib_filefunc_read;
    filefuncs.zwrite_file  = nullptr;
    filefuncs.ztell64_file = zlib_filefunc_tell;
    filefuncs.zseek64_file = zlib_filefunc_seek;
    filefuncs.zclose_file  = zlib_filefunc_close;
    filefuncs.zerror_file  = zlib_filefunc_testerror;
    filefuncs.opaque       = nullptr;

    zipFile = unzOpen2_64((const void*)&path, &filefuncs);
    return zipFile != nullptr;
}

bool Zip::CloseFile(ZipFile zipFile)
{
    return unzClose(zipFile) == UNZ_OK;
}

bool Zip::GetFileList(ZipFile zipFile, std::vector<std::filesystem::path>& fileList)
{
    unz_global_info64  zipInfo;

    if (unzGetGlobalInfo64(zipFile, &zipInfo) != UNZ_OK)
    {
        std::cerr << "unzGetGlobalInfo() Failed!" << std::endl;
        return false;
    }

    for (uint64_t i = 0; i < zipInfo.number_entry; i++)
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
        if ((uint64_t)(i + 1) >= zipInfo.number_entry)
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

