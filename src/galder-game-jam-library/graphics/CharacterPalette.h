//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_CHARACTERPALETTE_H
#define GALDER_GAME_JAM_2026_CHARACTERPALETTE_H

#include "../data/CharacterColors.hpp"
#include "../enums/PlayerPickedCharacter.h"
#include <random>
#include <vector>

namespace ggj
{
    /*!
     * One customizable part of a character sprite (cloth, scarf, ...). The sprite paints that part
     * with a ramp of exact colours; the user picks a single colour and the rest of the ramp is derived
     * from it so the shading survives the recolour.
     */
    struct PaletteSlot
    {
        const char* label { "" };
        std::vector<::Color> shades {}; ///< Exact sprite colours, lightest ("base") shade first.
    };

    /*!
     * Describes which sprite colours can be customized per character, and turns picked colours into
     * a PaletteSwap for the shader. Pure logic, no GPU state, so it can be unit tested.
     */
    class CharacterPalette
    {
        public:
        /*!
         * @param character Character whose sprite sheet the slots refer to
         * @return The customizable slots, at most CharacterColors::MAX_SLOTS. Empty for None.
         */
        [[nodiscard]] static const std::vector<PaletteSlot>& GetSlots(PlayerPickedCharacter character);

        /*!
         * @param character Character to get defaults for
         * @return Colours that reproduce the unmodified sprite (each slot's base shade).
         */
        [[nodiscard]] static CharacterColors GetDefaultColors(PlayerPickedCharacter character);

        /*!
         * Rolls a random colour for every customizable slot. Colours are picked in HSV with a floor on
         * saturation and brightness, so the result is always vivid instead of muddy grey or near-black.
         *
         * @param character Character whose slots to fill; unused slots stay zeroed
         * @param generator Source of randomness (injected so tests can seed it)
         * @return A random colour per slot, fully opaque
         */
        [[nodiscard]] static CharacterColors GetRandomColors(PlayerPickedCharacter character, std::mt19937& generator);

        /*!
         * Maps every shade of every slot to the picked colour, scaled by how much darker that shade
         * is than the slot's base shade.
         *
         * @param character Character whose slots to use
         * @param colors Picked colour per slot
         * @return The source -> target colour list for the palette swap shader
         */
        [[nodiscard]] static PaletteSwap BuildPaletteSwap(PlayerPickedCharacter character, const CharacterColors& colors);

        /*!
         * Resolves picked colours for the mask recolour shader (used by the painted HUD portraits).
         * Slots left at their default are not recoloured, so the hand-painted colours stay exact.
         *
         * @param character Character whose slots to use
         * @param colors Picked colour per slot
         * @return Base and target colour per slot
         */
        [[nodiscard]] static MaskRecolor BuildMaskRecolor(PlayerPickedCharacter character, const CharacterColors& colors);
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_CHARACTERPALETTE_H
