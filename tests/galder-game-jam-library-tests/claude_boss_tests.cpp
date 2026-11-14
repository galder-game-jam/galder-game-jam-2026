//
// Created by robin on 14.11.2026.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/game/enemies/Bomb.h"
#include "../../src/galder-game-jam-library/game/enemies/ClaudeEnemy.h"
#include "../../src/galder-game-jam-library/system/CollisionCategory.hpp"

using namespace ggj;

namespace
{
    [[nodiscard]] bool isSameRgb(const ::Color& lhs, const ::Color& rhs)
    {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
    }

    [[nodiscard]] int32_t findSwap(const PaletteSwap& swap, const ::Color& from)
    {
        for (int32_t i = 0; i < swap.count; ++i)
        {
            if (isSameRgb(swap.from[i], from))
                return i;
        }
        return -1;
    }
} // namespace

TEST_CASE("Bomb - fuse frame advances by thirds of the fuse", "[bomb]")
{
    constexpr float FUSE = 3.f;
    REQUIRE(Bomb::FuseFrameIndex(0.f, FUSE) == 0);
    REQUIRE(Bomb::FuseFrameIndex(0.99f, FUSE) == 0);
    REQUIRE(Bomb::FuseFrameIndex(1.f, FUSE) == 1);
    REQUIRE(Bomb::FuseFrameIndex(1.99f, FUSE) == 1);
    REQUIRE(Bomb::FuseFrameIndex(2.f, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(FUSE, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(10.f, FUSE) == 2);
    REQUIRE(Bomb::FuseFrameIndex(-1.f, FUSE) == 0);
}

TEST_CASE("Bomb - tint goes from white to strongly red", "[bomb]")
{
    const ::Color start = Bomb::FuseTint(0.f);
    REQUIRE(start.r == 255);
    REQUIRE(start.g == 255);
    REQUIRE(start.b == 255);

    const ::Color middle = Bomb::FuseTint(0.5f);
    const ::Color end = Bomb::FuseTint(1.f);
    REQUIRE(end.r == 255);
    REQUIRE(end.g < middle.g);
    REQUIRE(middle.g < start.g);
    REQUIRE(end.g <= 60);
    REQUIRE(end.g == end.b);
    REQUIRE(isSameRgb(Bomb::FuseTint(2.f), end));
}

TEST_CASE("Bomb - explosion walks the 4x4 frame grid", "[bomb]")
{
    const raylib::Rectangle first = Bomb::ExplosionFrameRect(0.f);
    REQUIRE(first.x == 0.f);
    REQUIRE(first.y == 0.f);
    REQUIRE(first.width == Bomb::FRAME_SIZE);
    REQUIRE(first.height == Bomb::FRAME_SIZE);

    // Frame 5 is the second column of the second row.
    const raylib::Rectangle fifth = Bomb::ExplosionFrameRect(Bomb::EXPLOSION_FRAME_DURATION * 5.5f);
    REQUIRE(fifth.x == Bomb::FRAME_SIZE);
    REQUIRE(fifth.y == Bomb::FRAME_SIZE);

    // Past the end it holds the last frame (bottom-right).
    const raylib::Rectangle last = Bomb::ExplosionFrameRect(100.f);
    REQUIRE(last.x == Bomb::FRAME_SIZE * 3.f);
    REQUIRE(last.y == Bomb::FRAME_SIZE * 3.f);
}

TEST_CASE("Claude enrage - no swap before the transform starts", "[claude]")
{
    REQUIRE(ClaudeEnemy::BuildEnragePaletteSwap(0.f).count == 0);
    REQUIRE(ClaudeEnemy::BuildEnragePaletteSwap(-1.f).count == 0);
}

TEST_CASE("Claude enrage - body turns black and eyes turn red", "[claude]")
{
    const PaletteSwap full = ClaudeEnemy::BuildEnragePaletteSwap(1.f);
    const int32_t body = findSwap(full, ClaudeEnemy::BODY_COLOR);
    const int32_t eyes = findSwap(full, ClaudeEnemy::EYE_COLOR);
    REQUIRE(body >= 0);
    REQUIRE(eyes >= 0);
    REQUIRE(isSameRgb(full.to[body], ClaudeEnemy::ENRAGED_BODY_COLOR));
    REQUIRE(isSameRgb(full.to[eyes], ClaudeEnemy::ENRAGED_EYE_COLOR));

    // Halfway through, both colours are partway between the two ends.
    const PaletteSwap half = ClaudeEnemy::BuildEnragePaletteSwap(0.5f);
    REQUIRE(half.to[body].r < ClaudeEnemy::BODY_COLOR.r);
    REQUIRE(half.to[body].r > ClaudeEnemy::ENRAGED_BODY_COLOR.r);
    REQUIRE(half.to[eyes].r > ClaudeEnemy::EYE_COLOR.r);
    REQUIRE(half.to[eyes].r < ClaudeEnemy::ENRAGED_EYE_COLOR.r);
}

namespace
{
    constexpr float STEP = 1.f / 60.f;

    /*! A box body like the map loader creates (static = wall, dynamic = player-like). */
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

    void stepFor(b2World& world, const float seconds)
    {
        for (float t = 0.f; t < seconds; t += STEP)
            world.Step(STEP, 8, 3);
    }
} // namespace

TEST_CASE("Bomb - bounces off a terrain wall", "[bomb]")
{
    b2World world { { 0.f, 0.f } };
    // Wall 3 m to the right of the bomb, tagged as terrain like solid static map objects are.
    b2Body* wall = createBox(world, b2_staticBody, { 3.f, 0.f }, { 0.5f, 5.f });
    CollisionCategory::MarkAsTerrain(*wall);

    b2Body* bomb = Bomb::CreateBody(world, BombSpawnEvent { { 0.f, 0.f }, { 20.f, 0.f }, 3.f });
    stepFor(world, 1.f);

    REQUIRE(bomb->GetPosition().x < 2.5f);       // Never got through the wall.
    REQUIRE(bomb->GetLinearVelocity().x <= 0.f); // Heading back (or stopped), not into the wall.
}

TEST_CASE("Bomb - passes through players and other bombs", "[bomb]")
{
    b2World world { { 0.f, 0.f } };
    // A dynamic body on the bomb's path, like a player, with the default collision filter.
    b2Body* player = createBox(world, b2_dynamicBody, { 3.f, 0.f }, { 0.5f, 0.5f });
    b2Body* bomb = Bomb::CreateBody(world, BombSpawnEvent { { 0.f, 0.f }, { 20.f, 0.f }, 3.f });
    b2Body* otherBomb = Bomb::CreateBody(world, BombSpawnEvent { { PhysicsObject::PPM, 0.f }, { 0.f, 0.f }, 3.f });
    stepFor(world, 1.f);

    REQUIRE(bomb->GetPosition().x > 4.f);               // Slid on past the player.
    REQUIRE(player->GetPosition().x == Approx(3.f));    // Was not pushed.
    REQUIRE(otherBomb->GetPosition().x == Approx(1.f)); // Bombs ignore each other (spawned 1 m in).
}
