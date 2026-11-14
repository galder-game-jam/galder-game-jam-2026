#ifndef GGJ_TREANTROOT_H
#define GGJ_TREANTROOT_H

#include "../../graphics/PhysicsObject.h"

namespace ggj
{
    class World;

    /*!
     * A root the treant throws at a player. It arcs through the air, lands and lies on the ground as
     * a warning for a random moment, then erupts in spikes that hurt any player standing on it.
     * Drawn procedurally, so it needs no texture. Not an enemy: it cannot be hit and touching it
     * is harmless until it spikes.
     */
    class TreantRoot : public PhysicsObject
    {
        public:
        static constexpr float ROOT_RADIUS = 28.f; ///< Pixels; how big the root looks lying on the ground.
        static constexpr float HIT_RADIUS = 20.f;  ///< Pixels; the area a player must stand in to be spiked. A bit smaller than the root looks, to be forgiving.
        static constexpr float ARC_HEIGHT = 70.f;  ///< Pixels the thrown root rises above the ground at the top of its arc.
        static constexpr float LANDING_SQUASH_DURATION = 0.2f;
        static constexpr float SPIKE_RISE_DURATION = 0.12f; ///< Spikes shoot up fast...
        static constexpr float SPIKE_DURATION = 0.6f;       ///< ...stay a moment and then sink away.
        static constexpr float SPIKE_HEIGHT = 56.f;

        /*!
         * @param body Sensor body at the throw origin, already launched towards the target
         * @param userData Should be ObjectType::Trap so players, hitboxes and projectiles ignore it
         * @param target Where the root lands (pixels)
         * @param flightTime Seconds spent in the air
         * @param spikeDelay Seconds between landing and spiking
         * @param world Damages the local player when the root spikes
         */
        TreantRoot(b2Body* body, const UserData& userData, const raylib::Vector2& target, float flightTime, float spikeDelay, World* world);

        void update(float timeDelta) override;
        void draw() override;

        /*!
         * Height of the thrown root above the ground as a fraction of ARC_HEIGHT.
         *
         * @param progress Flight progress, 0 (thrown) to 1 (landed)
         * @return 0 at both ends, 1 at the top of the arc
         */
        [[nodiscard]] static float ArcFraction(float progress);

        /*!
         * How far the spikes have grown.
         *
         * @param spikeElapsed Seconds since the spikes erupted
         * @return 0 when they erupt, 1 once fully risen, falling back to 0 as they sink away
         */
        [[nodiscard]] static float SpikeFraction(float spikeElapsed);

        private:
        enum class RootPhase
        {
            Flying,
            Landed,
            Spiking
        };

        void land();
        void spike();
        void drawRoot() const;
        void drawSpikes() const;

        raylib::Vector2 m_target {};
        World* m_world { nullptr };
        RootPhase m_phase { RootPhase::Flying };
        float m_flightTime { 0.5f };
        float m_spikeDelay { 1.5f };
        float m_elapsed { 0.f };     ///< Seconds spent in the current phase.
        float m_wigglePhase { 0.f }; ///< Random offset so roots do not wriggle in unison.
    };

} // namespace ggj

#endif // GGJ_TREANTROOT_H
