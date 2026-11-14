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
        raylib::Vector2 m_lastMoveDirection { 1.f, 0.f }; ///< Last non-zero input direction, used for projectile aim

        bool m_isAttacking { false };
        uint16_t m_attackFrames { 20 }; ///< How many frames an attack takes
        uint16_t m_attackCounter { 0 }; ///< Counts attack duration

        std::vector<std::unique_ptr<Projectile>> m_projectiles {};

        bool m_inDialog {};
        const DialogSpot* m_nearbyDialogSpot {};
        std::string m_currentDialogId;

        int m_previousKillCount { 0 };
        int m_totalEnemiesKilledPrevLevels { 0 };
    };

} // namespace ggj

#endif //GAME_DEV_TEMPLATE_PLAYER2_H
