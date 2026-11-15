#ifndef GGJ_FROZENBLOCK_HPP
#define GGJ_FROZENBLOCK_HPP

#include "raylib-cpp.hpp"
#include <cmath>

namespace ggj
{
    /*!
     * Draws a translucent block of ice around a frozen character, with a glint sweeping across it.
     * Call while drawing in world space, after the character itself.
     *
     * @param center Middle of the character's body, in world pixels
     * @param time Any steadily increasing time in seconds; drives the glint
     */
    inline void DrawFrozenBlock(const raylib::Vector2& center, float time)
    {
        constexpr float BLOCK_WIDTH = 34.f;
        constexpr float BLOCK_HEIGHT = 46.f;
        constexpr ::Color ICE_FILL { 160, 220, 255, 255 };
        constexpr ::Color ICE_EDGE { 225, 245, 255, 255 };

        const ::Rectangle block { center.x - BLOCK_WIDTH / 2.f, center.y - BLOCK_HEIGHT / 2.f, BLOCK_WIDTH, BLOCK_HEIGHT };
        DrawRectangleRounded(block, 0.3f, 6, Fade(ICE_FILL, 0.45f));
        DrawRectangleRoundedLines(block, 0.3f, 6, Fade(ICE_EDGE, 0.9f));

        // A bright diagonal glint that slides across the block and wraps round.
        const float sweep = std::fmod(time * 0.9f, 1.6f) - 0.3f;
        if (sweep > 0.f && sweep < 1.f)
        {
            const float x = block.x + block.width * sweep;
            DrawLineEx({ x, block.y + 6.f }, { x - 10.f, block.y + block.height - 6.f }, 3.f, Fade(WHITE, 0.55f));
        }
    }
} // namespace ggj

#endif // GGJ_FROZENBLOCK_HPP
