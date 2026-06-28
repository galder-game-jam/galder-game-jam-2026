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
            m_soundManager.playSound(SoundName::EnemyDie);
            destroy();
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            p->getOwner()->getUserData()->enemiesKilled++;
        }
    }

} // ggj
