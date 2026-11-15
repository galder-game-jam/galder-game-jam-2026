#ifndef GGJ_MAGEBOSSENEMY_H
#define GGJ_MAGEBOSSENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include "../../interfaces/graphics/ITintShader.h"
#include "../../data/MageBossConfig.hpp"
#include "ObstacleSteering.h"
#include <optional>
#include <vector>

namespace ggj
{
    class World;

    /*!
     * The phases of the mage's attack cycle: drift towards a player, then cast one of three spells.
     * Synced to clients as a uint8_t, so keep MAGE_STATE_COUNT in step.
     */
    enum class MageState : uint8_t
    {
        Idle,          ///< Standing still between cycles.
        Walk,          ///< Drifting towards a player.
        IceCast,       ///< Hovering with the ice circles on the floor, flashing blue until they spring shut.
        FireCast,      ///< Rooted and flashing orange while the lines of fire are telegraphed; they ignite when it ends.
        CrystalWindup, ///< Charging pink crystals, staring down his victim.
        CrystalBursts, ///< Firing the bursts of crystals at the victim.
        Recover,       ///< Catching his breath after a spell, before the next cycle.
        Dying          ///< Death throes: invulnerable and flickering; ends in a ring of crystals.
    };

    constexpr uint8_t MAGE_STATE_COUNT = 8; ///< Number of MageState values; guards states received from the network.

    /*!
     * Dungeon boss: a scholar gone bad. Slow on its own, but casts three spells.
     *  - Ice: two to four big circles of frost appear around the map and, a moment later, freeze
     *    whoever has not left them.
     *  - Fire: lines of fire shoot out from where it stands (4 directions, 8 once enraged) all the way
     *    to the edge of the map, so there is nowhere to hide on the row or column it picks.
     *  - Crystals: three bursts of pink crystals at one randomly chosen player.
     *
     * Below half health it enrages: faster, more fire lines and bigger bursts.
     *
     * On the host (or offline) it rolls its own attacks. Clients never do: they mirror the host's
     * state, target and lives, and send their hits to the host, so every peer fights the same boss
     * with the same health bar. The spells themselves are created on every peer from the host's
     * spawn events.
     */
    class MageBossEnemy : public PhysicsSprite
    {
        public:
        MageBossEnemy() = default;
        MageBossEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                      const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture,
                      const UserData& userData, int lives, float followSpeed, float knockback, World* world, ITintShader& tintShader, const MageBossConfig& config,
                      bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager { animationManager }, m_mapper { mapper },
              m_soundManager { soundManager }, m_followSpeed { followSpeed }, m_knockback { knockback }, m_maxLives { lives }, m_world { world }, m_tintShader { tintShader },
              m_config { config }
        {
            m_lives = lives;
            m_body->SetFixedRotation(true);
            m_animation = m_animationManager.getAnimation(AnimationName::EnemyMageBossIdle);
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
         * How the single sprite frame is deformed this frame to fake animation.
         */
        struct MagePose
        {
            float rotation { 0.f }; ///< Degrees around his feet: positive leans forward (the way he faces).
            float scaleX { 1.f };
            float scaleY { 1.f };
            float lift { 0.f }; ///< Pixels he hovers above the ground, in sprite space.
        };

        /*!
         * Picks the pose for a state: a gentle hover when idle, a bob when drifting, rising with
         * trembling arms for each spell and convulsions when dying.
         *
         * @param state Current state
         * @param progress How far through the state it is, 0 (just started) to 1 (about to end)
         * @param time Seconds since spawn; drives the wobble
         * @return Lean (degrees, positive is forward), squash/stretch and hover height
         */
        [[nodiscard]] static MagePose ComputePose(MageState state, float progress, float time);

        /*!
         * Picks what follows a walk from a roll.
         *
         * @param roll Uniform random number in [0, 1]
         * @param isEnraged Whether the mage is below half health
         * @return IceCast, FireCast, CrystalWindup or Idle
         */
        [[nodiscard]] static MageState ChooseAttack(float roll, bool isEnraged);

        /*!
         * How many ice circles one cast conjures.
         *
         * @param config Tuning with the smallest and largest count
         * @param roll Uniform random number in [0, 1]
         * @return A count between the config's minimum and maximum, both included
         */
        [[nodiscard]] static int32_t RollIceCircleCount(const MageBossConfig& config, float roll);

        /*!
         * How many lines of fire one cast shoots.
         *
         * @param isEnraged Whether the mage is below half health
         * @return FIRE_LINES, or ENRAGED_FIRE_LINES when enraged
         */
        [[nodiscard]] static int32_t FireLineCount(bool isEnraged);

        /*!
         * Directions of the lines of fire: evenly spaced all the way round, so there is always a gap
         * to stand in but never a long one.
         *
         * @param startAngle Direction of the first line (radians)
         * @param count Number of lines
         * @return One angle per line (radians); empty for zero or less
         */
        [[nodiscard]] static std::vector<float> FireLineAngles(float startAngle, int32_t count);

        static constexpr int32_t FIRE_LINES = 4;         ///< Lines of fire per cast.
        static constexpr int32_t ENRAGED_FIRE_LINES = 8; ///< Lines of fire per cast once enraged.
        static constexpr int32_t CRYSTAL_BURSTS = 3;     ///< Bursts of crystals per cast.
        static constexpr int32_t CRYSTALS_PER_BURST = 5; ///< Crystals in each burst.
        static constexpr int32_t ENRAGED_CRYSTALS_PER_BURST = 7;
        static constexpr float ICE_MIN_DELAY = 0.5f; ///< Shortest time from an ice circle appearing to it freezing.
        static constexpr float ICE_MAX_DELAY = 1.0f; ///< Longest time from an ice circle appearing to it freezing.

        private:
        /*!
         * Applies a hit. Only the host changes health; a client forwards the hit to it instead.
         *
         * @param remoteAttackerId Network player id of a client's hit, for kill credit
         */
        void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId = std::nullopt);

        /*! Starts the post-hit invulnerability window and its flashing. */
        void startInvulnerability();
        void setState(MageState newState);

        /*! Host: conjures the ice circles. */
        void castIce();
        /*! Host: shoots the lines of fire; they ignite when the cast state ends. @param warningSeconds Telegraph time. */
        void castFire(float warningSeconds);
        /*! Host: fires one burst of crystals at the player picked when the windup began. */
        void fireCrystalBurst();
        /*! Finishes the death throes: a ring of crystals, credits the kill and removes the mage. */
        void die();

        /*! Picks a random player to chase and, for the crystals, to shoot at. Only the host decides; clients receive the choice through applyNetworkState(). */
        void pickTarget();
        /*! Whether this peer decides the mage's attacks: offline or hosting. */
        [[nodiscard]] bool isAuthority() const;
        /*! Where the current target is; nothing when they have left or none was picked. */
        [[nodiscard]] std::optional<raylib::Vector2> targetPosition() const;
        /*! Unit vector from the mage towards his target; zero when there is none or they are on top of him. */
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
        MageBossConfig m_config;
        ObstacleSteering m_steering;
        std::optional<uint32_t> m_targetPlayerId {}; ///< Network player id being chased and, for the crystals, shot at; empty until picked.
        std::optional<uint32_t> m_remoteKillerId {}; ///< Client that landed the killing blow, when it was remote.
        PhysicsObject* m_killer { nullptr };         ///< Whoever landed the killing blow; credited once the death throes end.

        MageState m_state { MageState::Idle };
        float m_stateTimer { OPENING_IDLE_DURATION };
        float m_stateDuration { OPENING_IDLE_DURATION }; ///< Length of the current state, to turn m_stateTimer into progress.
        float m_animTime { 0.f };                        ///< Seconds since spawn; drives the procedural animation.
        int32_t m_burstsLeft { 0 };                      ///< Crystal bursts still to fire in this attack (host only).
        float m_burstTimer { 0.f };                      ///< Seconds until the next burst (host only).
        float m_enrageTint { 0.f };                      ///< 0..1, fades in once enraged so the red does not pop in.

        // Invulnerability + flash
        float m_invulTimer { 0.f };
        float m_flashTimer { 0.f };

        static constexpr float INVUL_DURATION = 1.f;
        static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
        static constexpr float FLASH_INTERVAL = 0.1f;

        static constexpr float OPENING_IDLE_DURATION = 2.f; ///< A moment to take in the arena before the first attack.
        static constexpr float IDLE_DURATION = 0.8f;
        static constexpr float ENRAGED_IDLE_DURATION = 0.4f;
        static constexpr float WALK_MIN_DURATION = 1.2f;
        static constexpr float WALK_MAX_DURATION = 2.2f;
        static constexpr float ENRAGED_SPEED_MULTIPLIER = 1.3f;
        static constexpr float ICE_CAST_DURATION = 1.25f; ///< Longer than ICE_MAX_DELAY so the last circle has sprung shut before he moves on.
        static constexpr float ENRAGED_ICE_CAST_DURATION = 1.15f;
        static constexpr float FIRE_WARNING_DURATION = 1.0f; ///< Telegraph of the fire lines; also how long he charges before they ignite.
        static constexpr float ENRAGED_FIRE_WARNING_DURATION = 0.75f;
        static constexpr float FIRE_CAST_AFTERGLOW = 0.35f; ///< He stays rooted a moment after the fire ignites, so the origin is not a safe place to run through.
        static constexpr float CRYSTAL_WINDUP_DURATION = 0.9f;
        static constexpr float ENRAGED_CRYSTAL_WINDUP_DURATION = 0.6f;
        static constexpr float CRYSTAL_BURST_INTERVAL = 0.55f;
        static constexpr float ENRAGED_CRYSTAL_BURST_INTERVAL = 0.42f;
        static constexpr float CRYSTAL_SPREAD_DEGREES = 26.f; ///< Width of the fan each burst is shot in.
        static constexpr float CRYSTAL_FINAL_LINGER = 0.4f;   ///< Time after the last burst before he recovers.
        static constexpr float RECOVER_DURATION = 0.5f;
        static constexpr float DYING_DURATION = 2.2f;
        static constexpr int32_t DEATH_CRYSTAL_COUNT = 20; ///< Crystals in the ring he dies in.
        static constexpr float CRYSTAL_RING_RADIUS = 30.f; ///< Pixels from the mage's centre the death crystals start at.
        static constexpr float ICE_ATTACK_CHANCE = 0.3f;   ///< Odds of the ice spell after a walk; the rest is split between fire, crystals and a pause.
        static constexpr float FIRE_ATTACK_CHANCE = 0.3f;
        static constexpr float CRYSTAL_ATTACK_CHANCE = 0.3f;
        static constexpr float ENRAGED_ICE_ATTACK_CHANCE = 0.3f;
        static constexpr float ENRAGED_FIRE_ATTACK_CHANCE = 0.33f;
        static constexpr float ENRAGED_CRYSTAL_ATTACK_CHANCE = 0.34f;
        static constexpr float HOVER_HEIGHT = 10.f; ///< Pixels he rises while casting, at the peak of a cast.
        static constexpr ::Color ENRAGE_TINT_COLOR { 230, 40, 40, 255 };
        static constexpr float ENRAGE_TINT_STRENGTH = 0.55f; ///< Share of the tint colour at full enrage; low enough to keep his shading readable.
        static constexpr float ENRAGE_TINT_FADE_DURATION = 1.f;
    };

} // namespace ggj

#endif // GGJ_MAGEBOSSENEMY_H
