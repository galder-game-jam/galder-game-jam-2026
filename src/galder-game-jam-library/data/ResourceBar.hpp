#ifndef GALDER_GAME_JAM_2026_PROJECT_RESOURCEBAR_HPP
#define GALDER_GAME_JAM_2026_PROJECT_RESOURCEBAR_HPP

#include <algorithm>
#include <cstdint>

namespace ggj
{
    /*!
     * Helpers that turn a raw resource value into the number of HUD bar segments to draw.
     * A resource bar never grows past RESOURCE_BAR_MAX_SEGMENTS; a larger cap makes each segment worth more instead.
     */
    namespace ResourceBar
    {
        constexpr int32_t RESOURCE_BAR_MAX_SEGMENTS = 10; ///< The most segments any HUD resource bar draws.

        /*!
         * Scales a resource so that its cap always fills exactly RESOURCE_BAR_MAX_SEGMENTS segments.
         *
         * @param current Current raw value (clamped into [0, maxValue])
         * @param maxValue Raw value that corresponds to a full bar
         * @return Segments to fill; the fraction is a partly filled last segment. 0 when maxValue is not positive.
         */
        [[nodiscard]] constexpr float scaledSegments(const float current, const float maxValue)
        {
            if (maxValue <= 0.f)
                return 0.f;
            const float ratio = std::clamp(current / maxValue, 0.f, 1.f);
            return ratio * static_cast<float>(RESOURCE_BAR_MAX_SEGMENTS);
        }

        /*!
         * One segment per unit (lives), capped at RESOURCE_BAR_MAX_SEGMENTS.
         *
         * @param count Number of units; negative counts draw nothing
         * @return Segments to fill
         */
        [[nodiscard]] constexpr float cappedSegments(const int32_t count)
        {
            return static_cast<float>(std::clamp(count, 0, RESOURCE_BAR_MAX_SEGMENTS));
        }
    } // namespace ResourceBar
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_RESOURCEBAR_HPP
