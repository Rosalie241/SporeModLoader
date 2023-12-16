/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <windows.h>

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

std::vector<std::filesystem::path> Path::GetCoreLibsPaths(void)
{
    std::vector<std::filesystem::path> coreLibsPaths;
    std::string legacyLibFile;
    std::filesystem::path legacyLibPath;
    std::filesystem::path coreLibPath;
    std::filesystem::path coreLibsPath;

    coreLibsPath = GetModLoaderPath();
    coreLibsPath += "\\CoreLibs";

    switch (Game::GetCurrentVersion())
    {
    case Game::GameVersion::Unknown:
        UI::ShowErrorMessage(L"Unknown Spore version!");
        throw std::exception();

    case Game::GameVersion::Origin_1_5_1:
    case Game::GameVersion::Origin_March2017:
        UI::ShowErrorMessage(L"Unsupported Spore version!");
        throw std::exception();

    case Game::GameVersion::GogOrSteam_1_5_1:
        UI::ShowErrorMessage(L"Update Spore to the latest version!");
        throw std::exception();

    case Game::GameVersion::GogOrSteam_March2017:
        coreLibsPath += "\\march2017\\";
        legacyLibFile = "SporeModAPI-steam_patched.dll";
        break;

    case Game::GameVersion::Disk_1_5_1:
        coreLibsPath += "\\disk\\";
        legacyLibFile = "SporeModAPI-disk.dll";
        break;
    }

    coreLibPath = coreLibsPath;
    coreLibPath += "SporeModAPI.dll";

    legacyLibPath = coreLibsPath;
    legacyLibPath += legacyLibFile;

    coreLibsPaths.push_back(coreLibPath);
    coreLibsPaths.push_back(legacyLibPath);

    return coreLibsPaths;
}

std::vector<std::filesystem::path> Path::GetModLibsPaths(void)
{
    std::vector<std::filesystem::path> modLibsPaths;
    std::filesystem::path modLibsPath;
    std::vector<std::wstring> excludePostfixes;

    modLibsPath = GetModLoaderPath();
    modLibsPath += "\\ModLibs";

    // we have to create an exclusion list
    // for the postfixes for the mod dlls
    // due to support for the legacy ModAPI DLLs
    // where games shipped a dll with either the
    // -steam, -steam_patched or -disk postfix
    if (Game::GetCurrentVersion() == Game::GameVersion::GogOrSteam_March2017)
    {
        excludePostfixes.push_back(L"-steam.dll");
        excludePostfixes.push_back(L"-disk.dll");
    }
    else
    {
        excludePostfixes.push_back(L"-steam.dll");
        excludePostfixes.push_back(L"-steam_patched.dll");
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(modLibsPath))
    {
        // skip non-files & non-dlls
        if (!entry.is_regular_file() ||
            !entry.path().has_extension() ||
            entry.path().extension() != ".dll")
        {
            continue;
        }

        // ensure we have an allowed postfix
        bool skipLib = false;
        std::wstring filename = entry.path().filename().wstring();
        for (const auto& postfix : excludePostfixes)
        {
            // TODO: use ends_with when we can use C++20
            if (filename.find(postfix) != std::wstring::npos)
            {
                skipLib = true;
                break;
            }
        }


        if (!skipLib)
        {
            modLibsPaths.push_back(entry.path());
        }
    }

    return modLibsPaths;
}

void Logger::Clear(void)
{
    std::filesystem::path logFilePath;

    logFilePath = Path::GetLogFilePath();

    if (std::filesystem::exists(logFilePath))
    {
        try
        {
            std::filesystem::remove(logFilePath);
        }
        catch (...)
        {
            std::wstring errorMessage;
            errorMessage = L"std::filesystem::remove(\"";
            errorMessage += logFilePath.wstring();
            errorMessage += L"\") Failed!";
            UI::ShowErrorMessage(errorMessage);
            throw std::exception();
        }
    }
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

bool Library::LoadAll(std::vector<std::filesystem::path> paths)
{
    for (const auto& path : paths)
    {
        // attempt to load library
        bool ret = LoadLibraryW(path.wstring().c_str()) != nullptr;

        std::wstring logMessage;
        logMessage = L"LoadLibraryW(\"";
        logMessage += path.wstring();
        logMessage += L"\") == ";
        logMessage += std::to_wstring((ret ? 1 : GetLastError()));
        Logger::AddMessage(logMessage);

        if (!ret)
        {
            std::wstring errorMessage;
            errorMessage = L"LoadLibraryW(\"";
            errorMessage += path.wstring();
            errorMessage += L"\") Failed!";
            UI::ShowErrorMessage(errorMessage);
            return false;
        }
    }

    return true;
}

Game::GameVersion Game::GetCurrentVersion(void)
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
    case 24904192:
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
