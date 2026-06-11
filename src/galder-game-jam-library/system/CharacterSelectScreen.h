//
// Created by robin on 23.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_CHARACTERSELECTSCREEN_H
#define GALDER_GAME_JAM_2025_PROJECT_CHARACTERSELECTSCREEN_H

#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../GalderGameJamLibrary.h"

namespace ggj
{
    enum class PlayerPickedCharacter
    {
        None = 0,
        Rogue = 1,
        Wizard = 2,
    };

    class World;
    class CharacterSelectScreen : public IGameObject<raylib::Vector2>
    {
        public:
            CharacterSelectScreen(IMapper &mapper, ggj::ITextureManager<TextureName, raylib::Texture> &textures,
                IInputManager<KeyboardKey> &inputManager) : m_mapper {mapper}, m_textures {textures}, m_inputManager {inputManager}
            {
                m_isVisible = true;
            }
            void initialize();
            bool isCharacterSelected();
            void setFont(raylib::Font* font);
            void setPlayer(Player* player);
            void setWorld(World* world);
            void rotate(float degrees) override;
            void scale(float scale) override;
            void move(const raylib::Vector2& toMove) override;
            void destroy() override;
            void update(float timeDelta) override;
            void draw() override;

        private:
            void drawDialogBox(raylib::Vector2 offset, int width, int height, const raylib::Color &backgroundColor = raylib::Color(143, 86, 59, 255)) const;

            IMapper &m_mapper;
            ITextureManager<TextureName, raylib::Texture> &m_textures;
            IInputManager<KeyboardKey> &m_inputManager;

            raylib::Texture *m_hudTexture {nullptr};
            raylib::Texture *m_roguePortraitTexture {nullptr};
            raylib::Texture *m_wizardPortraitTexture {nullptr};
            raylib::Texture *m_currentPortraitTexture {nullptr};

            raylib::Font *m_font = nullptr;

            Player *m_currentPlayer = nullptr;
            World *m_currentWorld = nullptr;

            raylib::Vector2 m_size {};
            raylib::Vector2 m_origin {};
            raylib::Color m_color {};
            UserData m_userData {};
            raylib::Vector2 m_velocity {};

            const float TILE_UNIT = 16.0f;
            const float GRID_UNIT = TILE_UNIT * 2;
            const int FONT_SIZE = 30;
            const raylib::Color HUD_FONT_COLOR = BLACK;
            PlayerPickedCharacter m_pickedCharacter {PlayerPickedCharacter::Rogue};
            bool m_isCharacterSelected {false};

            //Texts
            std::string m_title{"PICK YOUR CHARACTER"};
            std::string m_className{"ROGUE"};
            std::string m_weaponType{"MELEE"};
            std::string m_name{"GALDER"};
            std::vector<std::string> m_lore{
                "GALDER, THE CUNNING ROGUE,",
                "HAILS FROM THE SHADOWY ALLEYS",
                "OF ELDORIA. WITH UNMATCHED",
                "STEALTH AND AGILITY, HE",
                "NAVIGATES THE DARKEST",
                "CORNERS OF THE REALM,",
                "SEEKING FORTUNE AND JUSTICE."
            };

            raylib::Rectangle m_boxTopLeftRect {TILE_UNIT * 0, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxTopMiddleRect {TILE_UNIT * 2, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxTopRightRect {TILE_UNIT * 4, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxMiddleLeftRect {TILE_UNIT * 0, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxMiddleMiddleRect {TILE_UNIT * 2, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxMiddleRightRect {TILE_UNIT * 4, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxBottomLeftRect {TILE_UNIT * 0, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxBottomMiddleRect {TILE_UNIT * 2, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_boxBottomRightRect {TILE_UNIT * 4, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2};
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2025_PROJECT_CHARACTERSELECTSCREEN_H
