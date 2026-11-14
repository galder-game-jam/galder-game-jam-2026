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
        GameEvent,
        ProjectileFired,  ///< Either direction: a player fired a projectile (objectId = projectile id).
        ProjectileImpact, ///< Either direction: that projectile exploded at position.

        // --- Shared progression ---
        PortalEntered, ///< Either direction: a player walked into the portal of stageIndex; everyone follows.
        StageChanged,  ///< Server -> Client: the host moved on to stageIndex; clients switch to it.

        // --- Host-authoritative bosses ---
        ClaudlingSpawned, ///< Server -> Client: the boss spawned a Claudling (objectId, playerId = target).
        EnemyDamaged,     ///< Client -> Server: the client hit a boss (objectId, damage); the host applies it.
        BombSpawned,      ///< Server -> Client: the boss threw a bomb (position, velocity, fuseTime).
        RootSpawned,       ///< Server -> Client: the treant threw a root (position = origin, target, flightTime, spikeDelay).
        Chat ///< Either direction: a chat line. Carried by ChatMessage, not the Client/ServerMessage envelopes.
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETMESSAGETYPE_H
