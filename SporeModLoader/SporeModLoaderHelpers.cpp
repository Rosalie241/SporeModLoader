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
#include <Psapi.h>

#include <filesystem>
#include <fstream>

#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

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

std::filesystem::path Path::GetSporeModManagerStoragePath(void)
{
    static std::filesystem::path modManagerStoragePath;
    std::filesystem::path redirectStoragePath;
    std::wfstream redirectStorageFileStream;
    std::wstring redirectStorageFileLine;
    wchar_t envBuffer[MAX_PATH];

    if (!modManagerStoragePath.empty())
    {
        return modManagerStoragePath;
    }

    if (GetEnvironmentVariableW(L"PROGRAMDATA", envBuffer, MAX_PATH) == 0)
    {
        UI::ShowErrorMessage(L"GetEnvironmentVariableW() Failed!");
        return modManagerStoragePath;
    }

    modManagerStoragePath = envBuffer;
    modManagerStoragePath += "\\SporeModManagerStorage";
    redirectStoragePath = modManagerStoragePath;
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
                modManagerStoragePath = redirectStorageFileLine;
            }
        }
        redirectStorageFileStream.close();
    }

    return modManagerStoragePath;
}

std::filesystem::path Path::GetSporeModManagerInstallationPath(void)
{
    static std::filesystem::path modManagerInstallationPath;
    std::filesystem::path modManagerInstallPathInfoPath;
    std::wfstream installPathInfoFileStream;
    std::wstring installPathInfoFileLine;
    wchar_t envBuffer[MAX_PATH];

    if (!modManagerInstallationPath.empty())
    {
        return modManagerInstallationPath;
    }

    if (GetEnvironmentVariableW(L"PROGRAMDATA", envBuffer, MAX_PATH) == 0)
    {
        UI::ShowErrorMessage(L"GetEnvironmentVariableW() Failed!");
        return modManagerInstallationPath;
    }

    modManagerInstallPathInfoPath = envBuffer;
    modManagerInstallPathInfoPath += L"\\SporeModManagerStorage\\SporeModManager_InstallPath.info";

    if (!std::filesystem::exists(modManagerInstallPathInfoPath))
    {
        std::wstring errorMessage;
        errorMessage = L"\"";
        errorMessage += modManagerInstallPathInfoPath.wstring();
        errorMessage += L"\" doesn't exist!";
        UI::ShowErrorMessage(errorMessage);
        throw std::exception();
    }

    installPathInfoFileStream.open(modManagerInstallPathInfoPath);
    if (installPathInfoFileStream.is_open())
    {
        std::getline(installPathInfoFileStream, installPathInfoFileLine);
        if (!installPathInfoFileLine.empty())
        {
            modManagerInstallationPath = installPathInfoFileLine;
        }
    }
    installPathInfoFileStream.close();

    return modManagerInstallationPath;
}

std::filesystem::path Path::GetLogFilePath(void)
{
    std::filesystem::path logFilePath;

    logFilePath = Path::GetSporeModManagerStoragePath();
    logFilePath += "\\SporeModLoader.log";

    return logFilePath;
}

std::vector<std::filesystem::path> Path::GetCoreLibsPaths(void)
{
    static std::vector<std::filesystem::path> coreLibsPaths;
    std::filesystem::path modManagerStoragePath;
    std::filesystem::path legacyLibPath;
    std::filesystem::path coreLibInputPath;
    std::filesystem::path coreLibOutputPath;

    if (!coreLibsPaths.empty())
    {
        return coreLibsPaths;
    }

    modManagerStoragePath = Path::GetSporeModManagerStoragePath();

    legacyLibPath = modManagerStoragePath;
    legacyLibPath += "\\legacyLibs";
    coreLibInputPath = modManagerStoragePath;
    coreLibInputPath += "\\coreLibs";
    coreLibOutputPath = modManagerStoragePath;
    coreLibOutputPath += "\\mLibs\\SporeModAPI.dll";

    switch (Game::GetCurrentVersion())
    {
    default:
    case Game::GameVersion::Unknown:
        UI::ShowErrorMessage(L"Unknown Spore version!");
        throw std::exception();

    case Game::GameVersion::Origin_1_5_1:
    case Game::GameVersion::GogOrSteam_1_5_1:
        UI::ShowErrorMessage(L"Update Spore to the latest version!");
        throw std::exception();

    case Game::GameVersion::Origin_March2017:
    case Game::GameVersion::GogOrSteam_March2017:
        legacyLibPath += "\\SporeModAPI-steam_patched.dll";
        coreLibInputPath += "\\SporeModAPI.march2017.dll";
        break;

    case Game::GameVersion::Disk_1_5_1:
        legacyLibPath += "\\SporeModAPI-disk.dll";
        coreLibInputPath += "\\SporeModAPI.disk.dll";
        break;
    }

    try
    {
        std::filesystem::copy_file(coreLibInputPath, coreLibOutputPath, std::filesystem::copy_options::overwrite_existing);
    }
    catch (...)
    {
        std::wstring errorMessage;
        errorMessage = L"std::filesystem::copy_file(\"";
        errorMessage += coreLibInputPath.wstring();
        errorMessage += L"\", \"";
        errorMessage += coreLibOutputPath.wstring();
        errorMessage += L"\") Failed!";
        UI::ShowErrorMessage(errorMessage);
        throw std::exception();
    }

    coreLibsPaths.push_back(coreLibOutputPath);
    coreLibsPaths.push_back(legacyLibPath);

    return coreLibsPaths;
}

std::vector<std::filesystem::path> Path::GetModLibsPaths(void)
{
    std::vector<std::filesystem::path> modLibsPaths;
    std::vector<std::filesystem::path> coreLibsPaths;
    std::filesystem::path mLibsPath;

    coreLibsPaths = Path::GetCoreLibsPaths();

    mLibsPath = Path::GetSporeModManagerStoragePath();
    mLibsPath += "\\mLibs";

    for (const auto& entry : std::filesystem::recursive_directory_iterator(mLibsPath))
    {
        // skip non-files & non-dlls
        if (!entry.is_regular_file() ||
            !entry.path().has_extension() ||
            entry.path().extension() != ".dll")
        {
            continue;
        }

        // skip core libs
        if (std::find(coreLibsPaths.begin(), coreLibsPaths.end(), entry.path()) != coreLibsPaths.end())
        {
            continue;
        }

        modLibsPaths.push_back(entry.path());
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

bool SporeModManager::IsRunning(void)
{
    DWORD processIds[2048];
    DWORD amountOfProcessIds = 0;
    BOOL ret;
    DWORD processId;
    HANDLE processHandle;
    wchar_t processFileNameBuf[MAX_PATH];
    std::filesystem::path processFileNamePath;

    ret = EnumProcesses(processIds, sizeof(processIds), &amountOfProcessIds);
    if (ret == 0)
    {
        UI::ShowErrorMessage(L"EnumProcesses() Failed!");
        return false;
    }

    amountOfProcessIds = amountOfProcessIds / sizeof(DWORD);

    for (DWORD i = 0; i < amountOfProcessIds; i++)
    {
        processId = processIds[i];

        processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
        if (processHandle == nullptr)
        { // skip when we cant open the process
            continue;
        }
        
        ret = GetProcessImageFileNameW(processHandle, processFileNameBuf, MAX_PATH);
        if (ret == 0)
        {
            CloseHandle(processHandle);
            continue;
        }

        CloseHandle(processHandle);

        processFileNamePath = std::filesystem::path(processFileNameBuf);
        if (processFileNamePath.filename().string() == "Launch Spore.exe")
        {
            return true;
        }
    }

    return false;
}

void SporeModManager::Launch(void)
{
    STARTUPINFOW processStartupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };
    std::wstring processExecutable;

    processStartupInfo.cb = sizeof(STARTUPINFOW);

    processExecutable = Path::GetSporeModManagerInstallationPath().wstring();
    processExecutable += L"\\Launch Spore.exe";

    if (!std::filesystem::exists(processExecutable))
    {
        UI::ShowErrorMessage(L"\"" + processExecutable + L"\" doesn't exist!");
        return;
    }

    CreateProcessW(nullptr,
        const_cast<LPWSTR>(processExecutable.c_str()),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &processStartupInfo,
        &processInfo);
}


