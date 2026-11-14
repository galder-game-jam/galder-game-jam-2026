//
// Created by robin on 13.11.2026.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/graphics/CharacterPalette.h"
#include <algorithm>
#include <random>

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

TEST_CASE("Palette swap - default colours leave the sprite unchanged", "[palette]")
{
    for (const PlayerPickedCharacter character : { PlayerPickedCharacter::Rogue, PlayerPickedCharacter::Wizard })
    {
        const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(character, CharacterPalette::GetDefaultColors(character));
        REQUIRE(swap.count > 0);
        for (int32_t i = 0; i < swap.count; ++i)
            REQUIRE(isSameRgb(swap.from[i], swap.to[i]));
    }
}

TEST_CASE("Palette swap - every shade of every slot is mapped", "[palette]")
{
    REQUIRE(CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Rogue, CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Rogue)).count == 10);
    REQUIRE(CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Wizard, CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard)).count == 16);
    REQUIRE(CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::None, CharacterColors {}).count == 0);
}

TEST_CASE("Palette swap - Rogue face takes the picked colour, outline is untouched", "[palette]")
{
    CharacterColors colors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Rogue);
    colors.slots[2] = ::Color { 200, 10, 30, 255 };

    const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Rogue, colors);
    const int32_t index = findSwap(swap, ::Color { 0x00, 0x00, 0x00, 255 });
    REQUIRE(index >= 0);
    REQUIRE(isSameRgb(swap.to[index], ::Color { 200, 10, 30, 255 }));
    REQUIRE(findSwap(swap, ::Color { 0x14, 0x14, 0x12, 255 }) == -1);
}

TEST_CASE("Palette swap - darker shades are scaled down from the picked colour", "[palette]")
{
    CharacterColors colors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard);
    colors.slots[0] = ::Color { 0, 200, 100, 255 };

    const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Wizard, colors);
    const int32_t baseIndex = findSwap(swap, ::Color { 0x93, 0x2d, 0x8b, 255 });
    const int32_t darkestIndex = findSwap(swap, ::Color { 0x26, 0x1a, 0x26, 255 });
    REQUIRE(baseIndex >= 0);
    REQUIRE(darkestIndex >= 0);

    REQUIRE(isSameRgb(swap.to[baseIndex], ::Color { 0, 200, 100, 255 }));
    const ::Color& darkest = swap.to[darkestIndex];
    REQUIRE(darkest.r == 0);
    REQUIRE(darkest.g < 200);
    REQUIRE(darkest.b < 100);
    REQUIRE(darkest.g > darkest.b);
}

TEST_CASE("Palette swap - changing one slot leaves the others untouched", "[palette]")
{
    CharacterColors colors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard);
    colors.slots[1] = ::Color { 120, 60, 20, 255 };

    const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Wizard, colors);
    const int32_t clothIndex = findSwap(swap, ::Color { 0x5f, 0x2e, 0x5c, 255 });
    const int32_t beardIndex = findSwap(swap, ::Color { 0xfd, 0xfd, 0xfd, 255 });
    REQUIRE(clothIndex >= 0);
    REQUIRE(beardIndex >= 0);
    REQUIRE(isSameRgb(swap.to[clothIndex], swap.from[clothIndex]));
    REQUIRE(isSameRgb(swap.to[beardIndex], ::Color { 120, 60, 20, 255 }));
}

TEST_CASE("Palette swap - Wizard staff tip, wood and pages are separate slots", "[palette]")
{
    CharacterColors colors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard);
    colors.slots[2] = ::Color { 255, 0, 0, 255 };
    colors.slots[3] = ::Color { 0, 255, 0, 255 };

    const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Wizard, colors);
    const int32_t tipIndex = findSwap(swap, ::Color { 0x58, 0xa8, 0xf2, 255 });
    const int32_t woodIndex = findSwap(swap, ::Color { 0xf4, 0xa5, 0x68, 255 });
    const int32_t pagesIndex = findSwap(swap, ::Color { 0xdf, 0xcd, 0xbf, 255 });
    REQUIRE(tipIndex >= 0);
    REQUIRE(woodIndex >= 0);
    REQUIRE(pagesIndex >= 0);
    REQUIRE(isSameRgb(swap.to[tipIndex], ::Color { 255, 0, 0, 255 }));
    REQUIRE(isSameRgb(swap.to[woodIndex], ::Color { 0, 255, 0, 255 }));
    REQUIRE(isSameRgb(swap.to[pagesIndex], swap.from[pagesIndex]));
}

TEST_CASE("Palette swap - no source colour is claimed by two slots", "[palette]")
{
    for (const PlayerPickedCharacter character : { PlayerPickedCharacter::Rogue, PlayerPickedCharacter::Wizard })
    {
        const PaletteSwap swap = CharacterPalette::BuildPaletteSwap(character, CharacterPalette::GetDefaultColors(character));
        for (int32_t i = 0; i < swap.count; ++i)
            for (int32_t j = i + 1; j < swap.count; ++j)
                REQUIRE_FALSE(isSameRgb(swap.from[i], swap.from[j]));
    }
}

TEST_CASE("Mask recolor - default colours leave the portrait as painted", "[palette]")
{
    for (const PlayerPickedCharacter character : { PlayerPickedCharacter::Rogue, PlayerPickedCharacter::Wizard })
    {
        const MaskRecolor recolor = CharacterPalette::BuildMaskRecolor(character, CharacterPalette::GetDefaultColors(character));
        for (const bool isRecolored : recolor.isRecolored)
            REQUIRE_FALSE(isRecolored);
    }
}

TEST_CASE("Mask recolor - only changed slots are recoloured, relative to the sprite's base shade", "[palette]")
{
    CharacterColors colors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard);
    colors.slots[1] = ::Color { 120, 60, 20, 255 };

    const MaskRecolor recolor = CharacterPalette::BuildMaskRecolor(PlayerPickedCharacter::Wizard, colors);
    REQUIRE_FALSE(recolor.isRecolored[0]);
    REQUIRE(recolor.isRecolored[1]);
    REQUIRE(isSameRgb(recolor.base[1], ::Color { 0xfd, 0xfd, 0xfd, 255 }));
    REQUIRE(isSameRgb(recolor.target[1], ::Color { 120, 60, 20, 255 }));
    // Slots beyond the mask's four are sprite-only; the rest stay untouched while left at their default.
    REQUIRE_FALSE(recolor.isRecolored[2]);
    REQUIRE_FALSE(recolor.isRecolored[3]);
}

TEST_CASE("Mask recolor - None has nothing to recolour", "[palette]")
{
    CharacterColors colors {};
    colors.slots[0] = ::Color { 255, 0, 0, 255 };
    const MaskRecolor recolor = CharacterPalette::BuildMaskRecolor(PlayerPickedCharacter::None, colors);
    for (const bool isRecolored : recolor.isRecolored)
        REQUIRE_FALSE(isRecolored);
}

TEST_CASE("Random colours - every slot is filled with a vivid, opaque colour", "[palette]")
{
    std::mt19937 generator { 1234 };
    for (const PlayerPickedCharacter character : { PlayerPickedCharacter::Rogue, PlayerPickedCharacter::Wizard })
    {
        const size_t slotCount = CharacterPalette::GetSlots(character).size();
        for (int32_t attempt = 0; attempt < 50; ++attempt)
        {
            const CharacterColors colors = CharacterPalette::GetRandomColors(character, generator);
            for (size_t i = 0; i < slotCount; ++i)
            {
                const ::Color& color = colors.slots[i];
                const unsigned char brightest = std::max({ color.r, color.g, color.b });
                const unsigned char darkest = std::min({ color.r, color.g, color.b });
                REQUIRE(color.a == 255);
                REQUIRE(brightest >= 140); // value >= 0.55
                REQUIRE(brightest - darkest >= 60); // saturation >= 0.45
            }
        }
    }
}

TEST_CASE("Random colours - the same seed rolls the same colours and different seeds differ", "[palette]")
{
    std::mt19937 first { 7 };
    std::mt19937 second { 7 };
    std::mt19937 other { 8 };

    const CharacterColors a = CharacterPalette::GetRandomColors(PlayerPickedCharacter::Rogue, first);
    const CharacterColors b = CharacterPalette::GetRandomColors(PlayerPickedCharacter::Rogue, second);
    const CharacterColors c = CharacterPalette::GetRandomColors(PlayerPickedCharacter::Rogue, other);

    REQUIRE(isSameRgb(a.slots[0], b.slots[0]));
    REQUIRE_FALSE(isSameRgb(a.slots[0], c.slots[0]));
}
