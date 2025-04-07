# Spotlight++ - A macOS Spotlight-Inspired App Launcher for Linux

## Overview
Spotlight++ is a modern, keyboard-driven application launcher for Linux systems, heavily inspired by macOS's Spotlight feature. Built with Raylib in C++, it provides:

- Fast application searching from `/usr/share/applications` and `~/.local/share/applications`
- Built-in calculator with expression evaluation
- Quick web search/open functionality
- Minimalist UI with keyboard-only navigation
- TOML-based configuration

## Features
### Application Launcher
- Searches all `.desktop` files in standard locations
- Displays application names
- Instant filtering as you type

### Calculator
- Evaluates mathematical expressions automatically (no `=` required)
- Supports variables (use `ans` to reference last result)
- Basic arithmetic, trigonometric functions, and more

### Web Integration
- Direct URL opening in default browser
- Quick Google search fallback for non-URL queries

### Keyboard Navigation
- **Tab**/**↓** - Move selection down
- **Shift+Tab**/**↑** - Move selection up
- **Enter** - Launch selected app/execute action
- **Shift+Enter** - Save current expression result to ans variable
- **Esc** - Close launcher

## Installation
### Dependencies
- Raylib (for graphics)
- cpp-httplib (for url validation)
- tomlplusplus (for configuration)
- g++ (with C++17 support)

### Build Instructions
```sh
git clone https://github.com/vh8t/spotlightpp.git
cd spotlightpp
make
sudo make install  # Optional, installs system-wide
```

## Configuration
Edit `~/.config/spotlightpp/config.toml` to customize behavior:

```toml
builtins = true

[font]
size = 20

[prompt]
height = 40
width = 700
max-elements = 7

[window]
title = "spotlight++"
fps = 120

[theme]
bg1 = 0x1e1e1eff  # RGBA
bg2 = 0x505050ff
fg1 = 0xffffffff
fg2 = 0xc8c8c8ff
fg3 = 0x505050ff
```

## Known Issues
- **CPU Usage**: Due to Raylib's rendering approach, the launcher may use more CPU than traditional launchers. This shouldn't impact performance as it's designed for short-lived sessions.
- **Slow Machines**: Users with older hardware may experience slight input lag.

## Usage Tips
1. Just start typing to search apps or evaluate expressions
2. Prefix with `>` to force web search (e.g., `>linux news`)
3. Use `ans` in calculations to reference previous result
4. Common math functions are supported (`sin`, `cos`, `sqrt`, etc.)

## Credits
Spotlight++ builds upon several excellent open-source projects:

### Calculator Engine
- [TinyExpr](https://github.com/codeplea/tinyexpr) - The entire mathematical expression evaluation system is powered by TinyExpr, a fantastic lightweight expression parsing library by codeplea. This provides all the calculation capabilities including:
    - Basic arithmetic operations
    - Trigonometric functions
    - Algebraic operations
    - Variable handling (including our `ans` feature)

### Core Libraries
- [Raylib](https://www.raylib.com/) - The graphical framework enabling our minimalist, fast-rendering UI
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - Handles all web-related functionality including URL processing
- [toml++](https://github.com/marzer/tomlplusplus) - Provides our TOML configuration parsing capabilities

### Inspiration
- macOS Spotlight - For the general UX philosophy of a unified, keyboard-driven interface

### Special Thanks
- The open source community for creating and maintaining these essential components

We're grateful to these projects and maintainers for making Spotlight++ possible. If you appreciate this software, please consider starring or contributing to their original projects!
