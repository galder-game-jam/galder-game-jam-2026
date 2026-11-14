//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_ENEMYDAMAGEEVENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_ENEMYDAMAGEEVENT_HPP

#include <cstdint>

namespace ggj
{
    /*!
     * A client hit a host-authoritative enemy (a boss). The host applies the damage so every peer
     * sees one health pool, instead of each peer only counting its own hits.
     */
    struct EnemyDamageEvent
    {
        uint32_t objectId { 0 };         ///< Network id of the enemy that was hit.
        uint32_t attackerPlayerId { 0 }; ///< Network player id of the client that hit it.
        int32_t damage { 0 };            ///< Lives to remove.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_ENEMYDAMAGEEVENT_HPP
