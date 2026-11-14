project(galder-game-jam-library-tests VERSION 1.0.0)

add_executable(galder-game-jam-library-tests main_tests.cpp logger_tests.cpp input_tests.cpp palette_swap_tests.cpp claude_boss_tests.cpp treant_root_tests.cpp chat_tests.cpp talent_tests.cpp netplay_kick_tests.cpp)

# Shim dir resolving <catch2/catch.hpp> to the bundled Catch2, ahead of any system install.
target_include_directories(galder-game-jam-library-tests BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(galder-game-jam-library-tests galder-game-jam-library raylib_cpp raylib fmt box2d GameNetworkingSockets::shared)
