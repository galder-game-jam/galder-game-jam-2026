#ifndef GGJ_TREANTROOTCONFIG_HPP
#define GGJ_TREANTROOTCONFIG_HPP

#include <algorithm>
#include <cstdint>

namespace ggj
{
    /*!
     * Seconds between a root landing and spiking; each root picks a random time in this range.
     */
    struct SpikeDelayRange
    {
        SpikeDelayRange() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        SpikeDelayRange(float first, float second) : min { std::max(std::min(first, second), 0.f) }, max { std::max(std::max(first, second), 0.f) } {}

        float min { 1.f };
        float max { 2.f };
    };

    /*!
     * How many roots the treant throws in each stage and how long they wait before spiking, read from
     * the "treant" object in the map.
     */
    struct TreantRootConfig
    {
        TreantRootConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        TreantRootConfig(int32_t normal, int32_t enraged, int32_t death, const SpikeDelayRange& delay)
            : normalRootCount { normal > 0 ? normal : 0 }, enragedRootCount { enraged > 0 ? enraged : 0 }, deathRootCount { death > 0 ? death : 0 }, spikeDelay { delay }
        {
        }

        int32_t normalRootCount { 3 };  ///< root_count: roots per throw per player before the enrage, spread around each player.
        int32_t enragedRootCount { 5 }; ///< enraged_root_count: roots per throw per player once enraged.
        int32_t deathRootCount { 60 };  ///< death_root_count: roots spread across the whole map as the treant dies.

        SpikeDelayRange spikeDelay {}; ///< root_min_spike_delay / root_max_spike_delay: used by every stage.
    };
} // namespace ggj

#endif // GGJ_TREANTROOTCONFIG_HPP
