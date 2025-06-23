// #include "path.hpp"

// #include <algorithm>
// #include <sys/stat.h>
// #include <sstream>
// #include <filesystem>

// std::vector<std::string> PATH_COMMANDS;

// bool is_executable(const std::string& path) {
//   struct stat st;
//   return (stat(path.c_str(), &st) == 0) && (st.st_mode & S_IXUSR);
// }

// std::string find_in_path(const std::string& command) {
//   const char* path_env = getenv("PATH");
//   if (!path_env) return "";

//   std::string path(path_env);
//   std::istringstream iss(path);
//   std::string dir;

//   while (std::getline(iss, dir, ':')) {
//     if (dir.empty()) continue;

//     std::string full_path = dir + "/" + command;
//     if (is_executable(full_path)) {
//       return full_path;
//     }
//   }

//   return "";
// }

// std::vector<std::string> get_path_commands() {
//   std::vector<std::string> commands;
//   const char* path_env = getenv("PATH");
//   if (!path_env) return commands;

//   std::string path(path_env);
//   std::istringstream iss(path);
//   std::string dir;

//   while (std::getline(iss, dir, ':')) {
//     if (dir.empty()) continue;

//     try {
//       for (const auto& entry : std::filesystem::directory_iterator(dir)) {
//         if (entry.is_regular_file() && is_executable(entry.path().string())) {
//           commands.push_back(entry.path().filename().string());
//         }
//       }
//     } catch (const std::filesystem::filesystem_error& e) {
//       // Ignore errors related to directory access for now
//     }
//   }

//   std::sort(commands.begin(), commands.end());
//   commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
//   return commands;
// }