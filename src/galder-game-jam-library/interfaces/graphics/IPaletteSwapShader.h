//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_IPALETTESWAPSHADER_H
#define GALDER_GAME_JAM_2026_IPALETTESWAPSHADER_H

#include "../../data/CharacterColors.hpp"

namespace ggj
{
    /*!
     * Shader that swaps exact sprite colours for other colours while drawing. Used to recolour the
     * player characters with the colours picked on the character select screen.
     */
    class IPaletteSwapShader
    {
        public:
        virtual ~IPaletteSwapShader() noexcept = default;

        /*!
         * Loads the shader. Needs an active GL context, so call it after the window is created.
         *
         * @return false if the shader could not be loaded; begin()/end() then do nothing
         */
        virtual bool initialize() = 0;

        /*!
         * Starts drawing with the given colour swap. Everything drawn until end() is recoloured.
         *
         * @param swap Source -> target colours to apply
         */
        virtual void begin(const PaletteSwap& swap) = 0;

        /*!
         * Stops recolouring; subsequent draws use the default shader again.
         */
        virtual void end() = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_IPALETTESWAPSHADER_H
