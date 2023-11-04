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

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <cstring>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#ifdef _WIN32
#define arg_str(str) L##str
#define arg_str_type std::wstring
#else
#define arg_str(str) str
#define arg_str_type std::string
#endif // _WIN32

//
// Local Structures
//

struct option_argument
{
    arg_str_type shortArgument;
    arg_str_type longArgument;
    bool&        value;
};

struct path_argument
{
    arg_str_type           argument;
    std::filesystem::path& path;
};

//
// Local Functions
//

static void ShowUsage()
{
    std::cout << "SporeModManager is a commandline mod manager for Spore" << std::endl
              << std::endl
              << "Usage: SporeModManager [OPTION] [COMMAND]" << std::endl
              << std::endl
              << "Commands:" << std::endl
              << "  list-installed      lists installed mod(s) with id(s)" << std::endl
              << "  install file(s)     installs file(s)" << std::endl
              << "  update file(s)      updates mod(s) using file(s)" << std::endl
              << "  uninstall id(s)     uninstalls mod with id(s)" << std::endl
              << std::endl
              << "  help                display this help and exit" << std::endl
              << std::endl
              << "Options: " << std::endl
              << "  -n, --no-input      disables user input during installation of mods" << std::endl
              << "  -u, --update-needed updates mod when mod is already installed" << std::endl
              << "  -s, --save-paths    saves paths to the configuration file" << std::endl
              << "      --corelibs-path sets corelibs path" << std::endl
              << "      --modlibs-path  sets modlibs path"  << std::endl
              << "      --data-path     sets data path"     << std::endl
              << "      --ep1-path      sets ep1 data path" << std::endl
              << std::endl;
}

//
// Exported Functions
//

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif // _WIN32
{
    std::vector<arg_str_type> args(argv, argv + argc);

    // parse options
    bool hasNoInputOption   = false;
    bool hasUpdateOption    = false;
    bool hasSavePathsOption = false;
    std::filesystem::path coreLibsPath;
    std::filesystem::path modLibsPath;
    std::filesystem::path dataPath;
    std::filesystem::path ep1Path;

    struct option_argument optionArgs[] =
    {
        { arg_str("-n"), arg_str("--no-input"),      hasNoInputOption },
        { arg_str("-u"), arg_str("--update-needed"), hasUpdateOption },
        { arg_str("-s"), arg_str("--save-paths"),    hasSavePathsOption },
    };

    struct path_argument pathArgs[] =
    {
        { arg_str("--corelibs-path"), coreLibsPath },
        { arg_str("--modlibs-path"),  modLibsPath },
        { arg_str("--data-path"),     dataPath },
        { arg_str("--ep1-path"),      ep1Path },
    };

    for (size_t i = 0; i < args.size(); i++)
    {
        arg_str_type arg = args.at(i);

        for (const auto& optionArg : optionArgs)
        {
            if (arg == optionArg.shortArgument ||
                arg == optionArg.longArgument)
            {
                optionArg.value = true;
                args.erase(args.begin() + i);
                i -= 1;
            }
        }

        for (const auto& pathArg : pathArgs)
        {
            if (arg == pathArg.argument)
            {
                if (i == (args.size() - 1))
                {
                    ShowUsage();
                    return 1;
                }
                pathArg.path = args.at(i + 1);
                if (!std::filesystem::is_directory(pathArg.path))
                {
                    ShowUsage();
                    return 1;
                }
                args.erase(args.begin() + i + 1);
                args.erase(args.begin() + i);
                i -= 1;
            }
        }
    }

    // apply options
    UI::SetNoInputMode(hasNoInputOption);
    Path::SetDirectories(coreLibsPath, modLibsPath, ep1Path, dataPath);
    if (hasSavePathsOption)
    {
        if (!SporeMod::Xml::SaveDirectories(coreLibsPath, modLibsPath, ep1Path, dataPath))
        {
            std::cerr << "SporeMod::Xml::SaveDirectories() Failed!" << std::endl;
            return 1;
        }
    }

    // ensure we have a command
    if (args.size() < 2)
    {
        ShowUsage();
        return 1;
    }

    // parse commands
    arg_str_type command = args.at(1);
    if (command == arg_str("help"))
    {
        ShowUsage();
        return 0;
    }
    else if (command == arg_str("list-installed"))
    {
        if (args.size() != 2)
        {
            ShowUsage();
            return 1;
        }

        SporeModManager::ListInstalledMods();
        return 0;
    }
    else if (command == arg_str("install"))
    {
        if (!Path::CheckIfPathsExist())
        {
            return 1;
        }

        if (args.size() < 3)
        {
            ShowUsage();
            return 1;
        }

        for (size_t i = 2; i < args.size(); i++)
        {
            if (hasUpdateOption)
            {
                if (!SporeModManager::UpdateMod(args[i], false))
                {
                    return 1;
                }
            }
            else
            {
                if (!SporeModManager::InstallMod(args[i]))
                {
                    return 1;
                }
            }
        }
    }
    else if (command == arg_str("update"))
    {
        if (!Path::CheckIfPathsExist())
        {
            return 1;
        }

        if (args.size() < 3)
        {
            ShowUsage();
            return 1;
        }

        for (size_t i = 2; i < args.size(); i++)
        {
            if (!SporeModManager::UpdateMod(args[i]))
            {
                return 1;
            }
        }
    }
    else if (command == arg_str("uninstall"))
    {
        if (!Path::CheckIfPathsExist())
        {
            return 1;
        }

        if (args.size() < 3)
        {
            ShowUsage();
            return 1;
        }

        std::vector<int> ids;

        for (size_t i = 2; i < args.size(); i++)
        {
            try
            {
                ids.push_back(std::stoi(args[i]));
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
