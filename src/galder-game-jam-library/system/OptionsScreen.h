#ifndef GGJ_OPTIONSSCREEN_H
#define GGJ_OPTIONSSCREEN_H

#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/TextureName.h"
#include "../data/AudioOptions.hpp"
#include "../data/KeybindingOptions.hpp"
#include "../interfaces/manager/IOptionsManager.h"

namespace ggj
{
    /*!
     * Options screen shown after selecting "OPTIONS" from the main menu. Displays a Dear ImGui
     * dialog that lets the player adjust music and sound-effect volume and mute toggles. Changes
     * are applied immediately to the running audio; the options.json file is written when the
     * player presses "Back".
     */
    class OptionsScreen : public IGameObject<raylib::Vector2>
    {
        public:
            OptionsScreen(ITextureManager<TextureName, raylib::Texture>& textures,
                IOptionsManager& options, ISoundManager& sound)
                : m_textures { textures }, m_options { options }, m_sound { sound }
            {
                m_isVisible = true;
            }

            void initialize();

            void drawImGui();

            [[nodiscard]] bool isBack() const { return m_isBack; }

            /*! Syncs local state from options and clears the back flag. */
            void reset();

            void rotate(float degrees) override {}
            void scale(float scale) override {}
            void move(const raylib::Vector2& toMove) override {}
            void update(float timeDelta) override {}
            void draw() override;

        private:
            ITextureManager<TextureName, raylib::Texture>& m_textures;
            IOptionsManager& m_options;
            ISoundManager& m_sound;

            raylib::Texture* m_backgroundTexture { nullptr };
            AudioOptions      m_localOptions {};
            HudOptions        m_localHudOptions {};
            KeybindingOptions m_localKeybindingOptions {};

            int m_rebindingAction { -1 }; ///< Index of the action being rebound, or -1 when idle.

            bool m_isBack { false };

            static constexpr float SCREEN_WIDTH  = 800.0f;
            static constexpr float SCREEN_HEIGHT = 600.0f;
    };
} // namespace ggj

#endif // GGJ_OPTIONSSCREEN_H
