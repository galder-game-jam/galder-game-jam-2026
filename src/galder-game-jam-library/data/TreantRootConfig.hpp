#ifndef GGJ_TREANTROOTCONFIG_HPP
#define GGJ_TREANTROOTCONFIG_HPP

#include <cstdint>

namespace ggj
{
    /*!
     * How many roots the treant throws in each stage, read from the "treant" object in the map.
     */
    struct TreantRootConfig
    {
        TreantRootConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        TreantRootConfig(int32_t normal, int32_t enraged, int32_t death)
            : normalRootCount { normal > 0 ? normal : 0 }, enragedRootCount { enraged > 0 ? enraged : 0 }, deathRootCount { death > 0 ? death : 0 }
        {
        }

        int32_t normalRootCount { 3 };  ///< root_count: roots per throw per player before the enrage, spread around each player.
        int32_t enragedRootCount { 5 }; ///< enraged_root_count: roots per throw per player once enraged.
        int32_t deathRootCount { 60 };  ///< death_root_count: roots spread across the whole map as the treant dies.
    };
} // namespace ggj

#endif // GGJ_TREANTROOTCONFIG_HPP
