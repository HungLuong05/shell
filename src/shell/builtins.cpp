// #include "builtins.hpp"
// #include "path.hpp"

// #include <algorithm>
// #include <iostream>
// #include <filesystem>

// const std::vector<std::string> BUILTIN_COMMANDS = {
//   "exit",
//   "echo",
//   "type",
//   "pwd",
//   "cd",
//   "history",
// };

// bool is_builtin(const std::string& command) {
//     return std::find(BUILTIN_COMMANDS.begin(), BUILTIN_COMMANDS.end(), command) 
//            != BUILTIN_COMMANDS.end();
// }

// void executeEcho(const std::vector<std::string>& args) {
//     for (size_t i = 1; i < args.size(); i++) {
//         std::cout << args[i];
//         if (i < args.size() - 1) std::cout << " ";
//     }
//     std::cout << "\n";
// }

// void executeType(const std::vector<std::string>& args) {
//     if (args.size() == 2 && is_builtin(args[1])) {
//         std::cout << args[1] << " is a shell builtin\n";
//     } else {
//         std::string path = find_in_path(args[1]);
//         if (!path.empty()) {
//             std::cout << args[1] << " is " << path << "\n";
//         } else {
//             std::cerr << args[1] << ": not found\n";
//         }
//     }
// }

// void executePwd() {
//     std::cout << std::filesystem::current_path().string() << "\n";
// }

// void executeCd(const std::vector<std::string>& args) {
//     std::string target_dir = args[1];
//     if (args[1] == "~") {
//         target_dir = getenv("HOME");
//     }

//     try {
//         std::filesystem::current_path(target_dir);
//     } catch (const std::filesystem::filesystem_error& e) {
//         std::cerr << "cd: " << args[1] << ": No such file or directory \n";
//     }
// }