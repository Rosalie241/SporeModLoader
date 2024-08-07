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

static int (WINAPI* SporeAppEntry_real)(void) = nullptr;
static int SporeAppEntry_detoured(void)
{
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
        SporeAppEntry_real = (int (WINAPI*)(VOID))DetourGetEntryPoint(nullptr);
        if (SporeAppEntry_real == nullptr)
        {
            UI::ShowErrorMessage(L"DetourGetEntryPoint() Failed!");
        }
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)SporeAppEntry_real, (PVOID)SporeAppEntry_detoured);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if (SporeAppEntry_real != nullptr)
        {
            DetourDetach(&(PVOID&)SporeAppEntry_real, (PVOID)SporeAppEntry_detoured);
        }
        DetourTransactionCommit();
    }

    return TRUE;
}
