//
// Created by robin on 21.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP
#define GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP

#include "../data/PlayerPersistentState.hpp"
#include "../interfaces/manager/INetworkManager.h"
#include "../interfaces/graphics/IPaletteSwapShader.h"

#ifdef GAME_DEV_DEBUG
namespace ggj
{
    struct DebugState;
}
#endif

namespace ggj
{
    class ManagerDiCollection
    {
        public:
        ManagerDiCollection(ggj::IResourceManager<ResourceName>& resourceManager, ggj::ITextureManager<TextureName, raylib::Texture>& textureManager,
                            ggj::IInputManager<ggj::KeyboardKey>& inputManager, ggj::CollisionManager& collisionManager, ggj::IDebugManager& debugManager,
                            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IUserDataManager<PhysicsObject*>& userDataManager,
                            ggj::ISoundManager& soundManager, ggj::PlayerPersistentState& persistentState, ggj::INetworkManager& networkManager,
                            ggj::IPaletteSwapShader& paletteSwapShader
#ifdef GAME_DEV_DEBUG
                            ,
                            ggj::DebugState& debugState
#endif
                            )
            : resources { resourceManager }, textures { textureManager }, input { inputManager }, collision { collisionManager }, debug { debugManager },
              animation { animationManager }, userData { userDataManager }, sound { soundManager }, persistentState { persistentState }, network { networkManager },
              paletteSwap { paletteSwapShader }
#ifdef GAME_DEV_DEBUG
              ,
              debugState { debugState }
#endif
        {
        }

        ggj::IResourceManager<ResourceName>& resources;
        ggj::ITextureManager<TextureName, raylib::Texture>& textures;
        ggj::IInputManager<ggj::KeyboardKey>& input;
        ggj::CollisionManager& collision;
        ggj::IDebugManager& debug;
        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animation;
        IUserDataManager<PhysicsObject*>& userData;
        ggj::ISoundManager& sound;
        ggj::PlayerPersistentState& persistentState;
        ggj::INetworkManager& network;
        ggj::IPaletteSwapShader& paletteSwap;
#ifdef GAME_DEV_DEBUG
        ggj::DebugState& debugState;
#endif
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2025_PROJECT_MANAGERDICOLLECTION_HPP
