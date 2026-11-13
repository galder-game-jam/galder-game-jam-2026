//
// Created by robin on 27.06.2026.
//

#include "MainMenuScreen.h"

namespace ggj
{
    void MainMenuScreen::initialize()
    {
        m_backgroundTexture = m_textures.get(TextureName::MainMenuBackground);
    }

    void MainMenuScreen::setFont(raylib::Font* font)
    {
        m_font = font;
    }

    bool MainMenuScreen::isSinglePlayerSelected() const
    {
        return m_isSinglePlayerSelected;
    }

    bool MainMenuScreen::isNetplaySelected() const
    {
        return m_isNetplaySelected;
    }

    bool MainMenuScreen::isOptionsSelected() const
    {
        return m_isOptionsSelected;
    }

    bool MainMenuScreen::isExitGameSelected() const
    {
        return m_isExitGameSelected;
    }

    void MainMenuScreen::reset()
    {
        m_isSinglePlayerSelected = false;
        m_isNetplaySelected = false;
        m_isOptionsSelected = false;
        m_isExitGameSelected = false;
    }

    void MainMenuScreen::rotate(float degrees) {}
    void MainMenuScreen::scale(float scale) {}
    void MainMenuScreen::move(const raylib::Vector2& toMove) {}

    void MainMenuScreen::destroy()
    {
        IGameObject::destroy();
    }

    raylib::Rectangle MainMenuScreen::itemRect(const int index) const
    {
        // Full panel width so the hit area doesn't shift with the "> " selection marker.
        return raylib::Rectangle { 180.0f, MENU_START_Y + static_cast<float>(index) * MENU_SPACING,
            440.0f, static_cast<float>(MENU_FONT_SIZE) };
    }

    void MainMenuScreen::activateSelected()
    {
        switch (m_selectedOption)
        {
            case MainMenuOption::SinglePlayer:
                m_isSinglePlayerSelected = true;
                break;
            case MainMenuOption::Netplay:
                m_isNetplaySelected = true;
                break;
            case MainMenuOption::Options:
                m_isOptionsSelected = true;
                break;
            case MainMenuOption::ExitGame:
                m_isExitGameSelected = true;
                break;
            default:
                break;
        }
    }

    void MainMenuScreen::update(float timeDelta)
    {
        int option = static_cast<int>(m_selectedOption);

        if (m_inputManager.keyPressed(KeyboardKey::Up) || m_inputManager.keyPressed(KeyboardKey::W))
            option = (option - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;

        if (m_inputManager.keyPressed(KeyboardKey::Down) || m_inputManager.keyPressed(KeyboardKey::S))
            option = (option + 1) % MENU_ITEM_COUNT;

        if (option != static_cast<int>(m_selectedOption))
            m_soundManager.playSound(SoundName::DialogMumble);

        // Hovering only moves the cursor when the mouse actually moved, so it doesn't fight the keyboard.
        const raylib::Vector2 mouse = getVirtualMousePosition(SCREEN_WIDTH, SCREEN_HEIGHT);
        const bool hasMouseMoved = GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f;
        int hoveredOption = -1;
        for (int i = 0; i < MENU_ITEM_COUNT; ++i)
        {
            if (CheckCollisionPointRec(mouse, itemRect(i)))
                hoveredOption = i;
        }
        if (hasMouseMoved && hoveredOption >= 0 && hoveredOption != option)
        {
            option = hoveredOption;
            m_soundManager.playSound(SoundName::DialogMumble);
        }

        m_selectedOption = static_cast<MainMenuOption>(option);

        if (m_inputManager.keyPressed(KeyboardKey::Space) || m_inputManager.keyPressed(KeyboardKey::Enter)
            || (hoveredOption >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {
            activateSelected();
        }
    }

    void MainMenuScreen::draw()
    {
        if (!m_isVisible)
            return;

        m_backgroundTexture->Draw(raylib::Vector2{0.0f, 0.0f});

        DrawRectangle(180, 225, 440, 290, Color{0, 0, 0, 140});

        for (int i = 0; i < MENU_ITEM_COUNT; ++i)
        {
            bool isSelected = (static_cast<int>(m_selectedOption) == i);
            raylib::Color color = isSelected ? YELLOW : LIGHTGRAY;
            std::string label = isSelected ? ("> " + m_menuItems[i]) : ("  " + m_menuItems[i]);

            Vector2 textSize = MeasureTextEx(*m_font, label.c_str(), (float)MENU_FONT_SIZE, 0);
            float textX = (SCREEN_WIDTH - textSize.x) * 0.5f;
            float textY = MENU_START_Y + i * MENU_SPACING;

            m_font->DrawText(label, raylib::Vector2{textX, textY}, (float)MENU_FONT_SIZE, 0, color);
        }
    }
} // namespace ggj
