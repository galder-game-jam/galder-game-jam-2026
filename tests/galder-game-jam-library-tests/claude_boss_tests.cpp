//
// Created by robin on 14.11.2026.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/game/enemies/Bomb.h"
#include "../../src/galder-game-jam-library/game/enemies/ClaudeEnemy.h"

using namespace ggj;

namespace
{
    [[nodiscard]] bool isSameRgb(const ::Color& lhs, const ::Color& rhs)
    {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
    }

    [[nodiscard]] int32_t findSwap(const PaletteSwap& swap, const ::Color& from)
    {
        for (int32_t i = 0; i < swap.count; ++i)
        {
            if (isSameRgb(swap.from[i], from))
                return i;
        }
        return -1;
    }
} // namespace

TEST_CASE("Bomb - fuse frame advances by thirds of the fuse", "[bomb]")
{
    constexpr float FUSE = 3.f;
    REQUIRE(Bomb::FuseFrameIndex(0.f, FUSE) == 0);
    REQUIRE(Bomb::FuseFrameIndex(0.99f, FUSE) == 0);
    REQUIRE(Bomb::FuseFrameIndex(1.f, FUSE) == 1);
    REQUIRE(Bomb::FuseFrameIndex(1.99f, FUSE) == 1);
    REQUIRE(Bomb::FuseFrameIndex(2.f, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(FUSE, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(10.f, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(-1.f, FUSE) == 0);
}

TEST_CASE("Bomb - tint goes from white to strongly red", "[bomb]")
{
    const ::Color start = Bomb::FuseTint(0.f);
    REQUIRE(start.r == 255);
    REQUIRE(start.g == 255);
    REQUIRE(start.b == 255);

    const ::Color middle = Bomb::FuseTint(0.5f);
    const ::Color end = Bomb::FuseTint(1.f);
    REQUIRE(end.r == 255);
    REQUIRE(end.g < middle.g);
    REQUIRE(middle.g < start.g);
    REQUIRE(end.g <= 60);
    REQUIRE(end.g == end.b);
    REQUIRE(isSameRgb(Bomb::FuseTint(2.f), end));
}

TEST_CASE("Bomb - explosion walks the 4x4 frame grid", "[bomb]")
{
    const raylib::Rectangle first = Bomb::ExplosionFrameRect(0.f);
    REQUIRE(first.x == 0.f);
    REQUIRE(first.y == 0.f);
    REQUIRE(first.width == Bomb::FRAME_SIZE);
    REQUIRE(first.height == Bomb::FRAME_SIZE);

    // Frame 5 is the second column of the second row.
    const raylib::Rectangle fifth = Bomb::ExplosionFrameRect(Bomb::EXPLOSION_FRAME_DURATION * 5.5f);
    REQUIRE(fifth.x == Bomb::FRAME_SIZE);
    REQUIRE(fifth.y == Bomb::FRAME_SIZE);

    // Past the end it holds the last frame (bottom-right).
    const raylib::Rectangle last = Bomb::ExplosionFrameRect(100.f);
    REQUIRE(last.x == Bomb::FRAME_SIZE * 3.f);
    REQUIRE(last.y == Bomb::FRAME_SIZE * 3.f);
}

TEST_CASE("Claude enrage - no swap before the transform starts", "[claude]")
{
    REQUIRE(ClaudeEnemy::BuildEnragePaletteSwap(0.f).count == 0);
    REQUIRE(ClaudeEnemy::BuildEnragePaletteSwap(-1.f).count == 0);
}

TEST_CASE("Claude enrage - body turns black and eyes turn red", "[claude]")
{
    const PaletteSwap full = ClaudeEnemy::BuildEnragePaletteSwap(1.f);
    const int32_t body = findSwap(full, ClaudeEnemy::BODY_COLOR);
    const int32_t eyes = findSwap(full, ClaudeEnemy::EYE_COLOR);
    REQUIRE(body >= 0);
    REQUIRE(eyes >= 0);
    REQUIRE(isSameRgb(full.to[body], ClaudeEnemy::ENRAGED_BODY_COLOR));
    REQUIRE(isSameRgb(full.to[eyes], ClaudeEnemy::ENRAGED_EYE_COLOR));

    // Halfway through, both colours are partway between the two ends.
    const PaletteSwap half = ClaudeEnemy::BuildEnragePaletteSwap(0.5f);
    REQUIRE(half.to[body].r < ClaudeEnemy::BODY_COLOR.r);
    REQUIRE(half.to[body].r > ClaudeEnemy::ENRAGED_BODY_COLOR.r);
    REQUIRE(half.to[eyes].r > ClaudeEnemy::EYE_COLOR.r);
    REQUIRE(half.to[eyes].r < ClaudeEnemy::ENRAGED_EYE_COLOR.r);
}
