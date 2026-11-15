#ifndef GGJ_MAGEBOSSCONFIG_HPP
#define GGJ_MAGEBOSSCONFIG_HPP

#include <algorithm>
#include <cstdint>

namespace ggj
{
    /*!
     * Tuning for the mage boss, read from the "mageboss" object in the map. Every value has a
     * default, so a map only needs to set the ones it wants to change.
     */
    struct MageBossConfig
    {
        MageBossConfig() = default; ///< Non-aggregate so DI (if ever used) keeps the defaults below.

        // Ice circles
        int32_t minIceCircles { 2 };    ///< ice_circle_min_count: fewest ice circles per cast.
        int32_t maxIceCircles { 4 };    ///< ice_circle_max_count: most ice circles per cast.
        float iceCircleRadius { 80.f }; ///< ice_circle_radius: how big each circle is (pixels).
        float iceMinDelay { 0.5f };     ///< ice_min_delay: shortest time from a circle appearing to it freezing (seconds).
        float iceMaxDelay { 1.0f };     ///< ice_max_delay: longest time from a circle appearing to it freezing (seconds).
        float freezeSeconds { 2.f };    ///< freeze_seconds: how long a player caught in a circle stays frozen.

        // Lines of fire
        float fireWarningSeconds { 1.0f };         ///< fire_warning_time: seconds from the lines appearing to them igniting.
        float enragedFireWarningSeconds { 0.75f }; ///< enraged_fire_warning_time: the same once enraged.
        float fireBurnSeconds { 1.4f };            ///< fire_burn_time: how long a line of fire burns once it ignites.
        float fireSpeed { 1800.f };                ///< fire_speed: how fast the flames race along the line (pixels per second).

        // Crystals
        int32_t crystalsPerBurst { 5 };        ///< crystal_count: crystals in each burst.
        int32_t enragedCrystalsPerBurst { 7 }; ///< enraged_crystal_count: crystals in each burst once enraged.
        float crystalSpeed { 260.f };          ///< crystal_speed: how fast the crystals fly (pixels per second).
        float crystalBurstInterval { 0.55f };  ///< crystal_burst_interval: seconds between bursts; faster once enraged.

        /*!
         * Copy with nonsense map values pulled back into a playable range: at least one circle, a
         * maximum no smaller than the minimum, no negative times and no standing-still projectiles.
         *
         * @return The sanitised config; this one is left as it was
         */
        [[nodiscard]] MageBossConfig sanitized() const
        {
            MageBossConfig config = *this;
            config.minIceCircles = std::max(minIceCircles, 1);
            config.maxIceCircles = std::max(maxIceCircles, config.minIceCircles);
            config.iceCircleRadius = std::max(iceCircleRadius, 8.f);
            config.iceMinDelay = std::max(iceMinDelay, 0.f);
            config.iceMaxDelay = std::max(iceMaxDelay, config.iceMinDelay);
            config.freezeSeconds = std::max(freezeSeconds, 0.1f);
            config.fireWarningSeconds = std::max(fireWarningSeconds, 0.f);
            config.enragedFireWarningSeconds = std::max(enragedFireWarningSeconds, 0.f);
            config.fireBurnSeconds = std::max(fireBurnSeconds, 0.1f);
            config.fireSpeed = std::max(fireSpeed, 1.f);
            config.crystalsPerBurst = std::max(crystalsPerBurst, 1);
            config.enragedCrystalsPerBurst = std::max(enragedCrystalsPerBurst, 1);
            config.crystalSpeed = std::max(crystalSpeed, 1.f);
            config.crystalBurstInterval = std::max(crystalBurstInterval, 0.05f);
            return config;
        }
    };
} // namespace ggj

#endif // GGJ_MAGEBOSSCONFIG_HPP
