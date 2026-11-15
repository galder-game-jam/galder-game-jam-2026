#ifndef GGJ_BUTCHERCONFIG_HPP
#define GGJ_BUTCHERCONFIG_HPP

#include <algorithm>
#include <cstdint>

namespace ggj
{
    /*!
     * Tuning for the butcher boss, read from the "butcher" object in the map.
     */
    struct ButcherConfig
    {
        ButcherConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        ButcherConfig(int32_t normal, int32_t enraged, int32_t death, float speed, float spiderFlight)
            : normalCleaverCount { std::max(normal, 0) }, enragedCleaverCount { std::max(enraged, 0) }, deathCleaverCount { std::max(death, 0) },
              cleaverSpeed { std::max(speed, 1.f) }, spiderFlightTime { std::max(spiderFlight, 0.1f) }
        {
        }

        int32_t normalCleaverCount { 10 };  ///< cleaver_count: cleavers thrown in each ring before the enrage.
        int32_t enragedCleaverCount { 16 }; ///< enraged_cleaver_count: cleavers per ring once enraged.
        int32_t deathCleaverCount { 24 };   ///< death_cleaver_count: cleavers thrown as the butcher dies.
        float cleaverSpeed { 220.f };       ///< cleaver_speed: how fast cleavers fly (pixels per second).
        float spiderFlightTime { 0.9f };    ///< spider_flight_time: seconds the spider is in the air; shorter is harder to dodge.
    };
} // namespace ggj

#endif // GGJ_BUTCHERCONFIG_HPP
