//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_REMOTEPLAYER_HPP
#define GALDER_GAME_JAM_2026_PROJECT_REMOTEPLAYER_HPP

#include "../enums/PlayerPickedCharacter.h"
#include "raylib-cpp.hpp"
#include <cstdint>
#include <string>

namespace ggj
{
    /*!
     * Locally-tracked state for a player other than the local one. Driven by network snapshots and
     * smoothly interpolated toward the latest received position so remote movement looks fluid even
     * though updates arrive at the network tick rate rather than every frame.
     */
    struct RemotePlayer
    {
        uint32_t playerId { 0 };
        std::string name {};
        PlayerPickedCharacter character { PlayerPickedCharacter::Rogue };

        raylib::Vector2 position {};       ///< Rendered (interpolated) position.
        raylib::Vector2 targetPosition {}; ///< Latest authoritative position from the network.
        bool facingLeft { false };
        bool isMoving { false }; ///< Derived from movement, drives walk/idle animation.
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_REMOTEPLAYER_HPP
