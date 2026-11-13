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
        static constexpr size_t MAX_SLOTS = 4;

        ::Color slots[MAX_SLOTS] {};
    };

    /*!
     * A resolved source -> target colour mapping, ready to be uploaded to the palette swap shader.
     */
    struct PaletteSwap
    {
        static constexpr size_t MAX_COLORS = 16; ///< Must match the array size in palette_swap.fs.

        std::array<::Color, MAX_COLORS> from {};
        std::array<::Color, MAX_COLORS> to {};
        int32_t count { 0 };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_CHARACTERCOLORS_HPP
