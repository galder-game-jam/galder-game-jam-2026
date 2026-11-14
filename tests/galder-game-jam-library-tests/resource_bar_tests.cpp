//
// HUD resource bar scaling tests.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/data/ResourceBar.hpp"

using namespace ggj;

TEST_CASE("A full resource always fills exactly ten segments", "[resource_bar]")
{
    REQUIRE(ResourceBar::scaledSegments(200.f, 200.f) == Approx(10.f));
    REQUIRE(ResourceBar::scaledSegments(250.f, 250.f) == Approx(10.f));
    REQUIRE(ResourceBar::scaledSegments(150.f, 150.f) == Approx(10.f));
}

TEST_CASE("Segments scale with the resource cap", "[resource_bar]")
{
    // 100 of 200 is half a bar; a raised cap of 300 makes the same 100 worth less.
    REQUIRE(ResourceBar::scaledSegments(100.f, 200.f) == Approx(5.f));
    REQUIRE(ResourceBar::scaledSegments(100.f, 300.f) == Approx(10.f / 3.f));
}

TEST_CASE("Scaled segments are clamped and safe for a missing cap", "[resource_bar]")
{
    REQUIRE(ResourceBar::scaledSegments(500.f, 100.f) == Approx(10.f));
    REQUIRE(ResourceBar::scaledSegments(-5.f, 100.f) == Approx(0.f));
    REQUIRE(ResourceBar::scaledSegments(50.f, 0.f) == Approx(0.f));
}

TEST_CASE("Capped segments never exceed ten", "[resource_bar]")
{
    REQUIRE(ResourceBar::cappedSegments(3) == Approx(3.f));
    REQUIRE(ResourceBar::cappedSegments(10) == Approx(10.f));
    REQUIRE(ResourceBar::cappedSegments(25) == Approx(10.f));
    REQUIRE(ResourceBar::cappedSegments(-1) == Approx(0.f));
}
