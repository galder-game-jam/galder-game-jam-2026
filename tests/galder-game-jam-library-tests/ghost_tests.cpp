#include "catch.hpp"
#include "fakeit.hpp"
#include "../../src/galder-game-jam-library/game/enemies/Ghost.h"

#include <map>

using namespace ggj;
using namespace fakeit;

namespace
{
    /*! Fakes a locator whose players stand at the given positions (ids ascending, as World returns them). */
    void FakePlayers(Mock<IPlayerLocator>& locator, const std::map<uint32_t, raylib::Vector2>& positions)
    {
        std::vector<uint32_t> ids {};
        for (const auto& [id, position] : positions)
            ids.push_back(id);

        When(Method(locator, getActivePlayerIds)).AlwaysReturn(ids);
        When(Method(locator, getPlayerPosition)).AlwaysDo([positions](const uint32_t id) -> std::optional<raylib::Vector2>
        {
            const auto found = positions.find(id);
            return found != positions.end() ? std::optional<raylib::Vector2> { found->second } : std::nullopt;
        });
    }
}

TEST_CASE("Ghost - follows nobody while every player is out of range", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 100.f, 0.f } } });

    REQUIRE_FALSE(Ghost::PickTarget(std::nullopt, { 0.f, 0.f }, 64.f, locator.get()).has_value());
}

TEST_CASE("Ghost - picks up a player who comes into range", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 200.f, 0.f } }, { 2, { 30.f, 40.f } } });

    REQUIRE(Ghost::PickTarget(std::nullopt, { 0.f, 0.f }, 64.f, locator.get()) == 2u);
}

TEST_CASE("Ghost - range is inclusive", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 64.f, 0.f } } });

    REQUIRE(Ghost::PickTarget(std::nullopt, { 0.f, 0.f }, 64.f, locator.get()) == 1u);
}

TEST_CASE("Ghost - keeps its target while another player is also in range", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 10.f, 0.f } }, { 2, { 20.f, 0.f } } });

    REQUIRE(Ghost::PickTarget(2u, { 0.f, 0.f }, 64.f, locator.get()) == 2u);
}

TEST_CASE("Ghost - lets go of a target that leaves the range", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 100.f, 0.f } } });

    REQUIRE_FALSE(Ghost::PickTarget(1u, { 0.f, 0.f }, 64.f, locator.get()).has_value());
}

TEST_CASE("Ghost - switches to another player in range once the target escapes", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 100.f, 0.f } }, { 2, { 10.f, 0.f } } });

    REQUIRE(Ghost::PickTarget(1u, { 0.f, 0.f }, 64.f, locator.get()) == 2u);
}

TEST_CASE("Ghost - a target who left the session is dropped", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, {});

    REQUIRE_FALSE(Ghost::PickTarget(3u, { 0.f, 0.f }, 64.f, locator.get()).has_value());
}

TEST_CASE("Ghost - zero range never follows", "[ghost]")
{
    Mock<IPlayerLocator> locator;
    FakePlayers(locator, { { 1, { 0.f, 0.f } } });

    REQUIRE_FALSE(Ghost::PickTarget(std::nullopt, { 0.f, 0.f }, 0.f, locator.get()).has_value());
}
