//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * Client-side copy of an enemy's latest authoritative state from the host. The client still
     * runs the enemy's own AI, so its local facing drifts from the host's; both position and sprite
     * flip are therefore overwritten from this every frame.
     */
    struct EnemyNetworkState
    {
        raylib::Vector2 position {};
        bool isFlipped { false }; ///< Host's sprite flip (meaning of "flipped" is per enemy sprite).
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_ENEMYNETWORKSTATE_HPP
