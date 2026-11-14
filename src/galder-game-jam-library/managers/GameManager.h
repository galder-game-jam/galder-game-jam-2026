//
// Created by robin on 26.10.22.
//

#ifndef GAME_DEV_TEMPLATE_GAMEMANAGER_H
#define GAME_DEV_TEMPLATE_GAMEMANAGER_H

#include <memory>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/KeyboardKey.h"
#include "fmt/format.h"
#include "../enums/TextureName.h"
#include "../data/PlayerPersistentState.hpp"
#include "../interfaces/manager/IOptionsManager.h"

#ifdef GAME_DEV_DEBUG
    #include "DebugMenu.hpp"
#endif

namespace ggj
{
    class GameManager : public IGameManager
    {
        public:
        explicit GameManager(ggj::ILogger& logger, raylib::Window& window, IInputManager<ggj::KeyboardKey>& input, IWorldManager& worlds,
                             ggj::ITextureManager<TextureName, raylib::Texture>& textures, IDebugManager& debugManager,
                             PlayerPersistentState& persistentState, IOptionsManager& options, ISoundManager& sound
#ifdef GAME_DEV_DEBUG
                             ,
                             ggj::DebugState& debugState
#endif
                             )
            : m_logger { logger }, m_window { window }, m_input { input }, m_worlds { worlds }, m_textures { textures }, m_debugManager { debugManager },
              m_persistentState { persistentState }, m_options { options }, m_sound { sound }
#ifdef GAME_DEV_DEBUG
              ,
              m_debugState { debugState }
#endif
        {
        }

        ~GameManager() override;

        bool initialize() override;
        void update(float timeDelta) override;
        void draw() override;

        private:
        /*!
         * Calculates where the frame goes in the window: the virtual screen scaled up as far as it
         * fits while keeping its aspect ratio, centered, in whole pixels.
         *
         * @return The letterboxed area of the window, in window pixels
         */
        [[nodiscard]] raylib::Rectangle calculateViewport() const;

        /*!
         * (Re)creates the render texture when the window size has changed, so the frame is always
         * rendered at the window's real resolution rather than stretched from the virtual one.
         *
         * @param width Wanted texture width in pixels
         * @param height Wanted texture height in pixels
         */
        void ensureRenderTextureSize(int width, int height);

        /*!
         * Copies the finished frame onto the screen, then draws the debug overlay and ImGui on top.
         *
         * @param viewport Area of the window the frame is drawn to
         */
        void render(const raylib::Rectangle& viewport);

        raylib::Window& m_window;
        ILogger& m_logger;
        IInputManager<ggj::KeyboardKey>& m_input;
        IWorldManager& m_worlds;
        ggj::ITextureManager<TextureName, raylib::Texture>& m_textures;
        IDebugManager& m_debugManager;

        PlayerPersistentState& m_persistentState;
        IOptionsManager& m_options;
        ISoundManager& m_sound;
        raylib::Vector2 m_virtualSize; ///< Fixed screen size the game is laid out in, regardless of the window size.
        raylib::RenderTexture2D m_renderTexture;

        float m_lastTimeDelta {};

        //Just for testing
        raylib::Rectangle m_testRect { 50, 50, 100, 100 };
        bool m_testShouldDrawPinkRect {};
        raylib::Rectangle m_testRectPink { 150, 150, 50, 50 };
        raylib::Text m_testText { "Congrats! You created your first window!", 20, LIGHTGRAY };
        raylib::Text m_testTextKeys { "", 20, BLACK };

#ifdef GAME_DEV_DEBUG
        ggj::DebugState& m_debugState;
        ggj::DebugMenu m_debugMenu {};
#endif
    };
} // namespace ggj

#endif //GAME_DEV_TEMPLATE_GAMEMANAGER_H
