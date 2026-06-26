#ifndef GALDER_GAME_JAM_2026_SOUNDNAME_H
#define GALDER_GAME_JAM_2026_SOUNDNAME_H

#include <cstdint>

namespace ggj
{
    enum class SoundName : uint16_t
    {
        None = 0,
        CoinPickup = 1,  ///< Played when the player collects a coin.
        EnemyDie = 2,    ///< Played when any enemy is destroyed.
        PortalUse = 3,   ///< Played when a player steps through a portal.
        PlayerAttack = 4 ///< Played when the player triggers a melee attack.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_SOUNDNAME_H
