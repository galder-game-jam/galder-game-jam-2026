//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_COLLISIONCATEGORY_HPP
#define GALDER_GAME_JAM_2026_PROJECT_COLLISIONCATEGORY_HPP

#include "box2d/box2d.h"
#include <cstdint>

namespace ggj
{
    /*!
     * Box2D collision category bits. Everything keeps Box2D's default category and "collide with
     * everything" mask; the extra bits only let objects that must be picky (bombs) single out walls.
     */
    struct CollisionCategory
    {
        static constexpr uint16_t DEFAULT = 0x0001; ///< Box2D's default category.
        static constexpr uint16_t TERRAIN = 0x0002; ///< Solid static map objects (walls).
        static constexpr uint16_t BOMB = 0x0004;    ///< King Claude's bombs: collide with terrain only.

        /*!
         * Tags every fixture of a solid static map body as terrain, on top of the default
         * category, so it still collides with everything it did before.
         *
         * @param body A static, non-sensor body
         */
        static void MarkAsTerrain(b2Body& body)
        {
            for (b2Fixture* fixture = body.GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
            {
                b2Filter filter = fixture->GetFilterData();
                filter.categoryBits = DEFAULT | TERRAIN;
                fixture->SetFilterData(filter);
            }
        }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_COLLISIONCATEGORY_HPP
