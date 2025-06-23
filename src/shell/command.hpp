#pragma once

#include <vector>
#include <string>

struct Command {
  std::vector<std::string> args;
  std::string output_file;
  bool has_output_redirect = false;
  bool output_append = false;
  std::string error_file;
  bool has_error_redirect = false;
  bool error_append = false;
};