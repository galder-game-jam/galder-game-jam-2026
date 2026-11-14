//
// Created by robin on 13.11.2026.
//

#include "CharacterPalette.h"
#include <algorithm>
#include <cmath>

namespace ggj
{
    namespace
    {
        // Colours sampled from Rogue.png / Wizard.png. The Rogue's face is painted pure black so it
        // stays separate from the near-black (#141412) sprite outline.
        const std::vector<PaletteSlot> ROGUE_SLOTS {
            { "Cloth", { { 0x92, 0x82, 0x30, 255 }, { 0x5f, 0x56, 0x25, 255 }, { 0x37, 0x3a, 0x20, 255 }, { 0x25, 0x27, 0x16, 255 } } },
            { "Scarf", { { 0x93, 0x2d, 0x8b, 255 }, { 0x5f, 0x2e, 0x5c, 255 }, { 0x33, 0x1e, 0x32, 255 } } },
            { "Face", { { 0x00, 0x00, 0x00, 255 } } },
            { "Eyes", { { 0x0c, 0x98, 0xd6, 255 }, { 0x1c, 0x38, 0x45, 255 } } },
        };

        // #dfcdbf is left out of the beard on purpose: the book and staff use it too.
        const std::vector<PaletteSlot> WIZARD_SLOTS {
            { "Clothes", { { 0x93, 0x2d, 0x8b, 255 }, { 0x5f, 0x2e, 0x5c, 255 }, { 0x43, 0x23, 0x41, 255 }, { 0x26, 0x1a, 0x26, 255 } } },
            { "Beard & hair", { { 0xfd, 0xfd, 0xfd, 255 }, { 0xcb, 0xc3, 0xbc, 255 }, { 0x7c, 0x6c, 0x5e, 255 } } },
        };

        const std::vector<PaletteSlot> NO_SLOTS {};

        [[nodiscard]] float luminance(const ::Color& color)
        {
            return 0.299f * static_cast<float>(color.r) + 0.587f * static_cast<float>(color.g) + 0.114f * static_cast<float>(color.b);
        }

        [[nodiscard]] unsigned char scaleChannel(const unsigned char channel, const float factor)
        {
            const long scaled = std::lround(static_cast<float>(channel) * factor);
            return static_cast<unsigned char>(std::clamp(scaled, 0L, 255L));
        }
    } // namespace

    const std::vector<PaletteSlot>& CharacterPalette::GetSlots(const PlayerPickedCharacter character)
    {
        switch (character)
        {
            case PlayerPickedCharacter::Rogue:
                return ROGUE_SLOTS;
            case PlayerPickedCharacter::Wizard:
                return WIZARD_SLOTS;
            default:
                return NO_SLOTS;
        }
    }

    CharacterColors CharacterPalette::GetDefaultColors(const PlayerPickedCharacter character)
    {
        CharacterColors colors {};
        const std::vector<PaletteSlot>& slots = GetSlots(character);
        for (size_t i = 0; i < slots.size() && i < CharacterColors::MAX_SLOTS; ++i)
            colors.slots[i] = slots[i].shades.front();

        return colors;
    }

    PaletteSwap CharacterPalette::BuildPaletteSwap(const PlayerPickedCharacter character, const CharacterColors& colors)
    {
        PaletteSwap swap {};
        const std::vector<PaletteSlot>& slots = GetSlots(character);
        for (size_t i = 0; i < slots.size() && i < CharacterColors::MAX_SLOTS; ++i)
        {
            const ::Color& picked = colors.slots[i];
            const ::Color& base = slots[i].shades.front();
            // The artist's hand-picked ramp only scales to the base colour approximately, so keep it
            // exact while the slot is left at its default.
            const bool isDefault = picked.r == base.r && picked.g == base.g && picked.b == base.b;
            const float baseLuminance = luminance(base);
            for (const ::Color& shade : slots[i].shades)
            {
                if (swap.count >= static_cast<int32_t>(PaletteSwap::MAX_COLORS))
                    return swap;

                const float factor = (baseLuminance > 0.0f) ? luminance(shade) / baseLuminance : 1.0f;
                swap.from[swap.count] = shade;
                swap.to[swap.count] = isDefault ? shade : ::Color { scaleChannel(picked.r, factor), scaleChannel(picked.g, factor), scaleChannel(picked.b, factor), 255 };
                ++swap.count;
            }
        }

        return swap;
    }

    MaskRecolor CharacterPalette::BuildMaskRecolor(const PlayerPickedCharacter character, const CharacterColors& colors)
    {
        MaskRecolor recolor {};
        const std::vector<PaletteSlot>& slots = GetSlots(character);
        for (size_t i = 0; i < slots.size() && i < MaskRecolor::MAX_SLOTS; ++i)
        {
            const ::Color& picked = colors.slots[i];
            const ::Color& base = slots[i].shades.front();
            recolor.base[i] = base;
            recolor.target[i] = picked;
            recolor.isRecolored[i] = picked.r != base.r || picked.g != base.g || picked.b != base.b;
        }

        return recolor;
    }
} // namespace ggj
