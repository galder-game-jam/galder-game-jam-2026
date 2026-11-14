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
        void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage) override;

        private:
        void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage);
        void setState(TreantState newState);
        void faceTowards(const raylib::Vector2& direction);

        /*!
         * Whether the treant's body would run into something solid when moving along a direction. Casts
         * three parallel rays (centre and both flanks) so the treant's width is accounted for, not just
         * its centre line.
         *
         * @param direction Unit vector in world space
         * @return True when a non-sensor static fixture (a tree, a wall) lies within the lookahead distance
         */
        [[nodiscard]] bool isPathBlocked(const raylib::Vector2& direction) const;

        /*!
         * Bends a desired heading around static obstacles. Returns it untouched when the way is clear,
         * otherwise the closest free heading, preferring the side it last went around so it does not
         * dither in front of a tree.
         *
         * @param desired Unit vector towards where the treant wants to go
         * @return Unit vector to walk along
         */
        [[nodiscard]] raylib::Vector2 steerAroundObstacles(const raylib::Vector2& desired);

        /*! Uniform random float in [min, max]. */
        [[nodiscard]] static float randomRange(float min, float max);

        /*!
         * How the single sprite frame is deformed this frame to fake animation. Squash and stretch are
         * applied around the feet so the treant stays planted on the ground.
         */
        struct TreantPose
        {
            float rotation { 0.f }; ///< Degrees, clockwise; leans the treant around its feet.
            float scaleX { 1.f };
            float scaleY { 1.f };
        };

        /*!
         * Picks the pose for the current state: breathing when idle, a waddle when walking, a coiled
         * shake during the windup, a forward lean while charging and a slump while recovering.
         */
        [[nodiscard]] TreantPose computePose() const;

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
        float m_stateDuration { IDLE_DURATION }; ///< Length of the current state, to turn m_stateTimer into progress.
        float m_animTime { 0.f };                ///< Seconds since spawn; drives the procedural animation.
        int32_t m_chargesLeft { 0 };             ///< Charges still to perform before returning to Idle.
        raylib::Vector2 m_chargeDirection { 0.f, 0.f };
        float m_avoidSide { 1.f }; ///< +1/-1: which side of an obstacle the treant is currently going around.

        // Invulnerability + flash
        float m_invulTimer { 0.f };
        float m_flashTimer { 0.f };

        static constexpr float INVUL_DURATION = 1.f;
        static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
        static constexpr float FLASH_INTERVAL = 0.1f;

        static constexpr float IDLE_DURATION = 1.f;
        static constexpr float WALK_MIN_DURATION = 2.f;
        static constexpr float WALK_MAX_DURATION = 4.f;
        static constexpr float CHARGE_CHANCE = 0.5f; ///< Odds a chase ends in a charge rather than a pause.
        static constexpr float ENRAGED_CHARGE_CHANCE = 0.75f;
        static constexpr float OBSTACLE_LOOKAHEAD = 2.5f;    ///< Metres past the treant's edge to scan for obstacles.
        static constexpr float OBSTACLE_FLANK_FACTOR = 0.9f; ///< Flank rays sit this fraction of the radius off-centre.
        static constexpr float WINDUP_DURATION = 0.9f;
        static constexpr float ENRAGED_WINDUP_DURATION = 0.55f;
        static constexpr float CHARGE_DURATION = 0.8f;
        static constexpr float RECOVER_DURATION = 1.4f;
        static constexpr float CHARGE_SPEED_MULTIPLIER = 3.f;
        static constexpr float ENRAGED_SPEED_MULTIPLIER = 1.5f;
        static constexpr float CHARGE_KNOCKBACK_MULTIPLIER = 1.5f;
        static constexpr float WALK_STEPS_PER_SECOND = 2.f;
        static constexpr float CHARGE_STEPS_PER_SECOND = 6.f;
    };

} // namespace ggj

#endif //GGJ_TREANTENEMY_H
