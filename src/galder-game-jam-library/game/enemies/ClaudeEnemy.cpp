#include "ClaudeEnemy.h"
#include "../projectiles/Projectile.h"

namespace ggj
{
    void ClaudeEnemy::move(const raylib::Vector2& toMove)
    {
    }

    void ClaudeEnemy::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        PhysicsObject::update(timeDelta);

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

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
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
