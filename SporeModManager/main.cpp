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

#include "../version.h"

#include <filesystem>
#include <stdio.h>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#ifdef _WIN32
#define arg_str(str) L##str
#define arg_char(c) L##c
#define arg_str_type std::wstring
#define arg_cerr wcerr
#define arg_format "%ls"
#else
#define arg_str(str) str
#define arg_char(c) c
#define arg_str_type std::string
#define arg_cerr cerr
#define arg_format "%s"
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
    puts("SporeModManager is a commandline mod manager for Spore\n\n"
           "Usage: SporeModManager [OPTION] [COMMAND]\n\n"
           "Commands:\n"
           "  list-installed      lists installed mod(s) with id(s)\n"
           "  install file(s)     installs file(s)\n"
           "  update file(s)      updates mod(s) using file(s)\n"
           "  uninstall id(s)     uninstalls mod with id(s)\n\n"
           "  version             display version and exit\n"
           "  help                display this help and exit\n\n"
           "Options: \n"
           "  -v, --verbose       shows verbose output\n"
           "  -y, --no-input      disables user input during installation of mods\n"
           "  -n, --needed        only install mod when mod isn't installed\n"
           "  -u, --update-needed updates mod when mod is already installed\n"
           "  -s, --save-paths    saves paths to the configuration file\n"
           "      --corelibs-path sets corelibs path\n"
           "      --modlibs-path  sets modlibs path\n"
           "      --data-path     sets data path\n"
           "      --ep1-path      sets ep1 data path\n");
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
    bool hasVerboseOption   = false;
    bool hasNoInputOption   = false;
    bool hasNeededOption    = false;
    bool hasUpdateOption    = false;
    bool hasSavePathsOption = false;
    std::filesystem::path coreLibsPath;
    std::filesystem::path modLibsPath;
    std::filesystem::path dataPath;
    std::filesystem::path ep1Path;

    struct option_argument optionArgs[] =
    {
        { arg_str("v"), arg_str("verbose"),       hasVerboseOption },
        { arg_str("y"), arg_str("no-input"),      hasNoInputOption },
        { arg_str("n"), arg_str("needed"),        hasNeededOption  },
        { arg_str("u"), arg_str("update-needed"), hasUpdateOption },
        { arg_str("s"), arg_str("save-paths"),    hasSavePathsOption },
    };

    struct path_argument pathArgs[] =
    {
        { arg_str("corelibs-path"), coreLibsPath },
        { arg_str("modlibs-path"),  modLibsPath },
        { arg_str("data-path"),     dataPath },
        { arg_str("ep1-path"),      ep1Path },
    };

    for (size_t i = 0; i < args.size(); i++)
    {
        arg_str_type arg = args.at(i);

        // skip empty or invalid arguments
        if (arg.size() < 2)
        {
            continue;
        }

        if (arg.at(0) == arg_char('-'))
        {
            if (arg.at(1) != arg_char('-'))
            { // short option
                // strip '-'
                arg.erase(0, 1);
                for (const auto& optionArg : optionArgs)
                {
                    auto pos = arg.find(optionArg.shortArgument);
                    if (pos != arg_str_type::npos)
                    {
                        optionArg.value = true;
                        arg.erase(pos, 1);
                    }
                }
                if (!arg.empty())
                {
                    fprintf(stderr, "unrecognized option: -" arg_format, arg.c_str());
                    return 1;
                }
            }
            else
            { // long option
                // strip '--'
                arg.erase(0, 2);
                for (const auto& optionArg : optionArgs)
                {
                    if (arg == optionArg.longArgument)
                    {
                        optionArg.value = true;
                        arg.clear();
                    }
                }
                for (const auto& pathArg : pathArgs)
                {
                    if (arg.rfind(pathArg.argument, 0) == 0)
                    {
                        arg.erase(0, pathArg.argument.size());
                        if (!arg.empty() && arg.at(0) == arg_char('='))
                        { // use path after =
                            arg.erase(0, 1);
                            pathArg.path = arg;
                        }
                        else
                        { // use next argument as path
                            if (i == (args.size() - 1))
                            {
                                ShowUsage();
                                return 1;
                            }
                            pathArg.path = args.at(i + 1);
                            args.erase(args.begin() + i + 1);
                        }
                        if (!std::filesystem::is_directory(pathArg.path))
                        {
                            ShowUsage();
                            return 1;
                        }
                        arg.clear();
                    }
                }
                if (!arg.empty())
                {
                    fprintf(stderr, "unrecognized option: --" arg_format, arg.c_str());
                    return 1;
                }
            }
        }

        // when all options have been parsed,
        // strip the arg and continue
        if (arg.empty())
        {
            args.erase(args.begin() + i);
            i -= 1;
        }
    }

    // validate options
    if (hasUpdateOption && hasNeededOption)
    {
        fprintf(stderr, "--needed cannot be combined with --update-needed!");
        return 1;
    }

    // apply options
    UI::SetVerboseMode(hasVerboseOption);
    UI::SetNoInputMode(hasNoInputOption);
    Path::SetDirectories(coreLibsPath, modLibsPath, ep1Path, dataPath);
    if (hasSavePathsOption)
    {
        if (!SporeMod::Xml::SaveDirectories(coreLibsPath, modLibsPath, ep1Path, dataPath))
        {
            fprintf(stderr, "SporeMod::Xml::SaveDirectories() Failed!");
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

        std::vector<std::filesystem::path> paths(args.begin() + 2, args.end());

        if (hasUpdateOption)
        {
            if (!SporeModManager::UpdateMods(paths, false))
            {
                return 1;
            }
        }
        else
        {
            if (!SporeModManager::InstallMods(paths, false, hasNeededOption))
            {
                return 1;
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

        std::vector<std::filesystem::path> paths(args.begin() + 2, args.end());

        if (!SporeModManager::UpdateMods(paths, false))
        {
            return 1;
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

        // we support 2 types to parse the IDs,
        // either a range (i.e '0-3') or the IDs specified (i.e '0 1 2 3')
        if (args.size() == 3 && args.at(2).find(arg_char('-')) != arg_str_type::npos)
        {
            int startIndex = 0;
            int endIndex   = 0;
            std::vector<arg_str_type> splitString = String::Split(args.at(2), arg_char('-'));
            if (splitString.size() != 2)
            {
                fprintf(stderr, "range can only contain 2 positive numbers!");
                return 1;
            }

            try
            {
                startIndex = std::stoi(splitString.at(0));
                endIndex   = std::stoi(splitString.at(1));
            }
            catch (...)
            {
                fprintf(stderr, "range can only contain numbers!");
                return 1;
            }

            // validate start and end index
            if (startIndex == endIndex)
            {
                fprintf(stderr, "start number cannot be equal to the end number!");
                return 1;
            }
            else if (startIndex > endIndex)
            {
                fprintf(stderr, "start number must be less than the end number!");
                return 1;
            }

            for (int i = startIndex; i <= endIndex; i++)
            {
                ids.push_back(i);
            }
        }
        else
        {
            for (size_t i = 2; i < args.size(); i++)
            {
                try
                {
                    ids.push_back(std::stoi(args[i]));
                }
                catch (...)
                {
                    fprintf(stderr, "ID must be a number!");
                    return 1;
                }
            }
        }

        if (!SporeModManager::UninstallMods(ids))
        {
            return 1;
        }
    }
    else if (command == arg_str("version"))
    {
        fprintf(stdout, "SporeModManager %s\n", VERSION_STR);
        return 0;
    }
    else
    {
        ShowUsage();
        return 1;
    }

    return 0;
}
