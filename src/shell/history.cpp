#include "history.hpp"

#include <readline/history.h>
#include <iostream>
#include <fstream>

int next_history_position = 0;

void initialize_history() {
  using_history();

  const char* histfile_env = getenv("HISTFILE");
  if (histfile_env) {
    std::string histfile(histfile_env);
    read_history(histfile.c_str());
    next_history_position = history_length;
  }
}

void save_history_on_exit() {
  const char* histfile = getenv("HISTFILE");
  if (histfile) {
    write_history(histfile);
  }
}

void handle_history_command(const Command& cmd) {
  HIST_ENTRY **history_commands = history_list();
  if (cmd.args.size() == 1) {
    for (int i = 0; history_commands[i]; i++) {
      std::cout << "    " << i + 1 << "  " << history_commands[i]->line << "\n";
    }
  } else {
    if (cmd.args[1] == "-r") {
      std::string history_file = cmd.args[2];
      std::ifstream file(history_file);

      if (!file) {
        std::cerr << "history: " << history_file << ": No such file or directory\n";
        return;
      }

      std::string line;
      while (std::getline(file, line)) {
        add_history(line.c_str());
      }
      file.close();
    } else if (cmd.args[1] == "-w") {
      std::string history_file = cmd.args[2];
      std::ofstream file(history_file);

      if (!file) {
        std::cerr << "history: " << history_file << ": Could not open file for writing\n";
        return;
      }

      HIST_ENTRY **history_commands = history_list();
      for (int i = 0; history_commands[i]; i++) {
        file << history_commands[i]->line << "\n";
      }
      file.close();
    } else if (cmd.args[1] == "-a") {
      std::string history_file = cmd.args[2];
      std::ofstream file(history_file, std::ios::app);

      if (!file) {
        std::cerr << "history: " << history_file << ": Could not open file for appending\n";
        return;
      }

      HIST_ENTRY **history_commands = history_list();
      for (int i = next_history_position; history_commands[i]; i++) {
        file << history_commands[i]->line << "\n";
      }
      file.close();

      next_history_position = history_length;
    } else {
      int cnt = std::stoi(cmd.args[1]);
      int total = history_length;
      int start = std::max(0, total - cnt);
      for (int i = start; i < total; i++) {
        if (history_commands[i]) {
          std::cout << "    " << i + 1 << "  " << history_commands[i]->line << "\n";
        }
      }
    }
  }
}