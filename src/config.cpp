#include "config.hpp"
#include "utils.hpp"

#include <filesystem>
#include <iostream>
#include <raylib.h>
#include <string>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

Config load_config() {
  fs::path config_path;

  const char *home = std::getenv("HOME");
  if (home) {
    config_path = fs::path(home) / ".config" / "spotlightpp" / "config.toml";
  } else {
    config_path = fs::path(get_base_dir()) / "config.toml";
  }

  if (!fs::exists(config_path)) {
    return Config();
  }

  try {
    toml::table data = toml::parse_file(config_path.string());
    Config config;

    config.font_size = data["font"]["size"].value_or(20);

    config.prompt_height = data["prompt"]["height"].value_or(40);
    config.prompt_width = data["prompt"]["width"].value_or(700);
    config.prompt_max_elements = data["prompt"]["max-elements"].value_or(7);

    config.window_title = data["window"]["title"].value_or("spotlight++");
    config.target_fps = data["window"]["fps"].value_or(120);

    config.bg1 = GetColor(data["theme"]["bg1"].value_or(ColorToInt(BG_GRAY)));
    config.bg2 = GetColor(data["theme"]["bg2"].value_or(ColorToInt(DARKGRAY)));

    config.fg1 = GetColor(data["theme"]["fg1"].value_or(ColorToInt(WHITE)));
    config.fg2 = GetColor(data["theme"]["fg2"].value_or(ColorToInt(LIGHTGRAY)));
    config.fg3 = GetColor(data["theme"]["fg3"].value_or(ColorToInt(DARKGRAY)));

    config.builtins = data["builtins"].value_or(true);

    return config;
  } catch (const toml::parse_error &err) {
    std::cout << "Parsing failed: " << err << std::endl;
    return Config();
  }
}
