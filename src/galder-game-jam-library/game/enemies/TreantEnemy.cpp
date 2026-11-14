#include "TreantEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ggj
{
    namespace
    {
        /*! Ray cast callback that reports whether anything solid and immovable is in the way. */
        class ObstacleRayCallback : public b2RayCastCallback
        {
            public:
            explicit ObstacleRayCallback(const b2Body* self) : m_self { self } {}

            float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
            {
                const b2Body* body = fixture->GetBody();
                // Players, other enemies and projectiles are dynamic, triggers are sensors: only scenery blocks.
                if (fixture->IsSensor() || body == m_self || body->GetType() != b2_staticBody)
                    return -1.f;

                m_isBlocked = true;
                return 0.f; // Any hit is enough, stop the cast.
            }

            [[nodiscard]] bool isBlocked() const { return m_isBlocked; }

            private:
            const b2Body* m_self { nullptr };
            bool m_isBlocked { false };
        };

        raylib::Vector2 rotateByDegrees(const raylib::Vector2& v, float degrees)
        {
            const float radians = degrees * std::numbers::pi_v<float> / 180.f;
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            return { v.x * c - v.y * s, v.x * s + v.y * c };
        }
    } // namespace

    float TreantEnemy::randomRange(const float min, const float max)
    {
        constexpr int32_t STEPS = 1000;
        return min + (max - min) * static_cast<float>(GetRandomValue(0, STEPS)) / static_cast<float>(STEPS);
    }

    bool TreantEnemy::isPathBlocked(const raylib::Vector2& direction) const
    {
        if (m_body == nullptr || (direction.x == 0.f && direction.y == 0.f))
            return false;

        const b2Fixture* fixture = m_body->GetFixtureList();
        const float radius = (fixture != nullptr) ? fixture->GetShape()->m_radius : 1.f;
        const float reach = radius + OBSTACLE_LOOKAHEAD;
        const b2Vec2 origin = m_body->GetPosition();
        const b2Vec2 dir { direction.x, direction.y };
        const b2Vec2 side { -dir.y, dir.x };

        for (const float offset : { 0.f, radius * OBSTACLE_FLANK_FACTOR, -radius * OBSTACLE_FLANK_FACTOR })
        {
            const b2Vec2 start = origin + offset * side;
            ObstacleRayCallback callback { m_body };
            m_body->GetWorld()->RayCast(&callback, start, start + reach * dir);
            if (callback.isBlocked())
                return true;
        }

        return false;
    }

    raylib::Vector2 TreantEnemy::steerAroundObstacles(const raylib::Vector2& desired)
    {
        if (!isPathBlocked(desired))
            return desired;

        // Fan out from the desired heading, trying the side we last used first so we commit to one way round.
        for (const float angle : { 30.f, 60.f, 90.f, 120.f, 150.f })
        {
            for (const float side : { m_avoidSide, -m_avoidSide })
            {
                const raylib::Vector2 candidate = rotateByDegrees(desired, side * angle);
                if (!isPathBlocked(candidate))
                {
                    m_avoidSide = side;
                    return candidate;
                }
            }
        }

        return desired; // Boxed in; push on and let the physics resolve it.
    }

    void TreantEnemy::move(const raylib::Vector2& toMove) {}

    raylib::Vector2 TreantEnemy::directionToPlayer() const
    {
        if (m_player == nullptr)
            return { 0.f, 0.f };

        const raylib::Vector2 myPos = getPosition();
        const raylib::Vector2 target = m_player->getPosition();
        const float dx = target.x - myPos.x;
        const float dy = target.y - myPos.y;
        const float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 1.f)
            return { 0.f, 0.f };

        return { dx / dist, dy / dist };
    }

    void TreantEnemy::faceTowards(const raylib::Vector2& direction)
    {
        if (direction.x != 0.f)
            m_flip = direction.x < 0.f;
    }

    void TreantEnemy::setState(TreantState newState)
    {
        m_state = newState;

        switch (newState)
        {
            case TreantState::Idle:
                m_stateTimer = IDLE_DURATION;
                m_tint = WHITE;
                break;
            case TreantState::Walk:
                m_stateTimer = randomRange(WALK_MIN_DURATION, WALK_MAX_DURATION);
                m_tint = WHITE;
                // An enraged treant chains two charges per cycle; a calm one only throws one.
                m_chargesLeft = isEnraged() ? 2 : 1;
                break;
            case TreantState::Windup:
                m_stateTimer = isEnraged() ? ENRAGED_WINDUP_DURATION : WINDUP_DURATION;
                m_tint = RED;
                break;
            case TreantState::Charge:
                m_stateTimer = CHARGE_DURATION;
                m_tint = RED;
                // Lock the direction now so a player who keeps moving can sidestep the charge.
                m_chargeDirection = directionToPlayer();
                faceTowards(m_chargeDirection);
                --m_chargesLeft;
                break;
            case TreantState::Recover:
                m_stateTimer = RECOVER_DURATION;
                m_tint = WHITE;
                break;
        }

        m_stateDuration = m_stateTimer;
    }

    void TreantEnemy::update(float timeDelta)
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
                m_isVisible = true;
            }
        }

        m_stateTimer -= timeDelta;
        m_animTime += timeDelta;

        const float speedMultiplier = isEnraged() ? ENRAGED_SPEED_MULTIPLIER : 1.f;
        b2Vec2 velocity { 0.f, 0.f };

        switch (m_state)
        {
            case TreantState::Idle:
                if (m_stateTimer <= 0.f)
                    setState(TreantState::Walk);
                break;

            case TreantState::Walk:
            {
                const raylib::Vector2 direction = steerAroundObstacles(directionToPlayer());
                faceTowards(direction);
                velocity = { direction.x * m_followSpeed * speedMultiplier, direction.y * m_followSpeed * speedMultiplier };
                if (m_stateTimer <= 0.f)
                {
                    // Not every chase ends in a charge; sometimes the treant just stops to catch its breath.
                    const float chargeChance = isEnraged() ? ENRAGED_CHARGE_CHANCE : CHARGE_CHANCE;
                    setState(randomRange(0.f, 1.f) < chargeChance ? TreantState::Windup : TreantState::Idle);
                }
                break;
            }

            case TreantState::Windup:
                // Keep tracking the player while rooted so the telegraph points at them.
                faceTowards(directionToPlayer());
                if (m_stateTimer <= 0.f)
                    setState(TreantState::Charge);
                break;

            case TreantState::Charge:
            {
                const float speed = m_followSpeed * CHARGE_SPEED_MULTIPLIER * speedMultiplier;
                velocity = { m_chargeDirection.x * speed, m_chargeDirection.y * speed };
                if (m_stateTimer <= 0.f)
                    setState(m_chargesLeft > 0 ? TreantState::Windup : TreantState::Recover);
                break;
            }

            case TreantState::Recover:
                if (m_stateTimer <= 0.f)
                    setState(TreantState::Idle);
                break;
        }

        m_body->SetLinearVelocity(velocity);

        PhysicsObject::update(timeDelta);

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void TreantEnemy::draw()
    {
        if (!m_isVisible || m_texture == nullptr)
        {
            PhysicsSprite::draw();
            return;
        }

        // The sprite sheet is smaller than the boss, so scale the frame up to the sprite size from the map
        // (PhysicsSprite::draw would use the frame size as-is).
        const TreantPose pose = computePose();
        const float sourceWidth = m_flip ? -m_drawingRect.width : m_drawingRect.width;
        const raylib::Rectangle source { m_drawingRect.x, m_drawingRect.y, sourceWidth, m_drawingRect.height };
        const raylib::Rectangle destination { m_position.x, m_position.y, m_spriteSize.x * pose.scaleX, m_spriteSize.y * pose.scaleY };
        // The origin scales with the sprite so the feet stay on the same spot while squashing and stretching.
        const raylib::Vector2 origin { m_origin.x * pose.scaleX, m_origin.y * pose.scaleY };
        m_texture->Draw(source, destination, origin, m_rotation + pose.rotation, m_tint);
    }

    TreantEnemy::TreantPose TreantEnemy::computePose() const
    {
        TreantPose pose;

        switch (m_state)
        {
            case TreantState::Idle:
            {
                // Slow breathing: the chest swells while the sprite sways a touch.
                const float breath = std::sin(m_animTime * 2.5f);
                pose.scaleY = 1.f + 0.03f * breath;
                pose.scaleX = 1.f - 0.015f * breath;
                pose.rotation = 1.5f * std::sin(m_animTime * 1.2f);
                break;
            }

            case TreantState::Walk:
            {
                // Heavy waddle: rock from foot to foot, dipping a little on every step.
                const float phase = m_animTime * WALK_STEPS_PER_SECOND * std::numbers::pi_v<float>;
                pose.rotation = 6.f * std::sin(phase);
                pose.scaleY = 1.f - 0.05f * std::abs(std::sin(phase));
                pose.scaleX = 1.f + 0.03f * std::abs(std::sin(phase));
                break;
            }

            case TreantState::Windup:
            {
                // Coil up: crouch lower and wider the closer the charge gets, trembling harder as it builds.
                const float progress = m_stateDuration > 0.f ? 1.f - std::max(0.f, m_stateTimer) / m_stateDuration : 1.f;
                pose.scaleY = 1.f - 0.12f * progress;
                pose.scaleX = 1.f + 0.08f * progress;
                pose.rotation = (1.f + 4.f * progress) * std::sin(m_animTime * 60.f);
                break;
            }

            case TreantState::Charge:
            {
                // Lean into the run and pound the ground with each stride.
                const float phase = m_animTime * CHARGE_STEPS_PER_SECOND * std::numbers::pi_v<float>;
                const float lean = m_chargeDirection.x < 0.f ? -1.f : 1.f;
                pose.rotation = lean * 14.f + 3.f * std::sin(phase);
                pose.scaleX = 1.1f;
                pose.scaleY = 0.95f + 0.03f * std::abs(std::sin(phase));
                break;
            }

            case TreantState::Recover:
            {
                // Out of breath: hunched over and heaving.
                const float heave = std::sin(m_animTime * 7.f);
                pose.scaleY = 0.92f + 0.02f * heave;
                pose.scaleX = 1.04f;
                pose.rotation = 3.f * std::sin(m_animTime * 2.f);
                break;
            }
        }

        return pose;
    }

    void TreantEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage)
    {
        // Already dead (waiting for removal): don't count the boss kill a second time.
        if (shouldBeDestroyed() || m_invulTimer > 0.f)
            return;

        m_lives -= damage;

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

    void TreantEnemy::onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage)
    {
        takeDamage(soundManager, attacker, fromLeft, damage);
    }

    void TreantEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Player)
        {
            const raylib::Vector2 myPos = getPosition();
            const raylib::Vector2 playerPos = b->getPosition();
            const float dx = playerPos.x - myPos.x;
            const float dy = playerPos.y - myPos.y;
            const float len = std::sqrt(dx * dx + dy * dy);

            if (len > 0.001f)
            {
                const float force = (m_state == TreantState::Charge) ? m_knockback * CHARGE_KNOCKBACK_MULTIPLIER : m_knockback;
                b->getBody()->ApplyLinearImpulseToCenter({ (dx / len) * force, (dy / len) * force }, true);
            }
        }
        else if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            takeDamage(&m_soundManager, p->getOwner(), false, PROJECTILE_DAMAGE);
        }
    }

} // namespace ggj
