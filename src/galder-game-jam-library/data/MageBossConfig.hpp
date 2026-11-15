#ifndef GGJ_MAGEBOSSCONFIG_HPP
#define GGJ_MAGEBOSSCONFIG_HPP

#include <algorithm>
#include <cstdint>

namespace ggj
{
    /*!
     * Tuning for the mage boss, read from the "mageboss" object in the map.
     */
    struct MageBossConfig
    {
        MageBossConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.
        MageBossConfig(int32_t minIceCircles, int32_t maxIceCircles, float iceCircleRadius, float freezeSeconds, float fireBurnSeconds, float crystalSpeed)
            : minIceCircles { std::max(minIceCircles, 1) }, maxIceCircles { std::max(maxIceCircles, std::max(minIceCircles, 1)) },
              iceCircleRadius { std::max(iceCircleRadius, 8.f) }, freezeSeconds { std::max(freezeSeconds, 0.1f) }, fireBurnSeconds { std::max(fireBurnSeconds, 0.1f) },
              crystalSpeed { std::max(crystalSpeed, 1.f) }
        {
        }

        int32_t minIceCircles { 2 };    ///< ice_circle_min_count: fewest ice circles per cast.
        int32_t maxIceCircles { 4 };    ///< ice_circle_max_count: most ice circles per cast.
        float iceCircleRadius { 80.f }; ///< ice_circle_radius: how big each circle is (pixels).
        float freezeSeconds { 2.f };    ///< freeze_seconds: how long a player caught in a circle stays frozen.
        float fireBurnSeconds { 1.4f }; ///< fire_burn_time: how long a line of fire burns once it ignites.
        float crystalSpeed { 260.f };   ///< crystal_speed: how fast the pink crystals fly (pixels per second).
    };
} // namespace ggj

#endif // GGJ_MAGEBOSSCONFIG_HPP
