//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_NETMESSAGETYPE_H
#define GALDER_GAME_JAM_2026_PROJECT_NETMESSAGETYPE_H

#include <cstdint>

namespace ggj
{
    /*!
     * Tag identifying the kind of payload carried by a ClientMessage / ServerMessage envelope.
     * Sent as the first byte of every network message so the receiver knows how to interpret it.
     */
    enum class NetMessageType : uint8_t
    {
        None = 0,
        JoinRequest, ///< Client -> Server: announce name + chosen character on connect.
        JoinAccept,  ///< Server -> Client: assigns the client its authoritative player id.
        PlayerState, ///< Either direction: a single player's transform/animation snapshot.
        PlayerLeft,  ///< Server -> Client: a player disconnected (playerId in the envelope).

        // --- Phase 2 (world-state sync) ---
        WorldSnapshot,
        EnemyState,
        CoinState,
        GameEvent
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETMESSAGETYPE_H
