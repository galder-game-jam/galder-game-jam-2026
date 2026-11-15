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
        static constexpr uint16_t THROWN = 0x0008;  ///< Things bosses throw (cleavers, crystals): sensors that only notice terrain. Lobbed spiders use it with an empty mask so they fly over walls. Floor decals use it with an empty mask.

        /*!
         * Whether any fixture of @p body is tagged as terrain (a solid wall).
         *
         * @param body Body to inspect
         * @return True for walls; false for triggers such as the enemy turn objects
         */
        [[nodiscard]] static bool IsTerrain(const b2Body& body)
        {
            for (const b2Fixture* fixture = body.GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
            {
                if ((fixture->GetFilterData().categoryBits & TERRAIN) != 0)
                    return true;
            }
            return false;
        }

        /*!
         * Whether @p body currently overlaps a solid wall. Works for sensors too, which is how the
         * butcher's thrown cleavers and spiders notice that they hit something.
         *
         * @param body Body to inspect
         * @return True while any of its contacts is touching a terrain body
         */
        [[nodiscard]] static bool IsTouchingTerrain(const b2Body& body)
        {
            for (const b2ContactEdge* edge = body.GetContactList(); edge != nullptr; edge = edge->next)
            {
                if (edge->contact->IsTouching() && IsTerrain(*edge->other))
                    return true;
            }
            return false;
        }

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
