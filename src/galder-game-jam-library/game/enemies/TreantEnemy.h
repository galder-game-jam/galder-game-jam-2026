//

#ifndef GGJ_TREANTENEMY_H
#define GGJ_TREANTENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include <optional>
#include <vector>
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../interfaces/game/IPlayer.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include "../../interfaces/graphics/ITintShader.h"
#include "../../data/TreantRootConfig.hpp"
#include "ObstacleSteering.h"

namespace ggj
{
    class World;

    /*!
     * The phases of the treant's attack cycle: lumber towards the player, then either root itself and
     * glow red (the telegraph) before barrelling forward in a straight line, or rear up and throw
     * roots that spike the ground where they land.
     */
    enum class TreantState
    {
        Idle,      ///< Standing still between cycles.
        Walk,      ///< Slowly lumbering towards the player.
        Windup,    ///< Rooted and glowing red; the charge direction is locked when this ends.
        Charge,    ///< Fast, straight-line dash in the locked direction.
        Recover,   ///< Stunned after a charge, the best time to strike.
        RootThrow, ///< Rooted and glowing green; throws roots at the player when this ends.
        Dying      ///< Death throes: invulnerable and convulsing; ends by throwing roots across the whole map.
    };

    constexpr uint8_t TREANT_STATE_COUNT = 7; ///< Number of TreantState values; guards states received from the network.

    /*!
     * Forest boss. Slow on its own, but telegraphs a fast charge that punishes standing still. Below
     * half health it enrages: it moves faster, winds up quicker and chains two charges.
     */
    class TreantEnemy : public PhysicsSprite
    {
        public:
            /*! Impulse pushing a player away on contact when the map gives no `knockback`. */
            static constexpr float DEFAULT_KNOCKBACK = 0.f;

        TreantEnemy() = default;
        TreantEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                    const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData,
                    int lives, float followSpeed, float knockback, IPlayer* player, World* world, ITintShader& tintShader, const TreantRootConfig& rootConfig,
                    bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager { animationManager }, m_mapper { mapper },
              m_soundManager { soundManager }, m_followSpeed { followSpeed }, m_knockback { knockback }, m_maxLives { lives }, m_player { player }, m_world { world },
              m_tintShader { tintShader }, m_rootConfig { rootConfig }
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
        [[nodiscard]] EnemyNetworkState getNetworkState() const override;
        void applyNetworkState(const EnemyNetworkState& state) override;
        void onNetworkDamage(int32_t damage, uint32_t attackerPlayerId) override;

        private:
        /*!
         * Applies a hit. Only the host changes health; a client forwards the hit to it instead.
         *
         * @param remoteAttackerId Network player id of a client's hit, for kill credit
         */
        void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId = std::nullopt);

        /*! Starts the post-hit invulnerability window and its flashing. */
        void startInvulnerability();
        void setState(TreantState newState);

        /*!
         * Picks what follows a chase at random: a charge, a root throw or just a pause.
         *
         * @return Windup (charge), RootThrow or Idle
         */
        [[nodiscard]] TreantState chooseNextAttack() const;

        /*! Throws roots that land around every player, spread out and never inside walls or on top of each other. */
        void throwRoots();

        /*! Whether a landing spot is too close to one already chosen, which would make two roots overlap. */
        [[nodiscard]] static bool isTooCloseToAnyRoot(const std::vector<raylib::Vector2>& landings, const raylib::Vector2& landing);

        /*! Finishes the death throes: throws roots across the whole map, credits the kill and removes the treant. */
        void die();
        void faceTowards(const raylib::Vector2& direction);

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
         * Picks a random player to chase and charge. Avoids the previous target when someone else is
         * around, so consecutive charges go for different players. Only the host decides; clients
         * receive the choice through applyNetworkState().
         */
        void pickTarget();

        /*! Whether this peer decides the treant's targets: offline or hosting. */
        [[nodiscard]] bool isAuthority() const;

        /*!
         * Where the current target is.
         *
         * @return The target's position, or nothing when they have left (or there is no world to ask)
         */
        [[nodiscard]] std::optional<raylib::Vector2> targetPosition() const;

        /*!
         * Unit vector from the treant towards its current target.
         *
         * @return The direction, or a zero vector when there is no target or they are on top of the treant
         */
        [[nodiscard]] raylib::Vector2 directionToTarget() const;
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
        ITintShader& m_tintShader;
        TreantRootConfig m_rootConfig;
        std::optional<uint32_t> m_targetPlayerId {}; ///< Network player id being chased and charged; empty until picked.
        std::optional<uint32_t> m_remoteKillerId {}; ///< Client that landed the killing blow, when it was remote.
        PhysicsObject* m_killer { nullptr };         ///< Whoever landed the killing blow; credited once the death throes end.

        TreantState m_state { TreantState::Idle };
        float m_stateTimer { IDLE_DURATION };
        float m_stateDuration { IDLE_DURATION }; ///< Length of the current state, to turn m_stateTimer into progress.
        float m_animTime { 0.f };                ///< Seconds since spawn; drives the procedural animation.
        int32_t m_chargesLeft { 0 };             ///< Charges still to perform before returning to Idle.
        raylib::Vector2 m_chargeDirection { 0.f, 0.f };
        float m_enrageTint { 0.f }; ///< 0..1, fades in once enraged so the purple does not pop in.
        ObstacleSteering m_steering;

        // Invulnerability + flash
        float m_invulTimer { 0.f };
        float m_flashTimer { 0.f };

        static constexpr float INVUL_DURATION = 1.f;
        static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
        static constexpr float FLASH_INTERVAL = 0.1f;

        static constexpr float IDLE_DURATION = 1.f;
        static constexpr float ENRAGE_TINT_FADE_DURATION = 1.f;
        static constexpr ::Color ENRAGE_TINT_COLOR { 170, 60, 255, 255 };
        static constexpr float WALK_MIN_DURATION = 2.f;
        static constexpr float WALK_MAX_DURATION = 4.f;
        static constexpr float CHARGE_CHANCE = 0.4f; ///< Odds a chase ends in a charge; the rest is split between roots and a pause.
        static constexpr float ROOT_CHANCE = 0.4f;
        static constexpr float ENRAGED_CHARGE_CHANCE = 0.45f;
        static constexpr float ENRAGED_ROOT_CHANCE = 0.45f;

        static constexpr float ROOT_WINDUP_DURATION = 0.8f;
        static constexpr float ENRAGED_ROOT_WINDUP_DURATION = 0.55f;
        static constexpr float DYING_DURATION = 2.f;
        static constexpr float DEATH_ROOT_MIN_FLIGHT = 0.6f; ///< Death roots fly for different times so they rain down in a spread.
        static constexpr float DEATH_ROOT_MAX_FLIGHT = 1.4f;
        static constexpr float ROOT_FLIGHT_DURATION = 0.5f;
        static constexpr float ROOT_MIN_SCATTER = 70.f; ///< Pixels from a player the extra roots land at.
        static constexpr float ROOT_MAX_SCATTER = 220.f;
        static constexpr float ROOT_MIN_SPACING = 64.f; ///< Pixels between roots; a bit more than a root's diameter.
        static constexpr int32_t ROOT_PLACEMENT_ATTEMPTS = 12;
        static constexpr float ROOT_AIMED_SCATTER = 20.f;    ///< The first root lands almost on the player.
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
