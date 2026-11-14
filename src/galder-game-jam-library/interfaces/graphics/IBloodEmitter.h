#ifndef GALDER_GAME_JAM_2026_IBLOODEMITTER_H
#define GALDER_GAME_JAM_2026_IBLOODEMITTER_H

#include "raylib-cpp.hpp"

namespace ggj
{
    constexpr float KILL_BLOOD_STRENGTH = 2.f; ///< Burst strength of a killing blow; an ordinary hit is 1.

    /*!
     * Something that can splash blood into the world. Game objects depend on this instead of the
     * concrete particle system so they can be tested without one.
     */
    class IBloodEmitter
    {
        public:
        virtual ~IBloodEmitter() noexcept = default;

        /*!
         * Splashes blood out of a wound.
         *
         * @param position  Where the wound is, in world pixels
         * @param direction Direction the blow travelled; the spray fans out around it. A zero vector
         *                  sprays evenly in every direction.
         * @param strength  1.0 for an ordinary hit, higher for bigger bursts such as a killing blow
         */
        virtual void emitBlood(const raylib::Vector2& position, const raylib::Vector2& direction, float strength) = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_IBLOODEMITTER_H
