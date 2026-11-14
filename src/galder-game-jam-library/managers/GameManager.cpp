//
// Created by robin on 26.10.22.
//

#include "GameManager.h"

#include <algorithm>
#include <cmath>

#include "rlImGui.h"
#include "imgui.h"
#include "rlgl.h"

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
        ImGui::GetStyle().FontScaleMain = 1.0f;
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

    m_virtualSize = raylib::Vector2((float)m_window.GetWidth(), (float)m_window.GetHeight());

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
    const raylib::Rectangle viewport = calculateViewport();
    ensureRenderTextureSize(static_cast<int>(viewport.width), static_cast<int>(viewport.height));

    m_renderTexture.BeginMode();
    // BeginTextureMode() sets up a projection matching the texture's pixel size. Replace it with one
    // spanning the virtual screen, so everything keeps drawing in virtual coordinates while being
    // rasterised at the window's real resolution. That keeps text crisp instead of being drawn at
    // the virtual resolution and stretched afterwards. BeginMode2D()/EndMode2D() only touch the
    // modelview matrix, so this holds for the whole frame.
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0.0, static_cast<double>(m_virtualSize.x), static_cast<double>(m_virtualSize.y), 0.0, 0.0, 1.0);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    //drawTestStuff();
    m_worlds.draw();
    m_renderTexture.EndMode();

    render(viewport);
}

raylib::Rectangle ggj::GameManager::calculateViewport() const
{
    const float windowWidth = static_cast<float>(m_window.GetWidth());
    const float windowHeight = static_cast<float>(m_window.GetHeight());
    const float scale = std::min(windowWidth / m_virtualSize.x, windowHeight / m_virtualSize.y);

    // Whole pixels, so the render texture maps 1:1 onto the screen without any resampling.
    const float width = std::max(1.0f, std::round(m_virtualSize.x * scale));
    const float height = std::max(1.0f, std::round(m_virtualSize.y * scale));
    return raylib::Rectangle { std::floor((windowWidth - width) * 0.5f), std::floor((windowHeight - height) * 0.5f), width, height };
}

void ggj::GameManager::ensureRenderTextureSize(const int width, const int height)
{
    if (m_renderTexture.texture.id != 0 && m_renderTexture.texture.width == width && m_renderTexture.texture.height == height)
        return;

    m_renderTexture.Load(width, height);
}

/*!
 *  Copies the frame from m_renderTexture onto the screen, letterboxed into the window.
 */
void ggj::GameManager::render(const raylib::Rectangle& viewport)
{
    BeginDrawing();
    m_window.ClearBackground(BLACK);
    // Render textures are stored upside down, hence the negative source height.
    DrawTexturePro(m_renderTexture.texture, raylib::Rectangle(0.0f, 0.0f, (float)m_renderTexture.texture.width, (float)-m_renderTexture.texture.height), viewport, { 0, 0 }, 0.0f,
                   WHITE);
    m_debugManager.draw();

    // Apply scale before rlImGuiBegin so the font scale is set before NewFrame() computes
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
