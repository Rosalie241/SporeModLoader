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
#include <curl/curl.h>
#endif // _WIN32

using namespace SporeModManagerHelpers;

//
// Local Functions
//

#ifndef _WIN32
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
    // TODO: windows support
#else
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        std::cerr << "Error: failed to initialize cURL!" << std::endl;;
        return false;
    }

    std::ofstream fileStream(path, std::ios::binary);
    if (!fileStream.is_open())
    {
        curl_easy_cleanup(curl);
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
        std::cerr << "Error: failed to download file!" << std::endl;
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
#endif // _WIN32
}
