#pragma once

#include <string>
#include <vector>

struct App {
  std::string desktop_file;
  std::string app_name;
  std::string app_path;

  static std::vector<std::string> get_files(void);
  static std::vector<App> filter(const std::vector<App> &apps,
                                 const std::string &name);
  static App get_app(const std::string &path);
  void open(const std::vector<std::string> &args);
};
