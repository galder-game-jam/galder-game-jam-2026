//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PLAYERPERSISTENTSTATE_HPP
#define GALDER_GAME_JAM_2026_PLAYERPERSISTENTSTATE_HPP

#include "TalentState.hpp"

namespace ggj
{
    /*!
     * Snapshot of the stats that carry forward when the player moves to the next level.
     */
    struct PlayerPersistentState
    {
        PlayerPersistentState() = default;

        int lives              { 3 };
        int score              { 0 };
        int level              { 1 };
        int currentXp          { 0 };
        int totalXp            { 0 };
        int totalEnemiesKilled { 0 };
        int talentPoints       { 0 }; ///< Unspent talent points accumulated from level-ups.
        TalentState talentState {};   ///< Talents the player has already unlocked.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PLAYERPERSISTENTSTATE_HPP
