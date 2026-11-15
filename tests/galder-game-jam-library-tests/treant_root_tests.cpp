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

TEST_CASE("TreantRoot - spikes hurt a player whose feet are on the root", "[treant]")
{
    const raylib::Vector2 root { 100.f, 100.f };
    REQUIRE(TreantRoot::IsInsideSpikeArea(root, root));
    REQUIRE(TreantRoot::IsInsideSpikeArea(root, { 100.f + TreantRoot::HIT_RADIUS - 1.f, 100.f }));
    REQUIRE(TreantRoot::IsInsideSpikeArea(root, { 100.f, 100.f - TreantRoot::HIT_RADIUS * TreantRoot::GROUND_SQUASH + 1.f }));
}

TEST_CASE("TreantRoot - spikes miss a player standing beside the root", "[treant]")
{
    const raylib::Vector2 root { 100.f, 100.f };
    REQUIRE_FALSE(TreantRoot::IsInsideSpikeArea(root, { 100.f + TreantRoot::HIT_RADIUS + 1.f, 100.f }));
    // The ground area is flat: the same distance above or below the root is already outside.
    REQUIRE_FALSE(TreantRoot::IsInsideSpikeArea(root, { 100.f, 100.f + TreantRoot::HIT_RADIUS }));
    REQUIRE_FALSE(TreantRoot::IsInsideSpikeArea(root, { 100.f, 100.f - TreantRoot::HIT_RADIUS }));
}
