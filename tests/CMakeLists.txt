include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(core_tests
    test_board.cpp
    test_unit.cpp
    test_player.cpp
    test_game_fsm.cpp
    test_battle_engine.cpp
    test_pathfinder.cpp
    test_enemy_spawner.cpp
    test_hero_skills.cpp
    test_pve_round_runner.cpp
    test_drag_drop.cpp
    test_console_renderer.cpp
    test_scene_coord_mapper.cpp
)

target_link_libraries(core_tests PRIVATE game_core gui_mapper GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(core_tests)
