#define TE_POW_FROM_RIGHT
#define PADDING_LEFT 14

#include "apps.hpp"
#include "cli.hpp"
#include "config.hpp"
#include "tinyexpr.h"
#include "utils.hpp"

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <httplib.h>
#include <iomanip>
#include <raylib.h>
#include <sstream>
#include <sys/wait.h>

namespace fs = std::filesystem;

bool running = true;
Config config;
Font custom_font;

void DrawTextB(const char *text, int posX, int posY, int fontSize,
               Color color) {
  if (config.font_name.empty()) {
    DrawText(text, posX, posY, fontSize, color);
  } else {
    DrawTextEx(custom_font, text,
               {static_cast<float>(posX), static_cast<float>(posY)}, fontSize,
               0, color);
  }
}

int MeasureTextB(const char *text, int fontSize) {
  if (config.font_name.empty()) {
    return MeasureText(text, fontSize);
  } else {
    return MeasureTextEx(custom_font, text, fontSize, 0).x;
  }
}

void app_open(const std::string &name, const std::vector<std::string> &args) {
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Failed to fork process " << strerror(errno) << std::endl;
    return;
  }

  if (pid == 0) {
    std::vector<const char *> c_args = {name.c_str()};

    for (const auto &arg : args) {
      c_args.push_back(arg.c_str());
    }
    c_args.push_back(nullptr);

    execvp(c_args[0], const_cast<char *const *>(c_args.data()));

    std::cerr << "Failed to execute " << name << ": " << strerror(errno)
              << std::endl;
    exit(1);
  } else {
    int status;
    waitpid(pid, &status, WNOHANG);
  }
  return;
}

void run_builtin(const std::string &builtin, const std::string &buffer) {
  if (builtin == "--open-in-browser--") {
    if (is_url(buffer)) {
      app_open("xdg-open", {strip(buffer)});
    } else {
      std::string url = "https://www.google.com/search?q=" +
                        httplib::detail::encode_url(strip(buffer));
      app_open("xdg-open", {url});
    }
  } else if (builtin == "--quit--") {
    running = false;
  }
}

bool rate_limit_key(int &counter) {
  counter++;
  if (counter > (config.target_fps / 4)) {
    if (counter % (config.target_fps / 16) == 0) {
      return true;
    }
  } else if (counter == 1) {
    return true;
  }

  return false;
}

int main(int argc, const char **argv) {
  config = load_config();

  if (!parse_and_load(argc, argv, &config)) {
    return 0;
  }

  std::vector<std::string> app_files = get_app_files();
  std::vector<App> apps, filtered;

  for (const auto &app_file : app_files) {
    apps.push_back(get_app(app_file));
  }

  if (config.builtins) {
    apps.push_back(
        {"--spotlight++-builtin--", "Open in browser", "--open-in-browser--"});
    apps.push_back({"--spotlight++-builtin--", "Quit", "--quit--"});
  }

  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_BORDERLESS_WINDOWED_MODE |
                 FLAG_WINDOW_TRANSPARENT);
  InitWindow(config.prompt_width, config.prompt_height,
             config.window_title.c_str());
  SetWindowState(FLAG_WINDOW_UNDECORATED);

  int width = GetMonitorWidth(0);
  int height = GetMonitorHeight(0);

  int frame_counter = 0;
  int posX = 0;
  int posY = 0;

  switch (config.position) {
  case Position::TOP_LEFT: {
    posX = config.x_padding;
    posY = config.y_padding;
  } break;
  case Position::TOP: {
    posX = (width - config.prompt_width) / 2;
    posY = config.y_padding;
  } break;
  case Position::TOP_RIGHT: {
    posX = width - config.prompt_width - config.x_padding;
    posY = config.y_padding;
  } break;
  case Position::LEFT: {
    posX = config.x_padding;
    posY = (height - config.prompt_height) / 2;
  } break;
  case Position::CENTER: {
    posX = (width - config.prompt_width) / 2;
    posY = (height - config.prompt_height) / 2;
  } break;
  case Position::RIGHT: {
    posX = width - config.prompt_width - config.x_padding;
    posY = (height - config.prompt_height) / 2;
  } break;
  case Position::BOTTOM_LEFT: {
    posX = config.x_padding;
    posY = height - config.prompt_height - config.y_padding;
  } break;
  case Position::BOTTOM: {
    posX = (width - config.prompt_width) / 2;
    posY = height - config.prompt_height - config.y_padding;
  } break;
  case Position::BOTTOM_RIGHT: {
    posX = width - config.prompt_width - config.x_padding;
    posY = height - config.prompt_height - config.y_padding;
  } break;
  }

  SetWindowPosition(posX, posY);
  SetTargetFPS(config.target_fps);

  if (!config.font_name.empty()) {
    const char *home = std::getenv("HOME");
    if (!home) {
      std::cerr << "Failed to get the HOME path" << std::endl;
      config.font_name = "";
    } else {
      fs::path font_path = fs::path(home) / ".config" / "spotlightpp" /
                           "fonts" / (config.font_name + ".ttf");

      if (fs::exists(font_path)) {
        custom_font =
            LoadFontEx(font_path.c_str(), config.font_size, nullptr, 0);
      } else {
        std::cerr << "Failed to load font from: " << font_path << std::endl;
        config.font_name = "";
      }
    }
  }

  double result = NAN;
  double ans = 0.0f;
  bool change = false;
  bool show_eq = true;
  int elements = 0;
  int selected = 0;
  int offset = 0;

  std::string buffer;

  size_t index = 0;

  int backspace_counter = 0;
  int right_counter = 0;
  int left_counter = 0;

  te_variable vars[] = {{"ans", &ans}};
  Rectangle prompt = {0, 0, static_cast<float>(config.prompt_width),
                      static_cast<float>(config.prompt_height)};

  while (!WindowShouldClose() && running) {
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 125) {
        if (index == buffer.size()) {
          buffer.push_back(key);
        } else {
          buffer.insert(index, 1, key);
        }
        change = true;
        index++;
      }

      key = GetCharPressed();
    }

    if (IsKeyDown(KEY_BACKSPACE) && !buffer.empty() && index != 0) {
      if (rate_limit_key(backspace_counter)) {
        if (index == buffer.size()) {
          buffer.pop_back();
        } else {
          buffer.erase(index - 1, 1);
        }
        change = true;
        index--;
      }
    }

    if (IsKeyReleased(KEY_BACKSPACE)) {
      backspace_counter = 0;
    }

    if (IsKeyDown(KEY_LEFT)) {
      if (rate_limit_key(left_counter)) {
        if (index > 0) {
          index--;
        }
      }
    }

    if (IsKeyReleased(KEY_LEFT)) {
      left_counter = 0;
    }

    if (IsKeyDown(KEY_RIGHT)) {
      if (rate_limit_key(right_counter)) {
        if (index < buffer.size()) {
          index++;
        }
      }
    }

    if (IsKeyReleased(KEY_RIGHT)) {
      right_counter = 0;
    }

    if (IsKeyPressed(KEY_ENTER) && !buffer.empty()) {
      if (config.quit_on == buffer) {
        running = false;
      } else if (!std::isnan(result) &&
                 (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        ans = result;
        change = true;
      } else {
        if (filtered.empty()) {
          if (buffer[0] == '>') {
            run_builtin("--open-in-browser--", strip(buffer.substr(1)));
            running = false;
          }
        } else {
          App app = filtered[offset + selected];

          if (is_builtin(app)) {
            run_builtin(app.app_path, buffer);
          } else {
            app_open(app.app_path, {});
          }
          running = false;
        }
      }
    }

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP) ||
        IsKeyPressed(KEY_DOWN)) {
      if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ||
          IsKeyDown(KEY_UP)) {
        if (selected <= 0) {
          offset--;
          if (offset < 0) {
            offset = filtered.size() - elements;
            selected = elements - 1;
          }
        } else {
          selected--;
        }
      } else {
        if (selected >= elements - 1) {
          offset++;
          if (offset + selected >= static_cast<int>(filtered.size())) {
            selected = 0;
            offset = 0;
          }
        } else {
          selected++;
        }
      }
    }

    BeginDrawing();

    ClearBackground(BLANK);
    // ClearBackground(config.bg1);

    int prompt_y = 12;
    if (config.prompt_position == Position::BOTTOM) {
      prompt_y = 10 + elements * (config.font_size + 10);
      if (filtered.size() > 0)
        prompt_y += 10;
    }

    DrawRectangleRounded(prompt,
                         config.radius / (prompt.height / config.prompt_height),
                         0.0f, config.bg1);

    int buf_width =
        MeasureTextB(buffer.substr(0, index).c_str(), config.font_size);
    int buf_x = PADDING_LEFT;

    if (buffer.empty()) {
      DrawTextB("Type to search...", PADDING_LEFT, prompt_y, config.font_size,
                config.fg3);
    } else {
      if (config.prompt_width - PADDING_LEFT * 2 < buf_width) {
        buf_x = config.prompt_width - PADDING_LEFT - buf_width;
      }
      DrawTextB(buffer.c_str(), buf_x, prompt_y, config.font_size, config.fg1);
    }

    if (frame_counter / (config.target_fps / 2) % 2 == 0) {
      int carret_x = buf_x + buf_width + 1;

      DrawLineEx(
          {static_cast<float>(carret_x), static_cast<float>(prompt_y + 2)},
          {static_cast<float>(carret_x),
           static_cast<float>(prompt_y + config.font_size - 2)},
          1, config.fg2);
    }

    if (change) {
      if (buffer.empty()) {
        filtered.clear();
        elements = 0;
      } else {
        if (!buffer.empty() && buffer[0] == '>') {
          filtered.clear();
          elements = 0;
        } else {
          filtered = filter_apps(apps, buffer);
          elements = std::min<int>(filtered.size(), config.prompt_max_elements);
        }
      }

      int height = config.prompt_height;
      if (elements > 0) {
        height = config.prompt_height + 10 + elements * (config.font_size + 10);
        prompt.height = height;
        SetWindowSize(config.prompt_width, height);
      } else {
        prompt.height = config.prompt_height;
        SetWindowSize(config.prompt_width, config.prompt_height);
      }

      if (config.prompt_position == Position::BOTTOM) {
        SetWindowPosition(posX, posY - height + config.prompt_height);
      }

      selected = 0;
      offset = 0;
    }

    for (int i = 0; i < elements; i++) {
      int y_offset = 40;
      if (config.prompt_position == Position::BOTTOM) {
        y_offset = 0;
      }

      if (i == selected) {
        Rectangle rec = {
            6.0f,
            static_cast<float>(12 + y_offset + (config.font_size + 10) * i - 4),
            static_cast<float>(config.prompt_width - 12),
            config.font_size + 6.0f};
        DrawRectangleRounded(rec, config.radius, 0.0f, config.bg2);

        DrawTextB(filtered[i + offset].app_name.c_str(), 14,
                  12 + y_offset + (config.font_size + 10) * i, config.font_size,
                  config.fg1);
      } else {
        DrawTextB(filtered[i + offset].app_name.c_str(), 14,
                  12 + y_offset + (config.font_size + 10) * i, config.font_size,
                  config.fg3);
      }
    }

    if (!std::isnan(result) && (config.auto_eval || !show_eq)) {
      std::ostringstream oss;
      oss << std::setprecision(config.prompt_precision) << result;
      std::string res = oss.str();
      if (res.find('.') != std::string::npos) {
        res.erase(res.find_last_not_of('0') + 1, std::string::npos);
        if (res.back() == '.') {
          res.pop_back();
        }
      }

      if (show_eq)
        res = "= " + res;

      int x =
          PADDING_LEFT + 20 + MeasureTextB(buffer.c_str(), config.font_size);
      Rectangle rec = {
          static_cast<float>(x - 9), static_cast<float>(prompt_y - 4),
          static_cast<float>(MeasureTextB(res.c_str(), config.font_size) + 18),
          config.font_size + 6.0f};

      DrawRectangleRounded(rec, config.radius, 0.0f, config.bg2);
      DrawTextB(res.c_str(), x, prompt_y, config.font_size, config.fg2);
    }

    if (change) {
      auto parts = split(buffer, '=');
      if (parts.size() > 2 || parts.size() == 0) {
        result = NAN;
      } else {
        if (buffer.find('=') != std::string::npos)
          show_eq = false;
        else
          show_eq = true;

        te_expr *expr = te_compile(parts[0].c_str(), vars, 1, 0);

        if (expr) {
          result = te_eval(expr);
          te_free(expr);
        } else {
          result = NAN;
        }
      }
    }

    EndDrawing();

    frame_counter = (frame_counter + 1) % config.target_fps;
    change = false;
  }

  CloseWindow();
}
