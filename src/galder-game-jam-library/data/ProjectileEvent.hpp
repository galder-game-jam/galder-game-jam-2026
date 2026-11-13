//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_PROJECTILEEVENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_PROJECTILEEVENT_HPP

#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * A projectile fired or exploding on a remote peer. The World renders these as visual-only
     * projectiles: hits are resolved by the owner, which reports kills through the normal
     * object-destroyed sync.
     */
    struct ProjectileEvent
    {
        uint32_t ownerId { 0 };      ///< Network player id of the shooter.
        uint32_t projectileId { 0 }; ///< Unique per owner.
        bool isImpact { false };     ///< false = fired, true = exploded.
        raylib::Vector2 position {}; ///< Pixels.
        raylib::Vector2 velocity {}; ///< Box2D units (metres/s), as passed to the physics body.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_PROJECTILEEVENT_HPP
