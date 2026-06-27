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

#ifdef GAME_DEV_DEBUG
    #include "DebugMenu.hpp"
#endif

namespace ggj
{
    class GameManager : public IGameManager
    {
        public:
        explicit GameManager(ggj::ILogger& logger, raylib::Window& window, IInputManager<ggj::KeyboardKey>& input, IWorldManager& worlds,
                             ggj::ITextureManager<TextureName, raylib::Texture>& textures, IDebugManager& debugManager, PlayerPersistentState& persistentState
#ifdef GAME_DEV_DEBUG
                             ,
                             ggj::DebugState& debugState
#endif
                             )
            : m_logger { logger }, m_window { window }, m_input { input }, m_worlds { worlds }, m_textures { textures }, m_debugManager { debugManager },
              m_persistentState { persistentState }
#ifdef GAME_DEV_DEBUG
              ,
              m_debugState { debugState }
#endif
        {
        }

#ifdef GAME_DEV_DEBUG
        ~GameManager() override;
#endif

        bool initialize() override;
        void update(float timeDelta) override;
        void draw() override;

        private:
        void render();

        raylib::Window& m_window;
        ILogger& m_logger;
        IInputManager<ggj::KeyboardKey>& m_input;
        IWorldManager& m_worlds;
        ggj::ITextureManager<TextureName, raylib::Texture>& m_textures;
        IDebugManager& m_debugManager;

        PlayerPersistentState& m_persistentState;
        raylib::Vector2 m_originalWindowSize;
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
