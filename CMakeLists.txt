cmake_minimum_required(VERSION 3.20)
project(MyAutoArena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game_core
    src/core/Tile.cpp
    src/core/Board.cpp
    src/core/Unit.cpp
)

target_include_directories(game_core PUBLIC src)

add_executable(my_auto_arena src/main.cpp)
target_link_libraries(my_auto_arena PRIVATE game_core)

enable_testing()
add_subdirectory(tests)
