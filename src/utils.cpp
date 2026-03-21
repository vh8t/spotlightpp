#include <sstream>
#include <string>
#include <vector>

#include "utils.hpp"

std::string strip(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
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
  std::stringstream ss{str};
  std::string token;

  while (std::getline(ss, token, delimiter))
    if (!token.empty())
      tokens.push_back(token);

  return tokens;
}
