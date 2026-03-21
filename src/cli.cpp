#include <exception>
#include <iostream>
#include <print>

#include "argparse.hpp"
#include "cli.hpp"
#include "config.hpp"

namespace Cli {

static void create_arguments(argparse::ArgumentParser &prog) {
  prog.add_argument("--font-name").default_value(std::string(""));
  prog.add_argument("--font-size").scan<'u', unsigned int>();

  prog.add_argument("--prompt-height").scan<'u', unsigned int>();
  prog.add_argument("--prompt-width").scan<'u', unsigned int>();
  prog.add_argument("--max-elements").scan<'u', unsigned int>();
  prog.add_argument("--precision").scan<'u', unsigned int>();

  auto &group1 = prog.add_mutually_exclusive_group();

  group1.add_argument("--prompt-top").flag();
  group1.add_argument("--prompt-bottom").flag();

  prog.add_argument("--title").default_value(std::string(""));
  prog.add_argument("--fps").scan<'u', unsigned int>();
  prog.add_argument("--pad-x").scan<'u', unsigned int>();
  prog.add_argument("--pad-y").scan<'u', unsigned int>();

  auto &group2 = prog.add_mutually_exclusive_group();

  group2.add_argument("--top").flag();
  group2.add_argument("--left").flag();
  group2.add_argument("--center").flag();
  group2.add_argument("--right").flag();
  group2.add_argument("--bottom").flag();

  prog.add_argument("--bg1").scan<'x', unsigned int>();
  prog.add_argument("--bg2").scan<'x', unsigned int>();
  prog.add_argument("--fg1").scan<'x', unsigned int>();
  prog.add_argument("--fg2").scan<'x', unsigned int>();
  prog.add_argument("--fg3").scan<'x', unsigned int>();

  auto &group3 = prog.add_mutually_exclusive_group();
  group3.add_argument("--builtins").flag();
  group3.add_argument("--no-builtins").flag();

  prog.add_argument("-h", "--help").flag();
}

void print_help() {
  // clang-format off
  std::cout << "Usage: spotlight [OPTIONS]\n\n"

            << "Font Options:\n"
            << "  --font-name <str>      set font name\n"
            << "  --font-size <uint>     set font size\n\n"

            << "Prompt Options:\n"
            << "  --prompt-height <uint> set prompt height\n"
            << "  --prompt-width <uint>  set prompt width\n"
            << "  --max-elements <uint>  set prompt max elements\n"
            << "  --precision <uint>     set prompt precision\n\n"

            << "Prompt Position:\n"
            << "  --prompt-top\n"
            << "  --prompt-bottom\n\n"

            << "Window Options:\n"
            << "  --title <str>          set window title\n"
            << "  --fps <uint>           set target fps\n"
            << "  --pad-x <uint>         set padding on x axis\n"
            << "  --pad-y <uint>         set padding on y axis\n\n"
            
            << "Window Position:\n"
            << "  --top\n"
            << "  --left\n"
            << "  --center\n"
            << "  --right\n"
            << "  --bottom\n\n"

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

bool load(int argc, const char **argv, Config &config) {
  argparse::ArgumentParser prog("spotlightpp", "0.1.0",
                                argparse::default_arguments::none);
  create_arguments(prog);

  try {
    prog.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::println(stderr, "{}", err.what());
    print_help();
    return false;
  }

  if (prog["--help"] == true) {
    print_help();
    return false;
  }

  if (prog["--top"] == true)
    config.window.position = Position::TOP;
  else if (prog["--left"] == true)
    config.window.position = Position::LEFT;
  else if (prog["--center"] == true)
    config.window.position = Position::CENTER;
  else if (prog["--right"] == true)
    config.window.position = Position::RIGHT;
  else if (prog["--bottom"] == true)
    config.window.position = Position::BOTTOM;

  if (prog["--prompt-top"] == true)
    config.prompt.position = Position::TOP;
  else if (prog["--prompt-bottom"] == true)
    config.prompt.position = Position::BOTTOM;

  if (prog["--builtins"] == true)
    config.prompt.builtins = true;
  else if (prog["--no-builtins"] == true)
    config.prompt.builtins = false;

  if (prog.present<unsigned int>("--font-size"))
    config.font.size = prog.get<unsigned int>("--font-size");
  std::string font_name = prog.get<std::string>("--font-name");
  if (!font_name.empty())
    config.font.name = font_name;

  if (prog.present<int>("--prompt-height"))
    config.prompt.height = prog.get<unsigned int>("--prompt-height");
  if (prog.present<int>("--prompt-width"))
    config.prompt.width = prog.get<unsigned int>("--prompt-width");
  if (prog.present<int>("--max-elements"))
    config.prompt.max_elements = prog.get<unsigned int>("--max-elements");
  if (prog.present<int>("--precision"))
    config.prompt.precision = prog.get<unsigned int>("--precision");

  std::string title = prog.get<std::string>("--title");
  if (!title.empty())
    config.window.title = title;
  if (prog.present<int>("--fps"))
    config.window.target_fps = prog.get<int>("--fps");
  if (prog.present<int>("--pad-x"))
    config.window.x_padding = prog.get<int>("--pad-x");
  if (prog.present<int>("--pad-y"))
    config.window.y_padding = prog.get<int>("--pad-y");

  if (prog.present<unsigned int>("--bg1"))
    config.theme.bg1 = GetColor(prog.get<unsigned int>("--bg1"));
  if (prog.present<unsigned int>("--bg2"))
    config.theme.bg2 = GetColor(prog.get<unsigned int>("--bg2"));
  if (prog.present<unsigned int>("--fg1"))
    config.theme.fg1 = GetColor(prog.get<unsigned int>("--fg1"));
  if (prog.present<unsigned int>("--fg2"))
    config.theme.fg2 = GetColor(prog.get<unsigned int>("--fg2"));
  if (prog.present<unsigned int>("--fg3"))
    config.theme.fg3 = GetColor(prog.get<unsigned int>("--fg3"));

  return true;
}

} // namespace Cli
