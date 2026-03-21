#include <cstdlib>
#include <filesystem>
#include <print>
#include <raylib.h>
#include <string>
#include <toml++/toml.hpp>

#include "config.hpp"

namespace fs = std::filesystem;

Config Config::load(void) {
  const char *home = std::getenv("HOME");
  if (!home)
    return Config{};

  fs::path config_path =
      fs::path(home) / ".config" / "spotlightpp" / "config.toml";
  if (!fs::exists(config_path))
    return Config{};

  toml::table data;
  try {
    data = toml::parse_file(config_path.string());
  } catch (const toml::parse_error &err) {
    std::println(stderr, "Failed to parse '{}': {}", config_path.string(),
                 err.what());
    return Config{};
  }

  Config config{
      .font =
          {
              .size = data["font"]["size"].value_or(20u),
              .name = data["font"]["name"].value_or(""),
          },

      .prompt =
          {
              .height = data["prompt"]["height"].value_or(40u),
              .width = data["prompt"]["width"].value_or(700u),
              .max_elements = data["prompt"]["max-elements"].value_or(7u),
              .position =
                  position_from_str(data["promtp"]["position"].value_or("top")),
              .precision = data["prompt"]["precision"].value_or(6u),
              .radius = data["prompt"]["radius"].value_or(0.5f),
              .builtins = data["prompt"]["builtins"].value_or(true),
              .auto_eval = data["prompt"]["auto-eval"].value_or(true),
          },

      .window = {.title = data["window"]["title"].value_or("spotlight++"),
                 .quit_on = data["window"]["quit-on"].value_or(""),
                 .target_fps = data["window"]["fps"].value_or(120u),
                 .x_padding = data["window"]["pad-x"].value_or(50u),
                 .y_padding = data["window"]["pad-y"].value_or(50u),
                 .position = position_from_str(
                     data["window"]["position"].value_or("center"))},

      .theme =
          {
              .bg1 = GetColor(
                  data["theme"]["bg1"].value_or(ColorToInt(BG_DARK_GRAY))),
              .bg2 = GetColor(
                  data["theme"]["bg2"].value_or(ColorToInt(BG_LIGHT_GRAY))),

              .fg1 =
                  GetColor(data["theme"]["fg1"].value_or(ColorToInt(FG_WHITE))),
              .fg2 = GetColor(
                  data["theme"]["fg2"].value_or(ColorToInt(FG_LIGHT_GRAY))),
              .fg3 = GetColor(
                  data["theme"]["fg3"].value_or(ColorToInt(FG_DARK_GRAY))),
          },
  };

  return config;
}
