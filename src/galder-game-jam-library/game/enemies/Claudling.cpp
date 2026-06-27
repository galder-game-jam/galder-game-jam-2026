#include "Claudling.h"
#include "../projectiles/Projectile.h"
#include <cmath>

namespace ggj
{
    void Claudling::move(const raylib::Vector2& toMove)
    {
    }

    void Claudling::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        if (m_player != nullptr)
        {
            const raylib::Vector2 myPos  = getPosition();
            const raylib::Vector2 target = m_player->getPosition();

            const float dx = target.x - myPos.x;
            const float dy = target.y - myPos.y;
            const float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.f)
            {
                const float nx = dx / dist;
                const float ny = dy / dist;
                m_body->SetLinearVelocity({ nx * m_followSpeed, ny * m_followSpeed });
                m_flip = nx < 0.f;

                if (!m_isWalking)
                {
                    m_animation  = m_walkAnimation;
                    m_isWalking  = true;
                }
            }
            else
            {
                m_body->SetLinearVelocity({ 0.f, 0.f });
                if (m_isWalking)
                {
                    m_animation = m_idleAnimation;
                    m_isWalking = false;
                }
            }
        }

        PhysicsObject::update(timeDelta);

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void Claudling::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            m_soundManager.playSound(SoundName::EnemyDie);
            destroy();
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            p->getOwner()->getUserData()->enemiesKilled++;
        }
    }

} // ggj
