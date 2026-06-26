//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_EXPERIENCECONFIG_HPP
#define GALDER_GAME_JAM_2026_EXPERIENCECONFIG_HPP

namespace ggj
{
    /*!
     * Configurable values for the XP and levelling system.
     *
     * The user-declared constructor is required so that Boost.DI constructs this
     * as a non-aggregate and honours the member initializers (see CLAUDE.md).
     */
    struct ExperienceConfig
    {
        ExperienceConfig() = default;

        int xpPerKillMin   { 10  }; ///< Minimum XP awarded per enemy kill.
        int xpPerKillMax   { 20  }; ///< Maximum XP awarded per enemy kill.
        int xpPerPortal    { 20  }; ///< XP awarded when the player enters a portal.
        int xpPerCoin      { 2   }; ///< XP awarded when the player picks up a coin.
        int baseXpPerLevel { 20 }; ///< Base factor used in the level-up threshold formula.
        int maxLevel       { 40  }; ///< Maximum player level.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_EXPERIENCECONFIG_HPP
