#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <filesystem>
#include <readline/readline.h>
#include <cstring>

struct Command {
  std::vector<std::string> args;
  std::string output_file;
  bool has_output_redirect = false;
  bool output_append = false;
  std::string error_file;
  bool has_error_redirect = false;
  bool error_append = false;
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
        cmd.output_append = false;
        i++;
      }
    } else if (token == "2>") {
      if (i + 1 < tokens.size()) {
        cmd.error_file = tokens[i + 1];
        cmd.has_error_redirect = true;
        cmd.output_append = false;
        i++;
      }
    } else if (token == "1>>" || token == ">>") {
      if (i + 1 < tokens.size()) {
        cmd.output_file = tokens[i + 1];
        cmd.has_output_redirect = true;
        cmd.output_append = true;
        i++;
      }
    } else if (token == "2>>") {
      if (i + 1 < tokens.size()) {
        cmd.error_file = tokens[i + 1];
        cmd.has_error_redirect = true;
        cmd.error_append = true;
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

static std::vector<std::string> PATH_COMMANDS;

std::vector<std::string> get_path_commands() {
  std::vector<std::string> commands;
  const char* path_env = getenv("PATH");
  if (!path_env) return commands;

  std::string path(path_env);
  std::istringstream iss(path);
  std::string dir;

  while (std::getline(iss, dir, ':')) {
    if (dir.empty()) continue;

    try {
      for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && is_executable(entry.path().string())) {
          commands.push_back(entry.path().filename().string());
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      // Ignore errors in directory iteration
    }
  }

  std::sort(commands.begin(), commands.end());
  commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
  return commands;
}

char* command_generator(const char* text, int state) {
  static size_t builtin_index = 0, path_index = 0;
  static std::string current_text;

  if (state == 0) {
    builtin_index = 0;
    path_index = 0;
    current_text = text;
  }

  while (builtin_index < BUILTIN_COMMANDS.size()) {
    const std::string& arg = BUILTIN_COMMANDS[builtin_index++];
    if (arg.compare(0, current_text.size(), current_text) == 0) {
      return strdup(arg.c_str());
    }
  }

  while (path_index < PATH_COMMANDS.size()) {
    const std::string& arg = PATH_COMMANDS[path_index++];
    if (arg.compare(0, current_text.size(), current_text) == 0) {
      return strdup(arg.c_str());
    }
  }

  return nullptr;
}

char** command_completion(const char* text, int start, int end) {
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, command_generator);
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  rl_attempted_completion_function = command_completion;

  PATH_COMMANDS = get_path_commands();

  std::string input;
  while (true) {
    char* input_cstr = readline("$ ");

    if (!input_cstr) {
      std::cout << "\n";
      break;
    }

    std::string input(input_cstr);
    free(input_cstr);

    std::vector<std::string> tokens = parseInput(input);
    Command cmd = parseCommand(tokens);

    std::ofstream output_file;
    std::ostream& output = [&]() -> std::ostream& {
      if (cmd.has_output_redirect) {
        if (cmd.output_append) {
          output_file.open(cmd.output_file, std::ios::app);
        } else {
          output_file.open(cmd.output_file);
        }
        return output_file;
      }
      return std::cout;
    }();

    std::ofstream error_file;
    std::ostream& error_output = [&]() -> std::ostream& {
      if (cmd.has_error_redirect) {
        if (cmd.error_append) {
          error_file.open(cmd.error_file, std::ios::app);
        } else {
          error_file.open(cmd.error_file);
        }
        return error_file;
      }
      return std::cerr;
    }();

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
            error_output << cmd.args[1] << ": not found\n";
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
          error_output << "cd: " << cmd.args[1] << ": No such file or directory \n";
        }
      } else {
        std::string path = find_in_path(command);
        if (!path.empty()) {
          int res = system(input.c_str());
        } else {
          error_output << command << ": command not found\n";
        }
      }
    }
  }
}
