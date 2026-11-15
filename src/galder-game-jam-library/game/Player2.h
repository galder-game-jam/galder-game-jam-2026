//
// Created by robin on 05.11.22.
//

#ifndef GAME_DEV_TEMPLATE_PLAYER2_H
#define GAME_DEV_TEMPLATE_PLAYER2_H

#include "../graphics/PhysicsSprite.h"
#include "../enums/KeyboardKey.h"
#include "../managers/AnimationManager.h"
#include "../enums/PlayerState.h"
#include "../interfaces/system/IMapper.h"
#include "../interfaces/game/IPlayer.h"
#include "../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../interfaces/manager/IOptionsManager.h"

#include "projectiles/Projectile.h"

namespace ggj
{
    class World;

    class Player2 : public PhysicsSprite, public IPlayer
    {
        public:
        Player2() = default;
        Player2(ggj::World* world, int32_t playerIndex, ggj::IInputManager<ggj::KeyboardKey>& inputManager, ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager,
                IMapper& mapper, ggj::ISoundManager& soundManager, IOptionsManager& optionsManager, b2Body* body, const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize,
                const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData, bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_inputManager { inputManager }, m_animationManager { animationManager },
              m_mapper { mapper }, m_soundManager { soundManager }, m_optionsManager { optionsManager }
        {
            m_world = world;
            m_playerIndex = playerIndex;
            m_body->SetFixedRotation(true);
            m_animation = m_animationManager.getAnimation(AnimationName::PlayerIdleNG);
            m_startPos = ConvertToVector2(m_body->GetPosition());
            setPlayerState(PlayerState::Idle);
        }

        // IPlayer interface
        [[nodiscard]] raylib::Vector2 getPosition() const override { return PhysicsSprite::getPosition(); }
        [[nodiscard]] raylib::Vector2 getVisualCenter() const override { return { m_position.x, m_position.y - m_origin.y + m_spriteSize.y / 2.f }; }
        UserData* getUserData() override { return PhysicsSprite::getUserData(); }
        [[nodiscard]] bool cameraShouldFollowPlayer() const override;
        void setCameraFollowsPlayer(bool follow) override;
        [[nodiscard]] bool hasClearedLevel() const override;
        void setHasClearedLevel(bool hasClearedLevel) override;
        [[nodiscard]] int getScore() const override;
        [[nodiscard]] int getLives() const override;
        [[nodiscard]] int getLevel() const override;
        [[nodiscard]] int getCurrentXp() const override;
        [[nodiscard]] int getTotalXp() const override;
        [[nodiscard]] int getXpToNextLevel() const override;
        [[nodiscard]] int getEnemiesKilled() const override;
        [[nodiscard]] float getEnergy() const override { return 0.f; }    ///< The Wizard uses cooldowns, not energy
        [[nodiscard]] float getMaxEnergy() const override { return 0.f; }
        [[nodiscard]] float getMana() const override { return m_mana; }
        [[nodiscard]] float getMaxMana() const override;
        void restoreState(const PlayerPersistentState& state) override;
        /*!
         * The Wizard has no melee hitbox (balancing) — damage comes only from mage balls.
         *
         * @return Always nullptr
         */
        Hitbox* getHitbox() override;
        [[nodiscard]] bool isInDialog() const override { return m_inDialog; }
        std::string getDialogId() override { return m_currentDialogId; }
        void setHasFinishedDialog() override;
        [[nodiscard]] const DialogSpot* getNearbyDialogSpot() const override { return m_nearbyDialogSpot; }
        [[nodiscard]] PlayerState getPlayerState() const override { return m_playerState; }
        [[nodiscard]] bool isFacingLeft() const override { return m_flip; }

        void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
        void takeHit() override;
        void stun(float seconds) override;
        void freeze(float seconds) override;
        [[nodiscard]] bool isStunned() const override { return m_stunTimer > 0.f; }
        void update(float timeDelta) override;
        void draw() override;

        private:
        void shootMageBall();
        void handleInputs(float timeDelta);
        void setPlayerState(PlayerState playerState);

        ggj::World* m_world { nullptr };
        int32_t m_playerIndex { 0 };
        ggj::IInputManager<ggj::KeyboardKey>& m_inputManager;
        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
        ggj::ISoundManager& m_soundManager;
        IOptionsManager& m_optionsManager;
        bool m_cameraShouldFollowPlayer { true };
        Animation m_animation;
        IMapper& m_mapper;
        PlayerState m_playerState { PlayerState::Dead };

        raylib::Vector2 m_startPos {};
        static constexpr float RESPAWN_ENEMY_CLEARANCE = 200.f; ///< Enemies are moved away if closer than this (pixels) on respawn
        bool m_hasClearedLevel { false };
        bool m_isDead { false };
        static constexpr float DAMAGE_COOLDOWN_SECONDS = 2.f; ///< Invulnerability window after taking damage
        static constexpr float DAMAGE_BLINK_INTERVAL = 0.1f;  ///< Sprite blink half-period while invulnerable
        float m_damageCooldown { 0.f };                       ///< Seconds of invulnerability left
        bool m_isFrozen { false };                            ///< The current stun is a freeze: drawn as ice, not dizziness
        float m_stunTimer { 0.f };                            ///< Seconds of stun left; the player can neither move nor attack
        static constexpr ::Color STUN_TINT { 190, 255, 150, 255 }; ///< Sickly green: the spider's venom
        static constexpr ::Color FREEZE_TINT { 150, 210, 255, 255 }; ///< Icy blue: the mage's frost
        static constexpr float STUN_STARS_HEIGHT = 20.f;           ///< How far above the sprite's centre the dizzy stars circle (pixels)
        raylib::Vector2 m_lastMoveDirection { 1.f, 0.f }; ///< Last non-zero input direction, used for projectile aim

        bool m_isAttacking { false };
        uint16_t m_attackFrames { 20 }; ///< How many frames an attack takes
        uint16_t m_attackCounter { 0 }; ///< Counts attack duration

        std::vector<std::unique_ptr<Projectile>> m_projectiles {};

        static constexpr float BASE_MAX_MANA = 200.f;            ///< Mana cap at level 1
        static constexpr float MAX_MANA_PER_LEVEL = 10.f;        ///< Mana cap increase for every level after the first
        static constexpr float MANA_REGEN_PER_SECOND = 10.f;     ///< Mana regained every second once regeneration kicks in
        static constexpr float MANA_REGEN_DELAY_SECONDS = 2.f;   ///< Seconds without casting before mana starts regenerating
        static constexpr float FIREBALL_MANA_COST = 25.f;        ///< Mana spent per fireball
        float m_mana { BASE_MAX_MANA };
        float m_secondsSinceCast { MANA_REGEN_DELAY_SECONDS };   ///< Idle time since the last cast; regen runs once it reaches the delay

        bool m_inDialog {};
        const DialogSpot* m_nearbyDialogSpot {};
        std::string m_currentDialogId;

        int m_previousKillCount { 0 };
        int m_totalEnemiesKilledPrevLevels { 0 };
    };

} // namespace ggj

#endif //GAME_DEV_TEMPLATE_PLAYER2_H
