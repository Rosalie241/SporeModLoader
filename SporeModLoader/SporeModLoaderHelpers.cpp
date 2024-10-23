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
#include <optional>

#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

//
// Local Variables
//

static std::wofstream l_LogFileStream;

//
// Exported Functions
//

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

std::vector<std::filesystem::path> Path::GetCoreLibsPaths(void)
{
    std::vector<std::filesystem::path> coreLibsPaths;
    std::string legacyLibFile;
    std::filesystem::path legacyLibPath;
    std::filesystem::path coreLibPath;
    std::filesystem::path coreLibsPath;

    coreLibsPath = GetModLoaderPath();
    coreLibsPath += "\\CoreLibs\\";

    switch (Game::GetCurrentVersion())
    {
    case Game::GameVersion::GogOrSteam_March2017:
        legacyLibFile = "SporeModAPI-steam_patched.dll";
        break;

    case Game::GameVersion::Disk_1_5_1:
        legacyLibFile = "SporeModAPI-disk.dll";
        break;

    default:
        UI::ShowErrorMessage(L"Unsupported Spore version!");
        throw std::exception();
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

    for (const auto& entry : std::filesystem::directory_iterator(modLibsPath))
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
            // TODO: use ends_with with mingw when we can use C++20
#ifdef _MSC_VER
            if (filename.ends_with(postfix))
#else // mingw
            if (filename.find(postfix) != std::wstring::npos)
#endif
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

void Logger::Open(void)
{
    try
    {
        std::filesystem::path logFilePath;
        logFilePath = Path::GetModLoaderPath();
        logFilePath += "\\SporeModLoader.log";

        // try to open the log file at the modloader path,
        // if that fails, try the temporary path
        l_LogFileStream.open(logFilePath);
        if (!l_LogFileStream.good())
        {
            logFilePath = std::filesystem::temp_directory_path();
            logFilePath += "\\SporeModLoader.log";
            l_LogFileStream.open(logFilePath);
            if (!l_LogFileStream.good())
            {
                throw std::exception();
            }
        }
    }
    catch (...)
    {
        std::wstring errorMessage;
        errorMessage = L"Failed to find a working log file location!";
        UI::ShowErrorMessage(errorMessage);
        throw std::exception();
    }
}

void Logger::AddMessage(std::wstring message)
{
    l_LogFileStream << message << std::endl;
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
    static std::optional<Game::GameVersion> cachedGameVersion;
    std::filesystem::path currentExecutablePath;
    uintmax_t currentFileSize;

    if (cachedGameVersion.has_value())
    {
        return cachedGameVersion.value();
    }

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
        cachedGameVersion = Game::GameVersion::Disk_1_5_1;
        break;

    case 24885248:
    case 24895536:
        cachedGameVersion = Game::GameVersion::GogOrSteam_March2017;
        break;

    default:
        cachedGameVersion = Game::GameVersion::Unknown;
        break;
    }

    return cachedGameVersion.value();
}
