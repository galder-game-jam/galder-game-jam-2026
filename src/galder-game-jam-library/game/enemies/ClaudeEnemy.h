//

#ifndef GGJ_CLAUDEENEMY_H
#define GGJ_CLAUDEENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../interfaces/game/IPlayer.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"

namespace ggj
{
    class World;

    enum class ClaudeState { Idle, Walk, Attack };

    class ClaudeEnemy : public PhysicsSprite
    {
        public:
            ClaudeEnemy() = default;
            ClaudeEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                const raylib::Vector2& physicsSize,
                const raylib::Vector2& spriteSize,
                const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData,
                int lives, float followSpeed, float knockback, IPlayer* player, World* world, bool isVisible = true)
                : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible),
                  m_animationManager { animationManager }, m_mapper { mapper }, m_soundManager { soundManager },
                  m_followSpeed { followSpeed }, m_knockback { knockback }, m_player { player }, m_world { world }
            {
                m_lives = lives;
                m_body->SetFixedRotation(true);
                m_idleAnimation   = m_animationManager.getAnimation(AnimationName::EnemyClaudeIdle);
                m_walkAnimation   = m_animationManager.getAnimation(AnimationName::EnemyClaudeWalk);
                m_attackAnimation = m_animationManager.getAnimation(AnimationName::EnemyClaudeAttack);
                m_animation       = m_idleAnimation;
            }

            void move(const raylib::Vector2& toMove) override;
            void update(float timeDelta) override;
            void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
            void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage) override;

        private:
            void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage);
            void setBossState(ClaudeState newState);

            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
            IMapper& m_mapper;
            ggj::ISoundManager& m_soundManager;

            Animation m_idleAnimation;
            Animation m_walkAnimation;
            Animation m_attackAnimation;
            Animation m_animation;

            float m_followSpeed { 0.f };
            float m_knockback   { 0.f };

            IPlayer* m_player { nullptr };
            World*   m_world  { nullptr };

            // Boss state machine
            bool       m_isSittingDuck  { false };  ///< Force Idle for testing; set to true to disable AI.
            ClaudeState m_bossState     { ClaudeState::Idle };
            float m_stateTimer          { 2.5f };
            float m_stateInterval       { 2.5f };   ///< Seconds between state rolls.

            // Invulnerability + flash
            float m_invulTimer { 0.f };
            float m_flashTimer { 0.f };
            static constexpr float INVUL_DURATION = 1.f;
            static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
            static constexpr float FLASH_INTERVAL = 0.1f;
    };

} // ggj

#endif //GGJ_CLAUDEENEMY_H
