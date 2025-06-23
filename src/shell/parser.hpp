#pragma once

#include "command.hpp"

#include <vector>
#include <string>

std::vector<std::string> parse_input(const std::string& input);
std::vector<Command> parse_command(std::vector<std::string>& tokens);