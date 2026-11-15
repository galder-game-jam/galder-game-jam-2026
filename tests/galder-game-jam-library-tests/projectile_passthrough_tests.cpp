#include "catch.hpp"
#include "../../src/galder-game-jam-library/game/enemies/MeatCleaver.h"
#include "../../src/galder-game-jam-library/system/CollisionCategory.hpp"

using namespace ggj;

namespace
{
    constexpr float STEP = 1.f / 60.f;

    /*! A solid static wall, the way the map loader creates one. */
    b2Body* createWall(b2World& world, const bool isProjectilePassthroughAllowed)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position = { 0.f, 0.f };
        b2Body* body = world.CreateBody(&bodyDef);
        b2PolygonShape box;
        box.SetAsBox(1.f, 1.f);
        b2FixtureDef fd;
        fd.shape = &box;
        fd.density = 1.f;
        body->CreateFixture(&fd);

        CollisionCategory::MarkAsTerrain(*body);
        if (isProjectilePassthroughAllowed)
            CollisionCategory::AllowProjectilePassthrough(*body);
        return body;
    }

    /*! A small body overlapping the wall, filtered like the given kind of object. */
    b2Body* createOverlappingBody(b2World& world, const uint16_t categoryBits, const uint16_t maskBits, const bool isSensor)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = { 0.5f, 0.f };
        b2Body* body = world.CreateBody(&bodyDef);
        b2CircleShape circle;
        circle.m_radius = 0.25f;
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.f;
        fd.isSensor = isSensor;
        fd.filter.categoryBits = categoryBits;
        fd.filter.maskBits = maskBits;
        body->CreateFixture(&fd);
        return body;
    }

    [[nodiscard]] bool isTouching(const b2Body& body, const b2Body& other)
    {
        for (const b2ContactEdge* edge = body.GetContactList(); edge != nullptr; edge = edge->next)
        {
            if (edge->other == &other && edge->contact->IsTouching())
                return true;
        }
        return false;
    }

    /*! Creates a wall and an overlapping body, steps once and reports whether they touch. */
    [[nodiscard]] bool touchesWall(const bool isProjectilePassthroughAllowed, const uint16_t categoryBits, const uint16_t maskBits, const bool isSensor)
    {
        b2World world { { 0.f, 0.f } };
        const b2Body* wall = createWall(world, isProjectilePassthroughAllowed);
        const b2Body* body = createOverlappingBody(world, categoryBits, maskBits, isSensor);
        world.Step(STEP, 8, 3);
        return isTouching(*body, *wall);
    }

    constexpr uint16_t EVERYTHING = 0xFFFF;
} // namespace

TEST_CASE("Projectile passthrough - player projectiles fly through a passthrough wall", "[collision]")
{
    REQUIRE_FALSE(touchesWall(true, CollisionCategory::PROJECTILE, EVERYTHING, true));
}

TEST_CASE("Projectile passthrough - player projectiles still hit a normal wall", "[collision]")
{
    REQUIRE(touchesWall(false, CollisionCategory::PROJECTILE, EVERYTHING, true));
}

TEST_CASE("Projectile passthrough - players and enemies are still blocked by a passthrough wall", "[collision]")
{
    REQUIRE(touchesWall(true, CollisionCategory::DEFAULT, EVERYTHING, false));
}

TEST_CASE("Projectile passthrough - bombs still bounce off a passthrough wall", "[collision]")
{
    REQUIRE(touchesWall(true, CollisionCategory::BOMB, CollisionCategory::TERRAIN, false));
}

TEST_CASE("Projectile passthrough - the wall is still terrain", "[collision]")
{
    b2World world { { 0.f, 0.f } };
    const b2Body* wall = createWall(world, true);
    REQUIRE(CollisionCategory::IsTerrain(*wall));
}

TEST_CASE("Projectile passthrough - a thrown cleaver flies through a passthrough wall", "[collision]")
{
    b2World world { { 0.f, 0.f } };
    createWall(world, true);
    b2Body* cleaver = MeatCleaver::CreateBody(world, CleaverSpawnEvent { { -3.f, 0.f }, { 8.f, 0.f } });

    for (float t = 0.f; t < 1.f; t += STEP)
    {
        world.Step(STEP, 8, 3);
        REQUIRE_FALSE(CollisionCategory::IsTouchingTerrain(*cleaver));
    }
    REQUIRE(cleaver->GetPosition().x > 1.f); // Made it out the other side.
}
