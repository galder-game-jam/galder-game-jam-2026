//
// Created by robin on 13.11.2026.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/graphics/CharacterPalette.h"

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
    REQUIRE(CharacterPalette::BuildPaletteSwap(PlayerPickedCharacter::Wizard, CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard)).count == 7);
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
