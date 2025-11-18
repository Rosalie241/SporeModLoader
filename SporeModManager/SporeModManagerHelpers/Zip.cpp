/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "Zip.hpp"

#include <iostream>
#include <fstream>
#include <map>

#include <unzip.h>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#define UNZIP_READ_SIZE 67108860 /* 64 MiB */

//
// Local Variables
//

static std::map<std::filesystem::path, std::ifstream> l_ZipFileStreams;
static std::vector<char>                              l_ZipFileBuffer;

//
// Local Functions
//

static voidpf zlib_filefunc_open(voidpf /*opaque*/, const void* filename, int /*mode*/)
{
    const std::filesystem::path path = *static_cast<const std::filesystem::path*>(filename);

    // if the stream is cached, ensure it's closed
    auto iter = l_ZipFileStreams.find(path);
    if (iter != l_ZipFileStreams.end())
    {
        if (iter->second.is_open())
        {
            iter->second.close();
        }
    }

    // attempt to open file
    l_ZipFileStreams[path] = std::ifstream(path, std::ios::binary | std::ios::in);
    if (!l_ZipFileStreams[path].is_open())
    {
        return nullptr;
    }

    return (voidpf)&l_ZipFileStreams[path];
}

static uLong zlib_filefunc_read(voidpf /*opaque*/, voidpf stream, void* buf, uLong size)
{
    std::ifstream* fileStream = static_cast<std::ifstream*>(stream);
    fileStream->read(static_cast<char*>(buf), size);
    return fileStream->fail() ? (uLong)fileStream->gcount() : size;
}

static ZPOS64_T zlib_filefunc_tell(voidpf /*opaque*/, voidpf stream)
{
    std::ifstream* fileStream = static_cast<std::ifstream*>(stream);
    return fileStream->tellg();
}

static long zlib_filefunc_seek(voidpf /*opaque*/, voidpf stream, ZPOS64_T offset, int origin)
{
    std::ifstream* fileStream = static_cast<std::ifstream*>(stream);
    std::ios_base::seekdir seekOrigin;

    switch (origin)
    {
    default:
        return -1;
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

static int zlib_filefunc_close(voidpf /*opaque*/, voidpf stream)
{
    std::ifstream* fileStream = static_cast<std::ifstream*>(stream);
    fileStream->close();
    return fileStream->fail() ? -1 : 0;
}

static int zlib_filefunc_testerror(voidpf /*opaque*/, voidpf /*stream*/)
{
    return errno;
}

//
// Exported Functions
//

bool Zip::OpenFile(ZipFile& zipFile, const std::filesystem::path& path)
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

    zipFile = unzOpen2_64(&path, &filefuncs);
    if (zipFile == nullptr)
    {
        std::cerr << "Error: failed to open zip file: " << path << std::endl; 
    }
    return zipFile != nullptr;
}

bool Zip::CloseFile(ZipFile zipFile)
{
    return unzClose(zipFile) == UNZ_OK;
}

bool Zip::LocateFile(ZipFile zipFile, const std::filesystem::path& file)
{
    return unzLocateFile(zipFile, file.string().c_str(), 2) == UNZ_OK;
}

bool Zip::GetFileList(ZipFile zipFile, std::vector<std::filesystem::path>& fileList)
{
    unz_global_info64 zipInfo;
    unz_file_info     zipFileInfo;
    char              zipFileName[2048];
    int ret = 0;

    ret = unzGetGlobalInfo64(zipFile, &zipInfo);
    if (ret != UNZ_OK)
    {
        std::cerr << "Error: failed to retrieve zip file info: " << ret << std::endl;
        return false;
    }

    for (ZPOS64_T i = 0; i < zipInfo.number_entry; i++)
    {
        // skip invalid files from the file list
        if (unzGetCurrentFileInfo(zipFile, &zipFileInfo, zipFileName, 2048, nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            continue;
        }

        const std::filesystem::path path(zipFileName);

        // add filename to file list
        if (path.has_filename())
        {
            fileList.push_back(path);
        }

        // break when we've iterated over all entries
        if ((i + 1) >= zipInfo.number_entry)
        {
            break;
        }

        // move to next file
        ret = unzGoToNextFile(zipFile);
        if (ret != UNZ_OK)
        {
            std::cerr << "Error: failed to advance to next file in zip file: " << ret << std::endl;
            return false;
        }
    }

    return true;
}

bool Zip::ExtractFile(ZipFile zipFile, const std::filesystem::path& file, const std::filesystem::path& outputFile)
{
    int bytesRead = 0;
    std::ofstream outputFileStream;

    // ensure the global buffer has the size reserved
    l_ZipFileBuffer.reserve(UNZIP_READ_SIZE);

    // try to find file in zip
    if (unzLocateFile(zipFile, file.string().c_str(), 2) != UNZ_OK)
    {
        std::cerr << "Error: failed to find " << file << " in zip file!" << std::endl;
        return false;
    }

    outputFileStream.open(outputFile, std::ios::trunc | std::ios::binary);
    if (!outputFileStream.is_open())
    {
        std::cerr << "Error: failed to open " << outputFile << std::endl;
        return false;
    }

    bytesRead = unzOpenCurrentFile(zipFile);
    if (bytesRead != UNZ_OK)
    {
        std::cerr << "Error: failed to open file in zip file: " << bytesRead << std::endl;
        return false;
    }

    do
    {
        bytesRead = unzReadCurrentFile(zipFile, l_ZipFileBuffer.data(), UNZIP_READ_SIZE);
        if (bytesRead < 0)
        {
            unzCloseCurrentFile(zipFile);
            std::cerr << "Error: failed to read data from file in zip file: " << bytesRead << std::endl;
            return false;
        }
        else if (bytesRead > 0)
        {
            outputFileStream.write(l_ZipFileBuffer.data(), bytesRead);
        }
    } while (bytesRead > 0);

    unzCloseCurrentFile(zipFile);
    outputFileStream.flush();
    outputFileStream.close();
    return true;
}

bool Zip::ExtractFile(ZipFile zipFile, const std::filesystem::path& file, std::vector<char>& outBuffer)
{
    int bytesRead = 0;

    // ensure the global buffer has the size reserved
    l_ZipFileBuffer.reserve(UNZIP_READ_SIZE);

    // try to find file in zip
    if (!file.empty() && unzLocateFile(zipFile, file.string().c_str(), 2) != UNZ_OK)
    {
        std::cerr << "Error: failed to find " << file << " in zip file!" << std::endl;
        return false;
    }

    outBuffer.reserve(UNZIP_READ_SIZE);

    bytesRead = unzOpenCurrentFile(zipFile);
    if (bytesRead != UNZ_OK)
    {
        std::cerr << "Error: failed to open file in zip file: " << bytesRead << std::endl;
        return false;
    }

    do
    {
        bytesRead = unzReadCurrentFile(zipFile, l_ZipFileBuffer.data(), UNZIP_READ_SIZE);
        if (bytesRead < 0)
        {
            unzCloseCurrentFile(zipFile);
            std::cerr << "Error: failed to read data from file in zip file: " << bytesRead << std::endl;
            return false;
        }
        else if (bytesRead > 0)
        {
            outBuffer.insert(outBuffer.end(), l_ZipFileBuffer.begin(), l_ZipFileBuffer.begin() + bytesRead);
        }
    } while (bytesRead > 0);

    unzCloseCurrentFile(zipFile);
    return true;
}

