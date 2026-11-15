#include "catch.hpp"
#include <algorithm>
#include <vector>
#include "../../src/galder-game-jam-library/data/ButcherConfig.hpp"
#include "../../src/galder-game-jam-library/game/enemies/ButcherEnemy.h"
#include "../../src/galder-game-jam-library/game/enemies/MeatCleaver.h"
#include "../../src/galder-game-jam-library/game/enemies/ObstacleSteering.h"
#include "../../src/galder-game-jam-library/game/enemies/ThrownSpider.h"
#include "../../src/galder-game-jam-library/system/CollisionCategory.hpp"
#include <cmath>
#include <numbers>

using namespace ggj;

namespace
{
    constexpr float STEP = 1.f / 60.f;

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
        fd.friction = 0.3f;
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

    [[nodiscard]] float lengthOf(const raylib::Vector2& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }
} // namespace

TEST_CASE("MeatCleaver - a ring has one evenly spaced cleaver per count, all flying outwards", "[butcher]")
{
    constexpr int32_t COUNT = 8;
    constexpr float SPEED = 240.f;
    constexpr float RADIUS = 36.f;
    const raylib::Vector2 origin { 500.f, 300.f };

    const std::vector<CleaverSpawnEvent> ring = MeatCleaver::BuildRing(origin, COUNT, 0.f, SPEED, RADIUS);
    REQUIRE(ring.size() == static_cast<size_t>(COUNT));

    for (size_t i = 0; i < ring.size(); ++i)
    {
        const raylib::Vector2 offset { ring[i].position.x - origin.x, ring[i].position.y - origin.y };
        REQUIRE(lengthOf(offset) == Approx(RADIUS));
        // Speed is passed on in Box2D units (metres per second).
        REQUIRE(lengthOf(ring[i].velocity) == Approx(SPEED / PhysicsObject::PPM));
        // Flying straight away from the centre.
        const float dot = offset.x * ring[i].velocity.x + offset.y * ring[i].velocity.y;
        REQUIRE(dot > 0.f);
        REQUIRE(dot == Approx(lengthOf(offset) * lengthOf(ring[i].velocity)));
    }

    // Neighbours are 360 / COUNT degrees apart.
    const float expectedStep = 2.f * std::numbers::pi_v<float> / static_cast<float>(COUNT);
    for (size_t i = 0; i < ring.size(); ++i)
    {
        const raylib::Vector2& a = ring[i].velocity;
        const raylib::Vector2& b = ring[(i + 1) % ring.size()].velocity;
        const float angle = std::acos(std::clamp((a.x * b.x + a.y * b.y) / (lengthOf(a) * lengthOf(b)), -1.f, 1.f));
        REQUIRE(angle == Approx(expectedStep).margin(1e-4f));
    }
}

TEST_CASE("MeatCleaver - the first cleaver of a ring leaves in the start direction", "[butcher]")
{
    const float startAngle = std::numbers::pi_v<float> / 2.f; // Straight down on screen.
    const std::vector<CleaverSpawnEvent> ring = MeatCleaver::BuildRing({ 0.f, 0.f }, 4, startAngle, 300.f, 10.f);
    REQUIRE(ring.front().velocity.x == Approx(0.f).margin(1e-4f));
    REQUIRE(ring.front().velocity.y > 0.f);
}

TEST_CASE("MeatCleaver - no ring without cleavers", "[butcher]")
{
    REQUIRE(MeatCleaver::BuildRing({ 0.f, 0.f }, 0, 0.f, 300.f, 10.f).empty());
    REQUIRE(MeatCleaver::BuildRing({ 0.f, 0.f }, -3, 0.f, 300.f, 10.f).empty());
}

TEST_CASE("MeatCleaver - spins the way it flies, so every peer agrees", "[butcher]")
{
    REQUIRE(MeatCleaver::SpinFor({ 1.f, 0.f }) > 0.f);
    REQUIRE(MeatCleaver::SpinFor({ -1.f, 0.f }) < 0.f);
    REQUIRE(MeatCleaver::SpinFor({ 1.f, 5.f }) == Approx(-MeatCleaver::SpinFor({ -1.f, 5.f })));
}

TEST_CASE("MeatCleaver - a stuck cleaver fades out over its stuck time", "[butcher]")
{
    REQUIRE(MeatCleaver::StuckAlpha(0.f) == 255);
    REQUIRE(MeatCleaver::StuckAlpha(MeatCleaver::STUCK_DURATION) == 0);
    REQUIRE(MeatCleaver::StuckAlpha(MeatCleaver::STUCK_DURATION * 0.5f) < 255);
    REQUIRE(MeatCleaver::StuckAlpha(MeatCleaver::STUCK_DURATION * 0.5f) > 0);
    REQUIRE(MeatCleaver::StuckAlpha(MeatCleaver::STUCK_DURATION * 0.25f) > MeatCleaver::StuckAlpha(MeatCleaver::STUCK_DURATION * 0.75f));
    REQUIRE(MeatCleaver::StuckAlpha(-1.f) == 255);
    REQUIRE(MeatCleaver::StuckAlpha(100.f) == 0);
}

TEST_CASE("MeatCleaver - sticks into an obstacle instead of flying through it", "[butcher]")
{
    b2World world { { 0.f, 0.f } };
    // A pillar 3 m to the right, tagged as terrain like solid static map objects are.
    b2Body* pillar = createBox(world, b2_staticBody, { 3.f, 0.f }, { 0.5f, 1.f });
    CollisionCategory::MarkAsTerrain(*pillar);

    b2Body* cleaver = MeatCleaver::CreateBody(world, CleaverSpawnEvent { { 0.f, 0.f }, { 8.f, 0.f } });

    REQUIRE(touchesTerrainWithin(world, *cleaver, 1.f));
    REQUIRE(cleaver->GetPosition().x < 3.f); // Stopped in front of the pillar, not behind it.
}

TEST_CASE("MeatCleaver - flies past an obstacle that is not in its way", "[butcher]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* pillar = createBox(world, b2_staticBody, { 3.f, 4.f }, { 0.5f, 1.f });
    CollisionCategory::MarkAsTerrain(*pillar);

    b2Body* cleaver = MeatCleaver::CreateBody(world, CleaverSpawnEvent { { 0.f, 0.f }, { 8.f, 0.f } });

    REQUIRE_FALSE(touchesTerrainWithin(world, *cleaver, 1.f));
    REQUIRE(cleaver->GetPosition().x > 6.f);
}

TEST_CASE("MeatCleaver - ignores bodies that are not terrain and never pushes them", "[butcher]")
{
    b2World world { { 0.f, 0.f } };
    // A dynamic body on the cleaver's path, like a player, with the default collision filter.
    b2Body* player = createBox(world, b2_dynamicBody, { 3.f, 0.f }, { 0.5f, 0.5f });
    // A static trigger-like body that is not tagged as terrain (e.g. the enemy turn objects).
    createBox(world, b2_staticBody, { 5.f, 0.f }, { 0.5f, 5.f });

    b2Body* cleaver = MeatCleaver::CreateBody(world, CleaverSpawnEvent { { 0.f, 0.f }, { 8.f, 0.f } });

    REQUIRE_FALSE(touchesTerrainWithin(world, *cleaver, 1.f));
    REQUIRE(cleaver->GetPosition().x > 6.f);
    REQUIRE(player->GetPosition().x == Approx(3.f));
}

TEST_CASE("ThrownSpider - launch velocity lands it on the target when the flight ends", "[butcher]")
{
    const SpiderSpawnEvent spawn { { 100.f, 200.f }, { 400.f, 80.f }, 0.9f };
    const raylib::Vector2 velocity = ThrownSpider::LaunchVelocity(spawn);

    REQUIRE(velocity.x * PhysicsObject::PPM * spawn.flightTime == Approx(spawn.target.x - spawn.origin.x));
    REQUIRE(velocity.y * PhysicsObject::PPM * spawn.flightTime == Approx(spawn.target.y - spawn.origin.y));
}

TEST_CASE("ThrownSpider - no flight time means no launch", "[butcher]")
{
    const raylib::Vector2 velocity = ThrownSpider::LaunchVelocity(SpiderSpawnEvent { { 0.f, 0.f }, { 100.f, 0.f }, 0.f });
    REQUIRE(velocity.x == 0.f);
    REQUIRE(velocity.y == 0.f);
}

TEST_CASE("ThrownSpider - arc is flat at both ends and highest in the middle", "[butcher]")
{
    REQUIRE(ThrownSpider::ArcFraction(0.f) == Approx(0.f));
    REQUIRE(ThrownSpider::ArcFraction(1.f) == Approx(0.f));
    REQUIRE(ThrownSpider::ArcFraction(0.5f) == Approx(1.f));
    REQUIRE(ThrownSpider::ArcFraction(0.25f) == Approx(ThrownSpider::ArcFraction(0.75f)));
    REQUIRE(ThrownSpider::ArcFraction(-1.f) == Approx(0.f));
    REQUIRE(ThrownSpider::ArcFraction(2.f) == Approx(0.f));
}

TEST_CASE("ThrownSpider - legs flail through every frame of the strip and wrap around", "[butcher]")
{
    REQUIRE(ThrownSpider::FrameIndex(0.f) == 0);
    REQUIRE(ThrownSpider::FrameIndex(ThrownSpider::FRAME_DURATION * 1.5f) == 1);
    REQUIRE(ThrownSpider::FrameIndex(ThrownSpider::FRAME_DURATION * 3.5f) == 3);
    REQUIRE(ThrownSpider::FrameIndex(ThrownSpider::FRAME_DURATION * 4.5f) == 0);
    REQUIRE(ThrownSpider::FrameIndex(-1.f) == 0);
}

TEST_CASE("ThrownSpider - splats against an obstacle between the butcher and the player", "[butcher]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* pillar = createBox(world, b2_staticBody, { 4.f, 0.f }, { 0.5f, 1.f });
    CollisionCategory::MarkAsTerrain(*pillar);

    // Thrown at a player 8 m away, behind the pillar.
    const SpiderSpawnEvent spawn { { 0.f, 0.f }, { 8.f * PhysicsObject::PPM, 0.f }, 1.f };
    b2Body* spider = ThrownSpider::CreateBody(world, spawn);

    REQUIRE(touchesTerrainWithin(world, *spider, 1.f));
    REQUIRE(spider->GetPosition().x < 4.f);
}

TEST_CASE("ButcherConfig - clamps nonsense map values", "[butcher]")
{
    const ButcherConfig config { -5, -1, -9, -100.f, -2.f };
    REQUIRE(config.normalCleaverCount == 0);
    REQUIRE(config.enragedCleaverCount == 0);
    REQUIRE(config.deathCleaverCount == 0);
    REQUIRE(config.cleaverSpeed > 0.f);
    REQUIRE(config.spiderFlightTime > 0.f);

    const ButcherConfig sensible { 10, 16, 24, 220.f, 0.9f };
    REQUIRE(sensible.normalCleaverCount == 10);
    REQUIRE(sensible.enragedCleaverCount == 16);
    REQUIRE(sensible.deathCleaverCount == 24);
    REQUIRE(sensible.cleaverSpeed == Approx(220.f));
    REQUIRE(sensible.spiderFlightTime == Approx(0.9f));
}

TEST_CASE("ButcherConfig - defaults survive default construction", "[butcher]")
{
    const ButcherConfig config;
    REQUIRE(config.normalCleaverCount > 0);
    REQUIRE(config.enragedCleaverCount > config.normalCleaverCount);
    REQUIRE(config.cleaverSpeed > 0.f);
    REQUIRE(config.spiderFlightTime > 0.f);
}

TEST_CASE("ButcherEnemy - attack roll picks cleavers, then a spider, then a pause", "[butcher]")
{
    REQUIRE(ButcherEnemy::ChooseAttack(0.f, false) == ButcherState::CleaverWindup);
    REQUIRE(ButcherEnemy::ChooseAttack(0.49f, false) == ButcherState::CleaverWindup);
    REQUIRE(ButcherEnemy::ChooseAttack(0.5f, false) == ButcherState::SpiderWindup);
    REQUIRE(ButcherEnemy::ChooseAttack(0.89f, false) == ButcherState::SpiderWindup);
    REQUIRE(ButcherEnemy::ChooseAttack(0.95f, false) == ButcherState::Idle);
    REQUIRE(ButcherEnemy::ChooseAttack(1.f, false) == ButcherState::Idle);
}

TEST_CASE("ButcherEnemy - enraged he pauses less often", "[butcher]")
{
    // 0.92 is a pause for a calm butcher but a spider for an enraged one.
    REQUIRE(ButcherEnemy::ChooseAttack(0.92f, false) == ButcherState::Idle);
    REQUIRE(ButcherEnemy::ChooseAttack(0.92f, true) == ButcherState::SpiderWindup);
    REQUIRE(ButcherEnemy::ChooseAttack(0.97f, true) == ButcherState::Idle);
}

TEST_CASE("ButcherEnemy - a chained ring flies through the gaps of the first", "[butcher]")
{
    constexpr int32_t COUNT = 12;
    const float spacing = 2.f * std::numbers::pi_v<float> / static_cast<float>(COUNT);
    REQUIRE(ButcherEnemy::NextRingStartAngle(0.f, COUNT) == Approx(spacing / 2.f));
    REQUIRE(ButcherEnemy::NextRingStartAngle(1.f, COUNT) == Approx(1.f + spacing / 2.f));
    // Nothing to offset by when there are no cleavers.
    REQUIRE(ButcherEnemy::NextRingStartAngle(1.f, 0) == Approx(1.f));
}

TEST_CASE("ButcherEnemy - cleaver windup rears back and grows more tense", "[butcher]")
{
    const ButcherEnemy::ButcherPose early = ButcherEnemy::ComputePose(ButcherState::CleaverWindup, 0.f, 0.f);
    const ButcherEnemy::ButcherPose late = ButcherEnemy::ComputePose(ButcherState::CleaverWindup, 1.f, 0.f);
    REQUIRE(early.rotation < 0.f);
    REQUIRE(late.rotation < early.rotation); // Leans further back the nearer the throw.
    REQUIRE(late.scaleY > early.scaleY);     // Stretches up with the cleaver held high.
}

TEST_CASE("ButcherEnemy - the follow-through lunges forward and then settles", "[butcher]")
{
    const ButcherEnemy::ButcherPose start = ButcherEnemy::ComputePose(ButcherState::Recover, 0.f, 0.f);
    const ButcherEnemy::ButcherPose end = ButcherEnemy::ComputePose(ButcherState::Recover, 1.f, 0.f);
    REQUIRE(start.rotation > 0.f);
    REQUIRE(end.rotation == Approx(0.f));
    REQUIRE(end.scaleX == Approx(1.f));
    REQUIRE(end.scaleY == Approx(1.f));
}

TEST_CASE("ButcherEnemy - the spider windup is a crouch", "[butcher]")
{
    const ButcherEnemy::ButcherPose crouch = ButcherEnemy::ComputePose(ButcherState::SpiderWindup, 1.f, 0.f);
    REQUIRE(crouch.scaleY < 1.f);
    REQUIRE(crouch.scaleX > 1.f);
}

TEST_CASE("ButcherEnemy - every pose stays a sane size", "[butcher]")
{
    for (const ButcherState state : { ButcherState::Idle, ButcherState::Walk, ButcherState::CleaverWindup, ButcherState::SpiderWindup, ButcherState::Recover, ButcherState::Dying })
    {
        for (const float progress : { 0.f, 0.25f, 0.5f, 0.75f, 1.f, 3.f })
        {
            const ButcherEnemy::ButcherPose pose = ButcherEnemy::ComputePose(state, progress, 1.234f);
            REQUIRE(pose.scaleX > 0.8f);
            REQUIRE(pose.scaleX < 1.2f);
            REQUIRE(pose.scaleY > 0.8f);
            REQUIRE(pose.scaleY < 1.3f);
            REQUIRE(std::abs(pose.rotation) < 30.f);
        }
    }
}

TEST_CASE("ButcherEnemy - state count matches the enum so network states are range-checked", "[butcher]")
{
    REQUIRE(BUTCHER_STATE_COUNT == static_cast<uint8_t>(ButcherState::Dying) + 1);
}

namespace
{
    /*! A round dynamic body like the butcher (radius in metres), with the default collision filter. */
    b2Body* createWalker(b2World& world, const b2Vec2 position, const float radius)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = position;
        b2Body* body = world.CreateBody(&bodyDef);
        b2CircleShape circle;
        circle.m_radius = radius;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.f;
        body->CreateFixture(&fd);
        return body;
    }
} // namespace

TEST_CASE("ObstacleSteering - a clear path is left alone", "[steering]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* walker = createWalker(world, { 0.f, 0.f }, 0.9f);
    ObstacleSteering steering;

    const raylib::Vector2 heading = steering.steer(*walker, { 1.f, 0.f });
    REQUIRE(heading.x == Approx(1.f));
    REQUIRE(heading.y == Approx(0.f));
}

TEST_CASE("ObstacleSteering - walks round a pillar in the way", "[steering]")
{
    b2World world { { 0.f, 0.f } };
    createBox(world, b2_staticBody, { 3.f, 0.f }, { 1.f, 1.f });
    b2Body* walker = createWalker(world, { 0.f, 0.f }, 0.9f);
    ObstacleSteering steering;

    REQUIRE(ObstacleSteering::IsPathBlocked(*walker, { 1.f, 0.f }));
    const raylib::Vector2 heading = steering.steer(*walker, { 1.f, 0.f });
    REQUIRE_FALSE(ObstacleSteering::IsPathBlocked(*walker, heading));
    REQUIRE(heading.y != Approx(0.f).margin(0.1f)); // Bent sideways, not straight on.
}

TEST_CASE("ObstacleSteering - the wide body counts: a gap narrower than the walker is blocked", "[steering]")
{
    b2World world { { 0.f, 0.f } };
    // Two pillars with only 1 m between them; a walker of 0.9 m radius cannot fit.
    createBox(world, b2_staticBody, { 3.f, 1.5f }, { 0.5f, 1.f });
    createBox(world, b2_staticBody, { 3.f, -1.5f }, { 0.5f, 1.f });
    b2Body* walker = createWalker(world, { 0.f, 0.f }, 0.9f);

    REQUIRE(ObstacleSteering::IsPathBlocked(*walker, { 1.f, 0.f }));
}

TEST_CASE("ObstacleSteering - players, enemies and triggers do not block", "[steering]")
{
    b2World world { { 0.f, 0.f } };
    createBox(world, b2_dynamicBody, { 3.f, 0.f }, { 1.f, 1.f }); // Like a player or another enemy.
    b2Body* trigger = createBox(world, b2_staticBody, { 3.f, 3.f }, { 1.f, 1.f });
    trigger->GetFixtureList()->SetSensor(true);
    b2Body* walker = createWalker(world, { 0.f, 0.f }, 0.9f);

    REQUIRE_FALSE(ObstacleSteering::IsPathBlocked(*walker, { 1.f, 0.f }));
    REQUIRE_FALSE(ObstacleSteering::IsPathBlocked(*walker, { 0.f, 1.f }));
}

TEST_CASE("ObstacleSteering - a stationary request is never blocked", "[steering]")
{
    b2World world { { 0.f, 0.f } };
    b2Body* walker = createWalker(world, { 0.f, 0.f }, 0.9f);
    REQUIRE_FALSE(ObstacleSteering::IsPathBlocked(*walker, { 0.f, 0.f }));
}
