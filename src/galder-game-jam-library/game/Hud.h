//
// Created by robin on 22.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_HUD_H
#define GALDER_GAME_JAM_2025_PROJECT_HUD_H

#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../GalderGameJamLibrary.h"
#include "../interfaces/game/IPlayer.h"
#include "../enums/PlayerPickedCharacter.h"
#include "../interfaces/manager/IOptionsManager.h"
#include "../interfaces/graphics/IMaskRecolorShader.h"
#include "../data/CharacterColors.hpp"
#include "../data/RemotePlayer.hpp"

namespace ggj
{
    class World;
    class Minimap;
    class Hud : public IGameObject<raylib::Vector2>
    {
        public:
            Hud(IMapper &mapper, ggj::ITextureManager<TextureName, raylib::Texture> &textures,
                IInputManager<KeyboardKey> &inputManager, IOptionsManager &options, IMaskRecolorShader &maskRecolor)
                : m_mapper {mapper}, m_textures {textures}, m_inputManager {inputManager}, m_options {options}, m_maskRecolor {maskRecolor}
            {
                m_isVisible = true;
            }
            void initialize();
            void setFont(raylib::Font *font);
            void setPlayer(IPlayer *player);
            void setPlayerName(const std::string &name);
            void setPlayerNameColor(const raylib::Color& color) { m_playerNameColor = color; }
            void setWorld(World *world);
            void setMinimap(Minimap *minimap);
            /*!
             * Picks the portrait to show and recolours it to match the player's sprite.
             *
             * @param character The character the player picked
             * @param colors The colours the player picked for that character
             */
            void setPickedCharacter(PlayerPickedCharacter character, const CharacterColors& colors);
            void rotate(float degrees) override;
            void scale(float scale) override;
            void move(const raylib::Vector2 &toMove) override;
            void destroy() override;
            void update(float timeDelta) override;
            void draw() override;

        private:
            void drawQuestLog(raylib::Vector2 offset, int width, int height) const;
            /*!
             * Draws one row of resource bars (hearts, mana or energy) next to a portrait.
             *
             * @param counter Bars to fill; the fraction draws a partly filled last bar
             * @param rectResource Source rectangle of a single bar in the HUD texture
             * @param rectEnd Source rectangle of the end cap drawn after the last bar
             * @param barNo Row index below the portrait's top edge (0 = life, 1 = mana, 2 = energy)
             * @param portraitOrigin Top-left corner of the portrait frame
             * @param scale Draw scale; 1 for the player's own portrait, less for party members
             */
            void drawResourceBar(float counter, raylib::Rectangle rectResource, raylib::Rectangle rectEnd, float barNo,
                                 raylib::Vector2 portraitOrigin, float scale) const;
            void drawPlayerPortrait() const;
            /*! Draws a downscaled portrait with life/mana/energy for every other player, stacked below the player's own. */
            void drawPartyBars() const;
            /*!
             * Draws one party member's row: name, recoloured portrait, resource bars and level badge.
             *
             * @param member The remote player to show
             * @param origin Top-left corner of the row
             */
            void drawPartyMember(const RemotePlayer& member, raylib::Vector2 origin) const;
            void drawBar(raylib::Vector2 offset, int middleParts) const;
            void drawPlayerPositionBar(raylib::Vector2 offset) const;
            void drawBossBar() const;
            void drawXpBar() const;

            /*! Returns color with its alpha channel scaled by the current HUD alpha setting. */
            [[nodiscard]] raylib::Color tinted(raylib::Color color) const;

            IMapper &m_mapper;
            ITextureManager<TextureName, raylib::Texture> &m_textures;
            IInputManager<KeyboardKey> &m_inputManager;
            IOptionsManager &m_options;
            IMaskRecolorShader &m_maskRecolor;

            raylib::Texture *m_hudTexture {nullptr};
            raylib::Texture *m_playerPortraitTexture {nullptr};
            raylib::Texture *m_playerPortraitMaskTexture {nullptr}; ///< Marks the recolourable parts of the portrait.
            MaskRecolor m_playerPortraitRecolor {}; ///< Resolved from the picked colours; nothing recoloured by default.
            raylib::Texture *m_roguePortraitTexture {nullptr};   ///< Party members' portraits, picked by their character.
            raylib::Texture *m_roguePortraitMaskTexture {nullptr};
            raylib::Texture *m_wizardPortraitTexture {nullptr};
            raylib::Texture *m_wizardPortraitMaskTexture {nullptr};

            raylib::Font *m_font = nullptr;

            IPlayer *m_currentPlayer = nullptr;
            World *m_currentWorld = nullptr;
            Minimap *m_minimap    = nullptr;

            std::string m_playerName {};
            raylib::Color m_playerNameColor { RED };

            raylib::Vector2 m_size {};
            raylib::Vector2 m_origin {};
            raylib::Color m_color {};
            UserData m_userData {};
            raylib::Vector2 m_velocity {};


            const float TILE_UNIT = 16.0f;
            const int FONT_SIZE = 15;

            float m_health{};
            float m_mana{};                      ///< Mana in bars (fractional while regenerating)
            bool m_hasManaBar{};                 ///< False for characters without mana (the Rogue)
            static constexpr float MANA_PER_BAR = 20.0f; ///< Level 1 cap of 200 mana fills 10 bars; later levels add partial bars
            float m_energy{};                    ///< Energy in bars (fractional while regenerating)
            bool m_hasEnergyBar{};               ///< False for characters without energy (the Wizard)
            static constexpr float ENERGY_PER_BAR = 10.0f;

            static constexpr float PARTY_SCALE = 0.5f;          ///< Party portraits are drawn at half the size of the player's own.
            static constexpr float PARTY_NAME_HEIGHT = 13.0f;   ///< Room above each party portrait for the name.
            static constexpr float PARTY_ROW_GAP = 4.0f;        ///< Space between two party rows.
            static constexpr int PARTY_FONT_SIZE = 12;

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
            raylib::Rectangle m_coinRect {TILE_UNIT * 7, TILE_UNIT * 6, TILE_UNIT * 1, TILE_UNIT * 1};


            int m_level       {};
            int m_currentXp   {};
            int m_totalXp     {};
            int m_xpToNextLevel {};

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