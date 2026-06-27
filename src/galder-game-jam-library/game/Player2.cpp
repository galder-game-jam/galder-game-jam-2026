//
// Created by robin on 05.11.22.
//

#include "Player2.h"
#include "Hitbox.hpp"
#include "../system/World.h"
#include "projectiles/MageBall.hpp"

namespace ggj
{
    void Player2::draw()
    {
        PhysicsSprite::draw();
        m_hitbox.draw();
    }

    void Player2::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        if (m_isDead)
        {
            setPlayerState(PlayerState::Dead);
            m_body->SetEnabled(false);
        }
        PhysicsObject::update(timeDelta);

        int currentKills = getUserData()->enemiesKilled;
        if (currentKills > m_previousKillCount)
        {
            const ExperienceConfig& cfg = m_world->getXpConfig();
            for (int i = m_previousKillCount; i < currentKills; ++i)
                m_world->addPlayerXp(GetRandomValue(cfg.xpPerKillMin, cfg.xpPerKillMax));
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

    void Player2::handleInputs(float timeDelta)
    {
        if (m_isDead)
        {
            if (m_inputManager.keyDown(KeyboardKey::Q))
            {
                m_body->SetTransform(ConvertToB2Vec2(m_startPos), m_body->GetAngle());
                setPlayerState(PlayerState::Idle);
                m_isDead = false;
                m_world->resetPlayerLives();
                m_hitbox.setIsActive(true);
                m_body->SetEnabled(true);
            }
            else
                return;
        }

        //Should have a collection of commands or something assigned to the player for a best practice approach
        //But this is just meant to be a quick example
        b2Vec2 vel = m_body->GetLinearVelocity();
        m_velocity = raylib::Vector2 { vel.x, vel.y };

        bool isInAir = m_velocity.y > 0.2f || m_velocity.y < -0.2f;

        if (!m_isAttacking || isInAir)
        {
            if (m_inputManager.keyDown(KeyboardKey::A) && !m_inputManager.keyDown(KeyboardKey::D))
            {
                m_isLeftPosition = true;
                m_hitbox.setLeftPosition(true);
                m_flip = true;
                m_velocity = raylib::Vector2 { -7.f, m_velocity.y };
                if (m_velocity.y < 0.2f && m_velocity.y > -0.2f)
                    setPlayerState(PlayerState::Walk);
            }
            else if (m_inputManager.keyDown(KeyboardKey::D) && !m_inputManager.keyDown(KeyboardKey::A))
            {
                m_isLeftPosition = false;
                m_hitbox.setLeftPosition(false);
                m_flip = false;
                m_velocity = raylib::Vector2 { 7.f, m_velocity.y };
                if (m_velocity.y < 0.2f && m_velocity.y > -0.2f)
                    setPlayerState(PlayerState::Walk);
            }
            else
            {
                m_velocity = raylib::Vector2 { 0.f, m_velocity.y };
            }

            if (m_inputManager.keyDown(KeyboardKey::W) && !m_inputManager.keyDown(KeyboardKey::S))
            {
                m_velocity = raylib::Vector2 { m_velocity.x, -7.f };
                setPlayerState(PlayerState::Walk);
            }
            else if (m_inputManager.keyDown(KeyboardKey::S) && !m_inputManager.keyDown(KeyboardKey::W))
            {
                m_velocity = raylib::Vector2 { m_velocity.x, 7.f };
                setPlayerState(PlayerState::Walk);
            }
            else
            {
                m_velocity = raylib::Vector2 { m_velocity.x, 0.f };
            }
        }

        if (m_inputManager.keyPressed(KeyboardKey::C) && !m_isAttacking)
        {
            setPlayerState(PlayerState::AttackGrounded);

            m_isAttacking = true;
            m_attackCounter = 0;
            shootMageBall();
        }

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
            if (m_velocity.x == 0 && m_velocity.y == 0)
            {
                setPlayerState(PlayerState::Idle);
            }
        }

        if (m_isAttacking)
        {
            if (m_attackCounter >= m_attackFrames)
            {
                m_isAttacking = false;
            }
            ++m_attackCounter;
        }

        m_hitbox.setIsActive(m_isAttacking);
        if (m_isLeftPosition)
            m_hitbox.getBody()->SetTransform(PhysicsObject::ConvertToB2Vec2({ m_position.x - 12, m_position.y }), 0);
        else
            m_hitbox.getBody()->SetTransform(PhysicsObject::ConvertToB2Vec2({ m_position.x + 12, m_position.y }), 0);
    }

    bool Player2::cameraShouldFollowPlayer() const
    {
        return m_cameraShouldFollowPlayer;
    }

    void Player2::setCameraFollowsPlayer(bool follow)
    {
        m_cameraShouldFollowPlayer = follow;
    }

    void Player2::setPlayerState(PlayerState playerState)
    {
        if (m_playerState == playerState)
            return;

        m_playerState = playerState;
        m_animation = m_animationManager.getAnimation(m_mapper.getAnimationNameByPlayerState(playerState));
    }

    void Player2::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold); //Required to calculate manifold when circle hits circle

        if (b->getUserData()->getObjectType() == ObjectType::Enemy)
        {
            if (m_world->getPlayerScore() != 0)
                m_world->addPlayerScore(-1);

            m_world->decrementPlayerLives();

            if (m_world->getPlayerLives() <= 0)
                m_isDead = true;
        }
        if (b->getUserData()->getCommand() == "clear_level")
        {
            m_hasClearedLevel = true;
            m_world->addPlayerXp(m_world->getXpConfig().xpPerPortal);
        }
        if (b->getUserData()->getCommand() == "pickup_coin")
        {
            m_world->addPlayerScore(1);
            m_world->addPlayerXp(m_world->getXpConfig().xpPerCoin);
        }
    }

    int Player2::getScore() const
    {
        return m_world->getPlayerScore();
    }

    int Player2::getLives() const
    {
        return m_world->getPlayerLives();
    }

    int Player2::getLevel() const
    {
        return m_world->getPlayerLevel();
    }

    int Player2::getCurrentXp() const
    {
        return m_world->getPlayerCurrentXp();
    }

    int Player2::getTotalXp() const
    {
        return m_world->getPlayerTotalXp();
    }

    int Player2::getXpToNextLevel() const
    {
        return m_world->getPlayerXpToNextLevel();
    }

    int Player2::getEnemiesKilled() const
    {
        return m_totalEnemiesKilledPrevLevels + m_userData.enemiesKilled;
    }

    void Player2::restoreState(const PlayerPersistentState& state)
    {
        m_world->restorePlayerStats(state);
        m_totalEnemiesKilledPrevLevels = state.totalEnemiesKilled;
    }

    bool Player2::hasClearedLevel() const
    {
        return m_hasClearedLevel;
    }

    void Player2::setHasClearedLevel(bool hasClearedLevel)
    {
        m_hasClearedLevel = hasClearedLevel;
    }

    ggj::Hitbox* Player2::getHitbox()
    {
        return &m_hitbox;
    }

    void Player2::shootMageBall()
    {
        const raylib::Vector2 velocity { m_isLeftPosition ? -8.f : 8.f, 0.f };
        MageBall* ball =
            m_world->createProjectile<ggj::MageBall>(TextureName::MageBall, raylib::Vector2 { 16.f, 16.f }, raylib::Vector2 { m_position.x, m_position.y - 8 }, velocity, 1.f);
        ball->setOwner(this);
    }
} // namespace ggj
