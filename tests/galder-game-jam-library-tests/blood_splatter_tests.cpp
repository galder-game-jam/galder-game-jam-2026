//
// Blood splatter amount and lifetime tests.
//

#include "catch.hpp"
#include "fakeit.hpp"
#include "../../src/galder-game-jam-library/graphics/BloodSplatter.h"

using namespace ggj;
using namespace fakeit;

namespace
{
    /*! Fake options manager whose only meaningful answer is the blood amount. */
    struct BloodTestRig
    {
        explicit BloodTestRig(int32_t bloodAmount)
        {
            options.bloodAmount = bloodAmount;
            When(Method(fakeOptions, getGraphicsOptions)).AlwaysReturn(options);
        }

        GraphicsOptions options {};
        Mock<IOptionsManager> fakeOptions;
    };
} // namespace

TEST_CASE("Blood defaults to gorefest", "[blood]")
{
    REQUIRE(GraphicsOptions {}.bloodAmount == 10);
}

TEST_CASE("More blood setting makes more droplets", "[blood]")
{
    for (int32_t amount = GraphicsOptions::MIN_BLOOD_AMOUNT; amount < GraphicsOptions::MAX_BLOOD_AMOUNT; ++amount)
        REQUIRE(BloodSplatter::dropletCountFor(amount, 1.f) < BloodSplatter::dropletCountFor(amount + 1, 1.f));
}

TEST_CASE("Droplet count is clamped for out of range settings and extreme strength", "[blood]")
{
    REQUIRE(BloodSplatter::dropletCountFor(-5, 1.f) == BloodSplatter::dropletCountFor(1, 1.f));
    REQUIRE(BloodSplatter::dropletCountFor(99, 1.f) == BloodSplatter::dropletCountFor(10, 1.f));
    REQUIRE(BloodSplatter::dropletCountFor(1, 0.f) >= 1);
    REQUIRE(BloodSplatter::dropletCountFor(10, 1000.f) == BloodSplatter::MAX_DROPLETS_PER_BURST);
}

TEST_CASE("A killing blow bleeds more than an ordinary hit", "[blood]")
{
    REQUIRE(BloodSplatter::dropletCountFor(5, KILL_BLOOD_STRENGTH) > BloodSplatter::dropletCountFor(5, 1.f));
}

TEST_CASE("Droplets land as stains that eventually fade away", "[blood]")
{
    BloodTestRig rig { 10 };
    BloodSplatter blood { rig.fakeOptions.get() };

    blood.emitBlood({ 100.f, 100.f }, { 1.f, 0.f }, 1.f);
    REQUIRE(blood.getDropletCount() == static_cast<size_t>(BloodSplatter::dropletCountFor(10, 1.f)));
    REQUIRE(blood.getStainCount() == 1); // The pool under the wound.

    blood.update(1.f); // Past the longest flight time.
    REQUIRE(blood.getDropletCount() == 0);
    REQUIRE(blood.getStainCount() > 1);

    blood.update(BloodSplatter::stainLifetimeFor(10) + 1.f);
    REQUIRE(blood.getStainCount() == 0);
}

TEST_CASE("The floor never holds more than the stain cap", "[blood]")
{
    BloodTestRig rig { 10 };
    BloodSplatter blood { rig.fakeOptions.get() };

    for (int32_t i = 0; i < 200; ++i)
    {
        blood.emitBlood({ 0.f, 0.f }, { 0.f, 0.f }, KILL_BLOOD_STRENGTH);
        blood.update(1.f);
    }
    REQUIRE(blood.getStainCount() <= BloodSplatter::MAX_STAINS);
    REQUIRE(blood.getDropletCount() <= BloodSplatter::MAX_DROPLETS);
}

TEST_CASE("Clearing removes all blood", "[blood]")
{
    BloodTestRig rig { 3 };
    BloodSplatter blood { rig.fakeOptions.get() };

    blood.emitBlood({ 0.f, 0.f }, { 1.f, 1.f }, 1.f);
    blood.clear();
    REQUIRE(blood.getDropletCount() == 0);
    REQUIRE(blood.getStainCount() == 0);
}
