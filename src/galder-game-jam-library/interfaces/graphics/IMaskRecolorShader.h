//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_IMASKRECOLORSHADER_H
#define GALDER_GAME_JAM_2026_IMASKRECOLORSHADER_H

#include "../../data/CharacterColors.hpp"

namespace ggj
{
    /*!
     * Shader that recolours the parts of a texture marked in a slot mask, keeping the painted shading.
     * Used for the HUD portraits, whose many hand-painted shades rule out an exact palette swap.
     */
    class IMaskRecolorShader
    {
        public:
        virtual ~IMaskRecolorShader() noexcept = default;

        /*!
         * Loads the shader. Needs an active GL context, so call it after the window is created.
         *
         * @return false if the shader could not be loaded; begin()/end() then do nothing
         */
        virtual bool initialize() = 0;

        /*!
         * Starts drawing with the given recolour. Everything drawn until end() is recoloured, so only
         * draw textures that share the mask's layout (same size, drawn whole).
         *
         * @param recolor Colours per slot
         * @param mask Slot mask: red/green/blue/yellow = slot 0-3, transparent = untouched
         */
        virtual void begin(const MaskRecolor& recolor, const ::Texture2D& mask) = 0;

        /*!
         * Stops recolouring; subsequent draws use the default shader again.
         */
        virtual void end() = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_IMASKRECOLORSHADER_H
