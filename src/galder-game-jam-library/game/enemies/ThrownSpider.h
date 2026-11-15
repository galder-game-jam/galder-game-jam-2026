#ifndef GGJ_THROWNSPIDER_H
#define GGJ_THROWNSPIDER_H

#include "../../graphics/PhysicsSprite.h"
#include "../../data/SpiderSpawnEvent.hpp"
#include <cstdint>

namespace ggj
{
    class World;

    /*!
     * A spider the butcher hurls at one player. It tumbles through the air towards where the player
     * stood when it was thrown. If it meets a player on the way, that player is stunned; either
     * way it then splats into a pile of blood, whether on the player, on the floor where it
     * landed or against the first wall or obstacle in its path (so cover works against it too).
     * Not an enemy: it cannot be hit and touching it does nothing, only the splat stuns.
     */
    class ThrownSpider : public PhysicsSprite
    {
        public:
        static constexpr float FRAME_WIDTH = 32.f; ///< spider.png is a strip of 32x21 frames.
        static constexpr float FRAME_HEIGHT = 21.f;
        static constexpr int32_t FRAME_COUNT = 4;
        static constexpr float FRAME_DURATION = 0.07f;     ///< Seconds per frame; the legs flail while it flies.
        static constexpr float DRAW_SCALE = 2.f;           ///< Integer scale, so the pixel art stays crisp.
        static constexpr float BODY_RADIUS = 7.f;          ///< Pixels; the part of the spider that splats against walls.
        static constexpr float HIT_RADIUS = 18.f;          ///< Pixels from the spider that count as hitting a player.
        static constexpr float ARC_HEIGHT = 36.f;          ///< Pixels the spider rises above the ground at the top of its arc.
        static constexpr float TUMBLE_TURNS = 2.f;         ///< Full rotations over the whole flight.
        static constexpr float STUN_SECONDS = 1.5f;        ///< How long a hit player cannot move or attack.
        static constexpr float SPLAT_BLOOD_STRENGTH = 3.f; ///< Burst strength of the splat; an ordinary hit is 1, a killing blow 2.

        /*!
         * Creates the physics body for a thrown spider: a sensor circle that only notices terrain
         * (see CollisionCategory), launched in a straight line so it lands on the target exactly
         * when the flight ends.
         *
         * Every peer builds its own spider from the same SpiderSpawnEvent against the same static
         * walls, so where it splats needs no extra network traffic.
         *
         * @param world Physics world to create the body in
         * @param spawn Where it starts, where it lands and how long it takes
         * @return The new body, owned by @p world
         */
        [[nodiscard]] static b2Body* CreateBody(b2World& world, const SpiderSpawnEvent& spawn);

        /*!
         * Velocity that carries a spider from its origin to its target in the flight time.
         *
         * @param spawn Spawn event
         * @return Velocity in Box2D units (metres/s); zero if the flight time is not positive
         */
        [[nodiscard]] static raylib::Vector2 LaunchVelocity(const SpiderSpawnEvent& spawn);

        /*!
         * Height of the thrown spider above the ground as a fraction of ARC_HEIGHT.
         *
         * @param progress Flight progress, 0 (thrown) to 1 (landed)
         * @return 0 at both ends, 1 at the top of the arc
         */
        [[nodiscard]] static float ArcFraction(float progress);

        /*!
         * Which frame of the leg-flailing strip to show.
         *
         * @param elapsed Seconds since the spider was thrown
         * @return Frame index in [0, FRAME_COUNT - 1]
         */
        [[nodiscard]] static int32_t FrameIndex(float elapsed);

        /*!
         * @param body Sensor body from CreateBody(), already launched
         * @param texture spider.png
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param flightTime Seconds until it lands
         * @param world Stuns the local player and checks where the other players are
         */
        ThrownSpider(b2Body* body, raylib::Texture* texture, const UserData& userData, float flightTime, World* world);

        void update(float timeDelta) override;
        void draw() override;

        private:
        /*! Bursts into a pile of blood where the spider is and removes it. */
        void splat();

        World* m_world { nullptr };
        float m_flightTime { 1.f };
        float m_elapsed { 0.f };
        float m_tumbleDirection { 1.f }; ///< +1/-1, from the throw direction so every peer tumbles it the same way.
    };

} // namespace ggj

#endif // GGJ_THROWNSPIDER_H
