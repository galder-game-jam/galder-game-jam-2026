#ifndef GGJ_ICECIRCLESPAWNEVENT_HPP
#define GGJ_ICECIRCLESPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * An ice circle the mage conjured on the host. Clients create their own copy from this and
     * simulate it locally; each peer then only freezes its own player if they are still inside
     * when the circle snaps shut.
     */
    struct IceCircleSpawnEvent
    {
        raylib::Vector2 center {};   ///< Middle of the circle (pixels).
        float radius { 0.f };        ///< Radius (pixels).
        float delay { 0.f };         ///< Seconds between appearing and freezing whoever is still inside.
        float freezeSeconds { 0.f }; ///< How long the freeze lasts.
    };
} // namespace ggj

#endif // GGJ_ICECIRCLESPAWNEVENT_HPP
