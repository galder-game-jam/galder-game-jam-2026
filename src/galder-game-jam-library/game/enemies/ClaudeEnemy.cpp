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
                setBossState(static_cast<ClaudeState>(GetRandomValue(0, 2)));
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
                    m_world->spawnClaudling(getPosition(), m_followSpeed);
                break;
        }
    }

    void ClaudeEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft)
    {
        if (m_invulTimer > 0.f)
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

        if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            takeDamage(&m_soundManager, p->getOwner(), false);
        }
    }

} // ggj
