//
// Created by soberga on 09.11.24.
//

#include "Projectile.h"
#include "../../system/CollisionCategory.hpp"

void ggj::Projectile::update(float timeDelta)
{
    PhysicsSprite::update(timeDelta);
    m_timeAlive += timeDelta;
    

    if(m_animation.isValid())
    {
        m_animation.update(timeDelta);
        m_drawingRect = m_animation.getDrawingRect();
    }
    
    if(m_isImpactActive)
    {
        // Removing the hitbox can't happen inside a contact callback (world is locked), so do it here.
        if(!m_isBodyDisabled && m_body != nullptr)
        {
            m_body->SetEnabled(false);
            m_isBodyDisabled = true;
        }

        if(m_timeAlive >= m_timeToLive)
        {
            destroy();
        }
    }
    else
    {
        if(m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
        
        if(m_timeAlive >= m_timeToLive)
        {
            explode();
        }
    }
    
}

void ggj::Projectile::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
{
    UserData* userDataB = b->getUserData();

    if (userDataB->getObjectType() == ObjectType::Projectile)
    {
        Projectile* other = dynamic_cast<Projectile*>(b);
        if (other == nullptr || other->getOwner() != m_owner)
            destroy();
        return;
    }

    // Walls stop the projectile. Enemy turn objects are non-solid triggers, so they are never terrain.
    if (b->getBody() != nullptr && userDataB->getCommand() != "enemy_turn" && CollisionCategory::IsTerrain(*b->getBody()))
    {
        explode();
        return;
    }
    PhysicsSprite::beginContact(a, b, contact);
}

void ggj::Projectile::explode()
{
    if(m_isImpactActive)
        return;

    m_isImpactActive = true;
    m_timeAlive = 0;
    m_timeToLive = IMPACT_DURATION;
    m_velocity.x = 0.f;
    m_velocity.y = 0.f;
    m_animation = m_impactAnimation;
    m_body->SetAwake(false);
}

void ggj::Projectile::setOwner(PhysicsObject* owner)
{
    m_owner = owner;
}

ggj::PhysicsObject* ggj::Projectile::getOwner()
{
    return m_owner;
}
