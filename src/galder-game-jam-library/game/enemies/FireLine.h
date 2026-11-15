#ifndef GGJ_FIRELINE_H
#define GGJ_FIRELINE_H

#include "../../data/FireLineSpawnEvent.hpp"
#include "../../graphics/PhysicsObject.h"

namespace ggj
{
    class World;

    /*!
     * A line of fire the mage shoots from the middle of where it stands, in a straight line to the
     * first wall or the edge of the map. A red warning shows where it will go; then the fire races
     * along the line and burns for a while. Anyone it touches loses a life, at most once per line.
     * Not an enemy: it has no collision and cannot be hit, only its flames hurt.
     */
    class FireLine : public PhysicsObject
    {
        public:
        static constexpr float WIDTH = 30.f;          ///< Thickness of the fire (pixels); half of it either side of the line hurts.
        static constexpr float FADE_DURATION = 0.35f; ///< Seconds the fire takes to die down once it stops burning.

        /*!
         * Creates the physics body: a sensor that collides with nothing. PhysicsObject needs a body,
         * but the fire only ever acts through World::damageLocalPlayerAlongSegment().
         *
         * @param world Physics world to create the body in
         * @param spawn Where the fire starts
         * @return The new body, owned by @p world
         */
        [[nodiscard]] static b2Body* CreateBody(b2World& world, const FireLineSpawnEvent& spawn);

        /*!
         * Distance from a point to the closest point of a line segment, which is how close someone is to the fire.
         *
         * @param point Point to measure from
         * @param start One end of the segment
         * @param end The other end of the segment
         * @return Distance in pixels; the distance to @p start when the segment has no length
         */
        [[nodiscard]] static float DistanceToSegment(const raylib::Vector2& point, const raylib::Vector2& start, const raylib::Vector2& end);

        /*!
         * How far along the line the flames have raced, as a fraction of its length.
         *
         * @param burnElapsed Seconds since the fire ignited; zero or less before that
         * @param length Length of the line (pixels)
         * @param speed How fast the flames race along the line (pixels per second)
         * @return 0 before ignition, 1 once the flames reached the end
         */
        [[nodiscard]] static float IgnitedFraction(float burnElapsed, float length, float speed);

        /*!
         * @param body Body from CreateBody()
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param spawn What the host shot
         * @param world Hurts the local player the flames touch
         */
        FireLine(b2Body* body, const UserData& userData, const FireLineSpawnEvent& spawn, World* world);

        void update(float timeDelta) override;
        void draw() override;

        private:
        [[nodiscard]] float length() const;
        [[nodiscard]] bool isBurning() const;

        FireLineSpawnEvent m_spawn;
        World* m_world { nullptr };
        float m_elapsed { 0.f };
        bool m_hasHitLocalPlayer { false }; ///< Fire burns each player at most once, then keeps burning for show.
    };

} // namespace ggj

#endif // GGJ_FIRELINE_H
