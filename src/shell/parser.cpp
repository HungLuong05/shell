#include "parser.hpp"

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