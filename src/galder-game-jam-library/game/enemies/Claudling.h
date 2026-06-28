//

#ifndef GGJ_CLAUDLING_H
#define GGJ_CLAUDLING_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include "../../interfaces/game/IPlayer.h"

namespace ggj
{

    class Claudling : public PhysicsSprite
    {
        public:
            Claudling() = default;
            Claudling(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                const raylib::Vector2& physicsSize,
                const raylib::Vector2& spriteSize,
                const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData, float followSpeed, float knockback, IPlayer* player, bool isVisible = true)
                : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible),
                  m_animationManager { animationManager }, m_mapper { mapper }, m_soundManager { soundManager },
                  m_followSpeed { followSpeed }, m_knockback { knockback }, m_player { player }
            {
                m_body->SetFixedRotation(true);
                m_idleAnimation = m_animationManager.getAnimation(AnimationName::EnemyClaudlingIdle);
                m_walkAnimation = m_animationManager.getAnimation(AnimationName::EnemyClaudlingWalk);
                m_animation     = m_idleAnimation;
            }

            void move(const raylib::Vector2& toMove) override;
            void update(float timeDelta) override;
            void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;

        private:
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
            IMapper& m_mapper;
            ggj::ISoundManager& m_soundManager;
            Animation m_idleAnimation;
            Animation m_walkAnimation;
            Animation m_animation;
            float m_followSpeed { 0.f };
            float m_knockback   { 0.f };
            IPlayer* m_player { nullptr };
            bool m_isWalking { false };
    };

} // ggj

#endif //GGJ_CLAUDLING_H
