#ifndef GALDER_GAME_JAM_2026_PROJECT_NETWORKPLAYERSTATS_HPP
#define GALDER_GAME_JAM_2026_PROJECT_NETWORKPLAYERSTATS_HPP

#include <cstdint>

namespace ggj
{
    /*!
     * The local player's stats that are synced to every peer, so they can show it in ServerInfo
     * and in the party bars of the HUD.
     */
    struct NetworkPlayerStats
    {
        int32_t lives { 0 };
        int32_t score { 0 };
        int32_t kills { 0 };
        int32_t level { 1 };
        float mana { 0.f };      ///< Raw mana (not bars); 0 together with maxMana for characters without mana.
        float maxMana { 0.f };
        float energy { 0.f };    ///< Raw energy (not bars); 0 together with maxEnergy for characters without energy.
        float maxEnergy { 0.f };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_NETWORKPLAYERSTATS_HPP
