/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "Download.hpp"
#include "UI.hpp"

#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <urlmon.h>
#else
#include <dlfcn.h>
#endif // _WIN32

using namespace SporeModManagerHelpers;

//
// Local Functions
//

#ifndef _WIN32
// libcurl definitions
#define CURLOPT_URL             10002
#define CURLOPT_WRITEFUNCTION   20011
#define CURLOPT_WRITEDATA       10001
#define CURLOPT_REDIR_PROTOCOLS 182
#define CURLOPT_FOLLOWLOCATION  52
#define CURLPROTO_HTTPS         (1 << 1)
#define CURLE_OK                0

// libcurl function pointers
typedef void* (*ptr_curl_easy_init)(void);
typedef int   (*ptr_curl_easy_setopt)(void* curl, int option, ...);
typedef int   (*ptr_curl_easy_perform)(void* curl);
typedef void  (*ptr_curl_easy_cleanup)(void* curl);
static ptr_curl_easy_init    curl_easy_init    = nullptr;
static ptr_curl_easy_setopt  curl_easy_setopt  = nullptr;
static ptr_curl_easy_perform curl_easy_perform = nullptr;
static ptr_curl_easy_cleanup curl_easy_cleanup = nullptr;

static size_t curl_write_data(char *data, size_t size, size_t nmemb, void* stream)
{
    std::ofstream* fileStream = (std::ofstream*)(stream);
    std::streamoff position = fileStream->tellp();
    fileStream->write(data, size * nmemb);
    return fileStream->tellp() - position;
}
#endif


//
// Exported Functions
//

bool Download::DownloadFile(std::string url, std::filesystem::path path)
{
    std::cout << "-> Downloading " << path.filename() << std::endl;

    if (UI::GetVerboseMode())
    {
        std::cout << "--> Downloading " << url << std::endl;
    }

#ifdef _WIN32
    std::wstring wurl(url.begin(), url.end());

    if (URLDownloadToFileW(nullptr, wurl.c_str(), path.wstring().c_str(), 0, nullptr) != S_OK)
    {
        std::cerr << "Error: failed to initialize download!" << std::endl;
        return false;
    }

    return true;
#else
    void* libcurl = dlopen("libcurl.so.4", RTLD_LAZY);
    if (libcurl == nullptr)
    {
        std::cerr << "Error: failed to load libcurl: " << dlerror() << std::endl;
        return false;
    }

    curl_easy_init    = (ptr_curl_easy_init)dlsym(libcurl, "curl_easy_init");
    curl_easy_setopt  = (ptr_curl_easy_setopt)dlsym(libcurl, "curl_easy_setopt");
    curl_easy_perform = (ptr_curl_easy_perform)dlsym(libcurl, "curl_easy_perform");
    curl_easy_cleanup = (ptr_curl_easy_cleanup)dlsym(libcurl, "curl_easy_cleanup");
    if (curl_easy_init    == nullptr ||
        curl_easy_setopt  == nullptr ||
        curl_easy_perform == nullptr ||
        curl_easy_cleanup == nullptr)
    {
        dlclose(libcurl);
        std::cerr << "Error: failed to retrieve required symbols from libcurl!" << std::endl;
        return false;
    }

    void* curl = curl_easy_init();
    if (curl == nullptr)
    {
        std::cerr << "Error: failed to initialize cURL!" << std::endl;;
        return false;
    }

    std::ofstream fileStream(path, std::ios::binary);
    if (!fileStream.is_open())
    {
        curl_easy_cleanup(curl);
        dlclose(libcurl);
        std::cerr << "Error: failed to open " << path << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&fileStream);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        dlclose(libcurl);
        std::cerr << "Error: failed to download file!" << std::endl;
        return false;
    }

    curl_easy_cleanup(curl);
    dlclose(libcurl);
    return true;
#endif // _WIN32
}
