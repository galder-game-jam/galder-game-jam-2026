//
// Created by robin on 21.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP
#define GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP

namespace ggj
{
    class ManagerDiCollection
    {
        public:
            ManagerDiCollection( ggj::IResourceManager<ResourceName> &resourceManager,
                ggj::ITextureManager<TextureName, raylib::Texture> &textureManager,
                ggj::IInputManager<ggj::KeyboardKey> &inputManager,
                ggj::CollisionManager & collisionManager, ggj::IDebugManager &debugManager,
                ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &animationManager,
                IUserDataManager<PhysicsObject*> &userDataManager) :  resources{resourceManager}, textures{textureManager},
                           input{inputManager}, collision {collisionManager},
                          debug {debugManager}, animation {animationManager}, userData{userDataManager}
            {

            }

            ggj::IResourceManager<ResourceName> &resources;
            ggj::ITextureManager<TextureName, raylib::Texture> &textures;
            ggj::IInputManager<ggj::KeyboardKey> &input;
            ggj::CollisionManager &collision;
            ggj::IDebugManager &debug;
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &animation;
            IUserDataManager<PhysicsObject*> &userData;

    };
}

#endif //GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP