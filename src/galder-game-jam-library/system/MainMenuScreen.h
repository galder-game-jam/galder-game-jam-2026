//
// Created by robin on 27.06.2026.
//

#ifndef GGJ_MAINMENUSCREEN_H
#define GGJ_MAINMENUSCREEN_H

#include <string>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../GalderGameJamLibrary.h"
#include "MenuMouse.hpp"

namespace ggj
{
    class MainMenuScreen : public IGameObject<raylib::Vector2>
    {
        public:
            MainMenuScreen(IMapper& mapper, ITextureManager<TextureName, raylib::Texture>& textures,
                IInputManager<KeyboardKey>& inputManager, ISoundManager& soundManager)
                : m_mapper{mapper}, m_textures{textures}, m_inputManager{inputManager}, m_soundManager{soundManager}
            {
                m_isVisible = true;
            }

            void initialize();
            void setFont(raylib::Font* font);

            [[nodiscard]] bool isSinglePlayerSelected() const;
            [[nodiscard]] bool isNetplaySelected() const;
            [[nodiscard]] bool isOptionsSelected() const;
            [[nodiscard]] bool isExitGameSelected() const;

            /*! Clears all selection flags so the menu can be shown again (e.g. after going back). */
            void reset();

            void rotate(float degrees) override;
            void scale(float scale) override;
            void move(const raylib::Vector2& toMove) override;
            void destroy() override;
            void update(float timeDelta) override;
            void draw() override;

        private:
            /*! Clickable row for a menu item, in virtual screen coordinates. */
            [[nodiscard]] raylib::Rectangle itemRect(int index) const;
            void activateSelected();

            enum class MainMenuOption
            {
                SinglePlayer = 0,
                Netplay      = 1,
                Options      = 2,
                ExitGame     = 3,
            };

            IMapper& m_mapper;
            ITextureManager<TextureName, raylib::Texture>& m_textures;
            IInputManager<KeyboardKey>& m_inputManager;
            ISoundManager& m_soundManager;

            raylib::Texture* m_backgroundTexture {nullptr};
            raylib::Font* m_font {nullptr};

            MainMenuOption m_selectedOption {MainMenuOption::SinglePlayer};
            bool m_isSinglePlayerSelected {false};
            bool m_isNetplaySelected {false};
            bool m_isOptionsSelected {false};
            bool m_isExitGameSelected {false};

            const float SCREEN_WIDTH  = 800.0f;
            const float SCREEN_HEIGHT = 600.0f;
            const int MENU_FONT_SIZE  = 40;
            static constexpr float MENU_START_Y = 250.0f;
            static constexpr float MENU_SPACING = 70.0f;
            static constexpr int MENU_ITEM_COUNT = 4;

            const std::string m_menuItems[MENU_ITEM_COUNT] {"SINGLE PLAYER", "NETPLAY", "OPTIONS", "EXIT GAME"};
    };
} // namespace ggj

#endif // GGJ_MAINMENUSCREEN_H
