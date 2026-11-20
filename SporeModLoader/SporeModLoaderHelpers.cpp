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

static std::ofstream l_LogFileStream;

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
        UI::ShowErrorMessage("GetModuleFileNameW() Failed!");
        std::abort();
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

bool Path::GetCoreLibsPaths(std::vector<std::filesystem::path>& paths)
{
    std::string legacyLibFile;
    std::filesystem::path legacyLibPath;
    std::filesystem::path coreLibPath;
    std::filesystem::path coreLibsPath;
    std::string message;

    coreLibsPath = GetModLoaderPath();
    coreLibsPath += "\\CoreLibs\\";

    switch (Game::GetCurrentVersion())
    {
    case Game::GameVersion::GogOrSteam_March2017:
        legacyLibFile = "SporeModAPI-steam_patched.dll";
        break;

    case Game::GameVersion::Disc_1_5_1:
        legacyLibFile = "SporeModAPI-disk.dll";
        break;

    default:
        UI::ShowErrorMessage("Unsupported Spore version!");
        return false;
    }

    coreLibPath = coreLibsPath;
    coreLibPath += "SporeModAPI.dll";

    legacyLibPath = coreLibsPath;
    legacyLibPath += legacyLibFile;

    paths.push_back(coreLibPath);
    paths.push_back(legacyLibPath);

    for (const auto& path : paths)
    {
        bool file_exists = std::filesystem::is_regular_file(path);

        message = "std::filesystem::is_regular_file(\"";
        message += path.string();
        message += "\") == ";
        message += std::to_string(file_exists);
        Logger::AddMessage(message);

        if (!file_exists)
        {
            message = "\"";
            message += path.string();
            message += "\" doesn't exist or is not a regular file!";
            UI::ShowErrorMessage(message);
            return false;
        }
    }

    return true;
}

bool Path::GetModLibsPaths(std::vector<std::filesystem::path>& paths)
{
    std::filesystem::path modLibsPath;
    std::vector<std::wstring> excludePostfixes;
    std::string message;
    std::error_code errorCode;

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

    bool directory_exists = std::filesystem::is_directory(modLibsPath, errorCode);

    message = "std::filesystem::is_directory(\"";
    message += modLibsPath.string();
    message += "\") == ";
    message += std::to_string(directory_exists);
    if (errorCode)
    {
        message += ": ";
        message += errorCode.message();
    }
    Logger::AddMessage(message);
    
    if (!directory_exists)
    {
        message = "\"";
        message += modLibsPath.string();
        message += "\" doesn't exist or is not a directory!";
        UI::ShowErrorMessage(message);
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(modLibsPath, errorCode))
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
            if (filename.ends_with(postfix))
            {
                skipLib = true;
                break;
            }
        }


        if (!skipLib)
        {
            paths.push_back(entry.path());
        }
    }

    return true;
}

bool Logger::Open(void)
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
            UI::ShowErrorMessage("Failed to find a working log file location!");
            return false;
        }
    }

    return true;
}

void Logger::AddMessage(const std::string& message)
{
    l_LogFileStream << message << std::endl;
}

void UI::ShowErrorMessage(const std::string& message)
{
    MessageBoxA(nullptr, message.c_str(), "SporeModLoader", MB_OK | MB_ICONERROR);
}

bool Library::LoadAll(const std::vector<std::filesystem::path>& paths, std::vector<HMODULE>& modules)
{
    std::string message;

    for (const auto& path : paths)
    {
        // attempt to load library
        HMODULE hModule = LoadLibraryW(path.wstring().c_str());
        bool ret = hModule != nullptr;

        message = "LoadLibraryW(\"";
        message += path.string();
        message += "\") == ";
        message += std::to_string((ret ? 1 : GetLastError()));
        Logger::AddMessage(message);

        if (!ret)
        {
            message = "LoadLibraryW(\"";
            message += path.string();
            message += "\") Failed!";
            UI::ShowErrorMessage(message);
            return false;
        }
        else
        {
            modules.push_back(hModule);
        }
    }

    return true;
}

bool Library::UnloadAll(const std::vector<HMODULE>& modules, const std::vector<std::filesystem::path>& paths)
{
    std::string message;

    for (size_t i = 0; i < modules.size(); i++)    
    {
        const auto& module = modules[i];
        const auto& path   = paths[i];

        // attempt to load library
        bool ret = FreeLibrary(module);

        message = "FreeLibrary(\"";
        message += path.string();
        message += "\") == ";
        message += std::to_string((ret ? 1 : GetLastError()));
        Logger::AddMessage(message);

        if (!ret)
        {
            message = "FreeLibrary(\"";
            message += path.string();
            message += "\") Failed!";
            UI::ShowErrorMessage(message);
            return false;
        }
    }

    return true;
}

Game::GameVersion Game::GetCurrentVersion(void)
{
    static std::optional<Game::GameVersion> cachedGameVersion;
    std::filesystem::path currentExecutablePath;
    std::error_code errorCode;
    std::string message;
    uintmax_t currentFileSize;

    if (cachedGameVersion.has_value())
    {
        return cachedGameVersion.value();
    }

    currentExecutablePath = Path::GetCurrentExecutablePath();

    currentFileSize = std::filesystem::file_size(currentExecutablePath, errorCode);
    if (errorCode)
    {
        message = "std::filesystem::file_size(\"";
        message += currentExecutablePath.string();
        message += "\") == 0: ";
        message += errorCode.message();
        Logger::AddMessage(message);

        message = "std::filesystem::file_size(\"";
        message += currentExecutablePath.string();
        message += "\") Failed!";
        UI::ShowErrorMessage(message);
        return Game::GameVersion::Unknown;
    }

    switch (currentFileSize)
    {
    case 24909584: // Disc         v1.5.1
    case 24904192: // Cracked Disc v1.5.1
        cachedGameVersion = Game::GameVersion::Disc_1_5_1;
        break;

    case 24885248: // GOG/Steam March 2017
    case 24895536: // GOG October     2024
    case 25066744: // Steam October   2024
        cachedGameVersion = Game::GameVersion::GogOrSteam_March2017;
        break;

    default:
        cachedGameVersion = Game::GameVersion::Unknown;
        break;
    }

    return cachedGameVersion.value();
}
