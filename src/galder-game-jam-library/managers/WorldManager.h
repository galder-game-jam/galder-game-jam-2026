//
// Created by robin on 29.10.22.
//

#ifndef GAME_DEV_TEMPLATE_WORLDMANAGER_H
#define GAME_DEV_TEMPLATE_WORLDMANAGER_H

#include "../enums/MusicName.h"
#include "../enums/ResourceName.h"
#include "../../gamedev-base/GameDevBase.h"
#include "../system/World.h"
#include "tileson.hpp"
#include "../enums/KeyboardKey.h"
#include <map>
#include "CollisionManager.h"
#include "UserDataManager.hpp"
#include "../interfaces/manager/INetworkManager.h"
#include "../interfaces/manager/IOptionsManager.h"
#include "ClientInfoMenu.hpp"
#include "MessagePopup.hpp"

namespace ggj
{
    class Hud; // Forward declaration
    class CharacterSelectScreen; // Forward declaration
    class MainMenuScreen; // Forward declaration
    class NetplayScreen; // Forward declaration
    class OptionsScreen; // Forward declaration
    class InGameMenuScreen; // Forward declaration
    class TalentScreen; // Forward declaration

    class WorldManager : public ggj::IWorldManager
    {
        public:

            WorldManager(ggj::ILogger &logger, IMapper &mapper,
                         raylib::Window &window, ggj::ManagerDiCollection &managers, Hud &hud, Dialog &dialog,
                         CharacterSelectScreen &characterSelectScreen, MainMenuScreen &mainMenuScreen,
                         NetplayScreen &netplayScreen, OptionsScreen &optionsScreen,
                         InGameMenuScreen &inGameMenuScreen, TalentScreen &talentScreen,
                         IOptionsManager &optionsManager)
                    : m_managers {managers}, m_logger{logger},  m_resources {managers.resources}, m_mapper{mapper},
                      m_textures{managers.textures}, m_window{window}, m_input{managers.input}, m_collisionManager {managers.collision},
                      m_debugManager{managers.debug}, m_animationManager{managers.animation}, m_userDataManager{managers.userData},
                      m_soundManager{managers.sound},
                      m_hud {hud}, m_dialog {dialog}, m_characterSelectScreen {characterSelectScreen},
                      m_mainMenuScreen {mainMenuScreen}, m_netplayScreen {netplayScreen}, m_optionsScreen {optionsScreen},
                      m_inGameMenuScreen {inGameMenuScreen}, m_talentScreen {talentScreen}, m_network {managers.network},
                      m_optionsManager {optionsManager}
            {

            }

            bool initialize() override;
            void update(float timeDelta) override;
            void draw() override;
            void drawImGui() override;

        private:
            bool parseProject();
            void startMusicForStage(int32_t stageIndex);

            /*! Runs whichever screen (menu, character select, world, ...) is currently active. */
            void updateActiveScreen(float timeDelta);
            /*!
             * Drives netplay once per frame, also while a menu is open: a host that stops polling
             * stalls every client, and a peer that stops polling falls behind and then bursts.
             */
            void updateNetplay(float timeDelta);
            /*!
             * Switches to another stage, carrying the local player's progress over.
             *
             * @param stageIndex Stage to switch to; ignored if no such map was loaded
             */
            void changeStage(int32_t stageIndex);
            [[nodiscard]] int32_t getNextStageIndex() const;
            /*! Leaves the current game (ending any netplay session) and shows the main menu. */
            void returnToMainMenu();
            /*! Closes any in-game overlay and shows a blocking message that leads back to the main menu. */
            void showConnectionLostPopup(const std::string& message);

            const int32_t NO_MAP_ID = -1;
            const int32_t FINAL_MAP_ID = 99999;

            //Stage getStageByFilename(const std::string &filename);
            [[nodiscard]] int32_t getStageIndexByFilename(const std::string &filename) const;

            ggj::ManagerDiCollection m_managers;
            ggj::ILogger &m_logger;
            ggj::IResourceManager<ResourceName> &m_resources;
            IMapper &m_mapper;
            ggj::ITextureManager<TextureName, raylib::Texture> &m_textures;
            raylib::Window &m_window;
            ggj::IInputManager<ggj::KeyboardKey> &m_input;
            ggj::CollisionManager &m_collisionManager;
            ggj::IDebugManager &m_debugManager;
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &m_animationManager;
            IUserDataManager<PhysicsObject*> &m_userDataManager;
            ggj::ISoundManager &m_soundManager;
            Hud &m_hud;
            Dialog &m_dialog;
            CharacterSelectScreen &m_characterSelectScreen;
            MainMenuScreen &m_mainMenuScreen;
            NetplayScreen &m_netplayScreen;
            OptionsScreen &m_optionsScreen;
            InGameMenuScreen &m_inGameMenuScreen;
            TalentScreen     &m_talentScreen;
            INetworkManager  &m_network;
            IOptionsManager  &m_optionsManager;

            std::unique_ptr<tson::Project> m_project{nullptr};

            std::map<int32_t, std::unique_ptr<ggj::World>> m_worlds;
            //Stage m_currentStage{Stage::None};
            int32_t m_startupIndex{-1};
            int32_t m_currentStageIndex {-1};
            bool m_mainMenuScreenActive{true};
            bool m_characterSelectScreenActive{false};
            bool m_netplayScreenActive{false};
            bool m_optionsScreenActive{false};
            bool m_inGameMenuActive{false};
            bool m_talentScreenActive{false};
            bool m_optionsReturnToInGame{false};
            bool m_isNetplay{false};
            MessagePopup m_connectionLostPopup {};
            PlayerPickedCharacter m_pickedCharacter { PlayerPickedCharacter::Rogue };
            raylib::Font m_font;
            //std::vector<std::unique_ptr<tson::Map>> m_maps;

    };
}

#endif //GAME_DEV_TEMPLATE_WORLDMANAGER_H
