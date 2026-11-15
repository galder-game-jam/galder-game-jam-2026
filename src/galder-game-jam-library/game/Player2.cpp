//
// Created by robin on 05.11.22.
//

#include <algorithm>
#include "Player2.h"
#include "../system/World.h"
#include "projectiles/MageBall.hpp"

namespace ggj
{
    void Player2::draw()
    {
        if (m_world != nullptr)
            m_world->beginPlayerPalette();
        // Blink while invulnerable so the player can see the cooldown is active.
        const bool isBlinkedOut = m_damageCooldown > 0.f && static_cast<int32_t>(m_damageCooldown / DAMAGE_BLINK_INTERVAL) % 2 == 0;
        if (!isBlinkedOut)
            PhysicsSprite::draw();
        if (m_world != nullptr)
            m_world->endPlayerPalette();
    }

    void Player2::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        if (m_damageCooldown > 0.f)
            m_damageCooldown = std::max(0.f, m_damageCooldown - timeDelta);

        // The cap grows with level, so clamp every frame (and refill gradually, never instantly, on level-up).
        const TalentEffects& talentFx = m_world->getPlayerTalentEffects(m_playerIndex);
        const float maxMana = getMaxMana();
        m_secondsSinceCast += timeDelta;
        if (m_secondsSinceCast >= MANA_REGEN_DELAY_SECONDS * talentFx.manaRegenDelayMultiplier)
            m_mana += MANA_REGEN_PER_SECOND * talentFx.manaRegenMultiplier * timeDelta;
        m_mana = std::min(m_mana, maxMana);

        if (!m_isDead && m_world->tickPlayerLifeRegen(m_playerIndex, timeDelta))
        {
            m_world->spawnFloatingText(getPosition());
            m_soundManager.playSound(SoundName::ExtraLife);
        }

        if (m_isDead)
        {
            setPlayerState(PlayerState::Dead);
            m_body->SetEnabled(false);
        }
        if (m_inDialog)
        {
            setPlayerState(PlayerState::Idle);
        }
        PhysicsObject::update(timeDelta);

        int currentKills = getUserData()->enemiesKilled;
        if (currentKills > m_previousKillCount)
        {
            const ExperienceConfig& cfg = m_world->getXpConfig(m_playerIndex);
            const TalentEffects& fx = m_world->getPlayerTalentEffects(m_playerIndex);
            for (int i = m_previousKillCount; i < currentKills; ++i)
            {
                m_world->addPlayerXp(m_playerIndex, GetRandomValue(cfg.xpPerKillMin, cfg.xpPerKillMax));

                if (fx.bonusXpPerKill > 0)
                    m_world->addPlayerXp(m_playerIndex, fx.bonusXpPerKill);
            }
            m_previousKillCount = currentKills;
        }

        m_nearbyDialogSpot = (!m_isDead && !m_inDialog) ? m_world->findDialogSpotNear(getPosition()) : nullptr;
        handleInputs(timeDelta);
        m_body->SetLinearVelocity({ m_velocity.x, m_velocity.y });

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void Player2::handleInputs(float timeDelta)
    {
        const KeybindingOptions& kb = m_optionsManager.getKeybindingOptions();

        if (m_isDead)
        {
            if (m_inputManager.keyDown(kb.respawn))
            {
                m_world->keepEnemiesClearOf(m_startPos, RESPAWN_ENEMY_CLEARANCE);
                m_body->SetTransform(ConvertToB2Vec2(m_startPos), m_body->GetAngle());
                setPlayerState(PlayerState::Idle);
                m_isDead = false;
                m_world->resetPlayerLives(m_playerIndex);
                m_damageCooldown = 0.f;
                m_mana = getMaxMana();
                m_body->SetEnabled(true);
                m_soundManager.playSound(SoundName::PlayerRespawn);
            }
            else
                return;
        }
        if (m_inDialog)
        {
            m_velocity = raylib::Vector2 { 0, 0 };
            return;
        }

        // Space talks to a nearby NPC. Checked before attacking so the same press doesn't also swing/shoot.
        if (m_nearbyDialogSpot != nullptr && m_inputManager.keyPressed(KeyboardKey::Space))
        {
            m_inDialog = true;
            m_currentDialogId = m_nearbyDialogSpot->dialogId;
            m_nearbyDialogSpot = nullptr;
            m_velocity = raylib::Vector2 { 0, 0 };
            return;
        }

        //Should have a collection of commands or something assigned to the player for a best practice approach
        //But this is just meant to be a quick example
        b2Vec2 vel = m_body->GetLinearVelocity();
        m_velocity = raylib::Vector2 { vel.x, vel.y };

        bool isInAir = m_velocity.y > 0.2f || m_velocity.y < -0.2f;

        const float BASE_SPEED = 7.f;
        const float speed = BASE_SPEED * m_world->getPlayerTalentEffects(m_playerIndex).speedMultiplier;

        if (!m_isAttacking || isInAir)
        {
            if (m_inputManager.keyDown(kb.moveLeft) && !m_inputManager.keyDown(kb.moveRight))
            {
                m_flip = true;
                m_velocity = raylib::Vector2 { -speed, m_velocity.y };
                if (m_velocity.y < 0.2f && m_velocity.y > -0.2f)
                    setPlayerState(PlayerState::Walk);
            }
            else if (m_inputManager.keyDown(kb.moveRight) && !m_inputManager.keyDown(kb.moveLeft))
            {
                m_flip = false;
                m_velocity = raylib::Vector2 { speed, m_velocity.y };
                if (m_velocity.y < 0.2f && m_velocity.y > -0.2f)
                    setPlayerState(PlayerState::Walk);
            }
            else
            {
                m_velocity = raylib::Vector2 { 0.f, m_velocity.y };
            }

            if (m_inputManager.keyDown(kb.moveUp) && !m_inputManager.keyDown(kb.moveDown))
            {
                m_velocity = raylib::Vector2 { m_velocity.x, -speed };
                setPlayerState(PlayerState::Walk);
            }
            else if (m_inputManager.keyDown(kb.moveDown) && !m_inputManager.keyDown(kb.moveUp))
            {
                m_velocity = raylib::Vector2 { m_velocity.x, speed };
                setPlayerState(PlayerState::Walk);
            }
            else
            {
                m_velocity = raylib::Vector2 { m_velocity.x, 0.f };
            }

            if (m_velocity.x != 0.f || m_velocity.y != 0.f)
                m_lastMoveDirection = m_velocity;
        }

        const float fireballManaCost = FIREBALL_MANA_COST * m_world->getPlayerTalentEffects(m_playerIndex).spellManaCostMultiplier;
        if (m_inputManager.keyPressed(kb.attack) && !m_isAttacking && m_mana >= fireballManaCost)
        {
            m_mana -= fireballManaCost;
            m_secondsSinceCast = 0.f;
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
            const float atkMul = m_world->getPlayerTalentEffects(m_playerIndex).attackSpeedMultiplier;
            const auto effectiveFrames = static_cast<uint16_t>(static_cast<float>(m_attackFrames) * atkMul);
            if (m_attackCounter >= effectiveFrames)
            {
                m_isAttacking = false;
            }
            ++m_attackCounter;
        }
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

    void Player2::takeHit()
    {
        if (m_damageCooldown > 0.f || m_isDead)
            return;

        m_damageCooldown = DAMAGE_COOLDOWN_SECONDS * m_world->getPlayerTalentEffects(m_playerIndex).damageCooldownMultiplier;
        if (m_world->getPlayerScore(m_playerIndex) != 0)
            m_world->addPlayerScore(m_playerIndex, -1);

        m_soundManager.playSound(SoundName::PlayerHurt);
        m_world->decrementPlayerLives(m_playerIndex);

        const bool isKill = m_world->getPlayerLives(m_playerIndex) <= 0;
        emitBlood({ 0.f, 0.f }, isKill ? KILL_BLOOD_STRENGTH : 1.f); // The player doesn't know what hit them, so the blood goes every way.

        if (isKill)
        {
            m_soundManager.playSound(SoundName::PlayerDeath);
            m_isDead = true;
        }
    }

    void Player2::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold); //Required to calculate manifold when circle hits circle

        if (b->getUserData()->getObjectType() == ObjectType::Enemy)
            takeHit();
        if (b->getUserData()->getCommand() == "clear_level")
        {
            m_hasClearedLevel = true;
            m_world->addPlayerXp(m_playerIndex, m_world->getXpConfig(m_playerIndex).xpPerPortal);
        }
        if (b->getUserData()->getCommand() == "pickup_coin")
        {
            m_world->addPlayerScore(m_playerIndex, 1);
            m_world->addPlayerXp(m_playerIndex, m_world->getXpConfig(m_playerIndex).xpPerCoin);
            m_world->spawnCoinFloatingText(getPosition());
        }
    }

    void Player2::setHasFinishedDialog()
    {
        m_inDialog = false;
        m_currentDialogId = {};
    }

    int Player2::getScore() const
    {
        return m_world->getPlayerScore(m_playerIndex);
    }

    int Player2::getLives() const
    {
        return m_world->getPlayerLives(m_playerIndex);
    }

    int Player2::getLevel() const
    {
        return m_world->getPlayerLevel(m_playerIndex);
    }

    int Player2::getCurrentXp() const
    {
        return m_world->getPlayerCurrentXp(m_playerIndex);
    }

    int Player2::getTotalXp() const
    {
        return m_world->getPlayerTotalXp(m_playerIndex);
    }

    int Player2::getXpToNextLevel() const
    {
        return m_world->getPlayerXpToNextLevel(m_playerIndex);
    }

    float Player2::getMaxMana() const
    {
        const float baseMana = BASE_MAX_MANA + MAX_MANA_PER_LEVEL * static_cast<float>(std::max(0, getLevel() - 1));
        return baseMana * m_world->getPlayerTalentEffects(m_playerIndex).maxManaMultiplier;
    }

    int Player2::getEnemiesKilled() const
    {
        return m_totalEnemiesKilledPrevLevels + m_userData.enemiesKilled;
    }

    void Player2::restoreState(const PlayerPersistentState& state)
    {
        m_world->restorePlayerStats(m_playerIndex, state);
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
        return nullptr;
    }

    void Player2::shootMageBall()
    {
        m_soundManager.playSound(SoundName::WizardShoot);
        const float BASE_PROJ_SPEED = 14.f;
        const float projSpeed = BASE_PROJ_SPEED * m_world->getPlayerTalentEffects(m_playerIndex).projectileSpeedMultiplier;
        const Vector2 dir = Vector2Normalize({ m_lastMoveDirection.x, m_lastMoveDirection.y });
        const raylib::Vector2 velocity { dir.x * projSpeed, dir.y * projSpeed };
        MageBall* ball = m_world->createProjectile<ggj::MageBall>(TextureName::MageBall, raylib::Vector2 { 16.f, 16.f }, raylib::Vector2 { m_position.x, m_position.y - 8 },
                                                                  velocity, MageBall::DEFAULT_TTL);
        ball->setOwner(this);
        m_world->registerLocalProjectile(ball);
    }
} // namespace ggj
