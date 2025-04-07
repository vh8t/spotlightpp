#pragma once

#include <raylib.h>
#include <string>

#define BG_GRAY {30, 30, 30, 255}

struct Config {
  int font_size = 20;

  int prompt_height = 40;
  int prompt_width = 700;
  int prompt_max_elements = 7;

  std::string window_title = "spotlight++";
  int target_fps = 120;

  Color bg1 = BG_GRAY;
  Color bg2 = DARKGRAY;

  Color fg1 = WHITE;
  Color fg2 = LIGHTGRAY;
  Color fg3 = DARKGRAY;

  bool builtins = true;
};

Config load_config();
