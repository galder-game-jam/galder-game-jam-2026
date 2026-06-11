//
// Created by robin on 22.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_HUD_H
#define GALDER_GAME_JAM_2025_PROJECT_HUD_H

#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../GalderGameJamLibrary.h"

namespace ggj
{
    class World;
    class Hud : public IGameObject<raylib::Vector2>
    {
        public:
            Hud(IMapper &mapper, ggj::ITextureManager<TextureName, raylib::Texture> &textures,
                IInputManager<KeyboardKey> &inputManager) : m_mapper {mapper}, m_textures {textures}, m_inputManager {inputManager}
            {
                m_isVisible = true;
            }
            void initialize();
            void setFont(raylib::Font *font);
            void setPlayer(Player *player);
            void setWorld(World *world);
            void rotate(float degrees) override;
            void scale(float scale) override;
            void move(const raylib::Vector2 &toMove) override;
            void destroy() override;
            void update(float timeDelta) override;
            void draw() override;

        private:
            void drawQuestLog(raylib::Vector2 offset, int width, int height) const;
            void DrawResourceBar(const float counter, raylib::Rectangle rectResource, raylib::Rectangle rectEnd, float barNo) const;
            void drawPlayerPortrait() const;
            void drawBar(raylib::Vector2 offset, int middleParts) const;
            void drawCoinBar(raylib::Vector2 offset) const;
            void drawKillsBar(raylib::Vector2 offset) const;
            void drawPlayerPositionBar(raylib::Vector2 offset) const;

            IMapper &m_mapper;
            ITextureManager<TextureName, raylib::Texture> &m_textures;
            IInputManager<KeyboardKey> &m_inputManager;

            raylib::Texture *m_hudTexture {nullptr};
            raylib::Texture *m_playerPortraitTexture {nullptr};

            raylib::Font *m_font = nullptr;

            Player *m_currentPlayer = nullptr;
            World *m_currentWorld = nullptr;

            raylib::Vector2 m_size {};
            raylib::Vector2 m_origin {};
            raylib::Color m_color {};
            UserData m_userData {};
            raylib::Vector2 m_velocity {};


            const float TILE_UNIT = 16.0f;
            const int FONT_SIZE = 15;
            const raylib::Color HUD_FONT_COLOR = BLACK;
            const int HUD_ALPHA = 180;

            float m_health{};
            float m_mana{};
            float m_energy{};

            raylib::Vector2 m_playerPortraitOffset{};
            raylib::Vector2 m_portraitOffset{};
            raylib::Rectangle m_portraitRect {TILE_UNIT * 0, TILE_UNIT * 0, TILE_UNIT * 5 + 4, TILE_UNIT * 4};

            raylib::Rectangle m_lifeRect = {TILE_UNIT * 9, TILE_UNIT * 0, 8, 16};
            raylib::Rectangle m_manaRect = {TILE_UNIT * 9, TILE_UNIT * 1, 8, 16};
            raylib::Rectangle m_energyRect = {TILE_UNIT * 9, TILE_UNIT * 2, 8, 16};

            raylib::Rectangle m_lifeEnd = {TILE_UNIT * 6, TILE_UNIT * 0, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_manaEnd = {TILE_UNIT * 6, TILE_UNIT * 2, TILE_UNIT * 2, TILE_UNIT * 2};
            raylib::Rectangle m_energyEnd = {TILE_UNIT * 6, TILE_UNIT * 4, TILE_UNIT * 2, TILE_UNIT * 2};

            int m_coins{};
            raylib::Vector2 m_coinOffset{};
            raylib::Rectangle m_coinRect {TILE_UNIT * 7, TILE_UNIT * 6, TILE_UNIT * 1, TILE_UNIT * 1};

            int m_kills {};
            raylib::Vector2 m_killsOffset{};

            raylib::Vector2 m_playerPosition{};
            raylib::Vector2 m_playerPositionOffset{};

            raylib::Rectangle m_barRectLeft {TILE_UNIT * 0, TILE_UNIT * 6, TILE_UNIT * 2, TILE_UNIT * 1 + 10};
            raylib::Rectangle m_barRectMiddle {TILE_UNIT * 2, TILE_UNIT * 6, TILE_UNIT * 2, TILE_UNIT * 1 + 10};
            raylib::Rectangle m_barRectRight {TILE_UNIT * 4, TILE_UNIT * 6, TILE_UNIT * 2, TILE_UNIT * 1 + 10};

            raylib::Vector2 m_questLogOffset{};
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
} // ggj

#endif //GALDER_GAME_JAM_2025_PROJECT_HUD_H