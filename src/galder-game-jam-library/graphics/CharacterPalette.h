//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_CHARACTERPALETTE_H
#define GALDER_GAME_JAM_2026_CHARACTERPALETTE_H

#include "../data/CharacterColors.hpp"
#include "../enums/PlayerPickedCharacter.h"
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
         * Maps every shade of every slot to the picked colour, scaled by how much darker that shade
         * is than the slot's base shade.
         *
         * @param character Character whose slots to use
         * @param colors Picked colour per slot
         * @return The source -> target colour list for the palette swap shader
         */
        [[nodiscard]] static PaletteSwap BuildPaletteSwap(PlayerPickedCharacter character, const CharacterColors& colors);
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_CHARACTERPALETTE_H
