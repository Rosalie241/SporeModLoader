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

int main(int argc, char** argv)
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

    char* action = argv[1];
    if (std::strcmp(action, "help") == 0)
    {
        ShowUsage();
        return 0;
    }
    else if (std::strcmp(action, "list-installed") == 0)
    {
        if (argc != 2)
        {
            ShowUsage();
            return 1;
        }

        SporeModManager::ListInstalledMods();
        return 0;
    }
    else if (std::strcmp(action, "install") == 0)
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
    else if (std::strcmp(action, "update") == 0)
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
    else if (std::strcmp(action, "uninstall") == 0)
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
