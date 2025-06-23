#pragma once

#include "command.hpp"

#include <string>
#include <vector>

void initialize_history();
// void load_history_from_file();
// void save_history_to_file();
void save_history_on_exit();
// void append_new_history_to_file(const std::string& filename);
void handle_history_command(const Command& cmd);

extern int next_history_position;