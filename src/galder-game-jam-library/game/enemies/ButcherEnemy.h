#ifndef GGJ_BUTCHERENEMY_H
#define GGJ_BUTCHERENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include "../../interfaces/graphics/ITintShader.h"
#include "../../data/ButcherConfig.hpp"
#include "ObstacleSteering.h"
#include <optional>

namespace ggj
{
    class World;

    /*!
     * The phases of the butcher's attack cycle: lumber towards a player, then wind up either a ring
     * of thrown cleavers (flashing red) or a spider hurled at one random player (green). Synced to
     * clients as a uint8_t, so keep BUTCHER_STATE_COUNT in step.
     */
    enum class ButcherState : uint8_t
    {
        Idle,          ///< Standing still between cycles.
        Walk,          ///< Lumbering towards a player.
        CleaverWindup, ///< Rearing back, flashing red; throws a ring of cleavers when this ends.
        SpiderWindup,  ///< Hunched and glowing green, staring down his victim; throws the spider when this ends.
        Recover,       ///< Follow-through right after a throw, before the next volley (enraged) or a pause.
        Dying          ///< Death throes: invulnerable and convulsing; ends by throwing cleavers in every direction.
    };

    constexpr uint8_t BUTCHER_STATE_COUNT = 6; ///< Number of ButcherState values; guards states received from the network.

    /*!
     * Dungeon boss. Slow on its own, but fills the arena with spinning cleavers that only cover can
     * stop, and lobs a spider at a random player that stuns whoever it hits. Below half health it
     * enrages: it throws more cleavers, chains a second ring in between the first and winds up faster.
     *
     * On the host (or offline) it rolls its own attacks. Clients never do: they mirror the host's
     * state, target and lives, and send their hits to the host, so every peer fights the same boss
     * with the same health bar. The cleavers and spiders themselves are created on every peer from
     * the host's spawn events.
     */
    class ButcherEnemy : public PhysicsSprite
    {
        public:
            /*! Impulse pushing a player away on contact when the map gives no `knockback`. */
            static constexpr float DEFAULT_KNOCKBACK = 0.f;

        ButcherEnemy() = default;
        ButcherEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                     const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture,
                     const UserData& userData, int lives, float followSpeed, float knockback, World* world, ITintShader& tintShader, const ButcherConfig& config,
                     bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager { animationManager }, m_mapper { mapper },
              m_soundManager { soundManager }, m_followSpeed { followSpeed }, m_knockback { knockback }, m_maxLives { lives }, m_world { world }, m_tintShader { tintShader },
              m_config { config }
        {
            m_lives = lives;
            m_body->SetFixedRotation(true);
            m_animation = m_animationManager.getAnimation(AnimationName::EnemyButcherIdle);
        }

        void move(const raylib::Vector2& toMove) override;
        void update(float timeDelta) override;
        void draw() override;
        void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
        void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage) override;
        [[nodiscard]] EnemyNetworkState getNetworkState() const override;
        void applyNetworkState(const EnemyNetworkState& state) override;
        void onNetworkDamage(int32_t damage, uint32_t attackerPlayerId) override;

        /*!
         * How the single sprite frame is deformed this frame to fake animation. Squash and stretch are
         * applied around the feet so the butcher stays planted on the ground.
         */
        struct ButcherPose
        {
            float rotation { 0.f }; ///< Degrees around his feet: positive leans forward (the way he faces), negative rears back.
            float scaleX { 1.f };
            float scaleY { 1.f };
        };

        /*!
         * Picks the pose for a state: breathing when idle, a waddle when walking, rearing back for the
         * cleaver windup, a crouch for the spider, a lunge on the follow-through and convulsions when dying.
         *
         * @param state Current state
         * @param progress How far through the state it is, 0 (just started) to 1 (about to end)
         * @param time Seconds since spawn; drives the wobble
         * @return Lean (degrees, positive is forward) and squash/stretch
         */
        [[nodiscard]] static ButcherPose ComputePose(ButcherState state, float progress, float time);

        /*!
         * Picks what follows a walk from a roll.
         *
         * @param roll Uniform random number in [0, 1]
         * @param isEnraged Whether the butcher is below half health
         * @return CleaverWindup, SpiderWindup or Idle
         */
        [[nodiscard]] static ButcherState ChooseAttack(float roll, bool isEnraged);

        /*!
         * Direction of the first cleaver of the next ring in a chain. Offsets it by half the spacing so
         * the second ring flies through the gaps of the first and there is no safe wedge to stand in.
         *
         * @param previousStartAngle Start angle of the previous ring (radians)
         * @param cleaverCount Number of cleavers per ring
         * @return Start angle for the next ring (radians)
         */
        [[nodiscard]] static float NextRingStartAngle(float previousStartAngle, int32_t cleaverCount);

        private:
        /*!
         * Applies a hit. Only the host changes health; a client forwards the hit to it instead.
         *
         * @param remoteAttackerId Network player id of a client's hit, for kill credit
         */
        void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId = std::nullopt);

        /*! Starts the post-hit invulnerability window and its flashing. */
        void startInvulnerability();
        void setState(ButcherState newState);

        /*! Host: throws a ring of cleavers in every direction. @param startAngle Direction of the first one (radians). */
        void throwCleavers(float startAngle);
        /*! Host: throws a spider at the player picked when the windup began. */
        void throwSpider();
        /*! Finishes the death throes: throws cleavers in every direction, credits the kill and removes the butcher. */
        void die();

        /*! Picks a random player to chase and, for the spider, to throw it at. Only the host decides; clients receive the choice through applyNetworkState(). */
        void pickTarget();
        /*! Whether this peer decides the butcher's attacks: offline or hosting. */
        [[nodiscard]] bool isAuthority() const;
        /*! Where the current target is; nothing when they have left or none was picked. */
        [[nodiscard]] std::optional<raylib::Vector2> targetPosition() const;
        /*! Unit vector from the butcher towards his target; zero when there is none or they are on top of him. */
        [[nodiscard]] raylib::Vector2 directionToTarget() const;
        void faceTowards(const raylib::Vector2& direction);
        [[nodiscard]] bool isEnraged() const { return m_lives * 2 <= m_maxLives; }
        [[nodiscard]] float getStateProgress() const;
        /*! Uniform random float in [min, max]. */
        [[nodiscard]] static float randomRange(float min, float max);

        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
        IMapper& m_mapper;
        ggj::ISoundManager& m_soundManager;

        Animation m_animation;

        float m_followSpeed { 0.f };
        float m_knockback { 0.f };
        int32_t m_maxLives { 0 };

        World* m_world { nullptr };
        ITintShader& m_tintShader;
        ButcherConfig m_config;
        ObstacleSteering m_steering;
        std::optional<uint32_t> m_targetPlayerId {}; ///< Network player id being chased and, for the spider, targeted; empty until picked.
        std::optional<uint32_t> m_remoteKillerId {}; ///< Client that landed the killing blow, when it was remote.
        PhysicsObject* m_killer { nullptr };         ///< Whoever landed the killing blow; credited once the death throes end.

        ButcherState m_state { ButcherState::Idle };
        float m_stateTimer { OPENING_IDLE_DURATION };
        float m_stateDuration { OPENING_IDLE_DURATION }; ///< Length of the current state, to turn m_stateTimer into progress.
        float m_animTime { 0.f };                        ///< Seconds since spawn; drives the procedural animation.
        int32_t m_volleysLeft { 0 };                     ///< Cleaver rings still to throw in this attack (host only).
        int32_t m_ringsThrown { 0 };                     ///< Cleaver rings already thrown in this attack (host only).
        float m_lastRingAngle { 0.f };                   ///< Start angle of the last ring, so a chained ring can be offset from it (host only).
        float m_enrageTint { 0.f };                      ///< 0..1, fades in once enraged so the red does not pop in.

        // Invulnerability + flash
        float m_invulTimer { 0.f };
        float m_flashTimer { 0.f };

        static constexpr float INVUL_DURATION = 1.f;
        static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
        static constexpr float FLASH_INTERVAL = 0.1f;

        static constexpr float OPENING_IDLE_DURATION = 2.f; ///< A moment to take in the arena before the first attack.
        static constexpr float IDLE_DURATION = 0.9f;
        static constexpr float ENRAGED_IDLE_DURATION = 0.5f;
        static constexpr float WALK_MIN_DURATION = 1.5f;
        static constexpr float WALK_MAX_DURATION = 2.5f;
        static constexpr float ENRAGED_SPEED_MULTIPLIER = 1.3f;
        static constexpr float CLEAVER_WINDUP_DURATION = 1.1f;
        static constexpr float ENRAGED_CLEAVER_WINDUP_DURATION = 0.7f;
        static constexpr float CHAINED_WINDUP_DURATION = 0.5f; ///< Windup of the second ring in an enraged chain: only a beat after the first.
        static constexpr float SPIDER_WINDUP_DURATION = 1.1f;
        static constexpr float ENRAGED_SPIDER_WINDUP_DURATION = 0.8f;
        static constexpr float RECOVER_DURATION = 0.35f;
        static constexpr float DYING_DURATION = 2.2f;
        static constexpr int32_t ENRAGED_VOLLEYS = 2;        ///< Rings per cleaver attack once enraged; one before.
        static constexpr float CLEAVER_RING_RADIUS = 36.f;   ///< Pixels from the butcher's centre the cleavers start at, so they leave his body.
        static constexpr float CLEAVER_ATTACK_CHANCE = 0.5f; ///< Odds of a cleaver ring after a walk; the rest is split between spider and a pause.
        static constexpr float SPIDER_ATTACK_CHANCE = 0.4f;
        static constexpr float ENRAGED_CLEAVER_ATTACK_CHANCE = 0.5f;
        static constexpr float ENRAGED_SPIDER_ATTACK_CHANCE = 0.45f;
        static constexpr float WALK_STEPS_PER_SECOND = 2.f;
        static constexpr ::Color ENRAGE_TINT_COLOR { 230, 40, 40, 255 };
        static constexpr float ENRAGE_TINT_STRENGTH = 0.55f; ///< Share of the tint colour at full enrage; low enough to keep his shading readable.
        static constexpr float ENRAGE_TINT_FADE_DURATION = 1.f;
    };

} // namespace ggj

#endif // GGJ_BUTCHERENEMY_H
