cmake_minimum_required(VERSION 3.20)
project(MyAutoArena LANGUAGES CXX)

# 便于代理终端未继承 PATH 时仍能发现本机 Qt（与手动设置 CMAKE_PREFIX_PATH 等效）。
if(WIN32 AND EXISTS "D:/Programming/Qt/6.11.0/msvc2022_64")
    list(PREPEND CMAKE_PREFIX_PATH "D:/Programming/Qt/6.11.0/msvc2022_64")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game_core
    src/core/Tile.cpp
    src/core/Board.cpp
    src/core/Unit.cpp
    src/core/Player.cpp
    src/core/GameFSM.cpp
    src/core/BattleEngine.cpp
    src/core/Pathfinder.cpp
    src/core/EnemySpawner.cpp
    src/core/HeroUnits.cpp
    src/core/PvERoundRunner.cpp
    src/core/DragDropHandler.cpp
    src/ui/ConsoleRenderer.cpp
)

target_include_directories(game_core PUBLIC src)

add_library(gui_mapper
    src/ui/qt/SceneCoordMapper.cpp
)
target_include_directories(gui_mapper PUBLIC src)
target_link_libraries(gui_mapper PUBLIC game_core)

add_executable(my_auto_arena src/main.cpp)
target_link_libraries(my_auto_arena PRIVATE game_core)

option(BUILD_QT_GUI "Build Qt6 GUI front-end" ON)
if(BUILD_QT_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND)
        add_library(qt_gui_lib
            src/ui/qt/TileGraphicsItem.cpp
            src/ui/qt/UnitGraphicsItem.cpp
            src/ui/qt/ArenaScene.cpp
            src/ui/qt/UnitInfoPanel.cpp
            src/ui/qt/QtMainWindow.cpp
        )
        target_include_directories(qt_gui_lib PUBLIC src)
        target_link_libraries(qt_gui_lib PUBLIC game_core gui_mapper Qt6::Widgets)
        set_target_properties(qt_gui_lib PROPERTIES AUTOMOC ON)

        add_executable(my_auto_arena_qt src/qt_main.cpp)
        target_link_libraries(my_auto_arena_qt PRIVATE qt_gui_lib)
        set_target_properties(my_auto_arena_qt PROPERTIES AUTOMOC ON)

        message(STATUS "Qt6 found: enabling my_auto_arena_qt")
    else()
        message(STATUS "Qt6 not found: skipping GUI target")
    endif()
endif()

enable_testing()
add_subdirectory(tests)
