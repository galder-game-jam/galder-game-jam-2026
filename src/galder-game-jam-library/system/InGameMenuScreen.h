#ifndef GGJ_INGAMEMENUSCREEN_H
#define GGJ_INGAMEMENUSCREEN_H

#include <string>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/KeyboardKey.h"
#include "../enums/SoundName.h"
#include "MenuMouse.hpp"

namespace ggj
{
    /*!
     * Pause menu shown during gameplay when the player presses Esc. Draws a semi-transparent
     * overlay on top of the paused world and provides three actions: Resume, Options, and Exit
     * to Main Menu. Navigation matches the main menu (arrow keys / WASD + Space / Enter, or mouse hover + left click).
     * Esc is treated as an alias for Resume.
     */
    class InGameMenuScreen : public IGameObject<raylib::Vector2>
    {
        public:
            InGameMenuScreen(IInputManager<KeyboardKey>& inputManager, ISoundManager& soundManager)
                : m_inputManager { inputManager }, m_soundManager { soundManager }
            {
                m_isVisible = true;
            }

            void setFont(raylib::Font* font) { m_font = font; }

            [[nodiscard]] bool isResumeSelected() const { return m_isResumeSelected; }
            [[nodiscard]] bool isOptionsSelected() const { return m_isOptionsSelected; }
            [[nodiscard]] bool isExitSelected() const { return m_isExitSelected; }

            /*! Clears all selection flags and resets cursor to Resume. */
            void reset();

            void rotate(float degrees) override {}
            void scale(float scale) override {}
            void move(const raylib::Vector2& toMove) override {}
            void destroy() override { IGameObject::destroy(); }
            void update(float timeDelta) override;
            void draw() override;

        private:
            /*! Clickable row for a menu item, in virtual screen coordinates. */
            [[nodiscard]] raylib::Rectangle itemRect(int index) const;

            enum class InGameMenuOption
            {
                Resume     = 0,
                Options    = 1,
                ExitToMenu = 2,
            };

            IInputManager<KeyboardKey>& m_inputManager;
            ISoundManager& m_soundManager;
            raylib::Font* m_font { nullptr };

            InGameMenuOption m_selectedOption { InGameMenuOption::Resume };
            bool m_isResumeSelected  { false };
            bool m_isOptionsSelected { false };
            bool m_isExitSelected    { false };

            static constexpr float SCREEN_WIDTH  = 800.0f;
            static constexpr float SCREEN_HEIGHT = 600.0f;
            static constexpr int   MENU_FONT_SIZE  = 36;
            static constexpr int   TITLE_FONT_SIZE = 28;
            static constexpr int   MENU_ITEM_COUNT = 3;
            static constexpr float MENU_START_Y = 260.0f;
            static constexpr float MENU_SPACING = 60.0f;

            const std::string m_menuItems[MENU_ITEM_COUNT] { "RESUME", "OPTIONS", "EXIT TO MAIN MENU" };
    };
} // namespace ggj

#endif // GGJ_INGAMEMENUSCREEN_H
