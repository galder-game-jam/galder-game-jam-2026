#include "MeatCleaver.h"
#include "../../system/CollisionCategory.hpp"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ggj
{
    MeatCleaver::MeatCleaver(b2Body* body, raylib::Texture* texture, const UserData& userData, World* world)
        : PhysicsSprite(body, { FRAME_SIZE, FRAME_SIZE }, { FRAME_SIZE, FRAME_SIZE }, { 0.f, 0.f, FRAME_SIZE, FRAME_SIZE }, texture, userData), m_world { world }
    {
        // Drawn centred on the body (PhysicsSprite anchors at the feet by default).
        m_origin = raylib::Vector2 { FRAME_SIZE / 2.f, FRAME_SIZE / 2.f };
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
    }

    b2Body* MeatCleaver::CreateBody(b2World& world, const CleaverSpawnEvent& spawn)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(spawn.position);
        bodyDef.fixedRotation = false;
        b2Body* body = world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = BODY_RADIUS / PhysicsObject::PPM;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f;
        fd.isSensor = true; // Never shoves anything; the blade only hurts through World::damageLocalPlayerWithin().
        fd.filter.categoryBits = CollisionCategory::THROWN;
        fd.filter.maskBits = CollisionCategory::TERRAIN;
        body->CreateFixture(&fd);

        body->SetLinearVelocity({ spawn.velocity.x, spawn.velocity.y });
        body->SetAngularVelocity(SpinFor(spawn.velocity));
        return body;
    }

    std::vector<CleaverSpawnEvent> MeatCleaver::BuildRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius)
    {
        std::vector<CleaverSpawnEvent> ring;
        if (count <= 0)
            return ring;

        ring.reserve(static_cast<size_t>(count));
        const float metresPerSecond = speed / PhysicsObject::PPM;
        for (int32_t i = 0; i < count; ++i)
        {
            const float angle = startAngle + 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(count);
            const float dirX = std::cos(angle);
            const float dirY = std::sin(angle);
            ring.push_back(CleaverSpawnEvent { raylib::Vector2 { origin.x + dirX * ringRadius, origin.y + dirY * ringRadius },
                                               raylib::Vector2 { dirX * metresPerSecond, dirY * metresPerSecond } });
        }
        return ring;
    }

    float MeatCleaver::SpinFor(const raylib::Vector2& velocity)
    {
        const float direction = velocity.x < 0.f ? -1.f : 1.f;
        return direction * SPIN_DEGREES_PER_SECOND * std::numbers::pi_v<float> / 180.f;
    }

    uint8_t MeatCleaver::StuckAlpha(float stuckTime)
    {
        const float remaining = std::clamp(1.f - stuckTime / STUCK_DURATION, 0.f, 1.f);
        return static_cast<uint8_t>(std::lround(255.f * remaining));
    }

    void MeatCleaver::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        PhysicsObject::update(timeDelta);

        if (m_isStuck)
        {
            m_stuckTime += timeDelta;
            if (m_stuckTime >= STUCK_DURATION)
                destroy();
            return;
        }

        // Walls first: a cleaver that already stuck into an obstacle never reaches the player hiding behind it.
        if (CollisionCategory::IsTouchingTerrain(*m_body))
        {
            stickIntoWall();
            return;
        }

        if (m_elapsed >= MAX_FLIGHT_TIME)
        {
            destroy();
            return;
        }

        // Every peer runs its own copy and only hurts its own player, just like touching an enemy
        // is resolved by the player who touched it.
        if (!m_hasHitLocalPlayer && m_world != nullptr && m_world->damageLocalPlayerWithin(getPosition(), HIT_RADIUS))
            m_hasHitLocalPlayer = true;
    }

    void MeatCleaver::stickIntoWall()
    {
        m_isStuck = true;
        m_stuckTime = 0.f;
        m_body->SetLinearVelocity({ 0.f, 0.f });
        m_body->SetAngularVelocity(0.f);
    }

    void MeatCleaver::draw()
    {
        if (!m_isVisible || m_texture == nullptr)
            return;

        const float size = FRAME_SIZE * DRAW_SCALE;
        const raylib::Color tint { 255, 255, 255, m_isStuck ? StuckAlpha(m_stuckTime) : static_cast<uint8_t>(255) };
        m_texture->Draw(raylib::Rectangle { 0.f, 0.f, FRAME_SIZE, FRAME_SIZE }, raylib::Rectangle { m_position.x, m_position.y, size, size },
                        raylib::Vector2 { size / 2.f, size / 2.f }, m_rotation, tint);
    }
} // namespace ggj
