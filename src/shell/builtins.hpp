#pragma once

#include "command.hpp"

#include <vector>
#include <string>

extern const std::vector<std::string> BUILTIN_COMMANDS;

bool is_builtin(const std::string& command);
void execute_echo(const Command& cmd);
void execute_type(const Command& cmd);
void execute_pwd();
void execute_cd(const Command& cmd);