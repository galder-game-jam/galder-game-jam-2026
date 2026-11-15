#include "ThrownSpider.h"
#include "../../system/CollisionCategory.hpp"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>

namespace ggj
{
    ThrownSpider::ThrownSpider(b2Body* body, raylib::Texture* texture, const UserData& userData, float flightTime, World* world)
        : PhysicsSprite(body, { FRAME_WIDTH, FRAME_HEIGHT }, { FRAME_WIDTH, FRAME_HEIGHT }, { 0.f, 0.f, FRAME_WIDTH, FRAME_HEIGHT }, texture, userData), m_world { world },
          m_flightTime { std::max(flightTime, 0.01f) }
    {
        m_body->SetFixedRotation(true);
        // Drawn centred on the body (PhysicsSprite anchors at the feet by default).
        m_origin = raylib::Vector2 { FRAME_WIDTH / 2.f, FRAME_HEIGHT / 2.f };
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
        m_tumbleDirection = m_body->GetLinearVelocity().x < 0.f ? -1.f : 1.f;
    }

    b2Body* ThrownSpider::CreateBody(b2World& world, const SpiderSpawnEvent& spawn)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(spawn.origin);
        bodyDef.fixedRotation = true;
        b2Body* body = world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = BODY_RADIUS / PhysicsObject::PPM;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f;
        fd.isSensor = true; // Never shoves anything; the splat is decided in update().
        fd.filter.categoryBits = CollisionCategory::THROWN;
        fd.filter.maskBits = CollisionCategory::TERRAIN;
        body->CreateFixture(&fd);

        const raylib::Vector2 velocity = LaunchVelocity(spawn);
        body->SetLinearVelocity({ velocity.x, velocity.y });
        return body;
    }

    raylib::Vector2 ThrownSpider::LaunchVelocity(const SpiderSpawnEvent& spawn)
    {
        if (spawn.flightTime <= 0.f)
            return { 0.f, 0.f };
        return { (spawn.target.x - spawn.origin.x) / spawn.flightTime / PhysicsObject::PPM, (spawn.target.y - spawn.origin.y) / spawn.flightTime / PhysicsObject::PPM };
    }

    float ThrownSpider::ArcFraction(float progress)
    {
        const float t = std::clamp(progress, 0.f, 1.f);
        return 4.f * t * (1.f - t);
    }

    int32_t ThrownSpider::FrameIndex(float elapsed)
    {
        const int32_t frame = static_cast<int32_t>(std::max(elapsed, 0.f) / FRAME_DURATION);
        return frame % FRAME_COUNT;
    }

    void ThrownSpider::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        PhysicsObject::update(timeDelta);
        m_drawingRect = raylib::Rectangle { FRAME_WIDTH * static_cast<float>(FrameIndex(m_elapsed)), 0.f, FRAME_WIDTH, FRAME_HEIGHT };

        if (m_world != nullptr && m_world->isAnyPlayerWithin(getPosition(), HIT_RADIUS))
        {
            // Every peer runs its own copy and only stuns its own player; the splat shows for everyone.
            m_world->stunLocalPlayerWithin(getPosition(), HIT_RADIUS, STUN_SECONDS);
            splat();
            return;
        }

        // Splats against the first wall or obstacle, or on the floor where it lands.
        if (CollisionCategory::IsTouchingTerrain(*m_body) || m_elapsed >= m_flightTime)
            splat();
    }

    void ThrownSpider::splat()
    {
        emitBlood({ 0.f, 0.f }, SPLAT_BLOOD_STRENGTH);
        destroy();
    }

    void ThrownSpider::draw()
    {
        if (!m_isVisible || m_texture == nullptr)
            return;

        const float progress = std::clamp(m_elapsed / m_flightTime, 0.f, 1.f);
        const float lift = ARC_HEIGHT * ArcFraction(progress);

        // A shadow on the ground shows where it will land, and shrinks the higher the spider flies.
        const float shadowScale = 1.f - 0.4f * ArcFraction(progress);
        DrawEllipse(static_cast<int32_t>(m_position.x), static_cast<int32_t>(m_position.y), 12.f * shadowScale, 5.f * shadowScale, ::Color { 0, 0, 0, 90 });

        const float width = FRAME_WIDTH * DRAW_SCALE;
        const float height = FRAME_HEIGHT * DRAW_SCALE;
        const float tumble = m_tumbleDirection * 360.f * TUMBLE_TURNS * progress;
        m_texture->Draw(m_drawingRect, raylib::Rectangle { m_position.x, m_position.y - lift, width, height }, raylib::Vector2 { width / 2.f, height / 2.f }, tumble, WHITE);
    }
} // namespace ggj
