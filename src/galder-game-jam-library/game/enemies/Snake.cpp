#include "Snake.h"
#include "EnemyTurn.hpp"
#include "../projectiles/Projectile.h"

namespace ggj
{
    void Snake::update(float timeDelta)
    {
        if(m_body == nullptr)
            return;

        m_body->SetLinearVelocity({m_velocity.x, m_velocity.y});

        PhysicsObject::update(timeDelta);

        if(m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void Snake::beginContact(PhysicsObject *a, PhysicsObject *b, b2Contact *contact)
    {
        UserData* userDataB = b->getUserData();

        const std::optional<raylib::Vector2> turnDirection = EnemyTurn::DirectionFromCommand(userDataB->getCommand());

        if(userDataB->getCommand() == EnemyTurn::REVERSE)
        {
            turn();
        }
        else if(turnDirection.has_value())
        {
            turnTowards(*turnDirection);
        }
        else if(userDataB->getObjectType() == ObjectType::Player)
        {
            b2Body *body = b->getBody();
            const auto playerVelocity = b->getVelocity();

            if(m_velocity.x != 0.f)
            {
                if(m_velocity.x < 0.f && playerVelocity.x < 0.f)
                    body->ApplyLinearImpulseToCenter({m_knockback, 0.f}, true);
                else if(m_velocity.x > 0.f && playerVelocity.x > 0.f)
                    body->ApplyLinearImpulseToCenter({-m_knockback, 0.f}, true);
                else
                    body->ApplyLinearImpulseToCenter({m_velocity.x < 0.f ? -m_knockback : m_knockback, 0.f}, true);
            }
            if(m_velocity.y != 0.f)
            {
                if(m_velocity.y < 0.f && playerVelocity.y < 0.f)
                    body->ApplyLinearImpulseToCenter({0.f, m_knockback}, true);
                else if(m_velocity.y > 0.f && playerVelocity.y > 0.f)
                    body->ApplyLinearImpulseToCenter({0.f, -m_knockback}, true);
                else
                    body->ApplyLinearImpulseToCenter({0.f, m_velocity.y < 0.f ? -m_knockback : m_knockback}, true);
            }
        }
        else if(userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            // A kill must only be counted once: a second projectile can reach an enemy that is
            // already marked for destruction before it is actually removed.
            if (shouldBeDestroyed())
                return;
            emitBloodFrom(*p, KILL_BLOOD_STRENGTH);
            m_soundManager.playSound(SoundName::EnemyDie);
            destroy();
            p->getOwner()->getUserData()->enemiesKilled++;
        }
    }

    void Snake::turn()
    {
        m_flip = !m_flip;
        m_velocity = -m_velocity;
    }

    void Snake::turnTowards(const raylib::Vector2& direction)
    {
        m_velocity = EnemyTurn::Redirect(m_velocity, direction);
        // Only a horizontal turn changes which way the sprite faces; up/down keep the last facing.
        if(m_velocity.x != 0.f)
            m_flip = m_velocity.x < 0.f;
    }
} // ggj