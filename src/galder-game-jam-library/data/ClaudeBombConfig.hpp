//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CLAUDEBOMBCONFIG_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CLAUDEBOMBCONFIG_HPP

namespace ggj
{
    /*!
     * Tuning for King Claude's bomb phase, read from the "claude" object in the map.
     */
    struct ClaudeBombConfig
    {
        ClaudeBombConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        ClaudeBombConfig(float minFuse, float maxFuse, float launchWindup)
            : minFuseTime { minFuse < maxFuse ? minFuse : maxFuse }, maxFuseTime { minFuse < maxFuse ? maxFuse : minFuse }, launchWindupTime { launchWindup }
        {
        }

        float minFuseTime { 3.f };      ///< bomb_min_time: shortest fuse of a thrown bomb (seconds).
        float maxFuseTime { 5.f };      ///< bomb_max_time: longest fuse of a thrown bomb (seconds).
        float launchWindupTime { 3.f }; ///< bomb_spawn_idle_state_time: shaking before each bomb throw (seconds).
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CLAUDEBOMBCONFIG_HPP
