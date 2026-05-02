#pragma once

#include <raylib.h>
#include <string>

#define BG_DARK_GRAY Color{30, 30, 30, 255}
#define BG_LIGHT_GRAY Color{80, 80, 80, 255}

#define FG_WHITE Color{255, 255, 255, 255}
#define FG_DARK_GRAY Color{80, 80, 80, 255}
#define FG_LIGHT_GRAY Color{200, 200, 200, 255}

enum class Position {
  TOP,
  LEFT,
  CENTER,
  RIGHT,
  BOTTOM,
};

static Position position_from_str(const std::string &str) {
  if (str == "top")
    return Position::TOP;
  if (str == "left")
    return Position::LEFT;
  if (str == "center")
    return Position::CENTER;
  if (str == "right")
    return Position::RIGHT;
  if (str == "bottom")
    return Position::BOTTOM;
  return Position::TOP;
}

struct FontConfig {
  std::string name = "";
  unsigned int size = 20;
};

struct PromptConfig {
  std::string terminal = "xterm";
  unsigned int height = 40;
  unsigned int width = 700;
  unsigned int max_elements = 7;
  Position position = Position::TOP;
  unsigned int precision = 6;
  float radius = 0.5f;
  bool builtins = true;
  bool auto_eval = true;
};

struct WindowConfig {
  std::string title = "spotlight++";
  std::string quit_on = "";
  unsigned int target_fps = 120;
  unsigned int x_padding = 50;
  unsigned int y_padding = 50;
  Position position = Position::CENTER;
};

struct ThemeConfig {
  Color bg1 = BG_DARK_GRAY;
  Color bg2 = BG_LIGHT_GRAY;
  Color fg1 = FG_WHITE;
  Color fg2 = FG_LIGHT_GRAY;
  Color fg3 = FG_DARK_GRAY;
};

struct Config {
  FontConfig font;
  PromptConfig prompt;
  WindowConfig window;
  ThemeConfig theme;

  static Config load(void);
};
