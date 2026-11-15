#ifndef GGJ_MEATCLEAVER_H
#define GGJ_MEATCLEAVER_H

#include "../../graphics/PhysicsSprite.h"
#include "../../data/CleaverSpawnEvent.hpp"
#include <cstdint>
#include <vector>

namespace ggj
{
    class World;

    /*!
     * A meat cleaver the butcher throws. It spins through the air in a straight line, hurts the
     * player it passes through and sticks into the first wall or obstacle it meets, which is what
     * lets players hide behind pillars and crates. Not an enemy: it cannot be hit and touching it
     * does nothing, only its blade hurts. It passes through players, enemies and bombs.
     */
    class MeatCleaver : public PhysicsSprite
    {
        public:
        static constexpr float FRAME_SIZE = 28.f; ///< cleaver.png is one square frame.
        static constexpr float DRAW_SCALE = 2.f;  ///< Integer scale, so the pixel art stays crisp.
        static constexpr float BODY_RADIUS = 8.f; ///< Pixels; the part of the cleaver that sticks into walls.
        static constexpr float HIT_RADIUS = 14.f; ///< Pixels from the cleaver's centre that hurt the player.
        static constexpr float SPIN_DEGREES_PER_SECOND = 600.f;
        static constexpr float STUCK_DURATION = 0.8f; ///< Seconds a cleaver stays embedded before it fades away.
        static constexpr float MAX_FLIGHT_TIME = 6.f; ///< Safety net: a cleaver that never meets a wall is removed.

        /*!
         * Creates the physics body for a thrown cleaver: a sensor circle that only notices terrain
         * (see CollisionCategory), launched with the spawn velocity and spinning.
         *
         * Every peer builds its own cleaver from the same CleaverSpawnEvent against the same static
         * walls, so where it sticks needs no extra network traffic.
         *
         * @param world Physics world to create the body in
         * @param spawn Spawn position (pixels) and launch velocity (metres/s)
         * @return The new body, owned by @p world
         */
        [[nodiscard]] static b2Body* CreateBody(b2World& world, const CleaverSpawnEvent& spawn);

        /*!
         * Spawn events for a ring of cleavers flying outwards evenly spaced in every direction.
         *
         * @param origin Centre of the ring (pixels)
         * @param count Number of cleavers; none for zero or less
         * @param startAngle Direction of the first cleaver (radians), so consecutive rings can be offset
         * @param speed Cleaver speed in pixels per second
         * @param ringRadius Distance from @p origin each cleaver starts at (pixels), so they leave the thrower's body
         * @return One event per cleaver
         */
        [[nodiscard]] static std::vector<CleaverSpawnEvent> BuildRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius);

        /*!
         * How fast and which way a cleaver spins. Derived from the velocity so every peer spins
         * it the same way without sending anything extra.
         *
         * @param velocity Launch velocity
         * @return Angular velocity in radians per second
         */
        [[nodiscard]] static float SpinFor(const raylib::Vector2& velocity);

        /*!
         * Opacity of a cleaver that is stuck in a wall: fully visible at first, then fading out.
         *
         * @param stuckTime Seconds since it stuck
         * @return 255 right after sticking, 0 when STUCK_DURATION has passed
         */
        [[nodiscard]] static uint8_t StuckAlpha(float stuckTime);

        /*!
         * @param body Sensor body from CreateBody(), already launched
         * @param texture cleaver.png
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param world Damages the local player when the cleaver passes through them
         */
        MeatCleaver(b2Body* body, raylib::Texture* texture, const UserData& userData, World* world);

        void update(float timeDelta) override;
        void draw() override;

        private:
        void stickIntoWall();

        World* m_world { nullptr };
        float m_elapsed { 0.f };
        float m_stuckTime { 0.f };
        bool m_isStuck { false };
        bool m_hasHitLocalPlayer { false }; ///< A cleaver cuts each player at most once, then flies on.
    };

} // namespace ggj

#endif // GGJ_MEATCLEAVER_H
