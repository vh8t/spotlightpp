#include "apps.hpp"
#include "utils.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

std::vector<std::string> get_app_files() {
  std::vector<std::string> apps;
  const std::string app_path = "/usr/share/applications";

  if (fs::is_directory(app_path)) {
    try {
      for (const auto &entry : fs::directory_iterator(app_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
          apps.push_back(entry.path().string());
        }
      }
    } catch (const std::exception &err) {
      std::cerr << "Filesystem error: " << err.what() << std::endl;
    }
  }

  const char *home = std::getenv("HOME");
  if (home) {
    fs::path path = fs::path(home) / ".local" / "share" / "applications";
    try {
      for (const auto &entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
          apps.push_back(entry.path().string());
        }
      }
    } catch (const std::exception &err) {
      std::cerr << "Filesystem error: " << err.what() << std::endl;
    }
  }

  std::sort(apps.begin(), apps.end());

  return apps;
}

App get_app(const std::string &path) {
  App app;

  std::ifstream file(path);
  std::string line;
  bool in_desktop_entry = false;

  while (std::getline(file, line)) {
    std::string orig = line;

    line = line.substr(0, line.find('#'));
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

    if (line.empty())
      continue;

    if (line[0] == '[' && line.back() == ']') {
      in_desktop_entry = (line == "[DesktopEntry]");
      continue;
    }

    if (!in_desktop_entry)
      continue;

    size_t delimiter = orig.find('=');
    if (delimiter == std::string::npos)
      continue;

    std::string key = orig.substr(0, delimiter);
    std::string value = orig.substr(delimiter + 1);

    if (key == "Name") {
      app.app_name = strip(value);
    } else if (key == "Exec") {
      size_t d = value.find(' ');
      if (d != std::string::npos)
        value = value.substr(0, d);

      app.app_path = strip(value);
    }
  }

  return app;
}

std::vector<App> filter_apps(const std::vector<App> &apps,
                             const std::string &name) {
  std::vector<App> result;
  std::string new_name = trunc(name);

  std::copy_if(apps.begin(), apps.end(), std::back_inserter(result),
               [&new_name](const App &app) {
                 auto it = std::search(
                     app.app_name.begin(), app.app_name.end(), new_name.begin(),
                     new_name.end(), [](char a, char b) {
                       return std::tolower(a) == std::tolower(b);
                     });
                 return it != app.app_name.end() || is_builtin(app);
               });

  return result;
}
