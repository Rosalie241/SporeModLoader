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

static std::filesystem::path l_ModLoaderModLibsPath;
static std::filesystem::path l_ModLoaderLogPath;
static std::vector<std::filesystem::path> l_ModLoaderCoreLibs;

//
// Helper Functions
//

static void ShowErrorMessage(std::wstring message)
{
    MessageBoxW(nullptr, message.c_str(), L"SporeModLoader", MB_OK | MB_ICONERROR);
}

static void ShowErrorMessage(std::string message)
{
    MessageBoxA(nullptr, message.c_str(), "SporeModLoader", MB_OK | MB_ICONERROR);
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

static std::vector<std::filesystem::path> GetLibrariesInPath(std::filesystem::path path)
{
    std::vector<std::filesystem::path> libraries;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (!entry.is_regular_file() ||
            !entry.path().filename().wstring().ends_with(L".dll"))
        {
            continue;
        }

        libraries.push_back(entry.path());
    }

    return libraries;
}

static std::filesystem::path GetModManagerPath(void)
{
    std::filesystem::path modManagerPath;
    std::filesystem::path redirectStoragePath;
    std::fstream redirectStorageFileStream;
    std::string redirectStorageFileLine;

    wchar_t envBuffer[2048];
    
    if (GetEnvironmentVariableW(L"PROGRAMDATA", envBuffer, 2048) == 0)
    {
        return modManagerPath;
    }

    modManagerPath = envBuffer;
    modManagerPath += "\\SporeModManagerStorage";
    redirectStoragePath = modManagerPath;
    redirectStoragePath += "\\redirectStorage.txt";

    if (std::filesystem::exists(redirectStoragePath) &&
        std::filesystem::is_regular_file(redirectStoragePath))
    {
        redirectStorageFileStream.open(redirectStoragePath);
        if (redirectStorageFileStream.is_open())
        {
            std::getline(redirectStorageFileStream, redirectStorageFileLine);
            if (!redirectStorageFileLine.empty())
            {
                modManagerPath = redirectStorageFileLine;
            }
        }
        redirectStorageFileStream.close();
    }

    return modManagerPath;
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

static void TryCopyFile(std::filesystem::path inputPath, std::filesystem::path outputPath)
{
    try
    {
        std::filesystem::copy_file(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
    }
    catch (...)
    {
        std::wstring errorMessage;
        errorMessage = L"std::filesystem::copy_file(\"";
        errorMessage += inputPath.wstring();
        errorMessage += L"\", \"";
        errorMessage += outputPath.wstring();
        errorMessage += L"\") Failed!";
        ShowErrorMessage(errorMessage);
        throw std::exception();
    }
}

//
// Exported Functions
//

bool SporeModLoader::Initialize()
{
    try
    {
        std::wstring errorMessage;
        std::filesystem::path legacyCoreLibPath;
        std::filesystem::path coreLibsPath;
        std::filesystem::path coreLibFileInputPath;
        std::filesystem::path coreLibFileOutputPath;
        std::filesystem::path coreLibInputPath;
        std::filesystem::path coreLibOutputPath;
        std::filesystem::path sporeModManagerPath;
        
        sporeModManagerPath = GetModManagerPath();
        coreLibsPath = sporeModManagerPath;
        coreLibsPath += "\\coreLibs";
        l_ModLoaderModLibsPath = sporeModManagerPath;
        l_ModLoaderModLibsPath += "\\mLibs";
        l_ModLoaderLogPath = sporeModManagerPath;
        l_ModLoaderLogPath += "\\SporeModLoader.log";
        legacyCoreLibPath = sporeModManagerPath;
        legacyCoreLibPath += "\\legacyLibs";
        coreLibFileInputPath = coreLibsPath;
        coreLibFileInputPath += "\\SporeModAPI.lib";
        coreLibFileOutputPath = l_ModLoaderModLibsPath;
        coreLibFileOutputPath += "\\SporeModAPI.lib";
        coreLibInputPath = coreLibsPath;
        coreLibOutputPath = l_ModLoaderModLibsPath;
        coreLibOutputPath += "\\SporeModAPI.dll";

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

        // retrieve game version and set variables
        switch (GetCurrentGameVersion())
        {
        default:
        case SporeExecutableType::Unknown:
            ShowErrorMessage(L"Unknown Spore version!");
            throw std::exception();

        case SporeExecutableType::Origin_1_5_1:
        case SporeExecutableType::GogOrSteam_1_5_1:
            ShowErrorMessage(L"Update Spore to the latest version!");
            throw std::exception();

        case SporeExecutableType::Origin_March2017:
        case SporeExecutableType::GogOrSteam_March2017:
            legacyCoreLibPath += "\\SporeModAPI-steam_patched.dll";
            coreLibInputPath += "\\SporeModAPI.march2017.dll";
            break;

        case SporeExecutableType::Disk_1_5_1:
            legacyCoreLibPath += "\\SporeModAPI-disk.dll";
            coreLibInputPath += "\\SporeModAPI.disk.dll";
            break;
        }

        // make sure GetModManagerPath() didn't fail
        if (sporeModManagerPath.empty())
        {
            ShowErrorMessage("GetModManagerPath() Failed!");
            throw std::exception();
        }

        // make sure required directories exist
        for (const auto& path : { sporeModManagerPath, coreLibsPath })
        {
            if (!std::filesystem::is_directory(path))
            {
                errorMessage = L"\"" + path.wstring() + L"\" doesn't exist!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
        }

        if (!std::filesystem::exists(coreLibInputPath))
        {
            errorMessage = L"\"" + coreLibInputPath.wstring() + L"\" doesn't exist!";
            ShowErrorMessage(errorMessage);
        }
       
        if (!std::filesystem::exists(l_ModLoaderModLibsPath))
        {
            try
            {
                std::filesystem::create_directory(l_ModLoaderModLibsPath);
            }
            catch (...)
            {
                errorMessage = L"std::filesystem::create_directory(\"";
                errorMessage += l_ModLoaderModLibsPath.wstring();
                errorMessage += L"\") Failed!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
        }

        // copy SporeModAPI.lib
        TryCopyFile(coreLibFileInputPath, coreLibFileOutputPath);
        // copy SporeModAPI.dll
        TryCopyFile(coreLibInputPath, coreLibOutputPath);

        // make sure the core libs are added here  
        l_ModLoaderCoreLibs.push_back(coreLibOutputPath);
        l_ModLoaderCoreLibs.push_back(legacyCoreLibPath);

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
        for (const auto& library : l_ModLoaderCoreLibs)
        {
            if (!LoadModLibrary(library))
            {
                std::wstring errorMessage;
                errorMessage = L"LoadModLibrary(\"";
                errorMessage += library.wstring();
                errorMessage += L"\") Failed!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
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
        for (const auto& library : GetLibrariesInPath(l_ModLoaderModLibsPath))
        {
            if (std::find(l_ModLoaderCoreLibs.begin(), l_ModLoaderCoreLibs.end(), library) != l_ModLoaderCoreLibs.end())
            { // skip core libs
                continue;
            }

            if (!LoadModLibrary(library))
            {
                std::wstring errorMessage;
                errorMessage = L"LoadModLibrary(\"";
                errorMessage += library.wstring();
                errorMessage += L"\") Failed!";
                ShowErrorMessage(errorMessage);
                throw std::exception();
            }
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
