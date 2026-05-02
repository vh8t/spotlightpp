#include <chrono>
#include <raylib.h>
#include <thread>

#include "state.hpp"

int main(int argc, const char **argv) {
  State state{argc, argv};
  if (!state)
    return 1;

  const int target_fps = state.config.window.target_fps;
  using clock = std::chrono::high_resolution_clock;
  using ms = std::chrono::milliseconds;
  using ns = std::chrono::nanoseconds;

  const ns frame_duration{1'000'000'000 / target_fps};
  auto next_frame = clock::now();

  while (state.running) {
    state.running = !WindowShouldClose() && IsWindowFocused();
    auto frame_start = clock::now();

    state.handle_events();
    state.draw_frame();
    state.render_frame();

    next_frame += frame_duration;
    auto now = clock::now();

    if (next_frame > now)
      std::this_thread::sleep_until(next_frame);
    else
      next_frame = now;
  }

  CloseWindow();
}
