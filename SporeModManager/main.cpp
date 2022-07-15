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

using namespace SporeModManagerHelpers;

static void ShowUsage()
{
    std::cout << "Usage: SporeModManager [OPTION]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "  -l, --list-installed      lists installed mods with IDs" << std::endl
              << "  -i, --install FILE        installs FILE" << std::endl
              << "  -u, --update FILE         updates mod using FILE" << std::endl
              << "  -r, --uninstall ID        uninstalls mod with ID" << std::endl
              << std::endl
              << "      --help                display this help and exit" << std::endl;
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
    if (std::strcmp(action, "--help") == 0)
    {
        ShowUsage();
        return 0;
    }
    else if (std::strcmp(action, "-l") == 0 ||
        std::strcmp(action, "--list-installed") == 0)
    {
        if (argc != 2)
        {
            ShowUsage();
            return 1;
        }

        SporeModManager::ListInstalledMods();
        return 0;
    }
    else if (std::strcmp(action, "-i") == 0 ||
        std::strcmp(action, "--install") == 0)
    {
        if (argc != 3)
        {
            ShowUsage();
            return 1;
        }
        
        if (!SporeModManager::InstallMod(argv[2]))
        {
            return 1;
        }
    }
    else if (std::strcmp(action, "-u") == 0 ||
        std::strcmp(action, "--update") == 0)
    {
        if (argc != 3)
        {
            ShowUsage();
            return 1;
        }

        if (!SporeModManager::UpdateMod(argv[2]))
        {
            return 1;
        }
    }
    else if (std::strcmp(action, "-r") == 0 ||
        std::strcmp(action, "--uninstall") == 0)
    {
        if (argc != 3)
        {
            ShowUsage();
            return 1;
        }

        int id = 0;

        try
        {
            id = std::stoi(argv[2]);
        }
        catch (...)
        {
            std::cerr << "ID must be a number!" << std::endl;
            return 1;
        }

        if (!SporeModManager::UninstallMod(id))
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
