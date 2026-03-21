#pragma once

#include <cmath>
#include <raylib.h>
#include <string>
#include <vector>

#include "apps.hpp"
#include "config.hpp"

struct State {
  Config config;
  std::vector<App> apps, filtered;

  int width;
  int height;
  int frame_counter;
  int posX, posY;

  Font font;

  double result = NAN;
  double ans = 0.0f;

  int elements = 0;
  int selected = 0;
  int offset = 0;

  std::string buffer = "";
  Rectangle prompt;

  size_t index = 0;

  bool running = true;
  bool change = false;
  bool show_eq = true;
  bool custom_font = false;
  bool error = false;

  int backspace_counter = 0;
  int right_counter = 0;
  int left_counter = 0;

  State(int argc, const char **argv);

  bool rate_limit(int &counter);

  void draw_text(const char *text, int posX, int posY, Color color);
  int measure_text(const char *text);

  void handle_events(void);
  void draw_frame(void);
  void render_frame(void);

  operator bool() {
    return !error;
  }
};
