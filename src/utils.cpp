#include "utils.hpp"

#include <filesystem>

namespace fs = std::filesystem;

bool is_url(const std::string &string) {
  return string.compare(0, 7, "http://") == 0 ||
         string.compare(0, 8, "https://") == 0;
}

bool is_builtin(const App &app) {
  if (app.desktop_file != "--spotlight++-builtin--")
    return false;

  if (app.app_path.size() < 2)
    return false;

  return std::equal(app.app_path.begin(), app.app_path.begin() + 2, "--") &&
         std::equal(app.app_path.end() - 2, app.app_path.end(), "--");
}

std::string trunc(const std::string &str) {
  std::string result;
  bool was_empty = false;
  for (const char ch : str) {
    if (was_empty && ch != ' ') {
      result.push_back(' ');
      result.push_back(ch);
      was_empty = false;
    } else if (ch == ' ') {
      was_empty = true;
    } else {
      result.push_back(ch);
    }
  }

  return result;
}

std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;

  while (std::getline(ss, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

std::string strip(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

std::string get_base_dir() {
  std::string exe_dir = fs::canonical("/proc/self/exe").parent_path().string();
  return fs::path(exe_dir).parent_path().string();
}
