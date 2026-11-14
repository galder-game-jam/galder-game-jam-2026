//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CLAUDLINGSPAWNEVENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CLAUDLINGSPAWNEVENT_HPP

#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * A Claudling the host spawned mid-fight. Only the host decides when the boss spawns them, so
     * clients create theirs from these events, under the host's network id, instead of on their own.
     */
    struct ClaudlingSpawnEvent
    {
        uint32_t objectId { 0 };       ///< Network id the host assigned to the Claudling.
        uint32_t targetPlayerId { 0 }; ///< Network player id the Claudling chases.
        raylib::Vector2 position {};   ///< Spawn position in pixels.
        float followSpeed { 0.f };
        float knockback { 0.f };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CLAUDLINGSPAWNEVENT_HPP
