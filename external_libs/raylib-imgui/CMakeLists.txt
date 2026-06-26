project(raylib-imgui VERSION 1.0.0)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external_libs/imgui")

add_library(raylib_imgui STATIC
    rlImGui.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

target_include_directories(raylib_imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
)

target_compile_definitions(raylib_imgui PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_DISABLE_OBSOLETE_KEYIO
)

target_link_libraries(raylib_imgui PUBLIC raylib)
