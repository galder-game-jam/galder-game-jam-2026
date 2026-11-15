#ifndef GGJ_ICECIRCLE_H
#define GGJ_ICECIRCLE_H

#include "../../data/IceCircleSpawnEvent.hpp"
#include "../../graphics/PhysicsObject.h"

namespace ggj
{
    class World;

    /*!
     * A circle of frost the mage conjures on the floor. It shows up as a warning, and when the
     * countdown ends it snaps shut and freezes whoever is still standing inside, so the trick is to
     * run out in time. Not an enemy: it has no collision and cannot be hit. Purely a floor decal
     * with a timer, which is why it is drawn in the layer underneath the characters.
     */
    class IceCircle : public PhysicsObject
    {
        public:
        static constexpr float SHATTER_DURATION = 0.5f; ///< Seconds the burst of ice shards lingers after the freeze.

        /*!
         * Creates the physics body: a sensor that collides with nothing. PhysicsObject needs a body,
         * but the circle only ever acts through World::freezeLocalPlayerWithin().
         *
         * @param world Physics world to create the body in
         * @param spawn Where the circle is and how large
         * @return The new body, owned by @p world
         */
        [[nodiscard]] static b2Body* CreateBody(b2World& world, const IceCircleSpawnEvent& spawn);

        /*!
         * Whether a point is inside a circle. Used for the freeze itself, so what is drawn is what hurts.
         *
         * @param center Middle of the circle
         * @param radius Radius of the circle
         * @param point Point to test
         */
        [[nodiscard]] static bool IsInside(const raylib::Vector2& center, float radius, const raylib::Vector2& point);

        /*!
         * How far the countdown has come, for the shrinking ring.
         *
         * @param elapsed Seconds since the circle appeared
         * @param delay Seconds until it freezes
         * @return 0 when it just appeared, 1 when it freezes; stays 1 afterwards
         */
        [[nodiscard]] static float Countdown(float elapsed, float delay);

        /*!
         * @param body Body from CreateBody()
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param spawn What the host conjured
         * @param world Freezes the local player when the countdown ends
         */
        IceCircle(b2Body* body, const UserData& userData, const IceCircleSpawnEvent& spawn, World* world);

        void update(float timeDelta) override;
        void draw() override;

        private:
        void freeze();

        IceCircleSpawnEvent m_spawn;
        World* m_world { nullptr };
        float m_elapsed { 0.f };
        bool m_hasFrozen { false };
    };

} // namespace ggj

#endif // GGJ_ICECIRCLE_H
