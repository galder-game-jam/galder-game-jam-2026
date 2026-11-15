#include "catch.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>
#include "../../src/galder-game-jam-library/data/MageBossConfig.hpp"
#include "../../src/galder-game-jam-library/game/enemies/CrystalShard.h"
#include "../../src/galder-game-jam-library/game/enemies/FireLine.h"
#include "../../src/galder-game-jam-library/game/enemies/IceCircle.h"
#include "../../src/galder-game-jam-library/game/enemies/MageBossEnemy.h"
#include "../../src/galder-game-jam-library/system/CollisionCategory.hpp"

using namespace ggj;

namespace
{
    constexpr float STEP = 1.f / 60.f;
    constexpr float TWO_PI = 2.f * std::numbers::pi_v<float>;

    [[nodiscard]] float lengthOf(const raylib::Vector2& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    /*! A box body like the map loader creates (static = wall). */
    b2Body* createBox(b2World& world, const b2BodyType type, const b2Vec2 center, const b2Vec2 halfSize)
    {
        b2BodyDef bodyDef;
        bodyDef.type = type;
        bodyDef.position = center;
        b2Body* body = world.CreateBody(&bodyDef);
        b2PolygonShape box;
        box.SetAsBox(halfSize.x, halfSize.y);
        b2FixtureDef fd;
        fd.shape = &box;
        fd.density = 1.f;
        body->CreateFixture(&fd);
        return body;
    }

    /*! Steps the world and reports whether @p body touched terrain at any point. */
    bool touchesTerrainWithin(b2World& world, const b2Body& body, const float seconds)
    {
        for (float t = 0.f; t < seconds; t += STEP)
        {
            world.Step(STEP, 8, 3);
            if (CollisionCategory::IsTouchingTerrain(body))
                return true;
        }
        return false;
    }
} // namespace

namespace
{
    /*! A config with a fixed number of ice circles, for the roll tests. */
    MageBossConfig withIceCircles(const int32_t minCount, const int32_t maxCount)
    {
        MageBossConfig config;
        config.minIceCircles = minCount;
        config.maxIceCircles = maxCount;
        return config.sanitized();
    }
} // namespace

TEST_CASE("MageBossConfig - sanitizing pulls nonsense map values into a playable range", "[mage]")
{
    MageBossConfig nonsense;
    nonsense.minIceCircles = 0;
    nonsense.maxIceCircles = -3;
    nonsense.iceCircleRadius = 0.f;
    nonsense.iceMinDelay = -1.f;
    nonsense.iceMaxDelay = -2.f;
    nonsense.freezeSeconds = -1.f;
    nonsense.fireWarningSeconds = -1.f;
    nonsense.enragedFireWarningSeconds = -1.f;
    nonsense.fireBurnSeconds = 0.f;
    nonsense.fireSpeed = 0.f;
    nonsense.crystalsPerBurst = 0;
    nonsense.enragedCrystalsPerBurst = -4;
    nonsense.crystalSpeed = 0.f;
    nonsense.crystalBurstInterval = 0.f;

    const MageBossConfig config = nonsense.sanitized();
    REQUIRE(config.minIceCircles >= 1);
    REQUIRE(config.maxIceCircles >= config.minIceCircles);
    REQUIRE(config.iceCircleRadius > 0.f);
    REQUIRE(config.iceMinDelay >= 0.f);
    REQUIRE(config.iceMaxDelay >= config.iceMinDelay);
    REQUIRE(config.freezeSeconds > 0.f);
    REQUIRE(config.fireWarningSeconds >= 0.f);
    REQUIRE(config.enragedFireWarningSeconds >= 0.f);
    REQUIRE(config.fireBurnSeconds > 0.f);
    REQUIRE(config.fireSpeed > 0.f);
    REQUIRE(config.crystalsPerBurst >= 1);
    REQUIRE(config.enragedCrystalsPerBurst >= 1);
    REQUIRE(config.crystalSpeed > 0.f);
    REQUIRE(config.crystalBurstInterval > 0.f);
}

TEST_CASE("MageBossConfig - a maximum below the minimum is lifted to it", "[mage]")
{
    MageBossConfig config;
    config.minIceCircles = 3;
    config.maxIceCircles = 1;
    config.iceMinDelay = 0.8f;
    config.iceMaxDelay = 0.2f;

    const MageBossConfig sanitized = config.sanitized();
    REQUIRE(sanitized.minIceCircles == 3);
    REQUIRE(sanitized.maxIceCircles == 3);
    REQUIRE(sanitized.iceMinDelay == Approx(0.8f));
    REQUIRE(sanitized.iceMaxDelay == Approx(0.8f));
}

TEST_CASE("MageBossConfig - sensible values pass through sanitizing untouched", "[mage]")
{
    MageBossConfig config;
    config.minIceCircles = 1;
    config.maxIceCircles = 6;
    config.iceMinDelay = 0.3f;
    config.iceMaxDelay = 1.5f;
    config.fireSpeed = 900.f;
    config.crystalsPerBurst = 9;
    config.crystalBurstInterval = 0.2f;

    const MageBossConfig sanitized = config.sanitized();
    REQUIRE(sanitized.minIceCircles == 1);
    REQUIRE(sanitized.maxIceCircles == 6);
    REQUIRE(sanitized.iceMinDelay == Approx(0.3f));
    REQUIRE(sanitized.iceMaxDelay == Approx(1.5f));
    REQUIRE(sanitized.fireSpeed == Approx(900.f));
    REQUIRE(sanitized.crystalsPerBurst == 9);
    REQUIRE(sanitized.crystalBurstInterval == Approx(0.2f));
}

TEST_CASE("MageBossConfig - defaults give two to four circles with a half to one second delay", "[mage]")
{
    const MageBossConfig config;
    REQUIRE(config.minIceCircles == 2);
    REQUIRE(config.maxIceCircles == 4);
    REQUIRE(config.iceMinDelay == Approx(0.5f));
    REQUIRE(config.iceMaxDelay == Approx(1.0f));
    REQUIRE(config.iceCircleRadius > 0.f);
    REQUIRE(config.enragedFireWarningSeconds < config.fireWarningSeconds);
    REQUIRE(config.enragedCrystalsPerBurst > config.crystalsPerBurst);
}

TEST_CASE("MageBossEnemy - rolls two, three or four ice circles, evenly", "[mage]")
{
    const MageBossConfig config;
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 0.f) == 2);
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 0.2f) == 2);
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 0.4f) == 3);
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 0.7f) == 4);
    // A roll of exactly one must not land past the end.
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 1.f) == 4);
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, 5.f) == 4);
    REQUIRE(MageBossEnemy::RollIceCircleCount(config, -1.f) == 2);
}

TEST_CASE("MageBossEnemy - a single-count range always rolls that count", "[mage]")
{
    const MageBossConfig config = withIceCircles(3, 3);
    for (const float roll : { 0.f, 0.5f, 1.f })
        REQUIRE(MageBossEnemy::RollIceCircleCount(config, roll) == 3);
}

TEST_CASE("MageBossEnemy - four lines of fire, eight when enraged", "[mage]")
{
    REQUIRE(MageBossEnemy::FireLineCount(false) == 4);
    REQUIRE(MageBossEnemy::FireLineCount(true) == 8);
}

TEST_CASE("MageBossEnemy - lines of fire are evenly spaced all the way round", "[mage]")
{
    for (const int32_t count : { 4, 8 })
    {
        const std::vector<float> angles = MageBossEnemy::FireLineAngles(0.3f, count);
        REQUIRE(angles.size() == static_cast<size_t>(count));
        REQUIRE(angles.front() == Approx(0.3f));
        for (size_t i = 1; i < angles.size(); ++i)
            REQUIRE(angles[i] - angles[i - 1] == Approx(TWO_PI / static_cast<float>(count)));
    }
}

TEST_CASE("MageBossEnemy - no lines of fire for zero or less", "[mage]")
{
    REQUIRE(MageBossEnemy::FireLineAngles(0.f, 0).empty());
    REQUIRE(MageBossEnemy::FireLineAngles(0.f, -2).empty());
}

TEST_CASE("MageBossEnemy - attack roll picks ice, fire, crystals, then a pause", "[mage]")
{
    REQUIRE(MageBossEnemy::ChooseAttack(0.f, false) == MageState::IceCast);
    REQUIRE(MageBossEnemy::ChooseAttack(0.29f, false) == MageState::IceCast);
    REQUIRE(MageBossEnemy::ChooseAttack(0.3f, false) == MageState::FireCast);
    REQUIRE(MageBossEnemy::ChooseAttack(0.59f, false) == MageState::FireCast);
    REQUIRE(MageBossEnemy::ChooseAttack(0.6f, false) == MageState::CrystalWindup);
    REQUIRE(MageBossEnemy::ChooseAttack(0.89f, false) == MageState::CrystalWindup);
    REQUIRE(MageBossEnemy::ChooseAttack(0.95f, false) == MageState::Idle);
    REQUIRE(MageBossEnemy::ChooseAttack(1.f, false) == MageState::Idle);
}

TEST_CASE("MageBossEnemy - enraged he pauses less often", "[mage]")
{
    // 0.92 is a pause for a calm mage but crystals for an enraged one.
    REQUIRE(MageBossEnemy::ChooseAttack(0.92f, false) == MageState::Idle);
    REQUIRE(MageBossEnemy::ChooseAttack(0.92f, true) == MageState::CrystalWindup);
    REQUIRE(MageBossEnemy::ChooseAttack(0.97f, true) == MageState::Idle);
}

TEST_CASE("MageBossEnemy - every spell is picked by some roll", "[mage]")
{
    for (const bool isEnraged : { false, true })
    {
        bool isIce = false;
        bool isFire = false;
        bool isCrystal = false;
        for (float roll = 0.f; roll <= 1.f; roll += 0.01f)
        {
            const MageState state = MageBossEnemy::ChooseAttack(roll, isEnraged);
            isIce = isIce || state == MageState::IceCast;
            isFire = isFire || state == MageState::FireCast;
            isCrystal = isCrystal || state == MageState::CrystalWindup;
        }
        REQUIRE(isIce);
        REQUIRE(isFire);
        REQUIRE(isCrystal);
    }
}

TEST_CASE("MageBossEnemy - casting lifts him off the ground", "[mage]")
{
    for (const MageState state : { MageState::IceCast, MageState::FireCast })
    {
        const MageBossEnemy::MagePose start = MageBossEnemy::ComputePose(state, 0.f, 0.f);
        const MageBossEnemy::MagePose middle = MageBossEnemy::ComputePose(state, 0.5f, 0.f);
        REQUIRE(middle.lift > start.lift);
        REQUIRE(middle.lift == Approx(MageBossEnemy::ComputePose(state, 0.5f, 3.f).lift)); // Height does not depend on the wobble clock.
    }
}

TEST_CASE("MageBossEnemy - the fire cast rears back and grows more tense", "[mage]")
{
    const MageBossEnemy::MagePose early = MageBossEnemy::ComputePose(MageState::FireCast, 0.2f, 0.f);
    const MageBossEnemy::MagePose late = MageBossEnemy::ComputePose(MageState::FireCast, 1.f, 0.f);
    REQUIRE(early.rotation < 0.f);
    REQUIRE(late.rotation < early.rotation);
}

TEST_CASE("MageBossEnemy - every pose stays a sane size", "[mage]")
{
    for (uint8_t i = 0; i < MAGE_STATE_COUNT; ++i)
    {
        for (const float progress : { 0.f, 0.25f, 0.5f, 0.75f, 1.f, 3.f })
        {
            const MageBossEnemy::MagePose pose = MageBossEnemy::ComputePose(static_cast<MageState>(i), progress, 1.234f);
            REQUIRE(pose.scaleX > 0.8f);
            REQUIRE(pose.scaleX < 1.2f);
            REQUIRE(pose.scaleY > 0.8f);
            REQUIRE(pose.scaleY < 1.3f);
            REQUIRE(std::abs(pose.rotation) < 30.f);
            REQUIRE(pose.lift >= 0.f);
            REQUIRE(pose.lift < 30.f);
        }
    }
}

TEST_CASE("MageBossEnemy - state count matches the enum so network states are range-checked", "[mage]")
{
    REQUIRE(MAGE_STATE_COUNT == static_cast<uint8_t>(MageState::Dying) + 1);
}

TEST_CASE("IceCircle - inside means within the radius, edge included", "[mage]")
{
    const raylib::Vector2 center { 100.f, 100.f };
    REQUIRE(IceCircle::IsInside(center, 50.f, { 100.f, 100.f }));
    REQUIRE(IceCircle::IsInside(center, 50.f, { 130.f, 100.f }));
    REQUIRE(IceCircle::IsInside(center, 50.f, { 150.f, 100.f }));
    REQUIRE_FALSE(IceCircle::IsInside(center, 50.f, { 151.f, 100.f }));
    // The corner of the bounding box is outside: it is a circle, not a square.
    REQUIRE_FALSE(IceCircle::IsInside(center, 50.f, { 140.f, 140.f }));
}

TEST_CASE("IceCircle - the countdown runs from nothing to done and stays done", "[mage]")
{
    REQUIRE(IceCircle::Countdown(0.f, 1.f) == Approx(0.f));
    REQUIRE(IceCircle::Countdown(0.25f, 1.f) == Approx(0.25f));
    REQUIRE(IceCircle::Countdown(1.f, 1.f) == Approx(1.f));
    REQUIRE(IceCircle::Countdown(5.f, 1.f) == Approx(1.f));
    REQUIRE(IceCircle::Countdown(-1.f, 1.f) == Approx(0.f));
    REQUIRE(IceCircle::Countdown(0.5f, 0.f) == Approx(1.f)); // No delay: already due.
}

TEST_CASE("IceCircle - its body notices nothing, even a wall", "[mage]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* wall = createBox(world, b2_staticBody, { 0.f, 0.f }, { 2.f, 2.f });
    CollisionCategory::MarkAsTerrain(*wall);
    b2Body* player = createBox(world, b2_dynamicBody, { 0.f, 0.f }, { 0.5f, 0.5f });

    b2Body* circle = IceCircle::CreateBody(world, IceCircleSpawnEvent { { 0.f, 0.f }, 80.f, 0.7f, 2.f });

    REQUIRE_FALSE(touchesTerrainWithin(world, *circle, 0.5f));
    REQUIRE(circle->GetContactList() == nullptr);
    // It is a decal: the player standing in it is not pushed.
    REQUIRE(player->GetLinearVelocity().Length() == Approx(0.f));
}

TEST_CASE("FireLine - distance to a segment is measured to its closest point", "[mage]")
{
    const raylib::Vector2 start { 0.f, 0.f };
    const raylib::Vector2 end { 100.f, 0.f };
    REQUIRE(FireLine::DistanceToSegment({ 50.f, 0.f }, start, end) == Approx(0.f).margin(0.001));
    REQUIRE(FireLine::DistanceToSegment({ 50.f, 12.f }, start, end) == Approx(12.f));
    REQUIRE(FireLine::DistanceToSegment({ 50.f, -12.f }, start, end) == Approx(12.f));
    // Past either end the closest point is the end itself, not the infinite line.
    REQUIRE(FireLine::DistanceToSegment({ 130.f, 40.f }, start, end) == Approx(50.f));
    REQUIRE(FireLine::DistanceToSegment({ -30.f, -40.f }, start, end) == Approx(50.f));
}

TEST_CASE("FireLine - a segment without length is a point", "[mage]")
{
    REQUIRE(FireLine::DistanceToSegment({ 3.f, 4.f }, { 0.f, 0.f }, { 0.f, 0.f }) == Approx(5.f));
}

TEST_CASE("FireLine - works in every direction", "[mage]")
{
    // A diagonal line: a point on it is at distance zero, a point beside it at the perpendicular distance.
    REQUIRE(FireLine::DistanceToSegment({ 50.f, 50.f }, { 0.f, 0.f }, { 100.f, 100.f }) == Approx(0.f).margin(0.001));
    REQUIRE(FireLine::DistanceToSegment({ 60.f, 40.f }, { 0.f, 0.f }, { 100.f, 100.f }) == Approx(std::sqrt(200.f)));
    REQUIRE(FireLine::DistanceToSegment({ 0.f, 30.f }, { 0.f, 0.f }, { 0.f, 100.f }) == Approx(0.f).margin(0.001));
}

TEST_CASE("FireLine - the flames race out from the mage and then stop at the end", "[mage]")
{
    constexpr float SPEED = 1800.f;
    REQUIRE(FireLine::IgnitedFraction(0.f, 500.f, SPEED) == Approx(0.f));
    REQUIRE(FireLine::IgnitedFraction(-1.f, 500.f, SPEED) == Approx(0.f));
    REQUIRE(FireLine::IgnitedFraction(0.1f, 500.f, SPEED) > 0.f);
    REQUIRE(FireLine::IgnitedFraction(0.1f, 500.f, SPEED) < 1.f);
    REQUIRE(FireLine::IgnitedFraction(0.1f, 500.f, SPEED) < FireLine::IgnitedFraction(0.2f, 500.f, SPEED));
    REQUIRE(FireLine::IgnitedFraction(10.f, 500.f, SPEED) == Approx(1.f));
    REQUIRE(FireLine::IgnitedFraction(0.5f, 0.f, SPEED) == Approx(1.f)); // A line without length is lit at once.
}

TEST_CASE("FireLine - faster flames reach the end sooner", "[mage]")
{
    REQUIRE(FireLine::IgnitedFraction(0.1f, 500.f, 1000.f) > FireLine::IgnitedFraction(0.1f, 500.f, 500.f));
    REQUIRE(FireLine::IgnitedFraction(0.1f, 500.f, 2500.f) == Approx(0.5f));
    REQUIRE(FireLine::IgnitedFraction(1.f, 500.f, 500.f) == Approx(1.f));
}

TEST_CASE("FireLine - its body notices nothing", "[mage]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* wall = createBox(world, b2_staticBody, { 0.f, 0.f }, { 2.f, 2.f });
    CollisionCategory::MarkAsTerrain(*wall);

    b2Body* fire = FireLine::CreateBody(world, FireLineSpawnEvent { { 0.f, 0.f }, { 300.f, 0.f }, 1.f, 1.4f, 1800.f });

    REQUIRE_FALSE(touchesTerrainWithin(world, *fire, 0.5f));
}

TEST_CASE("CrystalShard - a fan has one crystal per count, spread edge to edge around the aim", "[mage]")
{
    constexpr int32_t COUNT = 5;
    constexpr float SPREAD = 30.f;
    constexpr float SPEED = 260.f;
    const raylib::Vector2 origin { 400.f, 300.f };
    const float aim = 0.7f;

    const std::vector<CrystalSpawnEvent> fan = CrystalShard::BuildFan(origin, aim, COUNT, SPREAD, SPEED);
    REQUIRE(fan.size() == static_cast<size_t>(COUNT));

    const float spreadRadians = SPREAD * std::numbers::pi_v<float> / 180.f;
    for (size_t i = 0; i < fan.size(); ++i)
    {
        REQUIRE(fan[i].position.x == Approx(origin.x));
        REQUIRE(fan[i].position.y == Approx(origin.y));
        // Speed is passed on in Box2D units (metres per second).
        REQUIRE(lengthOf(fan[i].velocity) == Approx(SPEED / PhysicsObject::PPM));
        const float angle = std::atan2(fan[i].velocity.y, fan[i].velocity.x);
        const float expected = aim + spreadRadians * (static_cast<float>(i) / static_cast<float>(COUNT - 1) - 0.5f);
        REQUIRE(angle == Approx(expected));
    }
}

TEST_CASE("CrystalShard - the middle crystal of an odd fan flies straight at the aim", "[mage]")
{
    const std::vector<CrystalSpawnEvent> fan = CrystalShard::BuildFan({ 0.f, 0.f }, 1.2f, 5, 40.f, 200.f);
    const CrystalSpawnEvent& middle = fan[2];
    REQUIRE(std::atan2(middle.velocity.y, middle.velocity.x) == Approx(1.2f));
}

TEST_CASE("CrystalShard - a lone crystal flies straight at the aim whatever the spread", "[mage]")
{
    const std::vector<CrystalSpawnEvent> fan = CrystalShard::BuildFan({ 0.f, 0.f }, -0.4f, 1, 60.f, 200.f);
    REQUIRE(fan.size() == 1);
    REQUIRE(std::atan2(fan[0].velocity.y, fan[0].velocity.x) == Approx(-0.4f));
}

TEST_CASE("CrystalShard - no crystals for zero or less", "[mage]")
{
    REQUIRE(CrystalShard::BuildFan({ 0.f, 0.f }, 0.f, 0, 30.f, 200.f).empty());
    REQUIRE(CrystalShard::BuildFan({ 0.f, 0.f }, 0.f, -1, 30.f, 200.f).empty());
    REQUIRE(CrystalShard::BuildRing({ 0.f, 0.f }, 0, 0.f, 200.f, 30.f).empty());
}

TEST_CASE("CrystalShard - a ring flies outwards, evenly spaced, starting at the start angle", "[mage]")
{
    constexpr int32_t COUNT = 12;
    constexpr float RADIUS = 30.f;
    const raylib::Vector2 origin { 500.f, 300.f };

    const std::vector<CrystalSpawnEvent> ring = CrystalShard::BuildRing(origin, COUNT, 0.5f, 240.f, RADIUS);
    REQUIRE(ring.size() == static_cast<size_t>(COUNT));
    REQUIRE(std::atan2(ring[0].velocity.y, ring[0].velocity.x) == Approx(0.5f));

    for (size_t i = 0; i < ring.size(); ++i)
    {
        const raylib::Vector2 offset { ring[i].position.x - origin.x, ring[i].position.y - origin.y };
        REQUIRE(lengthOf(offset) == Approx(RADIUS));
        const float dot = offset.x * ring[i].velocity.x + offset.y * ring[i].velocity.y;
        REQUIRE(dot == Approx(lengthOf(offset) * lengthOf(ring[i].velocity)));
    }
}

TEST_CASE("CrystalShard - shatters against a wall instead of flying through it", "[mage]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* pillar = createBox(world, b2_staticBody, { 3.f, 0.f }, { 0.5f, 1.f });
    CollisionCategory::MarkAsTerrain(*pillar);

    b2Body* crystal = CrystalShard::CreateBody(world, CrystalSpawnEvent { { 0.f, 0.f }, { 8.f, 0.f } });

    REQUIRE(touchesTerrainWithin(world, *crystal, 1.f));
    REQUIRE(crystal->GetPosition().x < 3.f);
}

TEST_CASE("CrystalShard - flies past things that are not terrain and never pushes them", "[mage]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* player = createBox(world, b2_dynamicBody, { 3.f, 0.f }, { 0.5f, 0.5f });
    createBox(world, b2_staticBody, { 5.f, 0.f }, { 0.5f, 5.f }); // A wall-like body that is not tagged as terrain.

    b2Body* crystal = CrystalShard::CreateBody(world, CrystalSpawnEvent { { 0.f, 0.f }, { 8.f, 0.f } });

    REQUIRE_FALSE(touchesTerrainWithin(world, *crystal, 1.f));
    REQUIRE(crystal->GetPosition().x > 6.f);
    REQUIRE(player->GetPosition().x == Approx(3.f));
}
