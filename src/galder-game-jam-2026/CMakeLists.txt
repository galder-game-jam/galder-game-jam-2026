# cmake_minimum_required(VERSION 3.23)
project(galder-game-jam-2026 VERSION 0.1.0)

if(LINUX)
    # Copies game content to the executable folder when CMake file is loaded.
    # file(COPY ${CMAKE_SOURCE_DIR}/content/game/content DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    add_executable(galder-game-jam-2026 main.cpp)
    target_link_libraries(galder-game-jam-2026 galder-game-jam-library gamedev_base raylib_cpp raylib fmt box2d)

    # Copies game content on compile
    add_custom_command(TARGET galder-game-jam-2026 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/content/game/content"
            "$<TARGET_FILE_DIR:galder-game-jam-2026>/content"
            COMMENT "Copying content files to app directory")

elseif(WINDOWS)
    # Copies game content to the executable folder when CMake file is loaded.
    # file(COPY ${CMAKE_SOURCE_DIR}/content/game/content DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    add_executable(galder-game-jam-2026 main.cpp ${CMAKE_SOURCE_DIR}/content/logo/icon.rc)
    target_link_libraries(galder-game-jam-2026 galder-game-jam-library gamedev_base raylib_cpp raylib fmt box2d)
    # target_link_options(galder-game-jam-2026 PRIVATE -mwindows)

    # Copies game content on compile
    add_custom_command(TARGET galder-game-jam-2026 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/content/game/content"
            "$<TARGET_FILE_DIR:galder-game-jam-2026>/content"
            COMMENT "Copying content files to app directory")
else()
    set(MACOSX_BUNDLE_BUNDLE_NAME "GalderGameJam2024")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "galder.game.jam.2024")
    set(MACOSX_BUNDLE_INFO_STRING "GalderGameJam2024")
    set(MACOSX_BUNDLE_ICON_FILE "AppIcon.icns")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING 0.1.2)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING 0.1.2)
    set(MACOSX_BUNDLE_BUNDLE_VERSION 0.1.2)

    set(icon_file_path "${CMAKE_SOURCE_DIR}/content/logo/AppIcon.icns")
    set_source_files_properties(${icon_file_path} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    add_executable(GalderGameJam2026 MACOSX_BUNDLE main.cpp)
    target_link_libraries(GalderGameJam2026 galder-game-jam-library gamedev_base raylib_cpp raylib fmt box2d)
    set_target_properties(GalderGameJam2026 PROPERTIES
            MACOSX_BUNDLE TRUE)
    # file(COPY ${CMAKE_SOURCE_DIR}/content/game/content DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/GalderGameJam2026.app/Contents/MacOS)
    # file(COPY ${CMAKE_SOURCE_DIR}/content/logo/AppIcon.icns DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/GalderGameJam2026.app/Contents/Resources)

    # Copies game content on compile
    add_custom_command(TARGET GalderGameJam2026 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/content/game/content"
            "${CMAKE_CURRENT_BINARY_DIR}/GalderGameJam2026.app/Contents/MacOS"
            COMMENT "Copying content files to app directory")

    add_custom_command(TARGET GalderGameJam2026 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/content/logo/AppIcon.icns"
            "${CMAKE_CURRENT_BINARY_DIR}/GalderGameJam2026.app/Contents/Resources"
            COMMENT "Copying app icon to MacOS app directory")
endif()