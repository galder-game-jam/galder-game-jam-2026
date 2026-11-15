#include "catch.hpp"
#include "../../src/galder-game-jam-library/data/QuestProgress.hpp"

using namespace ggj;

TEST_CASE("QuestProgress - defaults require only the enemies", "[quest]")
{
    QuestProgress quest {};
    REQUIRE(quest.isAllEnemiesRequired);
    REQUIRE_FALSE(quest.isAllCoinsRequired);

    quest.enemiesTotal = 3;
    quest.enemiesLeft = 1;
    quest.coinsTotal = 5;
    quest.coinsLeft = 5;
    REQUIRE_FALSE(quest.isComplete());

    quest.enemiesLeft = 0;
    REQUIRE(quest.isComplete());
}

TEST_CASE("QuestProgress - coins block the portal when required", "[quest]")
{
    QuestProgress quest {};
    quest.isAllCoinsRequired = true;
    quest.enemiesTotal = 2;
    quest.coinsTotal = 4;
    quest.coinsLeft = 1;
    REQUIRE_FALSE(quest.isComplete());

    quest.coinsLeft = 0;
    REQUIRE(quest.isComplete());
}

TEST_CASE("QuestProgress - enemies can be made optional", "[quest]")
{
    QuestProgress quest {};
    quest.isAllEnemiesRequired = false;
    quest.isAllCoinsRequired = true;
    quest.enemiesTotal = 6;
    quest.enemiesLeft = 6;
    quest.coinsTotal = 2;
    quest.coinsLeft = 0;
    REQUIRE(quest.isComplete());
}

TEST_CASE("QuestProgress - a map that requires nothing is complete", "[quest]")
{
    QuestProgress quest {};
    quest.isAllEnemiesRequired = false;
    quest.enemiesTotal = 1;
    quest.enemiesLeft = 1;
    REQUIRE(quest.isComplete());
}

TEST_CASE("QuestProgress - killed and collected counts", "[quest]")
{
    QuestProgress quest {};
    quest.enemiesTotal = 10;
    quest.enemiesLeft = 7;
    quest.coinsTotal = 12;
    quest.coinsLeft = 4;
    REQUIRE(quest.enemiesKilled() == 3);
    REQUIRE(quest.coinsCollected() == 8);
}
