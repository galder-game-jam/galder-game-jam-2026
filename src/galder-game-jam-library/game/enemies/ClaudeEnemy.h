//

#ifndef GGJ_CLAUDEENEMY_H
#define GGJ_CLAUDEENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"

namespace ggj
{

    class ClaudeEnemy : public PhysicsSprite
    {
        public:
            ClaudeEnemy() = default;
            ClaudeEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                const raylib::Vector2& physicsSize,
                const raylib::Vector2& spriteSize,
                const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData, int lives, bool isVisible = true)
                : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible),
                  m_animationManager { animationManager }, m_mapper { mapper }, m_soundManager { soundManager }, m_lives { lives }
            {
                m_body->SetFixedRotation(true);
                m_animation = m_animationManager.getAnimation(AnimationName::EnemyClaudeIdle);
            }

            void move(const raylib::Vector2& toMove) override;
            void update(float timeDelta) override;
            void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;

        private:
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
            IMapper& m_mapper;
            ggj::ISoundManager& m_soundManager;
            Animation m_animation;
            int m_lives { 3 };
    };

} // ggj

#endif //GGJ_CLAUDEENEMY_H
