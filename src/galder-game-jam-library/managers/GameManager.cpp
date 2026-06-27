//
// Created by robin on 26.10.22.
//

#include "GameManager.h"

#include "rlImGui.h"
#include "imgui.h"

namespace
{
    // Four fonts loaded at startup matching the four UI scale steps.
    // Index 0 = 1x (13 px), 1 = 1.5x (20 px), 2 = 2x (26 px), 3 = 4x (52 px).
    ImFont*     g_scaledFonts[4]  {};
    ImGuiStyle  g_baseStyle;
    bool        g_initialised     { false };

    constexpr float SCALE_VALUES[] = { 1.0f, 1.5f, 2.0f, 4.0f };
    constexpr float FONT_SIZES[]   = { 13.0f, 20.0f, 26.0f, 52.0f };

    ImFont* fontForScale(float scale)
    {
        // Walk backwards and return the largest font that doesn't exceed the requested scale.
        for (int i = 3; i >= 0; --i)
            if (scale >= SCALE_VALUES[i] - 0.01f)
                return g_scaledFonts[i];
        return g_scaledFonts[0];
    }

    void applyImGuiScale(float scale)
    {
        ImGui::GetStyle() = g_baseStyle;
        ImGui::GetStyle().ScaleAllSizes(scale);

        // Switch to the pre-rasterised font for this scale so text is crisp, not stretched.
        ImFont* font = fontForScale(scale);
        if (font)
            ImGui::GetIO().FontDefault = font;
        ImGui::GetIO().FontGlobalScale = 1.0f;
    }
}

ggj::GameManager::~GameManager()
{
    rlImGuiShutdown();
}

bool ggj::GameManager::initialize()
{
    if (!m_worlds.initialize())
    {
        m_logger.error("WorldManager initialization failed!");
        return false;
    }

    m_originalWindowSize = raylib::Vector2((float)m_window.GetWidth(), (float)m_window.GetHeight());
    m_renderTexture = raylib::RenderTexture((int)m_originalWindowSize.x, (int)m_originalWindowSize.y);

    rlImGuiSetup(true);

    if (!g_initialised)
    {
        // rlImGuiSetup already added the 13 px font; grab it as the 1x entry.
        ImGuiIO& io = ImGui::GetIO();
        g_scaledFonts[0] = io.Fonts->Fonts.empty() ? nullptr : io.Fonts->Fonts[0];

        // Add the remaining sizes. They will be packed into the atlas on the first frame.
        for (int i = 1; i < 4; ++i)
        {
            ImFontConfig cfg;
            cfg.SizePixels  = FONT_SIZES[i];
            cfg.PixelSnapH  = true;
            g_scaledFonts[i] = io.Fonts->AddFontDefault(&cfg);
        }

        g_baseStyle  = ImGui::GetStyle();
        g_initialised = true;
    }

    applyImGuiScale(m_options.getHudOptions().imguiScale);
    SetExitKey(KEY_NULL); // Esc is handled in-game; don't let raylib close the window

    return true;
}

void ggj::GameManager::update(float timeDelta)
{
    m_lastTimeDelta = timeDelta;
    m_debugManager.clearText();
    m_worlds.update(timeDelta);
}

void ggj::GameManager::draw()
{
    m_renderTexture.BeginMode();
    //drawTestStuff();
    m_worlds.draw();
    m_renderTexture.EndMode();

    render();
}

/*!
 *  Makes anything drawn to the m_renderTexture get rendered on the screen and scaled with the screen size.
 */
void ggj::GameManager::render()
{
    float scale = std::min((float)m_window.GetWidth() / m_originalWindowSize.x, (float)m_window.GetHeight() / m_originalWindowSize.y);
    BeginDrawing();
    m_window.ClearBackground(BLACK);
    DrawTexturePro(m_renderTexture.texture, raylib::Rectangle(0.0f, 0.0f, (float)m_renderTexture.texture.width, (float)-m_renderTexture.texture.height),
                   raylib::Rectangle(((float)m_window.GetWidth() - ((float)m_originalWindowSize.x * scale)) * 0.5f,
                                     ((float)m_window.GetHeight() - ((float)m_originalWindowSize.y * scale)) * 0.5f, (float)m_originalWindowSize.x * scale,
                                     (float)m_originalWindowSize.y * scale),
                   { 0, 0 }, 0.0f, WHITE);
    m_debugManager.draw();

    // Apply scale before rlImGuiBegin so FontGlobalScale is set before NewFrame() computes
    // g.FontBaseSize — setting it after NewFrame produces stale sizing for that frame.
    applyImGuiScale(m_options.getHudOptions().imguiScale);

    // ImGui is available in all builds (netplay dialog, server info, ...); the debug menu is
    // only drawn in debug builds.
    rlImGuiBegin();
    m_worlds.drawImGui();
#ifdef GAME_DEV_DEBUG
    if (m_debugState.isGameActive)
        m_debugMenu.draw(m_debugState, m_options, m_sound);
#endif
    rlImGuiEnd();

    EndDrawing();
}
