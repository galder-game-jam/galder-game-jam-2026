#include "ObstacleSteering.h"
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

    bool ObstacleSteering::IsPathBlocked(b2Body& body, const raylib::Vector2& direction)
    {
        if (direction.x == 0.f && direction.y == 0.f)
            return false;

        const b2Fixture* fixture = body.GetFixtureList();
        const float radius = (fixture != nullptr) ? fixture->GetShape()->m_radius : 1.f;
        const float reach = radius + LOOKAHEAD;
        const b2Vec2 origin = body.GetPosition();
        const b2Vec2 dir { direction.x, direction.y };
        const b2Vec2 side { -dir.y, dir.x };

        for (const float offset : { 0.f, radius * FLANK_FACTOR, -radius * FLANK_FACTOR })
        {
            const b2Vec2 start = origin + offset * side;
            ObstacleRayCallback callback { &body };
            body.GetWorld()->RayCast(&callback, start, start + reach * dir);
            if (callback.isBlocked())
                return true;
        }

        return false;
    }

    raylib::Vector2 ObstacleSteering::steer(b2Body& body, const raylib::Vector2& desired)
    {
        if (!IsPathBlocked(body, desired))
            return desired;

        // Fan out from the desired heading, trying the side we last used first so we commit to one way round.
        for (const float angle : { 30.f, 60.f, 90.f, 120.f, 150.f })
        {
            for (const float side : { m_avoidSide, -m_avoidSide })
            {
                const raylib::Vector2 candidate = rotateByDegrees(desired, side * angle);
                if (!IsPathBlocked(body, candidate))
                {
                    m_avoidSide = side;
                    return candidate;
                }
            }
        }

        return desired; // Boxed in; push on and let the physics resolve it.
    }
} // namespace ggj
