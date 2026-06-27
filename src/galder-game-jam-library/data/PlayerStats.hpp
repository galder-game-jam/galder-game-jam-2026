//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PLAYERSTATS_HPP
#define GALDER_GAME_JAM_2026_PLAYERSTATS_HPP

#include "ExperienceConfig.hpp"
#include "../game/ExperienceTracker.hpp"

namespace ggj
{
    /*!
     * All persistent stats that belong to a single player.
     *
     * World owns one entry per player slot so that multiple players can
     * each carry their own independent progression.
     */
    struct PlayerStats
    {
        int lives { 3 };
        int score { 0 };
        ExperienceTracker xpTracker {};
        ExperienceConfig  xpConfig  {};
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PLAYERSTATS_HPP
