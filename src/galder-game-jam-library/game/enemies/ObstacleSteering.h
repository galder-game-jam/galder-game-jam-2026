#ifndef GGJ_OBSTACLESTEERING_H
#define GGJ_OBSTACLESTEERING_H

#include "raylib-cpp.hpp"
#include "box2d/box2d.h"

namespace ggj
{
    /*!
     * Lets a big walking enemy go around walls, trees and furniture instead of pushing into them.
     * Casts rays ahead of its body and bends the heading to the nearest free direction. Only
     * static, solid fixtures count as obstacles: players, other enemies and triggers do not.
     */
    class ObstacleSteering
    {
        public:
        static constexpr float LOOKAHEAD = 2.5f;    ///< Metres past the body's edge to scan for obstacles.
        static constexpr float FLANK_FACTOR = 0.9f; ///< Flank rays sit this fraction of the radius off-centre.

        /*!
         * Whether a body would run into something solid when moving along a direction. Casts three
         * parallel rays (centre and both flanks) so the body's width is accounted for, not just its centre line.
         *
         * @param body The moving body; its first fixture's radius gives its width
         * @param direction Unit vector in world space
         * @return True when a non-sensor static fixture lies within the lookahead distance
         */
        [[nodiscard]] static bool IsPathBlocked(b2Body& body, const raylib::Vector2& direction);

        /*!
         * Bends a desired heading around static obstacles. Returns it untouched when the way is clear,
         * otherwise the closest free heading, preferring the side it last went around so it does not
         * dither in front of an obstacle.
         *
         * @param body The moving body
         * @param desired Unit vector towards where it wants to go
         * @return Unit vector to walk along
         */
        [[nodiscard]] raylib::Vector2 steer(b2Body& body, const raylib::Vector2& desired);

        private:
        float m_avoidSide { 1.f }; ///< +1/-1: which side of an obstacle it is currently going around.
    };
} // namespace ggj

#endif // GGJ_OBSTACLESTEERING_H
