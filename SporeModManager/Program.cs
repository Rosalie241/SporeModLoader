/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System;
using System.Linq;

namespace SporeModManager
{
    internal class Program
    {
        static void ShowUsage()
        {
            Console.WriteLine("Usage: SporeModManager.exe [OPTION]\n");
            Console.WriteLine("Options:");
            Console.WriteLine("  -l, --list-installed      lists installed mods with IDs");
            Console.WriteLine("  -i, --install FILE        installs FILE");
            Console.WriteLine("  -u, --uninstall ID        uninstalls mod with ID\n");
            Console.WriteLine("      --help                display this help and exit");
        }

        static void Main(string[] args)
        {
            SporeMods.Directory.ReadDirectoryConfig();
            SporeMods.Directory.EnsureDirectoriesExist();

            if (args.Length == 0 || args[0] == "--help")
            {
                ShowUsage();
                return;
            }
            else if (args[0] == "-l" || args[0] == "--list-installed")
            {
                SporeMods.SporeMods.ListInstalledMods();
                return;
            }
            else if (args[0] == "-i" || args[0] == "--install")
            {
                if (args.Skip(1).Count() == 0)
                {
                    ShowUsage();
                    return;
                }

                foreach (string file in args.Skip(1))
                {
                    SporeMods.SporeMods.InstallMod(file);
                }
                return;
            }
            else if (args[0] == "-u" || args[0] == "--uninstall")
            {
                foreach (string id in args.Skip(1))
                {
                    SporeMods.SporeMods.UninstallMod(Int32.Parse(id));
                }

                return;
            }
            else
            {
                ShowUsage();
                return;
            }
        }
    }
}
