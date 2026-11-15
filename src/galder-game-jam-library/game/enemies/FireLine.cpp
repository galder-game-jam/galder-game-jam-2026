#include "FireLine.h"
#include "../../system/CollisionCategory.hpp"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>

namespace ggj
{
    namespace
    {
        constexpr ::Color FIRE_RED { 220, 50, 20, 255 };
        constexpr ::Color FIRE_ORANGE { 255, 140, 30, 255 };
        constexpr ::Color FIRE_YELLOW { 255, 225, 110, 255 };
        constexpr float FLAME_SPACING = 14.f;
    } // namespace

    b2Body* FireLine::CreateBody(b2World& world, const FireLineSpawnEvent& spawn)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(spawn.origin);
        b2Body* body = world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = WIDTH / 2.f / PhysicsObject::PPM;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.isSensor = true;
        fd.filter.categoryBits = CollisionCategory::THROWN;
        fd.filter.maskBits = 0; // Notices nothing: the flames only hurt through World.
        body->CreateFixture(&fd);
        return body;
    }

    float FireLine::DistanceToSegment(const raylib::Vector2& point, const raylib::Vector2& start, const raylib::Vector2& end)
    {
        const float sx = end.x - start.x;
        const float sy = end.y - start.y;
        const float lengthSquared = sx * sx + sy * sy;

        float t = 0.f;
        if (lengthSquared > 0.f)
            t = std::clamp(((point.x - start.x) * sx + (point.y - start.y) * sy) / lengthSquared, 0.f, 1.f);

        const float dx = point.x - (start.x + sx * t);
        const float dy = point.y - (start.y + sy * t);
        return std::sqrt(dx * dx + dy * dy);
    }

    float FireLine::IgnitedFraction(float burnElapsed, float length, float speed)
    {
        if (burnElapsed <= 0.f)
            return 0.f;
        if (length <= 0.f)
            return 1.f;
        return std::clamp(burnElapsed * speed / length, 0.f, 1.f);
    }

    FireLine::FireLine(b2Body* body, const UserData& userData, const FireLineSpawnEvent& spawn, World* world)
        : PhysicsObject(body, { WIDTH, WIDTH }, PhysicsShape::Circle, userData), m_spawn { spawn }, m_world { world }
    {
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
    }

    float FireLine::length() const
    {
        const float dx = m_spawn.end.x - m_spawn.origin.x;
        const float dy = m_spawn.end.y - m_spawn.origin.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool FireLine::isBurning() const
    {
        return m_elapsed >= m_spawn.warningSeconds && m_elapsed < m_spawn.warningSeconds + m_spawn.burnSeconds;
    }

    void FireLine::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        if (m_elapsed >= m_spawn.warningSeconds + m_spawn.burnSeconds + FADE_DURATION)
        {
            destroy();
            return;
        }

        if (isBurning() && !m_hasHitLocalPlayer && m_world != nullptr)
        {
            // Only the part the flames have reached so far burns, so running ahead of the front is safe.
            const float reach = IgnitedFraction(m_elapsed - m_spawn.warningSeconds, length(), m_spawn.igniteSpeed);
            const raylib::Vector2 front { m_spawn.origin.x + (m_spawn.end.x - m_spawn.origin.x) * reach, m_spawn.origin.y + (m_spawn.end.y - m_spawn.origin.y) * reach };
            // Every peer runs its own copy and only hurts its own player, like bombs do.
            if (m_world->damageLocalPlayerAlongSegment(m_spawn.origin, front, WIDTH / 2.f))
                m_hasHitLocalPlayer = true;
        }

        PhysicsObject::update(timeDelta);
    }

    void FireLine::draw()
    {
        if (m_body == nullptr || !m_isVisible)
            return;

        const raylib::Vector2 start = m_spawn.origin;
        const float lineLength = length();
        if (lineLength < 1.f)
            return;
        const raylib::Vector2 direction { (m_spawn.end.x - start.x) / lineLength, (m_spawn.end.y - start.y) / lineLength };
        const raylib::Vector2 normal { -direction.y, direction.x };

        if (m_elapsed < m_spawn.warningSeconds)
        {
            // Telegraph: the outline of the fire's path, flickering faster as the ignition nears.
            const float progress = m_spawn.warningSeconds > 0.f ? m_elapsed / m_spawn.warningSeconds : 1.f;
            const float flicker = 0.5f + 0.5f * std::sin(m_elapsed * (10.f + 30.f * progress));
            const float alpha = 0.2f + 0.4f * progress + 0.25f * flicker * progress;
            const float half = WIDTH / 2.f;
            DrawLineEx({ start.x + normal.x * half, start.y + normal.y * half }, { m_spawn.end.x + normal.x * half, m_spawn.end.y + normal.y * half }, 2.f, Fade(FIRE_RED, alpha));
            DrawLineEx({ start.x - normal.x * half, start.y - normal.y * half }, { m_spawn.end.x - normal.x * half, m_spawn.end.y - normal.y * half }, 2.f, Fade(FIRE_RED, alpha));
            DrawLineEx(start, m_spawn.end, WIDTH, Fade(FIRE_RED, 0.08f + 0.14f * progress));
            return;
        }

        const float burnElapsed = m_elapsed - m_spawn.warningSeconds;
        const float reach = IgnitedFraction(burnElapsed, lineLength, m_spawn.igniteSpeed);
        const float burnEnd = m_spawn.burnSeconds;
        // The fire dies down over its last moments, and again at the end of its life.
        const float intensity = burnElapsed < burnEnd ? 1.f : std::clamp(1.f - (burnElapsed - burnEnd) / FADE_DURATION, 0.f, 1.f);
        const raylib::Vector2 front { start.x + direction.x * lineLength * reach, start.y + direction.y * lineLength * reach };

        // Glowing band under the flames: wide and dark red, then orange, then a hot core.
        DrawLineEx(start, front, WIDTH, Fade(FIRE_RED, 0.55f * intensity));
        DrawLineEx(start, front, WIDTH * 0.65f, Fade(FIRE_ORANGE, 0.75f * intensity));
        DrawLineEx(start, front, WIDTH * 0.3f, Fade(FIRE_YELLOW, 0.9f * intensity));

        // Tongues of flame rising along the whole stretch that has caught fire.
        const int32_t flameCount = static_cast<int32_t>(lineLength * reach / FLAME_SPACING);
        for (int32_t i = 0; i <= flameCount; ++i)
        {
            const float along = static_cast<float>(i) * FLAME_SPACING;
            const float wobble = std::sin(m_elapsed * 14.f + static_cast<float>(i) * 1.7f);
            const float height = (16.f + 9.f * wobble) * intensity;
            const float sway = 3.f * std::sin(m_elapsed * 9.f + static_cast<float>(i) * 0.9f);
            const raylib::Vector2 base { start.x + direction.x * along, start.y + direction.y * along };
            const float halfWidth = 7.f;
            // Counter-clockwise for DrawTriangle: tip, bottom left, bottom right.
            DrawTriangle({ base.x + sway, base.y - height }, { base.x - halfWidth, base.y + 2.f }, { base.x + halfWidth, base.y + 2.f }, Fade(FIRE_ORANGE, 0.85f * intensity));
            DrawTriangle({ base.x + sway, base.y - height * 0.6f }, { base.x - halfWidth * 0.5f, base.y + 2.f }, { base.x + halfWidth * 0.5f, base.y + 2.f },
                         Fade(FIRE_YELLOW, 0.9f * intensity));
        }
    }
} // namespace ggj
