#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

std::vector<std::string> parseInput(const std::string& input) {
  std::vector<std::string> tokens;
  std::istringstream iss(input);
  std::string token;

  while (iss >> token) {
    tokens.push_back(token);
  }

  return tokens;
}

const std::vector<std::string> BUILTIN_COMMANDS = {
  "exit",
  "echo",
  "type"
};

bool is_builtin(const std::string& command) {
    return std::find(BUILTIN_COMMANDS.begin(), BUILTIN_COMMANDS.end(), command) 
           != BUILTIN_COMMANDS.end();
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

    if (!tokens.empty()) {
      std::string command = tokens[0];
      std::vector<std::string> args(tokens.begin() + 1, tokens.end());

      if (command == "exit" && args.size() == 1 && args[0] == "0") {
        break;
      } else if (command == "echo") {
        if (args.empty()) {
          std::cout << "\n";
        } else {
          for (const auto& arg : args) {
            std::cout << arg << " ";
          }
          std::cout << "\n";
        }
      } else if (command == "type") {
        if (args.size() == 1 && is_builtin(args[0])) {
          std::cout << args[0] << " is a shell builtin\n";
        } else {
          std::cout << args[0] << ": not found\n";
        }
      } else {
        std::cout << input << ": command not found\n";
      }
    }
  }
}
