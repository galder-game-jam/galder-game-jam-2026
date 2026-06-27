//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_DISCOVEREDSERVER_HPP
#define GALDER_GAME_JAM_2026_PROJECT_DISCOVEREDSERVER_HPP

#include <cstdint>
#include <string>

namespace ggj
{
    /*!
     * A host found on the local network via the LAN discovery beacon. The IP/port can be fed
     * straight into a client connect.
     */
    struct DiscoveredServer
    {
        std::string ipAddress {};
        uint16_t port { 0 };
        std::string name {};
        int32_t playerCount { 0 };
        float secondsSinceSeen { 0.0f }; ///< Used to expire stale entries.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_DISCOVEREDSERVER_HPP
