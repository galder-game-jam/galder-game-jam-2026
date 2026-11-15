#include "catch.hpp"
#include "../../src/galder-game-jam-library/game/enemies/TreantRoot.h"

using namespace ggj;

TEST_CASE("TreantRoot - arc is flat at both ends and highest in the middle", "[treant]")
{
    REQUIRE(TreantRoot::ArcFraction(0.f) == Approx(0.f));
    REQUIRE(TreantRoot::ArcFraction(1.f) == Approx(0.f));
    REQUIRE(TreantRoot::ArcFraction(0.5f) == Approx(1.f));
    REQUIRE(TreantRoot::ArcFraction(0.25f) == Approx(TreantRoot::ArcFraction(0.75f)));
}

TEST_CASE("TreantRoot - arc clamps progress outside the flight", "[treant]")
{
    REQUIRE(TreantRoot::ArcFraction(-1.f) == Approx(0.f));
    REQUIRE(TreantRoot::ArcFraction(2.f) == Approx(0.f));
}

TEST_CASE("TreantRoot - spikes shoot up, hold and then sink away", "[treant]")
{
    REQUIRE(TreantRoot::SpikeFraction(0.f) == Approx(0.f));
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_RISE_DURATION / 2.f) == Approx(0.5f));
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_RISE_DURATION) == Approx(1.f));
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_DURATION * 0.5f) == Approx(1.f));
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_DURATION * 0.9f) < 1.f);
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_DURATION) == Approx(0.f));
    REQUIRE(TreantRoot::SpikeFraction(TreantRoot::SPIKE_DURATION + 1.f) == Approx(0.f));
}

namespace
{
    /*! A player 24 px wide whose body centre is at @p x, @p y and whose feet are 16 px lower, like the Rogue. */
    bool isSpiked(const raylib::Vector2& root, float x, float y)
    {
        return TreantRoot::IsInsideSpikeArea(root, { x - 12.f, y }, { x + 12.f, y + 16.f });
    }
} // namespace

TEST_CASE("TreantRoot - spikes hurt a player whose feet are on the root", "[treant]")
{
    const raylib::Vector2 root { 100.f, 100.f };
    REQUIRE(isSpiked(root, 100.f, 90.f)); // Body centre above the root, feet on it.
    REQUIRE(isSpiked(root, 100.f, 100.f));
    REQUIRE(isSpiked(root, 100.f + TreantRoot::HIT_RADIUS + 10.f, 100.f)); // The body's edge reaches into the root from the side.
}

TEST_CASE("TreantRoot - spikes miss a player standing clear of the root", "[treant]")
{
    const raylib::Vector2 root { 100.f, 100.f };
    REQUIRE_FALSE(isSpiked(root, 100.f + TreantRoot::HIT_RADIUS + 13.f, 100.f));
    // Feet well above the root: the lower body ends before the ring starts.
    REQUIRE_FALSE(isSpiked(root, 100.f, 100.f - 16.f - TreantRoot::HIT_RADIUS * TreantRoot::GROUND_SQUASH - 1.f));
    // Body centre below the ring: the lower body starts after it ends.
    REQUIRE_FALSE(isSpiked(root, 100.f, 100.f + TreantRoot::HIT_RADIUS * TreantRoot::GROUND_SQUASH + 1.f));
}
