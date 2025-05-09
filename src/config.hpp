#pragma once

#include <raylib.h>
#include <string>

#define BG_GRAY {30, 30, 30, 255}

enum class Position {
  TOP_LEFT,
  TOP,
  TOP_RIGHT,

  LEFT,
  CENTER,
  RIGHT,

  BOTTOM_LEFT,
  BOTTOM,
  BOTTOM_RIGHT,
};

struct Config {
  int font_size = 20;
  std::string font_name = "";

  int prompt_height = 40;
  int prompt_width = 700;
  int prompt_max_elements = 7;
  Position prompt_position = Position::CENTER;
  int prompt_precision = 6;
  bool builtins = true;
  bool auto_eval = true;
  float radius = 0.5f;

  std::string window_title = "spotlight++";
  std::string quit_on = "";
  int target_fps = 120;
  int x_padding = 50;
  int y_padding = 50;
  Position position = Position::CENTER;

  Color bg1 = BG_GRAY;
  Color bg2 = DARKGRAY;

  Color fg1 = WHITE;
  Color fg2 = LIGHTGRAY;
  Color fg3 = DARKGRAY;
};

Config load_config();
