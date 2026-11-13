//

#ifndef GGJ_TREANTENEMY_H
#define GGJ_TREANTENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../interfaces/game/IPlayer.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"

namespace ggj
{
    class World;

    /*!
     * The phases of the treant's attack cycle: lumber towards the player, root itself and glow red
     * (the telegraph), then barrel forward in a straight line and catch its breath.
     */
    enum class TreantState
    {
        Idle,   ///< Standing still between cycles.
        Walk,   ///< Slowly lumbering towards the player.
        Windup, ///< Rooted and glowing red; the charge direction is locked when this ends.
        Charge, ///< Fast, straight-line dash in the locked direction.
        Recover ///< Stunned after a charge, the best time to strike.
    };

    /*!
     * Forest boss. Slow on its own, but telegraphs a fast charge that punishes standing still. Below
     * half health it enrages: it moves faster, winds up quicker and chains two charges.
     */
    class TreantEnemy : public PhysicsSprite
    {
        public:
        TreantEnemy() = default;
        TreantEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                    const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData,
                    int lives, float followSpeed, float knockback, IPlayer* player, World* world, bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager { animationManager }, m_mapper { mapper },
              m_soundManager { soundManager }, m_followSpeed { followSpeed }, m_knockback { knockback }, m_maxLives { lives }, m_player { player }, m_world { world }
        {
            m_lives = lives;
            m_body->SetFixedRotation(true);
            m_animation = m_animationManager.getAnimation(AnimationName::EnemyTreantIdle);
        }

        void move(const raylib::Vector2& toMove) override;
        void update(float timeDelta) override;
        void draw() override;
        void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
        void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker) override;

        private:
        void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft);
        void setState(TreantState newState);
        void faceTowards(const raylib::Vector2& direction);

        /*!
             * Unit vector from the treant towards the player.
             *
             * @return The direction, or a zero vector when there is no player or they are on top of the treant
             */
        [[nodiscard]] raylib::Vector2 directionToPlayer() const;
        [[nodiscard]] bool isEnraged() const { return m_lives * 2 <= m_maxLives; }

        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
        IMapper& m_mapper;
        ggj::ISoundManager& m_soundManager;

        Animation m_animation;

        float m_followSpeed { 0.f };
        float m_knockback { 0.f };
        int m_maxLives { 0 };

        IPlayer* m_player { nullptr };
        World* m_world { nullptr };

        TreantState m_state { TreantState::Idle };
        float m_stateTimer { IDLE_DURATION };
        int32_t m_chargesLeft { 0 }; ///< Charges still to perform before returning to Idle.
        raylib::Vector2 m_chargeDirection { 0.f, 0.f };

        // Invulnerability + flash
        float m_invulTimer { 0.f };
        float m_flashTimer { 0.f };

        static constexpr float INVUL_DURATION = 1.f;
        static constexpr float FLASH_INTERVAL = 0.1f;

        static constexpr float IDLE_DURATION = 1.f;
        static constexpr float WALK_DURATION = 3.f;
        static constexpr float WINDUP_DURATION = 0.9f;
        static constexpr float ENRAGED_WINDUP_DURATION = 0.55f;
        static constexpr float CHARGE_DURATION = 0.8f;
        static constexpr float RECOVER_DURATION = 1.4f;
        static constexpr float CHARGE_SPEED_MULTIPLIER = 3.f;
        static constexpr float ENRAGED_SPEED_MULTIPLIER = 1.5f;
        static constexpr float CHARGE_KNOCKBACK_MULTIPLIER = 1.5f;
    };

} // namespace ggj

#endif //GGJ_TREANTENEMY_H
