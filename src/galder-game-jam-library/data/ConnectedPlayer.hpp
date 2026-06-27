//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CONNECTEDPLAYER_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CONNECTEDPLAYER_HPP

#include "raylib-cpp.hpp"
#include <cstdint>
#include <string>

namespace ggj
{
    /*!
     * A snapshot of a player as known by the host, for the in-game ServerInfo window.
     * Built on demand from the network manager's authoritative state.
     */
    struct ConnectedPlayer
    {
        uint32_t playerId { 0 };
        std::string name {};
        bool isHost { false };
        raylib::Vector2 position {};
        int32_t lives { 0 };
        int32_t score { 0 };
        int32_t kills { 0 };
        int32_t pingMs { -1 }; ///< -1 = unknown (host has no ping to itself).
        std::string localIp {};  ///< Player's LAN IP.
        std::string publicIp {}; ///< Player's public IP.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CONNECTEDPLAYER_HPP
