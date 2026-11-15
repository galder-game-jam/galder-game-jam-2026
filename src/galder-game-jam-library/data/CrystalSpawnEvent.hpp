#ifndef GGJ_CRYSTALSPAWNEVENT_HPP
#define GGJ_CRYSTALSPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A pink crystal the mage shot on the host. Clients create their own copy from this and
     * simulate it locally; each peer then only hurts its own player.
     */
    struct CrystalSpawnEvent
    {
        raylib::Vector2 position {}; ///< Where it starts (pixels).
        raylib::Vector2 velocity {}; ///< Launch velocity in Box2D units (metres per second).
    };
} // namespace ggj

#endif // GGJ_CRYSTALSPAWNEVENT_HPP
