#ifndef GGJ_FIRELINESPAWNEVENT_HPP
#define GGJ_FIRELINESPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A line of fire the mage shot on the host. The host works out where it ends so every peer
     * burns exactly the same stretch; each peer only hurts its own player.
     */
    struct FireLineSpawnEvent
    {
        raylib::Vector2 origin {};    ///< Where the fire starts: the middle of the mage (pixels).
        raylib::Vector2 end {};       ///< Where it stops: the first wall, or the edge of the map (pixels).
        float warningSeconds { 0.f }; ///< Seconds of telegraph before it ignites.
        float burnSeconds { 0.f };    ///< Seconds it burns once ignited.
    float igniteSpeed { 0.f };    ///< How fast the flames race along the line once it ignites (pixels per second).
    };
} // namespace ggj

#endif // GGJ_FIRELINESPAWNEVENT_HPP
