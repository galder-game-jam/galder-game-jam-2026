#ifndef GGJ_STUNSTARS_HPP
#define GGJ_STUNSTARS_HPP

#include "raylib-cpp.hpp"
#include <cmath>
#include <numbers>

namespace ggj
{
    /*!
     * Draws a ring of dizzy stars circling above a stunned character's head. Call while drawing in
     * world space.
     *
     * @param head Point just above the character's head, in world pixels
     * @param time Any steadily increasing time in seconds; drives the circling
     */
    inline void DrawStunStars(const raylib::Vector2& head, float time)
    {
        constexpr int32_t STAR_COUNT = 3;
        constexpr float ORBIT_WIDTH = 13.f; ///< Horizontal radius of the orbit (pixels).
        constexpr float ORBIT_HEIGHT = 4.f; ///< Vertical radius: the orbit is seen from a slant.
        constexpr float ORBIT_SPEED = 5.f;  ///< Radians per second.
        constexpr float TWO_PI = 2.f * std::numbers::pi_v<float>;

        for (int32_t i = 0; i < STAR_COUNT; ++i)
        {
            const float angle = time * ORBIT_SPEED + TWO_PI * static_cast<float>(i) / static_cast<float>(STAR_COUNT);
            const raylib::Vector2 position { head.x + std::cos(angle) * ORBIT_WIDTH, head.y + std::sin(angle) * ORBIT_HEIGHT };
            // Stars on the far side of the orbit are drawn smaller and dimmer.
            const bool isFront = std::sin(angle) > 0.f;
            DrawPoly(position, 4, isFront ? 3.5f : 2.5f, time * 180.f, isFront ? ::Color { 255, 235, 90, 255 } : ::Color { 200, 180, 70, 255 });
        }
    }
} // namespace ggj

#endif // GGJ_STUNSTARS_HPP
