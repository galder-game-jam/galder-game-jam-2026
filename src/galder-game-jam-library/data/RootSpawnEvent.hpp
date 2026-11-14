#ifndef GGJ_ROOTSPAWNEVENT_HPP
#define GGJ_ROOTSPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A root the treant threw on the host. Clients create their own copy from this and simulate it
     * locally; each peer then only checks the spike against its own player.
     */
    struct RootSpawnEvent
    {
        raylib::Vector2 origin {}; ///< Where the root is thrown from (pixels).
        raylib::Vector2 target {}; ///< Where it lands (pixels).
        float flightTime { 0.f };  ///< Seconds spent in the air.
        float spikeDelay { 0.f };  ///< Seconds between landing and spiking.
    };
} // namespace ggj

#endif // GGJ_ROOTSPAWNEVENT_HPP
