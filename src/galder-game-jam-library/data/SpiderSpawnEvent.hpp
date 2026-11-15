#ifndef GGJ_SPIDERSPAWNEVENT_HPP
#define GGJ_SPIDERSPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A spider the butcher threw at a player on the host. Clients create their own copy from this
     * and simulate it locally; each peer then only stuns its own player if the spider hits them.
     */
    struct SpiderSpawnEvent
    {
        raylib::Vector2 origin {}; ///< Where the spider is thrown from (pixels).
        raylib::Vector2 target {}; ///< Where it would land (pixels), i.e. where the victim stood when it was thrown.
        float flightTime { 0.f };  ///< Seconds from throw to landing.
    };
} // namespace ggj

#endif // GGJ_SPIDERSPAWNEVENT_HPP
