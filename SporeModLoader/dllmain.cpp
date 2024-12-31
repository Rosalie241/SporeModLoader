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
#include "detours.h"

#include "SporeModLoader.hpp"
#include "SporeModLoaderHelpers.hpp"

using namespace SporeModLoaderHelpers;

//
// Detoured Functions
//

static void (WINAPI* GetStartupInfoA_real)(LPSTARTUPINFOA) = GetStartupInfoA;
static void GetStartupInfoA_detoured(LPSTARTUPINFOA lpStartupInfo)
{
    static bool injected = false;

    if (!injected)
    {
        if (!SporeModLoader::Initialize())
        {
            std::abort();
        }

        if (!SporeModLoader::LoadCoreLibs())
        {
            std::abort();
        }

        if (!SporeModLoader::LoadModLibs())
        {
            std::abort();
        }

        injected = true;
    }

    return GetStartupInfoA_real(lpStartupInfo);
}

//
// Exported Functions
//

// The game calls this function but ignores the result, so just return E_FAIL.
extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN)
{
    return E_FAIL;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (DetourIsHelperProcess())
    {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)GetStartupInfoA_real, (PVOID)GetStartupInfoA_detoured);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)GetStartupInfoA_real, (PVOID)GetStartupInfoA_detoured);
        DetourTransactionCommit();
    }

    return TRUE;
}
