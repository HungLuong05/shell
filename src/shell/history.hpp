#pragma once

#include "command.hpp"

#include <string>
#include <vector>

void initialize_history();
void save_history_on_exit();
void handle_history_command(const Command& cmd);

extern int next_history_position;