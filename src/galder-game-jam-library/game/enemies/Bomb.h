//
// Created by robin on 14.11.2026.
//

#ifndef GGJ_BOMB_H
#define GGJ_BOMB_H

#include "../../graphics/PhysicsSprite.h"
#include <cstdint>

namespace ggj
{
    class World;

    /*!
     * A bomb King Claude throws in his enraged phase. It slides to a stop, ticks down its fuse
     * (frame and red tint advance with the timer) and then explodes, hurting any player within
     * BLAST_RADIUS. Not an enemy: it cannot be hit and touching it is harmless.
     */
    class Bomb : public PhysicsSprite
    {
        public:
        static constexpr float FRAME_SIZE = 64.f;
        static constexpr int32_t FUSE_FRAME_COUNT = 3;       ///< Frames in bomb.png, one per third of the fuse.
        static constexpr int32_t EXPLOSION_FRAME_COUNT = 16; ///< Frames in bomb-explosion.png (4x4 grid).
        static constexpr int32_t EXPLOSION_COLUMNS = 4;
        static constexpr float EXPLOSION_FRAME_DURATION = 0.04f;
        static constexpr float BLAST_RADIUS = 32.f;  ///< Pixels from the bomb centre (64px blast, matching the explosion frame).
        static constexpr float LINEAR_DAMPING = 3.f; ///< How quickly a thrown bomb slows down.

        /*!
             * @param body Sensor body, already launched with its initial velocity
             * @param texture bomb.png
             * @param explosionTexture bomb-explosion.png
             * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
             * @param fuseTime Seconds until it explodes
             * @param world Damages the local player when the bomb explodes
             */
        Bomb(b2Body* body, raylib::Texture* texture, raylib::Texture* explosionTexture, const UserData& userData, float fuseTime, World* world);

        void update(float timeDelta) override;

        /*!
             * Which fuse frame to show: the first third of the fuse shows frame 0, the second
             * third frame 1 and the rest frame 2.
             *
             * @param elapsed Seconds since the bomb was spawned
             * @param fuseTime Total fuse length in seconds
             * @return Frame index in [0, FUSE_FRAME_COUNT - 1]
             */
        [[nodiscard]] static int32_t FuseFrameIndex(float elapsed, float fuseTime);

        /*!
             * Tint that turns the bomb redder as it gets closer to exploding.
             *
             * @param progress Fuse progress, 0 (just spawned) to 1 (exploding)
             * @return White at 0, strongly red at 1
             */
        [[nodiscard]] static ::Color FuseTint(float progress);

        /*!
             * Source rectangle of the explosion frame to show.
             *
             * @param elapsed Seconds since the bomb exploded
             * @return Rectangle in bomb-explosion.png; the last frame once the animation is over
             */
        [[nodiscard]] static raylib::Rectangle ExplosionFrameRect(float elapsed);

        private:
        void explode();

        raylib::Texture* m_explosionTexture { nullptr };
        World* m_world { nullptr };
        float m_fuseTime { 1.f };
        float m_elapsed { 0.f }; ///< Fuse time while ticking, explosion time once exploded.
        bool m_hasExploded { false };

        static constexpr uint8_t MIN_TINT_CHANNEL = 40; ///< Green/blue left in the tint at full fuse.
    };

} // namespace ggj

#endif // GGJ_BOMB_H
