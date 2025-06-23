#include "completion.hpp"
#include "builtins.hpp"
#include "path.hpp"

#include <readline/readline.h>

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