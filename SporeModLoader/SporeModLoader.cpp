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

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "SporeModLoader.hpp"

//
// Local Enums
//

enum class SporeExecutableType
{
    Disk_1_5_1,
    Origin_1_5_1,
    Origin_March2017,
    GogOrSteam_1_5_1,
    GogOrSteam_March2017,
    Unknown
};

//
// Local Variables
//

static std::filesystem::path l_ModLoaderCoreLibsPath;
static std::filesystem::path l_ModLoaderModLibsPath;
static std::filesystem::path l_ModLoaderLogPath;

//
// Helper Functions
//

static void ShowErrorMessage(std::wstring message)
{
    MessageBoxW(nullptr, message.c_str(), L"SporeModLoader", MB_OK | MB_ICONERROR);
}

static void AddLogMessage(std::string message)
{
    std::ofstream logFile;
    logFile.open(l_ModLoaderLogPath, std::ios_base::app);
    logFile << message << "\n";
    logFile.close();
}

static bool LoadModLibrary(std::filesystem::path path)
{
    bool ret = false;
    std::string logMessage;
    logMessage = "LoadModLibrary(\"";
    logMessage += path.string();
    logMessage += "\")";

    AddLogMessage(logMessage);
    ret = LoadLibraryW(path.wstring().c_str()) != nullptr;
    AddLogMessage(logMessage + " == " + (ret ? "TRUE" : "FALSE"));

    return ret;
}

static bool LoadLibrariesInPath(std::wstring path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        // skip non-files & non-dlls
        if (!entry.is_regular_file() ||
            !entry.path().wstring().ends_with(L"dll"))
        {
            continue;
        }

        // attempt to load mod library
        if (!LoadModLibrary(entry.path()))
        {
            std::wstring errorMessage;
            errorMessage = L"LoadModLibrary(\"";
            errorMessage += entry.path().wstring();
            errorMessage += L"\") Failed!";
            ShowErrorMessage(errorMessage);
            return false;
        }
    }

    return true;
}

static SporeExecutableType GetCurrentGameVersion(void)
{
    wchar_t currentFileNameBuf[MAX_PATH];
    uintmax_t currentFileSize;

    if (GetModuleFileNameW(nullptr, currentFileNameBuf, MAX_PATH) == 0)
    {
        ShowErrorMessage(L"GetModuleFileNameW() Failed!");
        throw std::exception();
    }

    try
    {
        currentFileSize = std::filesystem::file_size(currentFileNameBuf);
    }
    catch (...)
    {
        std::wstring errorMessage;
        errorMessage = L"std::filesystem::file_size(\"";
        errorMessage += currentFileNameBuf;
        errorMessage += L"\") Failed!";
        ShowErrorMessage(errorMessage);
        throw std::exception();
    }

    switch (currentFileSize)
    {
        case 24909584:
            return SporeExecutableType::Disk_1_5_1;
        case 31347984:
            return SporeExecutableType::Origin_1_5_1;
        case 24898224:
            return SporeExecutableType::Origin_March2017;
        case 24888320:
            return SporeExecutableType::GogOrSteam_1_5_1;
        case 24885248:
            return SporeExecutableType::GogOrSteam_March2017;

        default:
            return SporeExecutableType::Unknown;
    }
}

//
// Exported Functions
//

bool SporeModLoader::Initialize()
{
    try
    {
        std::filesystem::path modLoaderPath;
        std::filesystem::path errorMessage;

        modLoaderPath = std::filesystem::current_path().wstring();
        modLoaderPath += L"\\SporeModLoader";

        l_ModLoaderCoreLibsPath = modLoaderPath;
        l_ModLoaderCoreLibsPath += "\\CoreLibs";
        l_ModLoaderModLibsPath = modLoaderPath;
        l_ModLoaderModLibsPath += "\\ModLibs";
        l_ModLoaderLogPath = modLoaderPath;
        l_ModLoaderLogPath += "\\SporeModLoader.log";

        // remove log file if it exists
        if (std::filesystem::exists(l_ModLoaderLogPath))
        {
            try
            {
                std::filesystem::remove(l_ModLoaderLogPath);
            }
            catch (...)
            {
                errorMessage = L"std::filesystem::remove(\"";
                errorMessage += l_ModLoaderLogPath.wstring();
                errorMessage += L"\") Failed!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
        }

        for (const auto& path : { l_ModLoaderCoreLibsPath , l_ModLoaderModLibsPath })
        {
            if (!std::filesystem::is_directory(path))
            {
                errorMessage = path;
                errorMessage += L" doesn't exist!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
        }

        switch (GetCurrentGameVersion())
        {
            case SporeExecutableType::Unknown:
                ShowErrorMessage(L"Unknown Spore version!");
                throw std::exception();

            case SporeExecutableType::Origin_1_5_1:
            case SporeExecutableType::GogOrSteam_1_5_1:
                ShowErrorMessage(L"Update Spore to the latest version!");
                throw std::exception();

            case SporeExecutableType::Origin_March2017:
            case SporeExecutableType::GogOrSteam_March2017:
                l_ModLoaderCoreLibsPath += L"\\march2017";
                break;

            case SporeExecutableType::Disk_1_5_1:
                l_ModLoaderCoreLibsPath += L"\\disk";
                break;
        }

        AddLogMessage("SporeModLoader::Initialize() == TRUE");
        return true;
    }
    catch (...)
    {
        AddLogMessage("SporeModLoader::Initialize() == FALSE");
        ShowErrorMessage(L"SporeModLoader::Initialize() Failed!");
        return false;
    }
}

bool SporeModLoader::LoadCoreLibs()
{
    try
    {
        AddLogMessage("SporeModLoader::LoadCoreLibs()");
        if (!LoadLibrariesInPath(l_ModLoaderCoreLibsPath))
        {
            throw std::exception();
        }
        AddLogMessage("SporeModLoader::LoadCoreLibs() == TRUE");
        return true;
    }
    catch (...)
    {
        AddLogMessage("SporeModLoader::LoadCoreLibs() == FALSE");
        ShowErrorMessage(L"SporeModLoader::LoadCoreLibs() Failed!");
        return false;
    }
}

bool SporeModLoader::LoadModLibs()
{
    try
    {
        AddLogMessage("SporeModLoader::LoadModLibs()");
        if (!LoadLibrariesInPath(l_ModLoaderModLibsPath))
        {
            throw std::exception();
        }
        AddLogMessage("SporeModLoader::LoadModLibs() == TRUE");
        return true;
    }
    catch (...)
    {
        AddLogMessage("SporeModLoader::LoadModLibs() == FALSE");
        ShowErrorMessage(L"SporeModLoader::LoadModLibs() Failed!");
        return false;
    }
}
