#include <iostream>

#include <readline/readline.h>
#include <readline/history.h>

#include "parser.hpp"
#include "path.hpp"
#include "completion.hpp"
#include "history.hpp"
#include "executor.hpp"

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  initialize_history();

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

    std::vector<std::string> tokens = parse_input(input);
    std::vector<Command> commands = parse_command(tokens);

    if (!commands.empty()) {
      add_history(input.c_str());
      execute_pipeline(commands);
    }
  }

  save_history_on_exit();
  return 0;
}
