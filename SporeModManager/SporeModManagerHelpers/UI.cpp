/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SporeModManagerHelpers.hpp"

#include <iostream>
#include <sstream>

using namespace SporeModManagerHelpers;

//
// Local Variables
//

static bool l_VerboseMode = false;
static bool l_NoInputMode = false;

//
// Exported Functions
//

void UI::SetVerboseMode(bool value)
{
    l_VerboseMode = value;
}

bool UI::GetVerboseMode(void)
{
    return l_VerboseMode;
}

void UI::SetNoInputMode(bool value)
{
    l_NoInputMode = value;
}

void UI::AskUserInput(std::string text, int& number, std::optional<int> defaultNumber, int min, int max)
{
    std::string inputLine;

    do
    {
        std::cout << text;

        if (l_NoInputMode)
        {
            std::cout << std::endl;
            number = defaultNumber.has_value() ? defaultNumber.value() : 0;
            return;
        }

        std::getline(std::cin, inputLine);
        if (inputLine.empty() && defaultNumber.has_value())
        {
            number = defaultNumber.value();
            return;
        }

        try
        {
            number = std::stoi(inputLine);
            if (number < min || number > max)
            {
                throw std::exception();
            }
            break;
        }
        catch (...)
        {
            std::cerr << "invalid input" << std::endl;
        }
    } while (true);
}

void UI::AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, std::vector<int> defaultNumbers, int min, int max)
{
    std::string       inputLine;
    std::stringstream stringStream;
    std::string       inputItem;
    int  number;
    bool validInput = false;
    do
    {
        numbers.clear();

        std::cout << text;

        if (l_NoInputMode)
        {
            std::cout << std::endl;
            numbers = defaultNumbers;
            return;
        }

        std::getline(std::cin, inputLine);
        if (inputLine.empty())
        {
            numbers = defaultNumbers;
            return;
        }
        else if (String::Lowercase(inputLine) == "n")
        {
            return;
        }

        stringStream = std::stringstream(inputLine);
        while (std::getline(stringStream, inputItem, delimiter))
        {
            try
            {
                number = std::stoi(inputItem);
                if (number < min || number > max)
                {
                    throw std::exception();
                }
                else
                {
                    numbers.push_back(number);
                    validInput = true;
                }
            }
            catch (...)
            {
                std::cout << "invalid input" << std::endl;
                validInput = false;
                break;
            }
        }        
    } while (!validInput);
}

void UI::AskUserInput(std::string text, bool& boolValue, bool defaultValue)
{
    std::string inputLine;
    bool validInput = false;
    do
    {
        std::cout << text;

        if (l_NoInputMode)
        {
            std::cout << std::endl;
            boolValue = true;
            return;
        }

        std::getline(std::cin, inputLine);
        inputLine = String::Lowercase(inputLine);
        if (inputLine.empty())
        {
            boolValue = defaultValue;
            validInput = true;
        }
        else if (inputLine == "y" ||
            inputLine == "n")
        {
            boolValue = (inputLine == "y");
            validInput = true;
        }
    } while (!validInput);
}

