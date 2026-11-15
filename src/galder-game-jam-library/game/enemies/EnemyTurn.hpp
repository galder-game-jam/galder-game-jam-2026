#ifndef GALDER_GAME_JAM_2026_PROJECT_ENEMYTURN_HPP
#define GALDER_GAME_JAM_2026_PROJECT_ENEMYTURN_HPP

#include "raylib-cpp.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <string_view>

namespace ggj
{
    /*!
     * Map-trigger commands that steer patrolling enemies. `enemy_turn` reverses the enemy, while the
     * directional commands send it off along one axis, which lets a map lay out patrol loops.
     */
    struct EnemyTurn
    {
        static constexpr std::string_view REVERSE = "enemy_turn";
        static constexpr std::string_view UP = "enemy_turn_up";
        static constexpr std::string_view DOWN = "enemy_turn_down";
        static constexpr std::string_view LEFT = "enemy_turn_left";
        static constexpr std::string_view RIGHT = "enemy_turn_right";

        /*!
         * Gets the unit direction a directional turn command points in.
         *
         * @param command The `userdata_command` of the object the enemy touched
         * @return The unit direction, or nothing if @p command is not a directional turn
         */
        [[nodiscard]] static std::optional<raylib::Vector2> DirectionFromCommand(const std::string_view command)
        {
            if (command == UP)
                return raylib::Vector2 { 0.f, -1.f };
            if (command == DOWN)
                return raylib::Vector2 { 0.f, 1.f };
            if (command == LEFT)
                return raylib::Vector2 { -1.f, 0.f };
            if (command == RIGHT)
                return raylib::Vector2 { 1.f, 0.f };
            return std::nullopt;
        }

        /*!
         * Points a velocity in a new direction while keeping its speed. Enemies only move along
         * one axis, so the speed is whichever of x and y is non-zero (the larger, if both are).
         *
         * @param velocity The enemy's current velocity
         * @param direction Unit direction from DirectionFromCommand()
         * @return The velocity, now pointing along @p direction
         */
        [[nodiscard]] static raylib::Vector2 Redirect(const raylib::Vector2& velocity, const raylib::Vector2& direction)
        {
            const float speed = std::max(std::abs(velocity.x), std::abs(velocity.y));
            return raylib::Vector2 { direction.x * speed, direction.y * speed };
        }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_ENEMYTURN_HPP
