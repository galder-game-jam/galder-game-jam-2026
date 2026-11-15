#include "catch.hpp"
#include "../../src/galder-game-jam-library/game/enemies/EnemyTurn.hpp"

using namespace ggj;

TEST_CASE("EnemyTurn - directional commands map to unit directions", "[enemy_turn]")
{
    REQUIRE(EnemyTurn::DirectionFromCommand("enemy_turn_up") == raylib::Vector2 { 0.f, -1.f });
    REQUIRE(EnemyTurn::DirectionFromCommand("enemy_turn_down") == raylib::Vector2 { 0.f, 1.f });
    REQUIRE(EnemyTurn::DirectionFromCommand("enemy_turn_left") == raylib::Vector2 { -1.f, 0.f });
    REQUIRE(EnemyTurn::DirectionFromCommand("enemy_turn_right") == raylib::Vector2 { 1.f, 0.f });
}

TEST_CASE("EnemyTurn - reverse and unrelated commands are not directional", "[enemy_turn]")
{
    REQUIRE_FALSE(EnemyTurn::DirectionFromCommand("enemy_turn").has_value());
    REQUIRE_FALSE(EnemyTurn::DirectionFromCommand("").has_value());
    REQUIRE_FALSE(EnemyTurn::DirectionFromCommand("pickup_coin").has_value());
}

TEST_CASE("EnemyTurn - redirect keeps the speed of whichever axis is moving", "[enemy_turn]")
{
    const raylib::Vector2 up { 0.f, -1.f };
    const raylib::Vector2 left { -1.f, 0.f };

    const raylib::Vector2 fromRight = EnemyTurn::Redirect({ 1.5f, 0.f }, up);
    REQUIRE(fromRight.x == Approx(0.f));
    REQUIRE(fromRight.y == Approx(-1.5f));

    const raylib::Vector2 fromUp = EnemyTurn::Redirect({ 0.f, -1.5f }, left);
    REQUIRE(fromUp.x == Approx(-1.5f));
    REQUIRE(fromUp.y == Approx(0.f));

    const raylib::Vector2 fromDown = EnemyTurn::Redirect({ 0.f, 2.f }, up);
    REQUIRE(fromDown.y == Approx(-2.f));
}

TEST_CASE("EnemyTurn - redirecting a stationary enemy leaves it stationary", "[enemy_turn]")
{
    const raylib::Vector2 result = EnemyTurn::Redirect({ 0.f, 0.f }, { 1.f, 0.f });
    REQUIRE(result.x == Approx(0.f));
    REQUIRE(result.y == Approx(0.f));
}
