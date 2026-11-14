//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_CHARACTERCOLORS_HPP
#define GALDER_GAME_JAM_2026_CHARACTERCOLORS_HPP

#include "raylib-cpp.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ggj
{
    /*!
     * The colours a player picked for their character, one per customizable palette slot (see
     * CharacterPalette for what each slot means per character). Unused slots are ignored.
     *
     * @note Trivially copyable POD: it is embedded in ClientMessage/ServerMessage and sent as raw bytes.
     */
    struct CharacterColors
    {
        static constexpr size_t MAX_SLOTS = 6;

        ::Color slots[MAX_SLOTS] {};
    };

    /*!
     * A resolved source -> target colour mapping, ready to be uploaded to the palette swap shader.
     */
    struct PaletteSwap
    {
        static constexpr size_t MAX_COLORS = 24; ///< Must match the array size in palette_swap.fs.

        std::array<::Color, MAX_COLORS> from {};
        std::array<::Color, MAX_COLORS> to {};
        int32_t count { 0 };
    };

    /*!
     * Per-slot colours for the mask recolour shader, which recolours painted art (the HUD portraits)
     * where a slot mask marks each customizable part instead of exact sprite colours.
     */
    struct MaskRecolor
    {
        static constexpr size_t MAX_SLOTS = 4; ///< Must match MAX_SLOTS in mask_recolor.fs. Portrait masks only encode four slots, so slots beyond these are sprite-only.

        std::array<::Color, MAX_SLOTS> base {};     ///< The sprite's base shade; a texel this bright gets exactly the target colour.
        std::array<::Color, MAX_SLOTS> target {};   ///< The picked colour.
        std::array<bool, MAX_SLOTS> isRecolored {}; ///< false keeps the slot as painted (unused or left at its default).
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_CHARACTERCOLORS_HPP
