#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <filesystem>

struct Command {
  std::vector<std::string> args;
  std::string output_file;
  bool has_output_redirect = false;
};

std::vector<std::string> parseInput(const std::string& input) {
  std::vector<std::string> args;
  std::string current_arg;
  bool in_single_quotes = false;
  bool in_double_quotes = false;

  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];

    if (c == '\'' && !in_double_quotes) {
      in_single_quotes = !in_single_quotes;
    } else if (c == '"' && !in_single_quotes) {
      in_double_quotes = !in_double_quotes;
    } else if (c == '\\' && in_double_quotes && i + 1 < input.length()) {
      char nxt = input[i + 1];
      if (nxt == '\\' || nxt == '$' || nxt == '"' || nxt == '\n') {
        current_arg += nxt;
        i++;
      } else {
        current_arg += c;
      }
    } else if (c == '\\' && !in_double_quotes && !in_single_quotes && i + 1 < input.length()) {
      char nxt = input[i + 1];
      current_arg += nxt;
      i++;
    } else if (c == ' ' && !in_single_quotes && !in_double_quotes) {
      if (!current_arg.empty()) {
        args.push_back(current_arg);
        current_arg.clear();
      }
    } else {
      current_arg += c;
    }
  }

  if (!current_arg.empty()) {
    args.push_back(current_arg);
  }

  return args;
}

Command parseCommand(std::vector<std::string>& tokens) {
  Command cmd;

  for (size_t i = 0; i < tokens.size(); i++) {
    const std::string& token = tokens[i];

    if (token == ">" || token == "1>") {
      if (i + 1 < tokens.size()) {
        cmd.output_file = tokens[i + 1];
        cmd.has_output_redirect = true;
        i++;
      }
    } else {
      cmd.args.push_back(token);
    }
  }

  return cmd;
}

const std::vector<std::string> BUILTIN_COMMANDS = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd"
};

bool is_builtin(const std::string& command) {
    return std::find(BUILTIN_COMMANDS.begin(), BUILTIN_COMMANDS.end(), command) 
           != BUILTIN_COMMANDS.end();
}

bool is_executable(const std::string& path) {
  struct stat st;
  return (stat(path.c_str(), &st) == 0) && (st.st_mode & S_IXUSR);
}

std::string find_in_path(const std::string& command) {
  const char* path_env = getenv("PATH");
  if (!path_env) return "";

  std::string path(path_env);
  std::istringstream iss(path);
  std::string dir;

  while (std::getline(iss, dir, ':')) {
    if (dir.empty()) continue;

    std::string full_path = dir + "/" + command;
    if (is_executable(full_path)) {
      return full_path;
    }
  }

  return "";
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string input;
  while (true) {
    std::cout << "$ ";
    std::getline(std::cin, input);

    std::vector<std::string> tokens = parseInput(input);
    Command cmd = parseCommand(tokens);

    std::ofstream output_file;
    std::ostream& output = cmd.has_output_redirect ? 
                          (output_file.open(cmd.output_file), output_file) : 
                          std::cout;

    if (!tokens.empty()) {
      std::string command = cmd.args[0];

      if (command == "exit" && cmd.args.size() == 2 && cmd.args[1] == "0") {
        break;
      } else if (command == "echo") {
        if (cmd.args.size() == 1) {
          output << "\n";
        } else {
          for (size_t i = 1; i < cmd.args.size(); i++) {
            output << cmd.args[i] << " ";
          }
          output << "\n";
        }
      } else if (command == "type") {
        if (cmd.args.size() == 2 && is_builtin(cmd.args[1])) {
          output << cmd.args[1] << " is a shell builtin\n";
        } else {
          std::string path = find_in_path(cmd.args[1]);
          if (!path.empty()) {
            output << cmd.args[1] << " is " << path << "\n";
          } else {
            output << cmd.args[1] << ": not found\n";
          }
        }
      } else if (command == "pwd") {
        output << std::filesystem::current_path().string() << "\n";
      } else if (command == "cd") {
        std::string target_dir = cmd.args[1];
        if (cmd.args[1] == "~") {
          target_dir = getenv("HOME");
        }

        try {
          std::filesystem::current_path(target_dir);
        } catch (const std::filesystem::filesystem_error& e) {
          std::cerr << "cd: " << cmd.args[1] << ": No such file or directory \n";
        }
      } else {
        std::string path = find_in_path(command);
        if (!path.empty()) {
          int res = system(input.c_str());
        } else {
          output << command << ": command not found\n";
        }
      }
    }
  }
}
