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

//
// Exported Functions
//

void SporeModLoaderHelpers::ShowErrorMessage(std::wstring message)
{
    MessageBoxW(nullptr, message.c_str(), L"SporeModLoader", MB_OK | MB_ICONERROR);
}

bool SporeModLoaderHelpers::IsSporeModManagerRunning(void)
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
        ShowErrorMessage(L"EnumProcesses() Failed!");
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

void SporeModLoaderHelpers::LaunchSporeModManager()
{
    STARTUPINFOW processStartupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };
    std::wstring processExecutable;
  
    processStartupInfo.cb = sizeof(STARTUPINFOW);

    processExecutable = GetSporeModManagerInstallationPath().wstring();
    processExecutable += L"\\Launch Spore.exe";

    if (!std::filesystem::exists(processExecutable))
    {
        ShowErrorMessage(L"\"" + processExecutable + L"\" doesn't exist!");
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

std::filesystem::path SporeModLoaderHelpers::GetSporeModManagerStoragePath(void)
{
    std::filesystem::path modManagerStoragePath;
    std::filesystem::path redirectStoragePath;
    std::fstream redirectStorageFileStream;
    std::string redirectStorageFileLine;
    wchar_t envBuffer[MAX_PATH];

    if (GetEnvironmentVariableW(L"PROGRAMDATA", envBuffer, MAX_PATH) == 0)
    {
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

std::filesystem::path SporeModLoaderHelpers::GetSporeModManagerInstallationPath(void)
{
    std::filesystem::path modManagerInstallationPath;
    std::filesystem::path modManagerInstallPathInfoPath;
    std::fstream installPathInfoFileStream;
    std::string installPathInfoFileLine;
    wchar_t envBuffer[MAX_PATH];

    if (GetEnvironmentVariableW(L"PROGRAMDATA", envBuffer, MAX_PATH) == 0)
    {
        return modManagerInstallationPath;
    }

    modManagerInstallPathInfoPath = envBuffer;
    modManagerInstallPathInfoPath += L"\\SporeModManagerStorage\\SporeModManager_InstallPath.info";

    if (std::filesystem::exists(modManagerInstallPathInfoPath) &&
        std::filesystem::is_regular_file(modManagerInstallPathInfoPath))
    {
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
    }

    return modManagerInstallationPath;
}

