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

using namespace SporeModManagerHelpers;

void UI::AskUserInput(std::string text, int& number, int min, int max)
{
    do
    {
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore();
        }

        std::cout << text;
        std::cin >> number;

        if (std::cin.fail() || number < min || number > max)
        {
            std::cerr << "invalid input" << std::endl;
            continue;
        }
        else
        {
            std::cin.clear();
            std::cin.ignore();
            break;
        }
    } while (true);
}

void UI::AskUserInput(std::string text, char delimiter, std::vector<int>& numbers, int min, int max)
{
    std::string inputLine;
    std::stringstream stringStream;
    std::string inputItem;
    int number;
    bool validInput = false;
    do
    {
        numbers.clear();

        std::cout << text;

        std::getline(std::cin, inputLine);
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

