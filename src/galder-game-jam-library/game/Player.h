//
// Created by robin on 05.11.22.
//

#ifndef GAME_DEV_TEMPLATE_PLAYER_H
#define GAME_DEV_TEMPLATE_PLAYER_H

#include "../graphics/PhysicsSprite.h"
#include "../enums/KeyboardKey.h"
#include "../managers/AnimationManager.h"
#include "../enums/PlayerState.h"
#include "../enums/SoundName.h"
#include "../interfaces/system/IMapper.h"
#include "../interfaces/game/IPlayer.h"
#include "../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../interfaces/manager/IOptionsManager.h"

#include "Hitbox.hpp"

namespace ggj
{
    class World; // forward declaration to avoid circular include

    class Player : public PhysicsSprite, public IPlayer
    {
        public:
        Player() = default;
        Player(ggj::IInputManager<ggj::KeyboardKey>& inputManager, ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IMapper& mapper,
               ggj::ISoundManager& soundManager, IOptionsManager& optionsManager, World* world, int32_t playerIndex, b2Body* body,
               const raylib::Vector2& physicsSize, const raylib::Vector2& spriteSize, const raylib::Rectangle& drawingRect, raylib::Texture* texture, const UserData& userData,
               bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_inputManager { inputManager }, m_animationManager { animationManager },
              m_mapper { mapper }, m_soundManager { soundManager }, m_optionsManager { optionsManager }, m_world { world }, m_playerIndex { playerIndex }
        {
            m_body->SetFixedRotation(true);
            m_animation = m_animationManager.getAnimation(AnimationName::PlayerIdleNG);
            m_startPos = ConvertToVector2(m_body->GetPosition());

            const raylib::Vector2 bodyPosition = ConvertToVector2(m_body->GetPosition());
            auto hitBoxPos = raylib::Vector2(bodyPosition.x + 16, bodyPosition.y);

            const UserData hitboxData { ObjectType::PlayerHitbox, { 0.f, 0.f }, "melee_attack", "", "melee_hitbox" };
            m_hitbox.create(this, m_body->GetWorld(), hitBoxPos, { 32, 32 }, hitboxData);
            m_hitbox.setSoundManager(&m_soundManager);
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
        void restoreState(const PlayerPersistentState& state) override;
        Hitbox* getHitbox() override;
        [[nodiscard]] bool isInDialog() const override;
        std::string getDialogId() override;
        void setHasFinishedDialog() override;

        void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) override;
        void update(float timeDelta) override;
        void draw() override;

        private:
        void handleInputs(float timeDelta);
        void setPlayerState(PlayerState playerState);

        ggj::IInputManager<ggj::KeyboardKey>& m_inputManager;
        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
        bool m_cameraShouldFollowPlayer { true };
        Animation m_animation;
        IMapper& m_mapper;
        ggj::ISoundManager& m_soundManager;
        IOptionsManager& m_optionsManager;
        PlayerState m_playerState { PlayerState::Dead };
        World* m_world { nullptr };
        int32_t m_playerIndex { 0 };

        raylib::Vector2 m_startPos {};
        bool m_hasClearedLevel { false };
        bool m_isDead { false };
        bool m_isLeftPosition { false };

        ggj::Hitbox m_hitbox {};
        bool m_isAttacking { false };
        uint16_t m_attackFrames { 28 }; ///< How many frames an attack takes
        uint16_t m_attackCounter { 0 }; ///< Counts attack duration

        bool m_inDialog {};
        std::string m_currentDialogId;

        int m_previousKillCount { 0 };
        int m_totalEnemiesKilledPrevLevels { 0 };
    };

} // namespace ggj

#endif //GAME_DEV_TEMPLATE_PLAYER_H
