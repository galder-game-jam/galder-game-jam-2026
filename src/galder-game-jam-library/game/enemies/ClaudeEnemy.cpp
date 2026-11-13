#include "ClaudeEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <cmath>

namespace ggj
{
    void ClaudeEnemy::move(const raylib::Vector2& toMove)
    {
    }

    void ClaudeEnemy::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        // Invulnerability flash
        if (m_invulTimer > 0.f)
        {
            m_invulTimer -= timeDelta;
            m_flashTimer += timeDelta;
            while (m_flashTimer >= FLASH_INTERVAL)
            {
                m_flashTimer -= FLASH_INTERVAL;
                m_isVisible = !m_isVisible;
            }
            if (m_invulTimer <= 0.f)
            {
                m_invulTimer = 0.f;
                m_isVisible  = true;
            }
        }

        // Boss AI
        if (!m_isSittingDuck)
        {
            m_stateTimer -= timeDelta;
            if (m_stateTimer <= 0.f)
            {
                ClaudeState newState;
                do { newState = static_cast<ClaudeState>(GetRandomValue(0, 2)); } while (newState == m_bossState);
                setBossState(newState);
                m_stateTimer = m_stateInterval;
            }

            if (m_bossState == ClaudeState::Walk && m_player != nullptr)
            {
                const raylib::Vector2 myPos  = getPosition();
                const raylib::Vector2 target = m_player->getPosition();
                const float dx   = target.x - myPos.x;
                const float dy   = target.y - myPos.y;
                const float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 1.f)
                {
                    m_body->SetLinearVelocity({ (dx / dist) * m_followSpeed, (dy / dist) * m_followSpeed });
                    m_flip = dx < 0.f;
                }
                else
                {
                    m_body->SetLinearVelocity({ 0.f, 0.f });
                }
            }
            else
            {
                m_body->SetLinearVelocity({ 0.f, 0.f });
            }
        }
        else
        {
            m_body->SetLinearVelocity({ 0.f, 0.f });
        }

        PhysicsObject::update(timeDelta);

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void ClaudeEnemy::setBossState(ClaudeState newState)
    {
        m_bossState = newState;
        switch (newState)
        {
            case ClaudeState::Idle:
                m_animation = m_idleAnimation;
                break;
            case ClaudeState::Walk:
                m_animation = m_walkAnimation;
                break;
            case ClaudeState::Attack:
                m_animation = m_attackAnimation;
                if (m_world != nullptr)
                    m_world->spawnClaudling(getPosition(), m_followSpeed / 2.f, m_knockback / 2.f);
                break;
        }
    }

    void ClaudeEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft)
    {
        // Already dead (waiting for removal): don't count the boss kill a second time.
        if (shouldBeDestroyed() || m_invulTimer > 0.f)
            return;

        --m_lives;

        if (m_body != nullptr)
            m_body->ApplyLinearImpulseToCenter({fromLeft ? -50.f : 50.f, 0.f}, true);

        if (m_lives <= 0)
        {
            if (soundManager != nullptr)
                soundManager->playSound(SoundName::EnemyDie);
            destroy();
            if (attacker != nullptr)
                attacker->getUserData()->enemiesKilled++;
            if (m_world != nullptr)
                m_world->clearBoss();
        }
        else
        {
            m_invulTimer = INVUL_DURATION;
            m_flashTimer = 0.f;
        }
    }

    void ClaudeEnemy::onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker)
    {
        takeDamage(soundManager, attacker, fromLeft);
    }

    void ClaudeEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Player)
        {
            b2Vec2 vel = m_body->GetLinearVelocity();
            float len = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            if (len < 0.001f)
            {
                const raylib::Vector2 myPos     = getPosition();
                const raylib::Vector2 playerPos = b->getPosition();
                const float dx = playerPos.x - myPos.x;
                const float dy = playerPos.y - myPos.y;
                len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f)
                {
                    vel = { dx / len, dy / len };
                    len = 1.f;
                }
            }
            if (len > 0.001f)
                b->getBody()->ApplyLinearImpulseToCenter({ (vel.x / len) * m_knockback, (vel.y / len) * m_knockback }, true);
        }
        else if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            takeDamage(&m_soundManager, p->getOwner(), false);
        }
    }

} // ggj
