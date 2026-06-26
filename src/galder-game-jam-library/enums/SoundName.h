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
        PlayerAttack = 4,  ///< Played when the player triggers a melee attack.
        DialogMumble = 5,   ///< Short mumble played when the selected dialog choice changes.
        DialogTalking1 = 6, ///< Talking syllable — A3, deep.
        DialogTalking2 = 7, ///< Talking syllable — D4, warm baritone.
        DialogTalking3 = 8, ///< Talking syllable — F#4, mid-low.
        DialogTalking4 = 9, ///< Talking syllable — A4, neutral.
        DialogTalking5 = 10, ///< Talking syllable — C#5, mid-high.
        DialogTalking6 = 11, ///< Talking syllable — F5, bright.
        DialogTalking7 = 12  ///< Talking syllable — A5, high.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_SOUNDNAME_H
