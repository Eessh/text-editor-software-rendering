cmake_minimum_required(VERSION 3.25)

project(text-editor-software-rendering)

set(CMAKE_CXX_STANDARD 20)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/cpp-tokenizer
  ${PROJECT_SOURCE_DIR}/log-boii
  ${PROJECT_SOURCE_DIR}/cairo-windows-1.17.2/include
  ${PROJECT_SOURCE_DIR}/freetype
  ${PROJECT_SOURCE_DIR}/freetype/freetype
  ${PROJECT_SOURCE_DIR}/freetype/freetype/config
  ${PROJECT_SOURCE_DIR}/SDL2-2.26.5/x86_64-w64-mingw32/include/SDL2
  ${PROJECT_SOURCE_DIR}/toml++
  ${PROJECT_SOURCE_DIR}/toml++/impl
)

find_library(cairo cairo ${PROJECT_SOURCE_DIR}/cairo-windows-1.17.2/lib/x64)
find_library(freetype freetype ${PROJECT_SOURCE_DIR}/freetype/lib/x86_64)
find_library(SDL2 libSDL2 ${PROJECT_SOURCE_DIR}/SDL2-2.26.5/x86_64-w64-mingw32/lib)
find_library(SDL2main libSDL2main ${PROJECT_SOURCE_DIR}/SDL2-2.26.5/x86_64-w64-mingw32/lib)

add_executable(text-editor-software-rendering
  ${PROJECT_SOURCE_DIR}/src/buffer.cpp
  ${PROJECT_SOURCE_DIR}/src/cairo_context.cpp
  ${PROJECT_SOURCE_DIR}/src/config_manager.cpp
  ${PROJECT_SOURCE_DIR}/src/cpp_tokenizer_cache.cpp
  ${PROJECT_SOURCE_DIR}/src/cursor_manager.cpp
  ${PROJECT_SOURCE_DIR}/src/incremental_render_update.cpp
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  ${PROJECT_SOURCE_DIR}/src/rocket_render.cpp
  ${PROJECT_SOURCE_DIR}/src/utils.cpp
  ${PROJECT_SOURCE_DIR}/src/window.cpp
  ${PROJECT_SOURCE_DIR}/log-boii/log_boii.c
  ${PROJECT_SOURCE_DIR}/cpp-tokenizer/cpp_tokenizer.cpp
)

target_link_libraries(text-editor-software-rendering
  ${cairo}
  ${freetype}
  ${SDL2main}
  ${SDL2}
)
