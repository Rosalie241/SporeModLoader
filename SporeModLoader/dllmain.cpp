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
// Detoured Game Functions
//

typedef void (WINAPI* ptr_InitializeGeneralAllocator)(void);
static void (WINAPI* InitializeGeneralAllocator_real)(void) = nullptr;
static void WINAPI InitializeGeneralAllocator_detoured(void)
{
    InitializeGeneralAllocator_real();

    static bool loaded = false;

    if (!loaded)
    {
        if (!SporeModLoader::LoadCoreLibs())
        {
            std::abort();
        }

        if (!SporeModLoader::LoadModLibs())
        {
            std::abort();
        }

        loaded = true;
    }
}

typedef void (WINAPI* ptr_DisposeGeneralAllocator)(void);
static void (WINAPI* DisposeGeneralAllocator_real)() = nullptr;
static void WINAPI DisposeGeneralAllocator_detoured(void)
{
    static bool unloaded = false;

    if (!unloaded)
    {
        if (!SporeModLoader::UnloadModLibs())
        {
            std::abort();
        }

        if (!SporeModLoader::UnloadCoreLibs())
        {
            std::abort();
        }

        unloaded = true;
    }

    DisposeGeneralAllocator_real();
}

//
// Local Helper Functions
//

static void attach_allocator_detours(void)
{
    Game::GameVersion version = Game::GetCurrentVersion();
    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);

#define SET_DETOUR_ADDRESS(name, disk, steam_patched) \
        name##_real = (ptr_##name)((base + ((version == Game::GameVersion::Disk_1_5_1) ? disk : steam_patched)) - 0x00400000);

    SET_DETOUR_ADDRESS(InitializeGeneralAllocator, 0x01390ca0, 0x0138e640);
    SET_DETOUR_ADDRESS(DisposeGeneralAllocator, 0x013cc060, 0x013c9950);

#undef SET_DETOUR_ADDRESS

    DetourTransactionBegin();
    DetourAttach(&(PVOID&)InitializeGeneralAllocator_real, (PVOID)InitializeGeneralAllocator_detoured);
    DetourAttach(&(PVOID&)DisposeGeneralAllocator_real, (PVOID)DisposeGeneralAllocator_detoured);
    DetourTransactionCommit();
}

//
// Detoured Win32 Functions
//

static void (WINAPI* GetStartupInfoA_real)(LPSTARTUPINFOA) = GetStartupInfoA;
static void GetStartupInfoA_detoured(LPSTARTUPINFOA lpStartupInfo)
{
    static bool initialized = false;

    if (!initialized)
    {
        if (!SporeModLoader::Initialize())
        {
            std::abort();
        }

        // previously, the library loading process looked like this:
        // 
        // SporeApp.exe -> SML detours GetStartupInfoA() -> SporeApp.exe initializes and calls GetStartupInfoA()
        // -> initialize SML -> SML injects core and mod DLLs
        // 
        // at that point the global allocator hasn't been set yet by Spore, 
        // then we have a problem at shutdown, because the data that mod DLLs
        // allocated statically are allocated by malloc/free, while at shutdown
        // the allocator has been set, so it'll try to free it with those
        // functions, which causes a crash.
        // to solve that, we detour the allocator set/unset functions
        // and inject the core and mod DLLs there, so now it looks like:
        // 
        // SporeApp.exe -> SML detours GetStartupInfoA() -> SporeApp.exe initializes and calls GetStartupInfoA()
        // -> initialize SML -> detour allocator functions
        // -> SporeApp.exe sets global allocator   -> SML injects core and mod DLLs 
        // -> SporeApp.exe unsets global allocator -> SML unloads core and mod DLLs
        // 
        // Thanks to @Zarklord for coming up with this method after
        // we discussed that this should be solved somehow instead
        // of making mods fix this by not using static variables
        attach_allocator_detours();

        initialized = true;
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
        DetourTransactionBegin();
        DetourAttach(&(PVOID&)GetStartupInfoA_real, (PVOID)GetStartupInfoA_detoured);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourDetach(&(PVOID&)GetStartupInfoA_real, (PVOID)GetStartupInfoA_detoured);
        DetourTransactionCommit();
    }

    return TRUE;
}
