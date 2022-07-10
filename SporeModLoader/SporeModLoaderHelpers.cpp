/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <Windows.h>

#include <fstream>

#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

std::filesystem::path Path::GetCurrentExecutablePath(void)
{
    static std::filesystem::path currentExecutablePath;
    wchar_t currentExecutablePathBuf[MAX_PATH];

    if (!currentExecutablePath.empty())
    {
        return currentExecutablePath;
    }

    if (GetModuleFileNameW(nullptr, currentExecutablePathBuf, MAX_PATH) == 0)
    {
        UI::ShowErrorMessage(L"GetModuleFileNameW() Failed!");
        throw std::exception();
    }

    currentExecutablePath = currentExecutablePathBuf;

    return currentExecutablePath;
}

std::filesystem::path Path::GetModLoaderPath(void)
{
    std::filesystem::path modLoaderPath;

    modLoaderPath = GetCurrentExecutablePath().parent_path().parent_path();
    modLoaderPath += "\\SporeModLoader";

    return modLoaderPath;
}

std::filesystem::path Path::GetLogFilePath(void)
{
    std::filesystem::path logFilePath;

    logFilePath = GetModLoaderPath();
    logFilePath += "\\SporeModLoader.log";

    return logFilePath;
}

std::filesystem::path Path::GetCoreLibsPath(void)
{
    std::filesystem::path coreLibsPath;

    coreLibsPath = GetModLoaderPath();
    coreLibsPath += "\\CoreLibs";

    switch (Game::GetCurrentVersion())
    {
    case Game::GameVersion::Unknown:
        UI::ShowErrorMessage(L"Unknown Spore version!");
        throw std::exception();

    case Game::GameVersion::Origin_1_5_1:
    case Game::GameVersion::GogOrSteam_1_5_1:
        UI::ShowErrorMessage(L"Update Spore to the latest version!");
        throw std::exception();

    case Game::GameVersion::Origin_March2017:
    case Game::GameVersion::GogOrSteam_March2017:
        coreLibsPath += "\\march2017";
        break;

    case Game::GameVersion::Disk_1_5_1:
        coreLibsPath += "\\disk";
        break;
    }

    return coreLibsPath;
}

std::filesystem::path Path::GetModLibsPath(void)
{
    std::filesystem::path modLibsPath;

    modLibsPath = GetModLoaderPath();
    modLibsPath += "\\ModLibs";

    return modLibsPath;
}


void Logger::AddMessage(std::wstring message)
{
    std::wofstream logFileStream;
    logFileStream.open(Path::GetLogFilePath(), std::ios_base::app);
    if (logFileStream.is_open())
    {
        logFileStream << message << "\n";
    }
    logFileStream.close();
}

void UI::ShowErrorMessage(std::wstring message)
{
    MessageBoxW(nullptr, message.c_str(), L"SporeModLoader", MB_OK | MB_ICONERROR);
}

bool Library::LoadAllInPath(std::filesystem::path path)
{
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        // skip non-files & non-dlls
        if (!entry.is_regular_file() ||
            !entry.path().has_extension() ||
            entry.path().extension() != ".dll")
        {
            continue;
        }

        // attempt to load library
        bool ret = LoadLibraryW(entry.path().wstring().c_str()) != nullptr;

        std::wstring logMessage;
        logMessage = L"LoadLibraryW(\"";
        logMessage += entry.path().wstring();
        logMessage += L"\") == ";
        logMessage += std::to_wstring((ret ? 1 : GetLastError()));
        Logger::AddMessage(logMessage);

        if (!ret)
        {
            std::wstring errorMessage;
            errorMessage = L"LoadLibraryW(\"";
            errorMessage += entry.path().wstring();
            errorMessage += L"\") Failed!";
            UI::ShowErrorMessage(errorMessage);
            return false;
        }
    }

    return true;
}

Game::GameVersion SporeModLoaderHelpers::Game::GetCurrentVersion(void)
{
    std::filesystem::path currentExecutablePath;
    uintmax_t currentFileSize;

    currentExecutablePath = Path::GetCurrentExecutablePath();

    try
    {
        currentFileSize = std::filesystem::file_size(currentExecutablePath);
    }
    catch (...)
    {
        std::wstring errorMessage;
        errorMessage = L"std::filesystem::file_size(\"";
        errorMessage += currentExecutablePath;
        errorMessage += L"\") Failed!";
        UI::ShowErrorMessage(errorMessage);
        throw std::exception();
    }

    switch (currentFileSize)
    {
    case 24909584:
        return Game::GameVersion::Disk_1_5_1;
    case 31347984:
        return Game::GameVersion::Origin_1_5_1;
    case 24898224:
        return Game::GameVersion::Origin_March2017;
    case 24888320:
        return Game::GameVersion::GogOrSteam_1_5_1;
    case 24885248:
        return Game::GameVersion::GogOrSteam_March2017;

    default:
        return Game::GameVersion::Unknown;
    }
}
