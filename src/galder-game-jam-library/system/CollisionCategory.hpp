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
        static constexpr uint16_t DEFAULT = 0x0001;                     ///< Box2D's default category.
        static constexpr uint16_t TERRAIN = 0x0002;                     ///< Solid static map objects (walls).
        static constexpr uint16_t BOMB = 0x0004;                        ///< King Claude's bombs: collide with terrain only.
        static constexpr uint16_t THROWN = 0x0008;                      ///< Things the butcher throws (cleavers, spiders): sensors that only notice terrain.
        static constexpr uint16_t PROJECTILE = 0x0010;                  ///< Player projectiles (mage balls). Not DEFAULT, so walls can opt out of them.
        static constexpr uint16_t ANY_PROJECTILE = PROJECTILE | THROWN; ///< Everything that flies over a projectile-passthrough wall.

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

        /*!
         * Lets every projectile (player shots and the butcher's cleavers and spiders) fly through
         * @p body, while it stays a wall for everything else. The filter stops Box2D from creating
         * a contact at all, so the projectiles' own wall checks never see the body.
         *
         * @param body A map body with the `allow_projectile_passthrough` property
         */
        static void AllowProjectilePassthrough(b2Body& body)
        {
            for (b2Fixture* fixture = body.GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
            {
                b2Filter filter = fixture->GetFilterData();
                filter.maskBits = static_cast<uint16_t>(filter.maskBits & ~ANY_PROJECTILE);
                fixture->SetFilterData(filter);
            }
        }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_COLLISIONCATEGORY_HPP
