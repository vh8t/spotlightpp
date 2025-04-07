#pragma once

#include "apps.hpp"

#include <string>
#include <vector>

bool is_url(const std::string &string);
bool is_builtin(const App &app);
std::string trunc(const std::string &str);
std::vector<std::string> split(const std::string &str, char delimiter = ' ');
std::string strip(const std::string &str);
std::string get_base_dir();
