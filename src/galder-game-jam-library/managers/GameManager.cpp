//
// Created by robin on 26.10.22.
//

#include "GameManager.h"

#include "rlImGui.h"

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
