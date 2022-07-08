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
#include "detours.h"

#include "SporeModLoader.hpp"
#include "SporeModLoaderHelpers.hpp"

//
// Detoured Functions
//

static int (WINAPI* SporeAppEntry_real)(void) = nullptr;
static int SporeAppEntry_detoured(void)
{
    if (!SporeModLoaderHelpers::IsSporeModManagerRunning())
    {
        SporeModLoaderHelpers::LaunchSporeModManager();
        return 1;
    }


    if (!SporeModLoader::Initialize())
    {
        return 1;
    }

    if (!SporeModLoader::LoadCoreLibs())
    {
        return 1;
    }

    if (!SporeModLoader::LoadModLibs())
    {
        return 1;
    }

    return SporeAppEntry_real();
}

//
// Exported Functions
//

// The game uses this function, however it doesn't seem to actually use any of it,
// so just return E_FAIL.
HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
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
        SporeAppEntry_real = (int (WINAPI*)(VOID))DetourGetEntryPoint(nullptr);
        if (SporeAppEntry_real == nullptr)
        {
            SporeModLoaderHelpers::ShowErrorMessage(L"DetourGetEntryPoint() Failed!");
        }
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)SporeAppEntry_real, SporeAppEntry_detoured);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if (SporeAppEntry_real != nullptr)
        {
            DetourDetach(&(PVOID&)SporeAppEntry_real, SporeAppEntry_detoured);
        }
        DetourTransactionCommit();
    }

    return TRUE;
}
