/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeModManager.hpp"
#include "SporeModManagerHelpers.hpp"

#include <iostream>
#include <cstring>

using namespace SporeModManagerHelpers;

static void ShowUsage()
{
    std::cout << "Usage: SporeModManager [OPTION]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "  list-installed      lists installed mod(s) with id(s)" << std::endl
              << "  install file(s)     installs file(s)" << std::endl
              << "  update file(s)      updates mod(s) using file(s)" << std::endl
              << "  uninstall id(s)     uninstalls mod with id(s)" << std::endl
              << std::endl
              << "  help                display this help and exit" << std::endl;
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif // _WIN32
{
    if (!Path::CheckIfPathsExist())
    {
        return 1;
    }

    if (argc < 2)
    {
        ShowUsage();
        return 1;
    }

#ifdef _WIN32
    // we know that wstring -> string conversion
    // will possibly lose data, but that doesn't 
    // matter for the first argument, so we can
    // safely disable the warning
#pragma warning(disable : 4244)
    std::wstring waction = std::wstring(argv[1]);
    std::string  action  = std::string(waction.begin(), waction.end());
#else
    std::string action = std::string(argv[1]);
#endif // _WIN32
    if (action == "help")
    {
        ShowUsage();
        return 0;
    }
    else if (action == "list-installed")
    {
        if (argc != 2)
        {
            ShowUsage();
            return 1;
        }

        SporeModManager::ListInstalledMods();
        return 0;
    }
    else if (action == "install")
    {
        if (argc < 3)
        {
            ShowUsage();
            return 1;
        }
        
        for (int i = 2; i < argc; i++)
        {
            if (!SporeModManager::InstallMod(argv[i]))
            {
                return 1;
            }
        }
    }
    else if (action == "update")
    {
        if (argc < 3)
        {
            ShowUsage();
            return 1;
        }

        for (int i = 2; i < argc; i++)
        {
            if (!SporeModManager::UpdateMod(argv[i]))
            {
                return 1;
            }
        }
    }
    else if (action == "uninstall")
    {
        if (argc < 3)
        {
            ShowUsage();
            return 1;
        }

        std::vector<int> ids;

        for (int i = 2; i < argc; i++)
        {
            try
            {
                ids.push_back(std::stoi(argv[i]));
            }
            catch (...)
            {
                std::cerr << "ID must be a number!" << std::endl;
                return 1;
            }
        }

        if (!SporeModManager::UninstallMods(ids))
        {
            return 1;
        }
    } 
    else
    {
        ShowUsage();
        return 1;
    }

    return 0;
}
