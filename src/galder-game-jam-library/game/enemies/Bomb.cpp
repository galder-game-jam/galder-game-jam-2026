//
// Created by robin on 14.11.2026.
//

#include "Bomb.h"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>

namespace ggj
{
    Bomb::Bomb(b2Body* body, raylib::Texture* texture, raylib::Texture* explosionTexture, const UserData& userData, float fuseTime, World* world)
        : PhysicsSprite(body, { FRAME_SIZE, FRAME_SIZE }, { FRAME_SIZE, FRAME_SIZE }, { 0.f, 0.f, FRAME_SIZE, FRAME_SIZE }, texture, userData),
          m_explosionTexture { explosionTexture }, m_world { world }, m_fuseTime { std::max(fuseTime, 0.01f) }
    {
        m_body->SetFixedRotation(true);
        // Draw the frame centred on the body (PhysicsSprite anchors at the feet by default).
        m_origin = raylib::Vector2 { FRAME_SIZE / 2.f, FRAME_SIZE / 2.f };
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
    }

    int32_t Bomb::FuseFrameIndex(float elapsed, float fuseTime)
    {
        if (fuseTime <= 0.f)
            return FUSE_FRAME_COUNT - 1;
        const int32_t frame = static_cast<int32_t>(std::floor(static_cast<float>(FUSE_FRAME_COUNT) * elapsed / fuseTime));
        return std::clamp(frame, 0, FUSE_FRAME_COUNT - 1);
    }

    ::Color Bomb::FuseTint(float progress)
    {
        const float t = std::clamp(progress, 0.f, 1.f);
        const uint8_t channel = static_cast<uint8_t>(std::lround(255.f - (255.f - static_cast<float>(MIN_TINT_CHANNEL)) * t));
        return ::Color { 255, channel, channel, 255 };
    }

    raylib::Rectangle Bomb::ExplosionFrameRect(float elapsed)
    {
        const int32_t frame = std::clamp(static_cast<int32_t>(std::max(elapsed, 0.f) / EXPLOSION_FRAME_DURATION), 0, EXPLOSION_FRAME_COUNT - 1);
        return raylib::Rectangle { FRAME_SIZE * static_cast<float>(frame % EXPLOSION_COLUMNS), FRAME_SIZE * static_cast<float>(frame / EXPLOSION_COLUMNS), FRAME_SIZE, FRAME_SIZE };
    }

    void Bomb::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        PhysicsObject::update(timeDelta);

        if (!m_hasExploded)
        {
            if (m_elapsed >= m_fuseTime)
            {
                explode();
                return;
            }
            const int32_t frame = FuseFrameIndex(m_elapsed, m_fuseTime);
            m_drawingRect = raylib::Rectangle { FRAME_SIZE * static_cast<float>(frame), 0.f, FRAME_SIZE, FRAME_SIZE };
            m_tint = FuseTint(m_elapsed / m_fuseTime);
            return;
        }

        m_drawingRect = ExplosionFrameRect(m_elapsed);
        if (m_elapsed >= EXPLOSION_FRAME_DURATION * static_cast<float>(EXPLOSION_FRAME_COUNT))
            destroy();
    }

    void Bomb::explode()
    {
        m_hasExploded = true;
        m_elapsed = 0.f;
        m_body->SetLinearVelocity({ 0.f, 0.f });
        m_tint = WHITE;
        if (m_explosionTexture != nullptr)
            m_texture = m_explosionTexture;
        m_drawingRect = ExplosionFrameRect(0.f);

        // Every peer runs its own copy of the bomb and only hurts its own player, just like
        // touching an enemy is resolved by the player who touched it.
        if (m_world != nullptr)
            m_world->damageLocalPlayerWithin(getPosition(), BLAST_RADIUS);
    }
} // namespace ggj
