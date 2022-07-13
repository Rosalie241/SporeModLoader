/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.Collections.Generic;

namespace SporeModManager.SporeMods.Xml
{
    internal class ModInfoComponentGroup
    {
        public string DisplayName { get; set; }
        public string UniqueName { get; set; }
        public List<ModInfoComponent> Components { get; set; }
    }
}
