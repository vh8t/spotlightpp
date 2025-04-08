#include "config.hpp"

#include <filesystem>
#include <iostream>
#include <raylib.h>
#include <string>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

Position str_to_pos(const std::string &str) {
  if (str == "top left") {
    return Position::TOP_LEFT;
  } else if (str == "top") {
    return Position::TOP;
  } else if (str == "top right") {
    return Position::TOP_RIGHT;
  } else if (str == "left") {
    return Position::LEFT;
  } else if (str == "center") {
    return Position::CENTER;
  } else if (str == "right") {
    return Position::RIGHT;
  } else if (str == "bottom left") {
    return Position::BOTTOM_LEFT;
  } else if (str == "bottom") {
    return Position::BOTTOM;
  } else if (str == "bottom right") {
    return Position::BOTTOM_RIGHT;
  }
  return Position::CENTER;
}

Config load_config() {
  const char *home = std::getenv("HOME");
  if (!home) {
    return Config();
  }

  fs::path config_path =
      fs::path(home) / ".config" / "spotlightpp" / "config.toml";

  if (!fs::exists(config_path)) {
    return Config();
  }

  try {
    toml::table data = toml::parse_file(config_path.string());
    Config config;

    config.font_size = data["font"]["size"].value_or(20);
    config.font_name = data["font"]["name"].value_or("");

    config.prompt_height = data["prompt"]["height"].value_or(40);
    config.prompt_width = data["prompt"]["width"].value_or(700);
    config.prompt_max_elements = data["prompt"]["max-elements"].value_or(7);
    config.prompt_precision = data["prompt"]["precision"].value_or(6);

    std::string prompt_position = data["prompt"]["position"].value_or("top");
    if (prompt_position == "bottom") {
      config.prompt_position = Position::BOTTOM;
    } else {
      config.prompt_position = Position::TOP;
    }

    config.prompt_precision =
        std::max(std::min(config.prompt_precision, 15), 0) + 1;

    config.window_title = data["window"]["title"].value_or("spotlight++");
    config.target_fps = data["window"]["fps"].value_or(120);
    config.x_padding = data["window"]["pad-x"].value_or(50);
    config.y_padding = data["window"]["pad-y"].value_or(50);

    std::string position = data["window"]["position"].value_or("center");
    config.position = str_to_pos(position);

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
