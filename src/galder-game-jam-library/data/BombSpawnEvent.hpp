//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_BOMBSPAWNEVENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_BOMBSPAWNEVENT_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * A bomb King Claude threw on the host. Clients create their own copy from this and simulate
     * it locally; each peer then only checks the blast against its own player.
     */
    struct BombSpawnEvent
    {
        raylib::Vector2 position {}; ///< Spawn position in pixels.
        raylib::Vector2 velocity {}; ///< Launch velocity in Box2D units (metres/s).
        float fuseTime { 0.f };      ///< Seconds until it explodes.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_BOMBSPAWNEVENT_HPP
