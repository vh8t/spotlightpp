#pragma once

#include <string>
#include <vector>

#include "config.hpp"

struct App {
  std::string desktop_file;
  std::string name;
  std::string path;
  bool terminal;

  static std::vector<std::string> get_files(void);
  static std::vector<App> filter(const std::vector<App> &apps,
                                 const std::string &name);
  static App get_app(const std::string &path);
  void open(const Config &config, const std::vector<std::string> &args);
};
