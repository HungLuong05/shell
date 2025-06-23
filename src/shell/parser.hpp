#pragma once

#include "command.hpp"

#include <vector>
#include <string>

std::vector<std::string> parseInput(const std::string& input);
std::vector<Command> parseCommand(std::vector<std::string>& tokens);