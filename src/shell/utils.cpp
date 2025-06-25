#include "utils.hpp"

#include <filesystem>

bool create_directories_for_file(const std::string& filepath) {
  size_t last_slash = filepath.find_last_of("/");
  if (last_slash == std::string::npos) return true;

  std::string dir = filepath.substr(0, last_slash);
  try {
    std::filesystem::create_directories(dir);
    return true;
  } catch (const std::filesystem::filesystem_error& e) {
    return false;
  }
}