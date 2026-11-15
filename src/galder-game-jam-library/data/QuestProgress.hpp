#ifndef GALDER_GAME_JAM_2026_PROJECT_QUESTPROGRESS_HPP
#define GALDER_GAME_JAM_2026_PROJECT_QUESTPROGRESS_HPP

#include <cstdint>

namespace ggj
{
    /*!
     * A stage's quest: which goals the map requires (the `require_all_monsters_killed` and
     * `require_all_coins_collected` map properties) and how far the players have come. The portal
     * opens once every required goal is done.
     */
    struct QuestProgress
    {
        bool isAllEnemiesRequired { true }; ///< Map property require_all_monsters_killed (default true).
        bool isAllCoinsRequired { false };  ///< Map property require_all_coins_collected (default false).
        int32_t enemiesTotal { 0 };
        int32_t enemiesLeft { 0 };
        int32_t coinsTotal { 0 };
        int32_t coinsLeft { 0 };

        [[nodiscard]] constexpr int32_t enemiesKilled() const { return enemiesTotal - enemiesLeft; }

        [[nodiscard]] constexpr int32_t coinsCollected() const { return coinsTotal - coinsLeft; }

        /*!
         * @return True when every goal the map requires is done; a map that requires nothing is complete.
         */
        [[nodiscard]] constexpr bool isComplete() const { return (!isAllEnemiesRequired || enemiesLeft <= 0) && (!isAllCoinsRequired || coinsLeft <= 0); }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_QUESTPROGRESS_HPP
