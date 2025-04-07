#pragma once

#include <string>
#include <vector>

struct App {
  std::string desktop_file;
  std::string app_name;
  std::string app_path;
};

std::vector<std::string> get_app_files();
App get_app(const std::string &path);
std::vector<App> filter_apps(const std::vector<App> &apps,
                             const std::string &name);
