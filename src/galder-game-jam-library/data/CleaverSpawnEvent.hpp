#ifndef GGJ_CLEAVERSPAWNEVENT_HPP
#define GGJ_CLEAVERSPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A meat cleaver the butcher threw on the host. Clients create their own copy from this and
     * simulate it locally; each peer then only checks the cleaver against its own player.
     */
    struct CleaverSpawnEvent
    {
        raylib::Vector2 position {}; ///< Spawn position in pixels.
        raylib::Vector2 velocity {}; ///< Launch velocity in Box2D units (metres/s).
    };
} // namespace ggj

#endif // GGJ_CLEAVERSPAWNEVENT_HPP
