#include "IceCircle.h"
#include "../../system/CollisionCategory.hpp"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ggj
{
    namespace
    {
        constexpr int32_t SPOKE_COUNT = 6;
        constexpr int32_t SHARD_COUNT = 14;
        constexpr ::Color ICE_FILL { 150, 215, 255, 255 };
        constexpr ::Color ICE_EDGE { 210, 240, 255, 255 };
        constexpr ::Color ICE_WARNING { 120, 190, 255, 255 };
        constexpr float TWO_PI = 2.f * std::numbers::pi_v<float>;
    } // namespace

    b2Body* IceCircle::CreateBody(b2World& world, const IceCircleSpawnEvent& spawn)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(spawn.center);
        b2Body* body = world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = spawn.radius / PhysicsObject::PPM; // Matches the freeze check, so "show hitboxes" is honest.
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.isSensor = true;
        fd.filter.categoryBits = CollisionCategory::THROWN;
        fd.filter.maskBits = 0; // Notices nothing: it is a decal with a timer.
        body->CreateFixture(&fd);
        return body;
    }

    bool IceCircle::IsInside(const raylib::Vector2& center, float radius, const raylib::Vector2& point)
    {
        const float dx = point.x - center.x;
        const float dy = point.y - center.y;
        return dx * dx + dy * dy <= radius * radius;
    }

    float IceCircle::Countdown(float elapsed, float delay)
    {
        if (delay <= 0.f)
            return 1.f;
        return std::clamp(elapsed / delay, 0.f, 1.f);
    }

    IceCircle::IceCircle(b2Body* body, const UserData& userData, const IceCircleSpawnEvent& spawn, World* world)
        : PhysicsObject(body, { spawn.radius * 2.f, spawn.radius * 2.f }, PhysicsShape::Circle, userData), m_spawn { spawn }, m_world { world }
    {
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
    }

    void IceCircle::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        if (!m_hasFrozen && m_elapsed >= m_spawn.delay)
            freeze();
        else if (m_hasFrozen && m_elapsed >= m_spawn.delay + SHATTER_DURATION)
        {
            destroy();
            return;
        }

        PhysicsObject::update(timeDelta);
    }

    void IceCircle::freeze()
    {
        m_hasFrozen = true;
        // Every peer runs its own copy and only freezes its own player, like bombs hurt only theirs.
        if (m_world != nullptr)
            m_world->freezeLocalPlayerWithin(m_spawn.center, m_spawn.radius, m_spawn.freezeSeconds);
    }

    void IceCircle::draw()
    {
        if (m_body == nullptr || !m_isVisible)
            return;

        const int32_t x = static_cast<int32_t>(m_position.x);
        const int32_t y = static_cast<int32_t>(m_position.y);
        const float radius = m_spawn.radius;

        if (m_hasFrozen)
        {
            // The circle snaps shut: a flash that fades while shards fly outwards.
            const float p = std::clamp((m_elapsed - m_spawn.delay) / SHATTER_DURATION, 0.f, 1.f);
            const float fade = 1.f - p;
            DrawCircle(x, y, radius, Fade(WHITE, 0.45f * fade));
            DrawCircleLines(x, y, radius, Fade(ICE_EDGE, fade));
            for (int32_t i = 0; i < SHARD_COUNT; ++i)
            {
                const float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(SHARD_COUNT) + 0.3f * static_cast<float>(i % 3);
                const float inner = radius * (0.35f + 0.65f * p);
                const float outer = inner + 14.f * fade;
                DrawLineEx({ x + std::cos(angle) * inner, y + std::sin(angle) * inner }, { x + std::cos(angle) * outer, y + std::sin(angle) * outer }, 3.f, Fade(WHITE, fade));
            }
            return;
        }

        // Warning: starts faint and brightens, pulsing faster, as the freeze gets closer.
        const float progress = Countdown(m_elapsed, m_spawn.delay);
        const float pulse = 0.5f + 0.5f * std::sin(m_elapsed * (8.f + 20.f * progress));
        DrawCircle(x, y, radius, Fade(ICE_FILL, 0.16f + 0.18f * progress));
        DrawCircleLines(x, y, radius, Fade(ICE_WARNING, 0.5f + 0.35f * pulse));
        DrawCircleLines(x, y, radius - 1.f, Fade(ICE_WARNING, 0.5f + 0.35f * pulse));

        // A ring shrinking towards the middle is the countdown: when it arrives, the circle freezes.
        DrawCircleLines(x, y, radius * (1.f - progress), Fade(WHITE, 0.55f + 0.4f * progress));

        // Frost pattern: spokes that slowly turn, with a tip across each.
        for (int32_t i = 0; i < SPOKE_COUNT; ++i)
        {
            const float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(SPOKE_COUNT) + m_elapsed * 0.6f;
            const raylib::Vector2 tip { x + std::cos(angle) * radius * 0.85f, y + std::sin(angle) * radius * 0.85f };
            DrawLineEx({ static_cast<float>(x), static_cast<float>(y) }, tip, 2.f, Fade(ICE_EDGE, 0.3f + 0.3f * progress));
            const float across = angle + std::numbers::pi_v<float> / 2.f;
            DrawLineEx({ tip.x + std::cos(across) * 7.f, tip.y + std::sin(across) * 7.f }, { tip.x - std::cos(across) * 7.f, tip.y - std::sin(across) * 7.f }, 2.f,
                       Fade(ICE_EDGE, 0.3f + 0.3f * progress));
        }
    }
} // namespace ggj
