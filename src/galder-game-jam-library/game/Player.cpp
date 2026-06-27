//
// Created by robin on 05.11.22.
//

#include "Player.h"
#include "../system/World.h"

namespace ggj
{
    void Player::draw()
    {
        PhysicsSprite::draw();
        m_hitbox.draw();
    }

    void Player::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        if (m_isDead)
        {
            setPlayerState(PlayerState::Dead);
            m_body->SetEnabled(false);
        }
        if (m_inDialog)
        {
            setPlayerState(PlayerState::Idle);
            //m_body->SetEnabled(false);
        }
        PhysicsObject::update(timeDelta);

        int currentKills = getUserData()->enemiesKilled;
        if (currentKills > m_previousKillCount)
        {
            const ExperienceConfig& cfg = m_world->getXpConfig(m_playerIndex);
            const TalentEffects& fx  = m_world->getPlayerTalentEffects(m_playerIndex);
            for (int i = m_previousKillCount; i < currentKills; ++i)
            {
                m_world->addPlayerXp(m_playerIndex, GetRandomValue(cfg.xpPerKillMin, cfg.xpPerKillMax));

                if (fx.killsPerExtraLife > 0 && (i + 1) % fx.killsPerExtraLife == 0)
                    m_world->addPlayerLives(m_playerIndex, 1);
            }
            m_previousKillCount = currentKills;
        }

        handleInputs(timeDelta);
        m_body->SetLinearVelocity({ m_velocity.x, m_velocity.y });

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }

        m_hitbox.update(timeDelta);
    }

    void Player::handleInputs(float timeDelta)
    {
        if (m_isDead)
        {
            if (m_inputManager.keyDown(KeyboardKey::Q))
            {
                m_body->SetTransform(ConvertToB2Vec2(m_startPos), m_body->GetAngle());
                setPlayerState(PlayerState::Idle);
                m_isDead = false;
                m_world->resetPlayerLives(m_playerIndex);
                m_hitbox.setIsActive(true);
                m_body->SetEnabled(true);
            }
            else
                return;
        }
        if (m_inDialog)
        {
            m_velocity = raylib::Vector2 { 0, 0 };
            return;
        }

        //Should have a collection of commands or something assigned to the player for a best practice approach
        //But this is just meant to be a quick example
        b2Vec2 vel = m_body->GetLinearVelocity();
        m_velocity = raylib::Vector2 { vel.x, vel.y };

        const float BASE_SPEED = 7.f;
        const float speed = BASE_SPEED * m_world->getPlayerTalentEffects(m_playerIndex).speedMultiplier;

        if (m_inputManager.keyDown(KeyboardKey::A) && !m_inputManager.keyDown(KeyboardKey::D))
        {
            m_isLeftPosition = true;
            m_hitbox.setLeftPosition(true);
            m_flip = true;
            m_velocity = raylib::Vector2 { -speed, m_velocity.y };
            if (m_velocity.y < 0.2f && m_velocity.y > -0.2f && !m_isAttacking)
                setPlayerState(PlayerState::Walk);
        }
        else if (m_inputManager.keyDown(KeyboardKey::D) && !m_inputManager.keyDown(KeyboardKey::A))
        {
            m_isLeftPosition = false;
            m_hitbox.setLeftPosition(false);
            m_flip = false;
            m_velocity = raylib::Vector2 { speed, m_velocity.y };
            if (m_velocity.y < 0.2f && m_velocity.y > -0.2f && !m_isAttacking)
                setPlayerState(PlayerState::Walk);
        }
        else
        {
            m_velocity = raylib::Vector2 { 0.f, m_velocity.y };
        }

        if (m_inputManager.keyDown(KeyboardKey::W) && !m_inputManager.keyDown(KeyboardKey::S))
        {
            m_velocity = raylib::Vector2 { m_velocity.x, -speed };
            if (!m_isAttacking)
                setPlayerState(PlayerState::Walk);
        }
        else if (m_inputManager.keyDown(KeyboardKey::S) && !m_inputManager.keyDown(KeyboardKey::W))
        {
            m_velocity = raylib::Vector2 { m_velocity.x, speed };
            if (!m_isAttacking)
                setPlayerState(PlayerState::Walk);
        }
        else
        {
            m_velocity = raylib::Vector2 { m_velocity.x, 0.f };
        }

        if (m_inputManager.keyPressed(KeyboardKey::C) && !m_isAttacking)
        {
            setPlayerState(PlayerState::AttackGrounded);
            m_isAttacking = true;
            m_attackCounter = 0;
            m_soundManager.playSound(SoundName::PlayerAttack);
        }

#ifdef GAME_DEV_DEBUG
        if (m_inputManager.keyPressed(KeyboardKey::Minus))
        {
            m_world->addPlayerLives(m_playerIndex, 1);
        }
#endif

        //Toggle player rotate. Fun!
        if (m_inputManager.keyPressed(KeyboardKey::Enter) || m_inputManager.keyPressed(KeyboardKey::V))
        {
            bool rotationFixedToggle = !m_body->IsFixedRotation();
            m_body->SetFixedRotation(rotationFixedToggle);
            if (rotationFixedToggle)
                m_body->SetTransform(m_body->GetPosition(), 0);
        }

        if (!m_isAttacking)
        {
            if (m_velocity.x == 0 && m_velocity.y == 0 && !m_isDead)
            {
                setPlayerState(PlayerState::Idle);
            }
        }

        if (m_isAttacking)
        {
            const float atkMul = m_world->getPlayerTalentEffects(m_playerIndex).attackSpeedMultiplier;
            const auto effectiveFrames = static_cast<uint16_t>(static_cast<float>(m_attackFrames) * atkMul);
            if (m_attackCounter >= effectiveFrames)
            {
                m_isAttacking = false;
            }
            ++m_attackCounter;
        }
        m_hitbox.setIsActive(m_isAttacking);
        if (m_isLeftPosition)
            m_hitbox.getBody()->SetTransform(PhysicsObject::ConvertToB2Vec2({ m_position.x - 12, m_position.y - 8 }), 0);
        else
            m_hitbox.getBody()->SetTransform(PhysicsObject::ConvertToB2Vec2({ m_position.x + 12, m_position.y - 8 }), 0);
    }

    bool Player::cameraShouldFollowPlayer() const
    {
        return m_cameraShouldFollowPlayer;
    }

    void Player::setCameraFollowsPlayer(bool follow)
    {
        m_cameraShouldFollowPlayer = follow;
    }

    void Player::setPlayerState(PlayerState playerState)
    {
        if (m_playerState == playerState)
            return;

        m_playerState = playerState;
        m_animation = m_animationManager.getAnimation(m_mapper.getAnimationNameByPlayerState(playerState));
    }

    void Player::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold); //Required to calculate manifold when circle hits circle

        if (b->getUserData()->getObjectType() == ObjectType::Enemy)
        {
            if (m_world->getPlayerScore(m_playerIndex) != 0)
                m_world->addPlayerScore(m_playerIndex, -1);

            m_world->decrementPlayerLives(m_playerIndex);

            if (m_world->getPlayerLives(m_playerIndex) <= 0)
                m_isDead = true;
        }
        if (b->getUserData()->getCommand() == "clear_level")
        {
            m_hasClearedLevel = true;
            m_world->addPlayerXp(m_playerIndex, m_world->getXpConfig(m_playerIndex).xpPerPortal);
        }
        if (b->getUserData()->getCommand() == "pickup_coin")
        {
            m_world->addPlayerScore(m_playerIndex, 1);
            m_world->addPlayerXp(m_playerIndex, m_world->getXpConfig(m_playerIndex).xpPerCoin);
        }
        if (b->getUserData()->getObjectType() == ObjectType::Dialog)
        {
            std::string dialogId = b->getUserData()->getDialogId();

            m_inDialog = true;
            m_currentDialogId = dialogId;
        }
    }

    int Player::getScore() const
    {
        return m_world->getPlayerScore(m_playerIndex);
    }

    int Player::getLives() const
    {
        return m_world->getPlayerLives(m_playerIndex);
    }

    int Player::getLevel() const
    {
        return m_world->getPlayerLevel(m_playerIndex);
    }

    int Player::getCurrentXp() const
    {
        return m_world->getPlayerCurrentXp(m_playerIndex);
    }

    int Player::getTotalXp() const
    {
        return m_world->getPlayerTotalXp(m_playerIndex);
    }

    int Player::getXpToNextLevel() const
    {
        return m_world->getPlayerXpToNextLevel(m_playerIndex);
    }

    int Player::getEnemiesKilled() const
    {
        return m_totalEnemiesKilledPrevLevels + m_userData.enemiesKilled;
    }

    void Player::restoreState(const PlayerPersistentState& state)
    {
        m_world->restorePlayerStats(m_playerIndex, state);
        m_totalEnemiesKilledPrevLevels = state.totalEnemiesKilled;
    }

    bool Player::hasClearedLevel() const
    {
        return m_hasClearedLevel;
    }

    void Player::setHasClearedLevel(bool hasClearedLevel)
    {
        m_hasClearedLevel = hasClearedLevel;
    }

    void Player::setHasFinishedDialog()
    {
        m_inDialog = false;
        m_currentDialogId = {};
        //m_body->SetEnabled(true);
    }

    Hitbox* Player::getHitbox()
    {
        return &m_hitbox;
    }

    bool Player::isInDialog() const
    {
        return m_inDialog;
    }

    std::string Player::getDialogId()
    {
        return m_currentDialogId;
    }
} // namespace ggj
