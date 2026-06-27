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

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void ClaudeEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();

            --m_lives;
            if (m_lives <= 0)
            {
                m_soundManager.playSound(SoundName::EnemyDie);
                destroy();
                p->getOwner()->getUserData()->enemiesKilled++;
            }
        }
    }

} // ggj
