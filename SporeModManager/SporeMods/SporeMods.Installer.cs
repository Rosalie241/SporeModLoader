/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using SporeModManager.SporeMods.Xml;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;

namespace SporeModManager.SporeMods
{
    internal partial class SporeMods
    {
        private static void InstallFileFromZip(ref ZipArchive zipArchive, InstalledModFile installedModFile)
        {
            ZipArchiveEntry zipArchiveEntry = null;

            zipArchiveEntry = zipArchive.Entries.FirstOrDefault(e => e.Name.ToLower() == installedModFile.FileName.ToLower());
            if (zipArchiveEntry == default)
            {
                throw new Exception($"SporeMod archive doesn't contain {installedModFile.FileName}!");
            }

            Console.WriteLine($"-> installing {installedModFile.FileName}");

            string directory = Directory.GetDirectoryFromInstallLocation(installedModFile.InstallLocation);

            using (var zipEntryStream = zipArchiveEntry.Open())
            {
                string outputFileName = Path.Combine(directory, installedModFile.FileName);
                var outputFileStream = File.OpenWrite(outputFileName);

                zipEntryStream.CopyTo(outputFileStream);
                outputFileStream.Close();
            }
        }

        private static void InstallSporeMod(ref InstalledMods installedMods, string filename)
        {
            FileStream fileStream = File.OpenRead(filename);
            ZipArchive zipArchive = new ZipArchive(fileStream);
            ZipArchiveEntry zipArchiveEntry = null;

            zipArchiveEntry = zipArchive.Entries.FirstOrDefault(e => e.Name.ToLower() == "modinfo.xml");
            if (zipArchiveEntry == default)
            {
                throw new Exception("SporeMod archive doesn't contain a modinfo.xml!");
            }

            var zipEntryStream = zipArchiveEntry.Open();   
            var modInfo = ModInfo.DeserializeFromStream(zipEntryStream);

            var installedMod = new InstalledMod
            {
                Name = modInfo.DisplayName,
                UniqueName = modInfo.UniqueName,
                Description = modInfo.Description,
                Files = new List<InstalledModFile>()
            };

            foreach (var componentGroup in modInfo.ComponentGroups)
            {
                ModInfoComponent component;
                Console.WriteLine($"-> {componentGroup.DisplayName}");

                for (int i = 0; i < componentGroup.Components.Count(); i++)
                {
                    component = componentGroup.Components[i];
                    Console.WriteLine($"[{i}] {component.DisplayName}");
                    if (!String.IsNullOrEmpty(component.Description.Trim()))
                    {
                        Console.WriteLine($"  {component.DisplayName}");
                    }
                }

                // request what component the user wants installed
                int id = -1;
                while (id == -1)
                {
                    Console.Write("-> select which component you want: ");
                    if (Int32.TryParse(Console.ReadLine(), out int userIndex))
                    {
                        if (userIndex >= 0 && userIndex < componentGroup.Components.Count())
                        {
                            id = userIndex;
                        }
                    }

                    if (id == -1)
                    {
                        Console.WriteLine("Invalid input, try again");
                    }
                }

                component = componentGroup.Components[id];

                if (component.FileNames.Count() != component.InstallLocations.Count())
                {
                    throw new Exception("component.FileNames.Count() != component.InstallLocations.Count()");
                }

                for (int i = 0; i < component.InstallLocations.Count(); i++)
                {
                    installedMod.Files.Add(new InstalledModFile()
                    {
                        FileName = component.FileNames[i],
                        InstallLocation = component.InstallLocations[i]
                    });
                }
            }

            if (modInfo.Components.Count() != 0)
            {
                for (int i = 0; i < modInfo.Components.Count(); i++)
                {
                    Console.WriteLine($"[{i}] {modInfo.Components[i].DisplayName}");
                    Console.WriteLine($"  {modInfo.Components[i].Description}");
                }

                // ask the user what components they want to install
                List<int> componentIds = new List<int>();
                bool userHasSelected = false;
                bool userHasInvalidSelection = false;
                while (!userHasSelected)
                {
                    Console.Write("-> select which components you want to install (comma seperated): ");
                    string userInputString = Console.ReadLine();
                    if (String.IsNullOrEmpty(userInputString))
                    {
                        break;
                    }

                    foreach (string num in userInputString.Split(','))
                    {
                        if (Int32.TryParse(num, out int userIndex))
                        {
                            if (userIndex >= 0 && userIndex < modInfo.Components.Count())
                            {
                                componentIds.Add(userIndex);
                            }
                            else
                            {
                                userHasInvalidSelection = true;
                            }
                        }
                    }

                    if (userHasInvalidSelection)
                    {
                        continue;
                    }
                    else
                    {
                        userHasSelected = true;
                    }
                }

                foreach (var componentId in componentIds)
                {
                    var component = modInfo.Components[componentId];

                    if (component.FileNames.Count() != component.InstallLocations.Count())
                    {
                        throw new Exception("component.FileNames.Count() != component.InstallLocations.Count()");
                    }

                    for (int i = 0; i < component.InstallLocations.Count(); i++)
                    {
                        installedMod.Files.Add(new InstalledModFile()
                        {
                            FileName = component.FileNames[i],
                            InstallLocation = component.InstallLocations[i]
                        });
                    }
                }
            }

            foreach (var prerequisite in modInfo.Prerequisites)
            {
                installedMod.Files.Add(new InstalledModFile()
                {
                    FileName = prerequisite.FileName,
                    InstallLocation = prerequisite.InstallLocation
                });
            }

            zipEntryStream.Close();

            int index = installedMods.InstalledModList.FindIndex(delegate (InstalledMod a) { return a.UniqueName == installedMod.UniqueName; });
            if (index != -1)
            {
                SporeMods.UninstallMod(index);
                installedMods.InstalledModList.RemoveAt(index);
                installedMods.InstalledModList.Insert(index, installedMod);
            }
            else
            {
                installedMods.InstalledModList.Add(installedMod);
            }

            foreach (var installedModFile in installedMod.Files)
            {
                InstallFileFromZip(ref zipArchive, installedModFile);
            }
        }

        private static void InstallPackageMod(ref InstalledMods installedMods, string filename)
        {
            string baseFileName = Path.GetFileName(filename);

            var installedMod = new InstalledMod()
            {
                Name = Path.GetFileNameWithoutExtension(filename),
                UniqueName = Path.GetFileNameWithoutExtension(filename),
                Files = new List<InstalledModFile>()
                {
                    new InstalledModFile()
                    {
                        InstallLocation = ModInfoInstallLocation.GalacticAdventuresData,
                        FileName = baseFileName
                    }
                }
            };

            Console.WriteLine($"-> installing {baseFileName}");

            File.Copy(filename, Path.Combine(Directory.GetGalacticAdventuresDataDirectory(), baseFileName), true);
            installedMods.InstalledModList.Add(installedMod);
        }

        public static void InstallMod(string filename)
        {
            var installedMods = InstalledMods.ReadFromXml();

            Console.WriteLine($"> installing {filename}");

            string extension = Path.GetExtension(filename).ToLower();
            if (extension == ".sporemod")
            {
                InstallSporeMod(ref installedMods, filename);
            } else if (extension == ".package")
            {
                InstallPackageMod(ref installedMods, filename);
            } else
            {
                throw new InvalidOperationException("mod type not supported!");
            }

            InstalledMods.SaveToXml(installedMods);

            Console.WriteLine($"> successfully installed {filename}");
        }
    }
}
