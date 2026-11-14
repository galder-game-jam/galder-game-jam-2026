#ifndef GGJ_ITINTSHADER_H
#define GGJ_ITINTSHADER_H

#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * Shader that recolours a sprite towards one tint colour while it is drawn, keeping the sprite's
     * shading. Used for effects such as the treant turning purple when it enrages.
     */
    class ITintShader
    {
        public:
        virtual ~ITintShader() noexcept = default;

        /*!
         * Loads the shader. Needs an active GL context, so call it after the window is created.
         *
         * @return false if the shader could not be loaded; begin()/end() then do nothing
         */
        virtual bool initialize() = 0;

        /*!
         * Starts drawing with the tint. Everything drawn until end() is recoloured.
         *
         * @param tint     Colour the sprite is pulled towards
         * @param strength 0 = unchanged, 1 = fully tinted
         */
        virtual void begin(const ::Color& tint, float strength) = 0;

        /*! Stops tinting; subsequent draws use the default shader again. */
        virtual void end() = 0;
    };
} // namespace ggj

#endif // GGJ_ITINTSHADER_H
