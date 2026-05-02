#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "apps.hpp"
#include "config.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;

std::vector<std::string> App::get_files(void) {
  std::vector<std::string> apps;
  const fs::path app_path = fs::path("/") / "usr" / "share" / "applications";
  std::println("[DEBUG] app_path: {}", app_path.string());

  if (fs::is_directory(app_path)) {
    try {
      for (const auto &entry : fs::directory_iterator(app_path))
        if (entry.is_regular_file() && entry.path().extension() == ".desktop")
          apps.push_back(entry.path().string());
    } catch (const std::exception &err) {
      std::println(stderr, "Filesystem error: {}", err.what());
    }
  }

  const char *home = std::getenv("HOME");
  if (home) {
    fs::path path = fs::path(home) / ".local" / "share" / "applications";
    try {
      for (const auto &entry : fs::directory_iterator(path))
        if (entry.is_regular_file() && entry.path().extension() == ".desktop")
          apps.push_back(entry.path().string());
    } catch (const std::exception &err) {
      std::println(stderr, "Filesystem error: {}", err.what());
    }
  }

  std::sort(apps.begin(), apps.end());

  return apps;
}

std::vector<App> App::filter(const std::vector<App> &apps,
                             const std::string &name) {
  std::vector<App> result;
  std::string new_name = trunc(name);

  std::copy_if(
      apps.begin(), apps.end(), std::back_inserter(result),
      [&new_name](const App &app) {
        auto it = std::search(
            app.name.begin(), app.name.end(), new_name.begin(), new_name.end(),
            [](char a, char b) { return std::tolower(a) == std::tolower(b); });
        return it != app.name.end();
      });

  return result;
}

App App::get_app(const std::string &path) {
  App app;

  std::ifstream file{path};
  std::string line;
  bool in_desktop_entry = false;

  while (std::getline(file, line)) {
    std::string original = line;

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

    size_t delimiter = original.find("=");
    if (delimiter == std::string::npos)
      continue;

    std::string key = original.substr(0, delimiter);
    std::string value = original.substr(delimiter + 1);

    if (key == "Name") {
      app.name = strip(value);
    } else if (key == "Exec") {
      size_t d = value.find(' ');
      if (d != std::string::npos)
        value = value.substr(0, d);
      app.path = strip(value);
    } else if (key == "Terminal") {
      app.terminal = value == "true";
    }
  }

  return app;
}

void App::open(const Config &config, const std::vector<std::string> &args) {
  pid_t pid = fork();
  if (pid < 0) {
    std::println(stderr, "Failed to fork process: {}", strerror(errno));
    return;
  }

  if (pid == 0) {
    setsid();
    pid_t grandchild = fork();
    if (grandchild < 0)
      _exit(1);
    if (grandchild > 0)
      _exit(0);

    int dev_null = ::open("/dev/null", O_RDWR);
    if (dev_null != -1) {
      dup2(dev_null, STDIN_FILENO);
      dup2(dev_null, STDOUT_FILENO);
      dup2(dev_null, STDERR_FILENO);
      if (dev_null > 2)
        ::close(dev_null);
    }

    std::vector<const char *> c_args = {};
    std::string full_cmd = path;

    if (terminal) {
      c_args.push_back(config.prompt.terminal.c_str());
      c_args.push_back("-e");

      for (const auto &arg : args)
        full_cmd += " " + arg;

      c_args.push_back(full_cmd.c_str());
    } else {
      c_args.push_back(path.c_str());
      for (const auto &arg : args)
        c_args.push_back(arg.c_str());
    }

    c_args.push_back(nullptr);

    execvp(c_args[0], const_cast<char *const *>(c_args.data()));

    std::println(stderr, "Failed to execute {}: {}", path, strerror(errno));
    _exit(1);
  }

  waitpid(pid, nullptr, 0);
}
