//
// Created by soberga on 12.11.22.
//

#ifndef GAME_DEV_TEMPLATE_OBJECTGENERATORDATA_HPP
#define GAME_DEV_TEMPLATE_OBJECTGENERATORDATA_HPP

#include <string>
#include "raylib-cpp.hpp"
#include "UserData.hpp"
#include "tileson.hpp"

namespace ggj
{

    struct ObjectGeneratorData
    {
        bool isStatic{};
        bool isTrigger{};
        bool isPlayer{};
        bool isKinematic{};
        bool isProjectilePassthroughAllowed{}; ///< Wall that projectiles fly through (`allow_projectile_passthrough`).
        std::string sprite{};
        raylib::Vector2 spriteSize{};
        UserData userData{};
        raylib::Vector2 velocity{};

        int   lives       { 1 };
        float followSpeed { 0.f };
        float followRange { 0.f }; ///< Ghost: pixels within which it starts following a player.
        bool isFacingLeft { false }; ///< Bat: faces left (true) or right (false) whichever way it flies.
        float knockback   { 0.f };
        float idleTime    { 2.5f }; ///< King Claude: base seconds spent in each state before rolling the next.

        bool        isBoss   { false };
        std::string bossName {};

        // King Claude's bomb phase (see ClaudeBombConfig).
        float bombMinTime { 3.f };
        float bombMaxTime { 5.f };
        float bombSpawnIdleStateTime { 3.f };
        int bombSpawnCount { 5 };

        // The treant's root throws (see TreantRootConfig).
        int rootCount { 3 };
        int enragedRootCount { 5 };
        int deathRootCount { 60 };
        float rootMinSpikeDelay { 1.f };
        float rootMaxSpikeDelay { 2.f };

        // The butcher's cleavers and spider (see ButcherConfig).
        int cleaverCount { 10 };
        int enragedCleaverCount { 16 };
        int deathCleaverCount { 24 };
        float cleaverSpeed { 220.f };
        float spiderFlightTime { 0.9f };

        int layerIndex{};
        tson::Object obj{};
        tson::Vector2i pos{};
        tson::Vector2i size{};
        tson::Vector2i originalSize{};
    };

} // ggj

#endif //GAME_DEV_TEMPLATE_OBJECTGENERATORDATA_HPP
