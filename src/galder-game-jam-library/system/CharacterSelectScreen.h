//
// Created by robin on 23.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_CHARACTERSELECTSCREEN_H
#define GALDER_GAME_JAM_2025_PROJECT_CHARACTERSELECTSCREEN_H

#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../GalderGameJamLibrary.h"
#include "../enums/PlayerPickedCharacter.h"
#include "../interfaces/game/IPlayer.h"
#include "../interfaces/graphics/IPaletteSwapShader.h"
#include "../interfaces/graphics/IMaskRecolorShader.h"
#include "../data/CharacterColors.hpp"
#include "../graphics/Animation.h"
#include "../enums/AnimationName.h"
#include <memory>
#include <random>

namespace ggj
{
    class World;
    class CharacterSelectScreen : public IGameObject<raylib::Vector2>
    {
        public:
            CharacterSelectScreen(IMapper &mapper, ggj::ITextureManager<TextureName, raylib::Texture> &textures,
                IInputManager<KeyboardKey> &inputManager, IAnimationManager<Animation, AnimationName> &animations,
                IPaletteSwapShader &paletteSwap, IMaskRecolorShader &maskRecolor)
                : m_mapper {mapper}, m_textures {textures}, m_inputManager {inputManager}, m_animations {animations},
                  m_paletteSwap {paletteSwap}, m_maskRecolor {maskRecolor}
            {
                m_isVisible = true;
            }
            void initialize();
            [[nodiscard]] bool isCharacterSelected() const;
            [[nodiscard]] PlayerPickedCharacter getPickedCharacter() const;
            /*! Colours the player picked for the currently selected character. */
            [[nodiscard]] const CharacterColors& getPickedColors() const;
            /*!
             * Clears the confirmed selection so the screen waits for input again.
             * Called when returning to the main menu so a new game shows character select.
             */
            void reset();
            void setFont(raylib::Font* font);
            void setPlayer(IPlayer* player);
            void setWorld(World* world);
            void rotate(float degrees) override;
            void scale(float scale) override;
            void move(const raylib::Vector2& toMove) override;
            void destroy() override;
            void update(float timeDelta) override;
            void draw() override;
            /*!
             * Draws the colour customization window: an animated, recoloured idle preview of the
             * selected character plus one colour picker per palette slot. Must be called inside an
             * ImGui frame and outside any raylib texture mode (it renders the preview to its own
             * render texture).
             */
            void drawImGui();

        private:
            [[nodiscard]] CharacterColors& pickedColors();
            void renderPreview();
            /*! Draws the big portrait of the selected character in the colours picked for it. */
            void drawPortrait(const raylib::Vector2& position) const;
            void drawDialogBox(raylib::Vector2 offset, int width, int height, const raylib::Color &backgroundColor = raylib::Color(143, 86, 59, 255)) const;

            IMapper &m_mapper;
            ITextureManager<TextureName, raylib::Texture> &m_textures;
            IInputManager<KeyboardKey> &m_inputManager;
            IAnimationManager<Animation, AnimationName> &m_animations;
            IPaletteSwapShader &m_paletteSwap;
            IMaskRecolorShader &m_maskRecolor;

            raylib::Texture *m_hudTexture {nullptr};
            raylib::Texture *m_roguePortraitTexture {nullptr};
            raylib::Texture *m_wizardPortraitTexture {nullptr};
            raylib::Texture *m_currentPortraitTexture {nullptr};
            raylib::Texture *m_roguePortraitMaskTexture {nullptr}; ///< Marks the recolourable parts of the portrait.
            raylib::Texture *m_wizardPortraitMaskTexture {nullptr}; ///< Marks the recolourable parts of the portrait.
            raylib::Texture *m_rogueSpriteTexture {nullptr};
            raylib::Texture *m_wizardSpriteTexture {nullptr};

            Animation m_previewAnimation {};
            std::unique_ptr<raylib::RenderTexture> m_previewTexture {}; ///< Created in initialize(): needs a GL context.
            CharacterColors m_rogueColors {};
            CharacterColors m_wizardColors {};
            std::mt19937 m_randomGenerator { std::random_device {}() }; ///< Drives the Randomize button.

            raylib::Font *m_font = nullptr;

            IPlayer *m_currentPlayer = nullptr;
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

            static constexpr int PREVIEW_SPRITE_SIZE = 64;
            static constexpr float PREVIEW_SCALE = 2.0f;

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
