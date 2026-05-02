cmake_minimum_required(VERSION 3.20)

project(
  spotlightpp
  LANGUAGES CXX C
  VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(raylib REQUIRED)
find_package(tomlplusplus REQUIRED)
find_package(Fontconfig REQUIRED)

set(SOURCES
  src/main.cpp
  src/apps.cpp
  src/cli.cpp
  src/config.cpp
  src/state.cpp
  src/utils.cpp
  src/vendor/tinyexpr.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
  "${CMAKE_SOURCE_DIR}/include/vendor"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  m 
  raylib 
  tomlplusplus::tomlplusplus
  Fontconfig::Fontconfig
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g -DDEBUG -rdynamic -no-pie>
  $<$<CONFIG:Release>:-O3>
)

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-no-pie>
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
