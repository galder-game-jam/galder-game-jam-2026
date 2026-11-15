#include "CrystalShard.h"
#include "../../system/CollisionCategory.hpp"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ggj
{
    namespace
    {
        constexpr ::Color CRYSTAL_PINK { 255, 95, 200, 255 };
        constexpr ::Color CRYSTAL_LIGHT { 255, 200, 240, 255 };
        constexpr ::Color CRYSTAL_DARK { 175, 40, 140, 255 };
        constexpr int32_t TRAIL_COUNT = 3;
    } // namespace

    b2Body* CrystalShard::CreateBody(b2World& world, const CrystalSpawnEvent& spawn)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(spawn.position);
        bodyDef.fixedRotation = true;
        b2Body* body = world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = BODY_RADIUS / PhysicsObject::PPM;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f;
        fd.isSensor = true; // Never shoves anything; the crystal only hurts through World::damageLocalPlayerWithin().
        fd.filter.categoryBits = CollisionCategory::THROWN;
        fd.filter.maskBits = CollisionCategory::TERRAIN;
        body->CreateFixture(&fd);

        body->SetLinearVelocity({ spawn.velocity.x, spawn.velocity.y });
        return body;
    }

    std::vector<CrystalSpawnEvent> CrystalShard::BuildFan(const raylib::Vector2& origin, float aimAngle, int32_t count, float spreadDegrees, float speed)
    {
        std::vector<CrystalSpawnEvent> fan;
        if (count <= 0)
            return fan;

        fan.reserve(static_cast<size_t>(count));
        const float metresPerSecond = speed / PhysicsObject::PPM;
        const float spread = spreadDegrees * std::numbers::pi_v<float> / 180.f;
        for (int32_t i = 0; i < count; ++i)
        {
            // A lone crystal goes straight at its target; more spread out evenly from edge to edge.
            const float t = count > 1 ? static_cast<float>(i) / static_cast<float>(count - 1) - 0.5f : 0.f;
            const float angle = aimAngle + spread * t;
            fan.push_back(CrystalSpawnEvent { origin, raylib::Vector2 { std::cos(angle) * metresPerSecond, std::sin(angle) * metresPerSecond } });
        }
        return fan;
    }

    std::vector<CrystalSpawnEvent> CrystalShard::BuildRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius)
    {
        std::vector<CrystalSpawnEvent> ring;
        if (count <= 0)
            return ring;

        ring.reserve(static_cast<size_t>(count));
        const float metresPerSecond = speed / PhysicsObject::PPM;
        for (int32_t i = 0; i < count; ++i)
        {
            const float angle = startAngle + 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(count);
            const float dirX = std::cos(angle);
            const float dirY = std::sin(angle);
            ring.push_back(CrystalSpawnEvent { raylib::Vector2 { origin.x + dirX * ringRadius, origin.y + dirY * ringRadius },
                                               raylib::Vector2 { dirX * metresPerSecond, dirY * metresPerSecond } });
        }
        return ring;
    }

    CrystalShard::CrystalShard(b2Body* body, const UserData& userData, World* world)
        : PhysicsObject(body, { BODY_RADIUS * 2.f, BODY_RADIUS * 2.f }, PhysicsShape::Circle, userData), m_world { world }
    {
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
        const b2Vec2 velocity = m_body->GetLinearVelocity();
        m_heading = std::atan2(velocity.y, velocity.x);
    }

    void CrystalShard::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        PhysicsObject::update(timeDelta);

        // Walls first: a crystal that already shattered against an obstacle never reaches the player behind it.
        if (CollisionCategory::IsTouchingTerrain(*m_body) || m_elapsed >= MAX_FLIGHT_TIME)
        {
            shatter();
            return;
        }

        if (m_world != nullptr && m_world->isAnyPlayerWithin(getPosition(), HIT_RADIUS))
        {
            // Every peer runs its own copy and only hurts its own player; the crystal breaks for everyone.
            m_world->damageLocalPlayerWithin(getPosition(), HIT_RADIUS);
            shatter();
        }
    }

    void CrystalShard::shatter()
    {
        destroy();
    }

    void CrystalShard::draw()
    {
        if (m_body == nullptr || !m_isVisible)
            return;

        const float cosH = std::cos(m_heading);
        const float sinH = std::sin(m_heading);
        const auto point = [&](float along, float across, float scale) -> raylib::Vector2
        { return { m_position.x + cosH * along * scale - sinH * across * scale, m_position.y + sinH * along * scale + cosH * across * scale }; };

        // A short fading trail behind it, so the direction of flight reads at a glance.
        for (int32_t i = TRAIL_COUNT; i >= 1; --i)
        {
            const float back = static_cast<float>(i) * 7.f;
            const float fade = 0.35f - 0.09f * static_cast<float>(i);
            DrawCircleV(point(-back, 0.f, 1.f), 4.f - static_cast<float>(i) * 0.7f, Fade(CRYSTAL_PINK, fade));
        }

        DrawCircleV(m_position, LENGTH * 0.55f, Fade(CRYSTAL_PINK, 0.22f)); // Soft glow

        // An elongated diamond: tip, upper side, tail, lower side. Two halves give it a facet.
        const raylib::Vector2 tip = point(LENGTH * 0.6f, 0.f, 1.f);
        const raylib::Vector2 tail = point(-LENGTH * 0.4f, 0.f, 1.f);
        const raylib::Vector2 upper = point(LENGTH * 0.1f, -WIDTH / 2.f, 1.f);
        const raylib::Vector2 lower = point(LENGTH * 0.1f, WIDTH / 2.f, 1.f);
        // Counter-clockwise for DrawTriangle in screen space.
        DrawTriangle(tip, upper, tail, CRYSTAL_PINK);
        DrawTriangle(tip, tail, lower, CRYSTAL_DARK);
        DrawTriangle(tip, point(LENGTH * 0.1f, -WIDTH * 0.15f, 1.f), point(-LENGTH * 0.1f, 0.f, 1.f), CRYSTAL_LIGHT);
        DrawLineV(upper, tip, CRYSTAL_LIGHT);
    }
} // namespace ggj
