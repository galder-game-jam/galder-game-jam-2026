//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP

#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * An enemy's authoritative state, published by the host and applied on every client. The client
     * still runs the enemy's own AI between updates, so everything that AI decides on its own
     * (position, facing, and for bosses state/target/lives) is overwritten from this every frame.
     */
    struct EnemyNetworkState
    {
        raylib::Vector2 position {};
        bool isFlipped { false };      ///< Host's sprite flip (meaning of "flipped" is per enemy sprite).
        int32_t lives { 0 };           ///< Host's remaining lives; only bosses apply it.
        uint8_t aiState { 0 };         ///< Enemy-specific AI state (e.g. ClaudeState); 0 when unused.
        uint32_t targetPlayerId { 0 }; ///< Network player id the enemy is chasing; unused by most enemies.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP
