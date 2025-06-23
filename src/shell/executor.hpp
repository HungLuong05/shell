#pragma once

#include <string>
#include <vector>

#include "command.hpp"

void execute_command(const Command& cmd);
void execute_pipeline(const std::vector<Command>& commands);