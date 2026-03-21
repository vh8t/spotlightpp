#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fontconfig/fontconfig.h>
#include <iomanip>
#include <print>
#include <raylib.h>
#include <sstream>
#include <unordered_set>

#include "cli.hpp"
#include "config.hpp"
#include "state.hpp"
#include "tinyexpr.h"
#include "utils.hpp"

State::State(int argc, const char **argv) {
  config = Config::load();
  if (!Cli::load(argc, argv, config))
    std::exit(0);

  std::vector<std::string> app_files = App::get_files();
  std::unordered_set<std::string> seen_apps;

  for (const auto &app_file : app_files) {
    App app = App::get_app(app_file);

    std::string name_key = app.app_name;
    if (name_key.empty() || app.app_path.empty() || seen_apps.count(name_key))
      continue;

    seen_apps.insert(name_key);
    apps.push_back(app);
  }

  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_BORDERLESS_WINDOWED_MODE |
                 FLAG_WINDOW_TRANSPARENT);
  InitWindow(config.prompt.width, config.prompt.height,
             config.window.title.c_str());
  SetWindowState(FLAG_WINDOW_UNDECORATED);

  int monitor = GetCurrentMonitor();
  width = GetMonitorWidth(monitor);
  height = GetMonitorHeight(monitor);

  frame_counter = 0;
  posX = 0;
  posY = 0;

  switch (config.window.position) {
  case Position::TOP:
    posX = (width - config.prompt.width) / 2;
    posY = config.window.y_padding;
    break;

  case Position::LEFT:
    posX = config.window.x_padding;
    posY = (height - config.prompt.height) / 2;
    break;

  case Position::CENTER:
    posX = (width - config.prompt.width) / 2;
    posY = (height - config.prompt.height) / 2;
    break;

  case Position::RIGHT:
    posX = width - config.prompt.width - config.window.x_padding;
    posY = (height - config.prompt.height) / 2;
    break;

  case Position::BOTTOM:
    posX = (width - config.prompt.width) / 2;
    posY = height - config.prompt.height - config.window.y_padding;
    break;
  }

  SetWindowPosition(posX, posY);

  if (!config.font.name.empty()) {
    if (!FcInit()) {
      std::println(stderr, "Failed to initialized fontconfig");
      error = true;
      return;
    }

    FcPattern *pattern = FcNameParse((const FcChar8 *)config.font.name.c_str());
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern *matched = FcFontMatch(nullptr, pattern, &result);

    std::string font_path;
    if (matched) {
      FcChar8 *file = nullptr;
      if (FcPatternGetString(matched, FC_FILE, 0, &file) == FcResultMatch)
        font_path = reinterpret_cast<char *>(file);
      FcPatternDestroy(matched);
    }

    FcPatternDestroy(pattern);
    FcFini();

    font = LoadFontEx(font_path.c_str(), config.font.size, nullptr, 0);
    custom_font = true;
  }

  prompt = {0, 0, static_cast<float>(config.prompt.width),
            static_cast<float>(config.prompt.height)};
}

void State::draw_text(const char *text, int posX, int posY, Color color) {
  if (custom_font)
    DrawTextEx(font, text,
               {
                   static_cast<float>(posX),
                   static_cast<float>(posY),
               },
               config.font.size, 0, color);
  else
    DrawText(text, posX, posY, config.font.size, color);
}

int State::measure_text(const char *text) {
  if (custom_font)
    return MeasureTextEx(font, text, config.font.size, 0).x;
  else
    return MeasureText(text, config.font.size);
}

bool State::rate_limit(int &counter) {
  counter++;
  if (counter > config.window.target_fps / 4) {
    if (counter % (config.window.target_fps / 16) == 0)
      return true;
  } else if (counter == 1)
    return true;
  return false;
}

void State::handle_events(void) {
  int key = GetCharPressed();
  while (key > 0) {
    if (32 <= key && key <= 125) {
      if (index == buffer.size())
        buffer.push_back(key);
      else
        buffer.insert(index, 1, key);
      change = true;
      index++;
    }

    key = GetCharPressed();
  }

  if (IsKeyDown(KEY_BACKSPACE) && !buffer.empty() && index != 0) {
    if (rate_limit(backspace_counter)) {
      if (index == buffer.size())
        buffer.pop_back();
      else
        buffer.erase(index - 1, 1);
      change = true;
      index--;
    }
  }

  if (IsKeyReleased(KEY_BACKSPACE))
    backspace_counter = 0;

  if (IsKeyDown(KEY_LEFT))
    if (rate_limit(left_counter))
      if (index > 0)
        index--;

  if (IsKeyReleased(KEY_LEFT))
    left_counter = 0;

  if (IsKeyDown(KEY_RIGHT))
    if (rate_limit(right_counter))
      if (index < buffer.size())
        index++;

  if (IsKeyReleased(KEY_RIGHT))
    right_counter = 0;

  if (IsKeyPressed(KEY_ENTER) && !buffer.empty()) {
    if (buffer == config.window.quit_on)
      running = false;
    else if (!std::isnan(result) &&
             (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
      ans = result;
      change = true;
    } else {
      App app = filtered[offset + selected];
      app.open({});
      running = false;
    }
  }

  if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ||
        IsKeyDown(KEY_UP)) {
      if (selected <= 0) {
        offset--;
        if (offset < 0) {
          offset = filtered.size() - elements;
          selected = elements - 1;
        }
      } else
        selected--;
    } else {
      if (selected >= elements - 1) {
        offset++;
        if (offset + selected >= static_cast<int>(filtered.size())) {
          selected = 0;
          offset = 0;
        }
      } else
        selected++;
    }
  }

  if (change) {
    if (buffer.empty() || buffer[0] == '>') {
      filtered.clear();
      elements = 0;
    } else {
      filtered = App::filter(apps, buffer);
      elements = std::min<int>(filtered.size(), config.prompt.max_elements);
    }

    int height = config.prompt.height;
    if (elements > 0)
      height += 10 + elements * (config.font.size + 10);

    prompt.height = height;
    SetWindowSize(config.prompt.width, height);
    if (config.prompt.position == Position::BOTTOM)
      SetWindowPosition(posX, posY - height + config.prompt.height);

    selected = 0;
    offset = 0;
  }
}

void State::draw_frame(void) {
#define PADDING_LEFT 14

  BeginDrawing();
  ClearBackground(BLANK);

  int prompt_y = 10;
  if (config.prompt.position == Position::BOTTOM) {
    prompt_y = 10 + elements * (config.font.size + 10);
    if (filtered.size() > 0)
      prompt_y += 10;
  }

  DrawRectangleRounded(
      prompt, config.prompt.radius / (prompt.height / config.prompt.height),
      0.f, config.theme.bg1);

  int buf_width = measure_text(buffer.substr(0, index).c_str());
  int buf_x = PADDING_LEFT;

  if (buffer.empty())
    draw_text("Type to search...", PADDING_LEFT, prompt_y, config.theme.fg3);
  else {
    if (config.prompt.width - PADDING_LEFT * 2 < buf_width)
      buf_x = config.prompt.width - PADDING_LEFT - buf_width;
    draw_text(buffer.c_str(), buf_x, prompt_y, config.theme.fg1);
  }

  if (frame_counter / (config.window.target_fps / 2) % 2 == 0) {
    int carret_x = buf_x + buf_width + 1;
    DrawLineEx(
        {
            static_cast<float>(carret_x),
            static_cast<float>(prompt_y + 2),
        },
        {
            static_cast<float>(carret_x),
            static_cast<float>(prompt_y + config.font.size - 2),
        },
        1, config.theme.fg2);
  }

  for (int i = 0; i < elements; i++) {
    int y_offset = 40;
    if (config.prompt.position == Position::BOTTOM)
      y_offset = 0;

    if (i == selected) {
      Rectangle rect{
          6.f,
          static_cast<float>(12 + y_offset + (config.font.size + 10) * i - 4),
          static_cast<float>(config.prompt.width - 12),
          config.font.size + 6.f,
      };

      DrawRectangleRounded(rect, config.prompt.radius, 0.f, config.theme.bg2);
      draw_text(filtered[i + offset].app_name.c_str(), 14,
                12 + y_offset + (config.font.size + 10) * i, config.theme.fg1);
    } else
      draw_text(filtered[i + offset].app_name.c_str(), 14,
                12 + y_offset + (config.font.size + 10) * i, config.theme.fg3);
  }

  if (!std::isnan(result) && (config.prompt.auto_eval || !show_eq)) {
    std::ostringstream oss;
    oss << std::setprecision(config.prompt.precision) << result;
    std::string result = oss.str();

    if (result.find('.') != std::string::npos) {
      result.erase(result.find_last_not_of('0') + 1, std::string::npos);
      if (result.back() == '.')
        result.pop_back();
    }

    if (show_eq)
      result = "= " + result;

    int x = PADDING_LEFT + 20 + measure_text(buffer.c_str());
    Rectangle rect{
        static_cast<float>(x - 9),
        static_cast<float>(prompt_y - 4),
        static_cast<float>(measure_text(result.c_str()) + 18),
        config.font.size + 6.0f,
    };

    DrawRectangleRounded(rect, config.prompt.radius, 0.0f, config.theme.bg2);
    draw_text(result.c_str(), x, prompt_y, config.theme.fg2);
  }

#undef PADDING_LEFT
}

void State::render_frame(void) {
  if (change) {
    auto parts = split(buffer, '=');
    if (parts.size() > 2 || parts.size() == 0)
      result = NAN;
    else {
      if (buffer.find('=') != std::string::npos)
        show_eq = false;
      else
        show_eq = true;

      te_expr *expr = te_compile(parts[0].c_str(), nullptr, 0, 0);
      if (expr) {
        result = te_eval(expr);
        te_free(expr);
      } else
        result = NAN;
    }
  }

  EndDrawing();

  frame_counter = (frame_counter + 1) % config.window.target_fps;
  change = false;
}
