#pragma once

#include "command.hpp"

#include <vector>
#include <string>

extern const std::vector<std::string> BUILTIN_COMMANDS;

bool is_builtin(const std::string& command);
void executeEcho(const Command& cmd);
void executeType(const Command& cmd);
void executePwd();
void executeCd(const Command& cmd);