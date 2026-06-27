#include "InGameMenuScreen.h"

namespace ggj
{
    void InGameMenuScreen::reset()
    {
        m_selectedOption    = InGameMenuOption::Resume;
        m_isResumeSelected  = false;
        m_isOptionsSelected = false;
        m_isExitSelected    = false;
    }

    void InGameMenuScreen::update(float /*timeDelta*/)
    {
        int option = static_cast<int>(m_selectedOption);

        if (m_inputManager.keyPressed(KeyboardKey::Up) || m_inputManager.keyPressed(KeyboardKey::W))
            option = (option - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;

        if (m_inputManager.keyPressed(KeyboardKey::Down) || m_inputManager.keyPressed(KeyboardKey::S))
            option = (option + 1) % MENU_ITEM_COUNT;

        if (option != static_cast<int>(m_selectedOption))
            m_soundManager.playSound(SoundName::DialogMumble);

        m_selectedOption = static_cast<InGameMenuOption>(option);

        if (m_inputManager.keyPressed(KeyboardKey::Escape))
        {
            m_isResumeSelected = true;
            return;
        }

        if (m_inputManager.keyPressed(KeyboardKey::Space) || m_inputManager.keyPressed(KeyboardKey::Enter))
        {
            switch (m_selectedOption)
            {
                case InGameMenuOption::Resume:
                    m_isResumeSelected = true;
                    break;
                case InGameMenuOption::Options:
                    m_isOptionsSelected = true;
                    break;
                case InGameMenuOption::ExitToMenu:
                    m_isExitSelected = true;
                    break;
            }
        }
    }

    void InGameMenuScreen::draw()
    {
        if (!m_isVisible || m_font == nullptr)
            return;

        // Dim the world behind the menu
        DrawRectangle(0, 0, static_cast<int>(SCREEN_WIDTH), static_cast<int>(SCREEN_HEIGHT),
            Color { 0, 0, 0, 100 });

        // Menu panel
        DrawRectangle(180, 185, 440, 250, Color { 0, 0, 0, 100 });

        // "PAUSED" title
        const std::string title = "PAUSED";
        const Vector2 titleSize = MeasureTextEx(*m_font, title.c_str(), static_cast<float>(TITLE_FONT_SIZE), 0);
        const float titleX = (SCREEN_WIDTH - titleSize.x) * 0.5f;
        m_font->DrawText(title, raylib::Vector2 { titleX, 200.0f },
            static_cast<float>(TITLE_FONT_SIZE), 0, WHITE);

        // Menu items
        const float MENU_START_Y = 260.0f;
        const float MENU_SPACING = 60.0f;

        for (int i = 0; i < MENU_ITEM_COUNT; ++i)
        {
            const bool isSelected = (static_cast<int>(m_selectedOption) == i);
            const raylib::Color color = isSelected ? YELLOW : LIGHTGRAY;
            const std::string label = isSelected ? ("> " + m_menuItems[i]) : ("  " + m_menuItems[i]);

            const Vector2 textSize = MeasureTextEx(*m_font, label.c_str(), static_cast<float>(MENU_FONT_SIZE), 0);
            const float textX = (SCREEN_WIDTH - textSize.x) * 0.5f;
            const float textY = MENU_START_Y + static_cast<float>(i) * MENU_SPACING;

            m_font->DrawText(label, raylib::Vector2 { textX, textY },
                static_cast<float>(MENU_FONT_SIZE), 0, color);
        }
    }
} // namespace ggj
