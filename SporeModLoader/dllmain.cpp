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


//
// Detoured Functions
//

static int (WINAPI* SporeAppEntry_real)(void) = NULL;
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

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    typedef HRESULT(WINAPI* DirectInput8Create_function)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
    DirectInput8Create_function DirectInput8Create_real;
    HMODULE hModule;
    HRESULT result;

    hModule = LoadLibraryA("C:\\Windows\\System32\\dinput8.dll");
    if (hModule == NULL)
    {
        return E_FAIL;
    }

    DirectInput8Create_real = (DirectInput8Create_function)GetProcAddress(hModule, "DirectInput8Create");
    if (DirectInput8Create_real == NULL)
    {
        return E_FAIL;
    }

    result = DirectInput8Create_real(hinst, dwVersion, riidltf, ppvOut, punkOuter);

    // free library
    FreeLibrary(hModule);

    return result;
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

        SporeAppEntry_real = (int (WINAPI*)(VOID))DetourGetEntryPoint(NULL);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)SporeAppEntry_real, SporeAppEntry_detoured);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)SporeAppEntry_real, SporeAppEntry_detoured);
        DetourTransactionCommit();
    }

    return TRUE;
}
