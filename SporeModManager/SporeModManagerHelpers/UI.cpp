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

#include <cstdio>
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
    char inputBuffer[256] = {0};
    std::string inputLine;

    do
    {
        fprintf(stdout, "%s", text.c_str());

        if (l_NoInputMode)
        {
            fprintf(stdout, "\n");
            number = defaultNumber.has_value() ? defaultNumber.value() : 0;
            return;
        }

        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        inputLine = inputBuffer;
        if ((inputLine.empty() || inputLine == "\n") && defaultNumber.has_value())
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
            fprintf(stderr, "invalid input\n");
        }
    } while (true);
}

void UI::AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, std::vector<int> defaultNumbers, int min, int max)
{
    char inputBuffer[256] = {0};
    std::string       inputLine;
    std::stringstream stringStream;
    std::string       inputItem;
    int  number;
    bool validInput = false;
    do
    {
        numbers.clear();

        fprintf(stdout, "%s", text.c_str());

        if (l_NoInputMode)
        {
            fprintf(stdout, "\n");
            numbers = defaultNumbers;
            return;
        }

        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        inputLine = inputBuffer;
        if (inputLine.empty() || inputLine == "\n")
        {
            numbers = defaultNumbers;
            return;
        }
        else if (String::Lowercase(inputLine) == "n\n")
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
                fprintf(stderr, "invalid input\n");
                validInput = false;
                break;
            }
        }        
    } while (!validInput);
}

void UI::AskUserInput(std::string text, bool& boolValue, bool defaultValue)
{
    char inputBuffer[256] = {0};
    std::string inputLine;
    bool validInput = false;
    do
    {
        fprintf(stdout, "%s", text.c_str());

        if (l_NoInputMode)
        {
            fprintf(stdout, "\n");
            boolValue = true;
            return;
        }

        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        inputLine = inputBuffer;
        inputLine = String::Lowercase(inputLine);
        if (inputLine.empty() || inputLine == "\n")
        {
            boolValue = defaultValue;
            validInput = true;
        }
        else if (inputLine == "y\n" ||
            inputLine == "n\n")
        {
            boolValue = (inputLine == "y\n");
            validInput = true;
        }
    } while (!validInput);
}

