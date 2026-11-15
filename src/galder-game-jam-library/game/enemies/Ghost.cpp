//
// Created by soberga on 12.11.22.
//

#include "Ghost.h"
#include "../projectiles/Projectile.h"
#include <cmath>

namespace ggj
{
    void Ghost::move(const raylib::Vector2 &toMove)
    {

    }

    std::optional<uint32_t> Ghost::PickTarget(const std::optional<uint32_t> currentTarget, const raylib::Vector2& ghostPosition, const float followRange,
                                              const IPlayerLocator& players)
    {
        if (followRange <= 0.f)
            return std::nullopt;

        const auto isInRange = [&](const uint32_t playerId)
        {
            const std::optional<raylib::Vector2> position = players.getPlayerPosition(playerId);
            return position.has_value() && ghostPosition.Distance(*position) <= followRange;
        };

        // Stick with the current target so two players in range don't make the ghost flip-flop.
        if (currentTarget.has_value() && isInRange(*currentTarget))
            return currentTarget;

        for (const uint32_t playerId : players.getActivePlayerIds())
        {
            if (isInRange(playerId))
                return playerId;
        }
        return std::nullopt;
    }

    void Ghost::beginContact(PhysicsObject *a, PhysicsObject *b, b2Contact *contact)
    {
        UserData* userDataB = b->getUserData();

        if(userDataB->getObjectType() == ObjectType::Player)
        {
            // The ghost can come from any direction, so push the player straight away from it.
            static constexpr float KNOCKBACK_IMPULSE = 50.f;
            const raylib::Vector2 away = b->getPosition() - getPosition();
            const float length = away.Length();
            if (length > 0.001f)
                b->getBody()->ApplyLinearImpulseToCenter({ away.x / length * KNOCKBACK_IMPULSE, away.y / length * KNOCKBACK_IMPULSE }, true);
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

    void Ghost::update(float timeDelta)
    {
        if(m_body == nullptr)
            return;

        const raylib::Vector2 position = getPosition();
        m_targetPlayerId = m_players != nullptr ? PickTarget(m_targetPlayerId, position, m_followRange, *m_players) : std::nullopt;

        const std::optional<raylib::Vector2> target = m_targetPlayerId.has_value() ? m_players->getPlayerPosition(*m_targetPlayerId) : std::nullopt;
        const raylib::Vector2 toTarget = target.has_value() ? *target - position : raylib::Vector2 { 0.f, 0.f };
        const float distance = toTarget.Length();

        // Stop when on top of the target, or it would jitter back and forth across it.
        if (distance > 1.f && m_followSpeed > 0.f)
        {
            const raylib::Vector2 direction = toTarget / distance;
            m_body->SetLinearVelocity({ direction.x * m_followSpeed, direction.y * m_followSpeed });
            if (direction.x != 0.f)
                m_flip = direction.x > 0.f;
        }
        else
        {
            m_body->SetLinearVelocity({ 0.f, 0.f });
        }

        PhysicsObject::update(timeDelta);

        if(m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }
} // ggj
