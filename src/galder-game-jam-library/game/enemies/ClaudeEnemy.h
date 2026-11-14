//

#ifndef GGJ_CLAUDEENEMY_H
#define GGJ_CLAUDEENEMY_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include "../../interfaces/graphics/IPaletteSwapShader.h"
#include "../../data/ClaudeBombConfig.hpp"
#include <optional>

namespace ggj
{
    class World;

    /*! Boss AI state. Synced to clients as a uint8_t, so keep CLAUDE_STATE_COUNT in step. */
    enum class ClaudeState : uint8_t
    {
        Idle,
        Walk,
        Attack,
        Transform, ///< One-off enrage at half health: invulnerable, shaking, recolouring; ends with a bomb throw.
        BombLaunch, ///< Enraged only: shakes for a while, then throws bombs at every player.
        Dying       ///< Out of lives: invulnerable and shaking, then dies in a ring of bombs.
    };
    constexpr uint8_t CLAUDE_STATE_COUNT = 6;

    /*!
     * The "King Claude" boss. On the host (or offline) it rolls a new state every few seconds:
     * idle, chase a randomly picked player, or spawn one Claudling per player. At half health it
     * transforms (black body, red eyes) and from then on also throws bombs and picks states faster.
     * Clients never roll their own: they mirror the host's state, target and lives, and send their
     * hits to the host, so every peer fights the same boss with the same health bar.
     */
    class ClaudeEnemy : public PhysicsSprite
    {
        public:
            ClaudeEnemy() = default;
            ClaudeEnemy(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper, ggj::ISoundManager& soundManager, b2Body* body,
                        const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture,
                        const UserData& userData, int lives, float followSpeed, float knockback, World* world, IPaletteSwapShader& paletteSwap, const ClaudeBombConfig& bombConfig,
                        bool isVisible = true)
                : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager { animationManager }, m_mapper { mapper },
                  m_soundManager { soundManager }, m_paletteSwap { paletteSwap }, m_followSpeed { followSpeed }, m_knockback { knockback }, m_world { world },
                  m_bombConfig { bombConfig }, m_maxLives { lives }
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
            void draw() override;
            void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
            void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage) override;

            [[nodiscard]] EnemyNetworkState getNetworkState() const override;
            void applyNetworkState(const EnemyNetworkState& state) override;
            void onNetworkDamage(int32_t damage, uint32_t attackerPlayerId) override;

            /*!
             * Colour swap for the enrage: the orange body fades to near-black and the black eyes to red.
             *
             * @param progress 0 = normal colours (empty swap), 1 = fully enraged
             * @return Swap to pass to the palette swap shader
             */
            [[nodiscard]] static PaletteSwap BuildEnragePaletteSwap(float progress);

            static constexpr ::Color BODY_COLOR { 217, 119, 87, 255 };
            static constexpr ::Color EYE_COLOR { 0, 0, 0, 255 };
            static constexpr ::Color ENRAGED_BODY_COLOR { 28, 28, 28, 255 };
            static constexpr ::Color ENRAGED_EYE_COLOR { 220, 30, 30, 255 };

        private:
            /*!
             * Applies a hit (host/offline) or forwards it to the host (client).
             *
             * @param attacker Local player that hit, for kill credit; nullptr for a remote hit
             * @param remoteAttackerId Network player id of a client's hit, for kill credit
             */
            void takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId = std::nullopt);
            /*! Host: the death shake is over. Throws the bomb ring and removes the boss. */
            void die();
            void setBossState(ClaudeState newState);
            /*! Host: the current state's timer ran out. Throws bombs if due, then moves on. */
            void finishState();
            /*! Host: picks the next state at random, never repeating the current one. */
            void rollNextState();
            void throwBombs();
            /*! How far the enrage recolour has come: 0 before, rising during Transform, 1 after. */
            [[nodiscard]] float getEnrageProgress() const;
            [[nodiscard]] bool isShaking() const { return m_bossState == ClaudeState::Transform || m_bossState == ClaudeState::BombLaunch || m_bossState == ClaudeState::Dying; }
            /*! Picks a random player in the session to chase. Host/offline only. */
            void pickTarget();
            /*! Starts the post-hit invulnerability window and its sprite flashing. */
            void startInvulnerability();
            [[nodiscard]] bool isAuthority() const;

            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
            IMapper& m_mapper;
            ggj::ISoundManager& m_soundManager;
            IPaletteSwapShader& m_paletteSwap;

            Animation m_idleAnimation;
            Animation m_walkAnimation;
            Animation m_attackAnimation;
            Animation m_animation;

            float m_followSpeed { 0.f };
            float m_knockback { 0.f };

            World* m_world { nullptr };
            uint32_t m_targetPlayerId { 0 }; ///< Network player id chased in ClaudeState::Walk.
            ClaudeBombConfig m_bombConfig {};
            int32_t m_maxLives { 1 };

            // Boss state machine
            bool m_isSittingDuck { false }; ///< Force Idle for testing; set to true to disable AI.
            ClaudeState m_bossState { ClaudeState::Idle };
            float m_stateTimer { 2.5f };
            float m_stateInterval { 2.5f }; ///< Seconds between state rolls; shortened once enraged.

            // Enrage (half health) + bombs
            bool m_hasTransformed { false };
            float m_transformElapsed { 0.f }; ///< Seconds into ClaudeState::Transform; drives the recolour.
            float m_shakeTime { 0.f };        ///< Free-running clock for the shake offset.
            static constexpr float TRANSFORM_DURATION = 5.f;
            static constexpr int32_t BOMBS_PER_PLAYER = 3;
            static constexpr float ENRAGED_INTERVAL_FACTOR = 0.75f; ///< Enraged state rolls come 25% sooner.
            static constexpr float SHAKE_AMPLITUDE = 3.f;           ///< Pixels.

            // Death
            PhysicsObject* m_killer { nullptr };         ///< Local player that landed the killing blow.
            std::optional<uint32_t> m_remoteKillerId {}; ///< Client that landed it, when it was remote.
            static constexpr float DEATH_DURATION = 3.f; ///< Shaking between the killing blow and the death.
            static constexpr int32_t DEATH_BOMB_COUNT = 8;
            static constexpr float DEATH_BOMB_RING_RADIUS = 40.f; ///< Pixels from the boss centre.
            static constexpr float DEATH_BOMB_SLIDE = 40.f;       ///< Pixels each bomb slides outwards.

            // Invulnerability + flash
            float m_invulTimer { 0.f };
            float m_flashTimer { 0.f };
            static constexpr float INVUL_DURATION = 1.f;
            static constexpr int32_t PROJECTILE_DAMAGE = 1; ///< Lives removed per mage-ball hit
            static constexpr float FLASH_INTERVAL = 0.1f;
    };

} // ggj

#endif //GGJ_CLAUDEENEMY_H
