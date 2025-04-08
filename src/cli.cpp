#include "cli.hpp"
#include "argparse.hpp"

#include <iostream>

argparse::ArgumentParser program("spotlightpp", "0.0.2",
                                 argparse::default_arguments::none);

void print_help() {
  // clang-format off
  std::cout << "Usage: spotlight [OPTIONS]\n\n"

            << "Font Options:\n"
            << "  --font-name <str>      set font name\n"
            << "  --font-size <int>      set font size\n\n"

            << "Prompt Options:\n"
            << "  --prompt-height <int>  set prompt height\n"
            << "  --prompt-width <int>   set prompt width\n"
            << "  --max-elements <int>   set prompt max elements\n"
            << "  --precision <int>      set prompt precision\n\n"

            << "Prompt Position:\n"
            << "  --prompt-top\n"
            << "  --prompt-bottom\n\n"

            << "Window Options:\n"
            << "  --title <str>          set window title\n"
            << "  --fps <int>            set target fps\n"
            << "  --pad-x <int>          set padding on x axis\n"
            << "  --pad-y <int>          set padding on y axis\n\n"
            
            << "Window Position:\n"
            << "  --top-left\n"
            << "  --top\n"
            << "  --top-right\n"
            << "  --left\n"
            << "  --center\n"
            << "  --right\n"
            << "  --bottom-left\n"
            << "  --bottom\n"
            << "  --bottom-right\n\n"

            << "Theme Options:\n"
            << "  --bg1 <hex>            set background color\n"
            << "  --bg2 <hex>            set background highlight\n"
            << "  --fg1 <hex>            set primary text color\n"
            << "  --fg2 <hex>            set secondary text color\n"
            << "  --fg3 <hex>            set hint text color\n\n"

            << "General Options:\n"
            << "  --builtins             enable builtin functions\n"
            << "  --no-builtins          disable builtin funcstions\n\n"

            << "  -h, --help             show this message\n"
            << std::endl;
  // clang-format on
}

void create_argumenst() {
  program.add_argument("--font-name").default_value(std::string(""));
  program.add_argument("--font-size").scan<'i', int>();

  program.add_argument("--prompt-height").scan<'i', int>();
  program.add_argument("--prompt-width").scan<'i', int>();
  program.add_argument("--max-elements").scan<'i', int>();
  program.add_argument("--precision").scan<'i', int>();

  auto &group2 = program.add_mutually_exclusive_group(false);

  group2.add_argument("--prompt-top").flag();
  group2.add_argument("--prompt-bottom").flag();

  program.add_argument("--title").default_value(std::string(""));
  program.add_argument("--fps").scan<'d', int>();
  program.add_argument("--pad-x").scan<'d', int>();
  program.add_argument("--pad-y").scan<'d', int>();

  auto &group1 = program.add_mutually_exclusive_group(false);

  group1.add_argument("--top-left").flag();
  group1.add_argument("--top").flag();
  group1.add_argument("--top-right").flag();
  group1.add_argument("--left").flag();
  group1.add_argument("--center").flag();
  group1.add_argument("--right").flag();
  group1.add_argument("--bottom-left").flag();
  group1.add_argument("--bottom").flag();
  group1.add_argument("--bottom-right").flag();

  program.add_argument("--bg1").scan<'x', unsigned int>();
  program.add_argument("--bg2").scan<'x', unsigned int>();
  program.add_argument("--fg1").scan<'x', unsigned int>();
  program.add_argument("--fg2").scan<'x', unsigned int>();
  program.add_argument("--fg3").scan<'x', unsigned int>();

  auto &group3 = program.add_mutually_exclusive_group(false);
  group3.add_argument("--builtins").flag();
  group3.add_argument("--no-builtins").flag();

  program.add_argument("-h", "--help").flag();
}

bool parse_and_load(int argc, const char **argv, Config *config) {
  create_argumenst();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    print_help();
    return false;
  }

  if (program["--help"] == true) {
    print_help();
    return false;
  }

  if (program["--top-left"] == true) {
    config->position = Position::TOP_LEFT;
  } else if (program["--top"] == true) {
    config->position = Position::TOP;
  } else if (program["--top-right"] == true) {
    config->position = Position::TOP_RIGHT;
  } else if (program["--left"] == true) {
    config->position = Position::LEFT;
  } else if (program["--center"] == true) {
    config->position = Position::CENTER;
  } else if (program["--right"] == true) {
    config->position = Position::RIGHT;
  } else if (program["--bottom-left"] == true) {
    config->position = Position::BOTTOM_LEFT;
  } else if (program["--bottom"] == true) {
    config->position = Position::BOTTOM;
  } else if (program["--bottom-right"] == true) {
    config->position = Position::BOTTOM_RIGHT;
  }

  if (program["--prompt-top"] == true) {
    config->prompt_position = Position::TOP;
  } else if (program["--prompt-bottom"] == true) {
    config->prompt_position = Position::BOTTOM;
  }

  if (program["--builtins"] == true) {
    config->builtins = true;
  } else if (program["--no-builtins"] == true) {
    config->builtins = false;
  }

  if (program.present<int>("--font-size")) {
    config->font_size = program.get<int>("--font-size");
  }

  std::string font_name = program.get<std::string>("--font-name");
  if (!font_name.empty()) {
    config->font_name = font_name;
  }

  if (program.present<int>("--prompt-height")) {
    config->prompt_height = program.get<int>("--prompt-height");
  }

  if (program.present<int>("--prompt-width")) {
    config->prompt_width = program.get<int>("--prompt-width");
  }

  if (program.present<int>("--max-elements")) {
    config->prompt_max_elements = program.get<int>("--max-elements");
  }

  if (program.present<int>("--precision")) {
    config->prompt_precision = program.get<int>("--precision");
  }

  std::string title = program.get<std::string>("--title");
  if (!title.empty()) {
    config->window_title = title;
  }

  if (program.present<int>("--fps")) {
    config->target_fps = program.get<int>("--fps");
  }

  if (program.present<int>("--pad-x")) {
    config->x_padding = program.get<int>("--pad-x");
  }

  if (program.present<int>("--pad-y")) {
    config->y_padding = program.get<int>("--pad-y");
  }

  if (program.present<unsigned int>("--bg1")) {
    config->bg1 = GetColor(program.get<unsigned int>("--bg1"));
  }

  if (program.present<unsigned int>("--bg2")) {
    config->bg2 = GetColor(program.get<unsigned int>("--bg2"));
  }

  if (program.present<unsigned int>("--fg1")) {
    config->fg1 = GetColor(program.get<unsigned int>("--fg1"));
  }

  if (program.present<unsigned int>("--fg2")) {
    config->fg2 = GetColor(program.get<unsigned int>("--fg2"));
  }

  if (program.present<unsigned int>("--fg3")) {
    config->fg3 = GetColor(program.get<unsigned int>("--fg3"));
  }

  return true;
}
