//
// Created by robin on 29.10.22.
//

#ifndef GAME_DEV_TEMPLATE_WORLDMANAGER_H
#define GAME_DEV_TEMPLATE_WORLDMANAGER_H

#include "../enums/ResourceName.h"
#include "../../gamedev-base/GameDevBase.h"
#include "../system/World.h"
#include "tileson.hpp"
#include "../enums/KeyboardKey.h"
#include <map>
#include "CollisionManager.h"
#include "UserDataManager.hpp"


namespace ggj
{
    class Hud; // Forward declaration
    class CharacterSelectScreen; // Forward declaration

    class WorldManager : public ggj::IWorldManager
    {
        public:

            WorldManager(ggj::ILogger &logger, IMapper &mapper,
                         raylib::Window &window, ggj::ManagerDiCollection &managers, Hud &hud, Dialog &dialog, CharacterSelectScreen &characterSelectScreen)
                    : m_managers {managers}, m_logger{logger},  m_resources {managers.resources}, m_mapper{mapper},
                      m_textures{managers.textures}, m_window{window}, m_input{managers.input}, m_collisionManager {managers.collision},
                      m_debugManager{managers.debug}, m_animationManager{managers.animation}, m_userDataManager{managers.userData},
                      m_hud {hud}, m_dialog {dialog}, m_characterSelectScreen {characterSelectScreen}
            {

            }

            bool initialize() override;
            void update(float timeDelta) override;
            void draw() override;

        private:
            bool parseProject();

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
            Hud &m_hud;
            Dialog &m_dialog;
            CharacterSelectScreen &m_characterSelectScreen;

            std::unique_ptr<tson::Project> m_project{nullptr};

            std::map<int32_t, std::unique_ptr<ggj::World>> m_worlds;
            //Stage m_currentStage{Stage::None};
            int32_t m_startupIndex{-1};
            int32_t m_currentStageIndex {-1};
            bool m_characterSelectScreenActive{true};
            raylib::Font m_font;
            //std::vector<std::unique_ptr<tson::Map>> m_maps;

    };
}

#endif //GAME_DEV_TEMPLATE_WORLDMANAGER_H
