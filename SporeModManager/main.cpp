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
#include <iostream>

using namespace SporeModManagerHelpers;

//
// Local Defines
//

#ifdef _WIN32
#define arg_str(str) L##str
#define arg_char(c) L##c
#define arg_str_type std::wstring
#define arg_cerr wcerr
#else
#define arg_str(str) str
#define arg_char(c) c
#define arg_str_type std::string
#define arg_cerr cerr
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
              << "  version             display version and exit"   << std::endl
              << "  help                display this help and exit" << std::endl
              << std::endl
              << "Options: " << std::endl
              << "  -v, --verbose       shows verbose output" << std::endl
              << "  -y, --no-input      disables user input during installation of mods" << std::endl
              << "  -n, --needed        only install mod when mod isn't installed" << std::endl
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
                    std::arg_cerr << arg_str("Error: unrecognized option: -") << arg << std::endl;
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
                    std::arg_cerr << arg_str("Error: unrecognized option: --") << arg << std::endl;
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
        std::cerr << "Error: --needed cannot be combined with --update-needed!" << std::endl;
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
            std::cerr << "Error: SporeMod::Xml::SaveDirectories() Failed!" << std::endl;
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
    else if (command == arg_str("version"))
    {
        std::cout << "SporeModManager " << VERSION_STR << std::endl;
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

        if (!SporeModManager::UpdateMods(paths))
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
                std::cerr << "Error: range can only contain 2 positive numbers!" << std::endl;
                return 1;
            }

            try
            {
                startIndex = std::stoi(splitString.at(0));
                endIndex   = std::stoi(splitString.at(1));
            }
            catch (...)
            {
                std::cerr << "Error: range can only contain numbers!" << std::endl;
                return 1;
            }

            // validate start and end index
            if (startIndex == endIndex)
            {
                std::cerr << "Error: start number cannot be equal to the end number!" << std::endl;
                return 1;
            }
            else if (startIndex > endIndex)
            {
                std::cerr << "Error: start number must be less than the end number!" << std::endl;
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
                    std::cerr << "Error: ID must be a number!" << std::endl;
                    return 1;
                }
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
