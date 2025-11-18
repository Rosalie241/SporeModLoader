/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_UI_HPP
#define SPOREMODMANAGERHELPERS_UI_HPP

#include <vector>
#include <string>
#include <optional>

namespace SporeModManagerHelpers
{
    namespace UI
    {
        /// <summary>
        ///     Sets verbose mode
        /// </summary>
        void SetVerboseMode(bool value);

        /// <summary>
        ///     Gets verbose mode
        /// </summary>
        bool GetVerboseMode(void);

        /// <summary>
        ///     Sets no input mode
        /// </summary>
        void SetNoInputMode(bool value);

        /// <summary>
        ///     Asks user for an integer
        /// </summary>
        void AskUserInput(std::string text, int& number, std::optional<int> defaultNumber, int min, int max);

        /// <summary>
        ///     Asks user for an integers
        /// </summary>
        void AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, 
                          const std::vector<int>& defaultNumbers, int min, int max);

        /// <summary>
        ///     Asks user for a boolean
        /// </summary>
        void AskUserInput(std::string text, bool& boolValue, bool defaultValue);
    }
}

#endif // SPOREMODMANAGERHELPERS_UI_HPP