#ifndef GGJ_CRYSTALSHARD_H
#define GGJ_CRYSTALSHARD_H

#include "../../data/CrystalSpawnEvent.hpp"
#include "../../graphics/PhysicsObject.h"
#include <cstdint>
#include <vector>

namespace ggj
{
    class World;

    /*!
     * A pink crystal the mage fires. It flies in a straight line, hurts the first player it
     * touches and shatters, or shatters against the first wall it meets. Not an enemy: it cannot
     * be hit, and it passes through enemies, bombs and other projectiles.
     */
    class CrystalShard : public PhysicsObject
    {
        public:
        static constexpr float BODY_RADIUS = 6.f;     ///< Pixels; the part of the crystal that shatters on walls.
        static constexpr float HIT_RADIUS = 12.f;     ///< Pixels from the crystal's centre that hurt the player.
        static constexpr float LENGTH = 22.f;         ///< Drawn length of the crystal (pixels).
        static constexpr float WIDTH = 10.f;          ///< Drawn width of the crystal (pixels).
        static constexpr float MAX_FLIGHT_TIME = 6.f; ///< Safety net: a crystal that never meets anything is removed.

        /*!
         * Creates the physics body for a crystal: a sensor circle that only notices terrain
         * (see CollisionCategory), launched with the spawn velocity.
         *
         * Every peer builds its own crystal from the same CrystalSpawnEvent against the same static
         * walls, so where it shatters needs no extra network traffic.
         *
         * @param world Physics world to create the body in
         * @param spawn Spawn position (pixels) and launch velocity (metres/s)
         * @return The new body, owned by @p world
         */
        [[nodiscard]] static b2Body* CreateBody(b2World& world, const CrystalSpawnEvent& spawn);

        /*!
         * Spawn events for a fan of crystals aimed at one point, evenly spread across the fan.
         *
         * @param origin Where the crystals leave from (pixels)
         * @param aimAngle Direction of the middle of the fan (radians)
         * @param count Number of crystals; none for zero or less
         * @param spreadDegrees Total width of the fan; zero sends them all down the same line
         * @param speed Crystal speed in pixels per second
         * @return One event per crystal, from one edge of the fan to the other
         */
        [[nodiscard]] static std::vector<CrystalSpawnEvent> BuildFan(const raylib::Vector2& origin, float aimAngle, int32_t count, float spreadDegrees, float speed);

        /*!
         * Spawn events for a ring of crystals flying outwards evenly spaced in every direction.
         *
         * @param origin Centre of the ring (pixels)
         * @param count Number of crystals; none for zero or less
         * @param startAngle Direction of the first crystal (radians)
         * @param speed Crystal speed in pixels per second
         * @param ringRadius Distance from @p origin each crystal starts at (pixels), so they leave the caster's body
         * @return One event per crystal
         */
        [[nodiscard]] static std::vector<CrystalSpawnEvent> BuildRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius);

        /*!
         * @param body Sensor body from CreateBody(), already launched
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param world Damages the local player when the crystal reaches them
         */
        CrystalShard(b2Body* body, const UserData& userData, World* world);

        void update(float timeDelta) override;
        void draw() override;

        private:
        void shatter();

        World* m_world { nullptr };
        float m_elapsed { 0.f };
        float m_heading { 0.f }; ///< Direction of flight (radians); the crystal is drawn pointing along it.
    };

} // namespace ggj

#endif // GGJ_CRYSTALSHARD_H
