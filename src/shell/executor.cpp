#include "executor.hpp"
#include "command.hpp"
#include "utils.hpp"
#include "builtins.hpp"
#include "history.hpp"
#include "path.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


void execute_command(const Command& cmd) {
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
    } else {
      perror("open error file");
      exit(EXIT_FAILURE);
    }
  }

  if (!cmd.args.empty()) {
    std::string command = cmd.args[0];

    if (command == "exit" && cmd.args.size() == 2 && cmd.args[1] == "0") {
      save_history_on_exit();
      exit(0);
    } else if (command == "echo") {
      execute_echo(cmd);
    } else if (command == "type") {
      execute_type(cmd);
    } else if (command == "pwd") {
      execute_pwd();
    } else if (command == "cd") {
      execute_cd(cmd);
    } else if (command == "history") {
      handle_history_command(cmd);
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

void execute_pipeline(const std::vector<Command>& commands) {
  if (commands.size() == 1) {
    if (is_builtin(commands[0].args[0])) {
      execute_command(commands[0]);
    } else {
      pid_t pid = fork();
      if (pid == 0) {
        execute_command(commands[0]);
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

      execute_command(commands[i]);
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