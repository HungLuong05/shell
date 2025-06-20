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
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/history.h>

struct Command {
  std::vector<std::string> args;
  std::string output_file;
  bool has_output_redirect = false;
  bool output_append = false;
  std::string error_file;
  bool has_error_redirect = false;
  bool error_append = false;
};

const std::vector<std::string> BUILTIN_COMMANDS = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd",
  "history",
};

int next_history_position = 0;

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

std::vector<Command> parseCommand(std::vector<std::string>& tokens) {
  std::vector<Command> commands;
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
    } else if (token == "|") {
      if (!cmd.args.empty()) {
        commands.push_back(cmd);
        cmd = Command();
      }
    } else {
      cmd.args.push_back(token);
    }
  }

  if (!cmd.args.empty()) {
    commands.push_back(cmd);
  }

  return commands;
}



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

bool create_directories_for_file(const std::string& filepath) {
  size_t last_slash = filepath.find_last_of("/");
  if (last_slash == std::string::npos) return true; // No directory

  std::string dir = filepath.substr(0, last_slash);
  try {
    std::filesystem::create_directories(dir);
    return true;
  } catch (const std::filesystem::filesystem_error& e) {
    return false;
  }
}

void executeCommand(const Command& cmd) {
  int stdout_backup = -1, stderr_backup = -1;

  if (cmd.has_output_redirect) {
    if (!create_directories_for_file(cmd.output_file)) {
      std::cerr << "Error creating directories for output file: " << cmd.output_file << "\n";
      return;
    }

    stdout_backup = dup(STDOUT_FILENO);
    int flags = O_WRONLY | O_CREAT;
    flags |= cmd.output_append ? O_APPEND : O_TRUNC;

    int fd = open(cmd.output_file.c_str(), flags, 0644);
    if (fd != -1) {
      dup2(fd, STDOUT_FILENO);
      close(fd);
    } else {
      perror("open output file");
      exit(EXIT_FAILURE);
    }
  }

  if (cmd.has_error_redirect) {
    if (!create_directories_for_file(cmd.error_file)) {
      std::cerr << "Error creating directories for error file: " << cmd.error_file << "\n";
      return;
    }

    stderr_backup = dup(STDERR_FILENO);
    int flags = O_WRONLY | O_CREAT;
    flags |= cmd.error_append ? O_APPEND : O_TRUNC;

    int fd = open(cmd.error_file.c_str(), flags, 0644);
    if (fd != -1) {
      dup2(fd, STDERR_FILENO);
      close(fd);
    }
  }

  if (!cmd.args.empty()) {
    std::string command = cmd.args[0];

    if (command == "exit" && cmd.args.size() == 2 && cmd.args[1] == "0") {
      const char* histfile = getenv("HISTFILE");
      if (histfile) {
        write_history(histfile);
      }
      exit(0);
    } else if (command == "echo") {
      for (size_t i = 1; i < cmd.args.size(); i++) {
        std::cout << cmd.args[i];
        if (i < cmd.args.size() - 1) std::cout << " ";
      }
      std::cout << "\n";
    } else if (command == "type") {
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
    } else if (command == "pwd") {
      std::cout << std::filesystem::current_path().string() << "\n";
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
    } else if (command == "history") {
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
    } else {
      std::string path = find_in_path(command);
      if (!path.empty()) {
        std::vector<char*> c_args;
        for (const auto& arg : cmd.args) {
          c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(path.c_str(), c_args.data());
      } else {
        std::cerr << command << ": command not found\n";
      }
    }
  }

  if (stdout_backup != -1) {
    dup2(stdout_backup, STDOUT_FILENO);
    close(stdout_backup);
  }
  if (stderr_backup != -1) {
    dup2(stderr_backup, STDERR_FILENO);
    close(stderr_backup);
  }
}

void executePipeline(const std::vector<Command>& commands) {
  if (commands.size() == 1) {
    if (is_builtin(commands[0].args[0])) {
      executeCommand(commands[0]);
    } else {
      pid_t pid = fork();
      if (pid == 0) {
        executeCommand(commands[0]);
        exit(0);
      } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
      } else {
        perror("fork failed");
      }
    }
    
    
    return;
  }

  std::vector<int> pipes;

  for (size_t i = 0; i < commands.size() - 1; i++) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      perror("pipe");
      return;
    }
    pipes.push_back(pipefd[0]);
    pipes.push_back(pipefd[1]);
  }

  for (size_t i = 0; i < commands.size(); i++) {
    pid_t pid = fork();
    if (pid == 0) {
      if (i > 0) {
        dup2(pipes[(i - 1) * 2], STDIN_FILENO);
      }

      if (i < commands.size() - 1) {
        dup2(pipes[i * 2 + 1], STDOUT_FILENO);
      }

      for (int fd: pipes) {
        close(fd);
      }

      executeCommand(commands[i]);
      exit(0);
    }
  }

  for (size_t i = 0; i < pipes.size(); i++) {
    close(pipes[i]);
  }

  for (size_t i = 0; i < commands.size(); i++) {
    wait(nullptr);
  }
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  using_history();

  const char* histfile_env = getenv("HISTFILE");
  if (histfile_env) {
    std::string histfile(histfile_env);
    read_history(histfile.c_str());
    next_history_position = history_length;
  }

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
    std::vector<Command> commands = parseCommand(tokens);

    if (!commands.empty()) {
      add_history(input.c_str());
      executePipeline(commands);
    }
  }

  const char* histfile = getenv("HISTFILE");
  if (histfile) {
    write_history(histfile);
  }
  return 0;
}
