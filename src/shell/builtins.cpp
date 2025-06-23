#include "builtins.hpp"
#include "path.hpp"

#include <algorithm>
#include <iostream>
#include <filesystem>

const std::vector<std::string> BUILTIN_COMMANDS = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd",
  "history",
};

bool is_builtin(const std::string& command) {
    return std::find(BUILTIN_COMMANDS.begin(), BUILTIN_COMMANDS.end(), command) 
           != BUILTIN_COMMANDS.end();
}

void execute_echo(const Command& cmd) {
    for (size_t i = 1; i < cmd.args.size(); i++) {
        std::cout << cmd.args[i];
        if (i < cmd.args.size() - 1) std::cout << " ";
    }
    std::cout << "\n";
}

void execute_type(const Command& cmd) {
    if (cmd.args.size() == 2 && is_builtin(cmd.args[1])) {
        std::cout << cmd.args[1] << " is a shell builtin\n";
    } else {
        std::string path = find_in_path(cmd.args[1]);
        if (!path.empty()) {
            std::cout << cmd.args[1] << " is " << path << "\n";
        } else {
            std::cerr << cmd.args[1] << ": not found\n";
        }
    }
}

void execute_pwd() {
    std::cout << std::filesystem::current_path().string() << "\n";
}

void execute_cd(const Command& cmd) {
    std::string target_dir = cmd.args[1];
    if (cmd.args[1] == "~") {
        target_dir = getenv("HOME");
    }

    try {
        std::filesystem::current_path(target_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "cd: " << cmd.args[1] << ": No such file or directory \n";
    }
}