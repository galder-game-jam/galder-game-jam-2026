//
// Talent table integrity and effect composition tests.
//

#include "catch.hpp"
#include "../../src/galder-game-jam-library/data/TalentState.hpp"

#include <set>
#include <string>
#include <vector>

using namespace ggj;

namespace
{
    [[nodiscard]] const TalentDef& findTalent(const TalentId id)
    {
        for (const TalentDef& def : ALL_TALENTS)
        {
            if (def.id == id)
                return def;
        }
        FAIL("Talent missing from ALL_TALENTS");
        return ALL_TALENTS.front();
    }

    [[nodiscard]] TalentState withTalents(const std::vector<TalentId>& ids)
    {
        TalentState state {};
        for (const TalentId id : ids)
            state.unlockTalent(id);
        return state;
    }

    [[nodiscard]] size_t countForClass(const TalentClass cls)
    {
        size_t count = 0;
        for (const TalentDef& def : ALL_TALENTS)
        {
            if (def.characterClass == cls)
                ++count;
        }
        return count;
    }
} // namespace

TEST_CASE("Talent table is internally consistent", "[talents]")
{
    std::set<TalentId> seen {};
    for (const TalentDef& def : ALL_TALENTS)
    {
        INFO(std::string(def.name));
        REQUIRE(def.id != TalentId::None);
        REQUIRE(seen.insert(def.id).second); // ids are unique
        REQUIRE_FALSE(def.name.empty());
        REQUIRE_FALSE(def.description.empty());

        if (def.prerequisite != TalentId::None)
        {
            const TalentDef& prerequisite = findTalent(def.prerequisite);
            REQUIRE(prerequisite.characterClass == def.characterClass);
            REQUIRE(static_cast<int>(prerequisite.tier) < static_cast<int>(def.tier));
            REQUIRE(prerequisite.requiredLevel <= def.requiredLevel);
        }
        else
        {
            REQUIRE(def.tier == TalentTier::Tier1);
        }
    }
}

TEST_CASE("Talents are grouped by class then ordered by tier for the talent screen", "[talents]")
{
    int previousClass = 0;
    int previousTier = 0;
    for (const TalentDef& def : ALL_TALENTS)
    {
        const int cls = static_cast<int>(def.characterClass);
        const int tier = static_cast<int>(def.tier);
        REQUIRE(cls >= previousClass);
        if (cls != previousClass)
            previousTier = 0;
        REQUIRE(tier >= previousTier);
        previousClass = cls;
        previousTier = tier;
    }
}

TEST_CASE("Both classes have the same number of talent choices", "[talents]")
{
    REQUIRE(countForClass(TalentClass::Rogue) == countForClass(TalentClass::Wizard));
    REQUIRE(countForClass(TalentClass::Rogue) > 5);
}

TEST_CASE("No talents means no effect", "[talents]")
{
    const TalentEffects fx = computeTalentEffects(TalentState {});
    REQUIRE(fx.energyRegenMultiplier == Approx(1.f));
    REQUIRE(fx.maxEnergyBonus == Approx(0.f));
    REQUIRE(fx.attackEnergyCostMultiplier == Approx(1.f));
    REQUIRE(fx.meleeDamageBonus == 0);
    REQUIRE(fx.manaRegenMultiplier == Approx(1.f));
    REQUIRE(fx.manaRegenDelayMultiplier == Approx(1.f));
    REQUIRE(fx.maxManaMultiplier == Approx(1.f));
    REQUIRE(fx.spellManaCostMultiplier == Approx(1.f));
    REQUIRE(fx.lifeRegenIntervalSeconds == Approx(0.f));
    REQUIRE(fx.damageCooldownMultiplier == Approx(1.f));
}

TEST_CASE("Rogue resource talents change energy and melee stats", "[talents]")
{
    const TalentEffects fx = computeTalentEffects(withTalents({ TalentId::RogueSecondWind, TalentId::RogueDeepReserves, TalentId::RogueTireless, TalentId::RogueDeadlyEdge }));
    REQUIRE(fx.energyRegenMultiplier == Approx(1.5f));
    REQUIRE(fx.maxEnergyBonus == Approx(50.f));
    REQUIRE(fx.attackEnergyCostMultiplier == Approx(0.65f));
    REQUIRE(fx.meleeDamageBonus == 1);
}

TEST_CASE("Wizard resource talents change mana stats", "[talents]")
{
    const TalentEffects fx = computeTalentEffects(withTalents({ TalentId::WizardManaSpring, TalentId::WizardDeepWell, TalentId::WizardThriftyCasting, TalentId::WizardOverflow }));
    REQUIRE(fx.manaRegenMultiplier == Approx(1.5f * 1.25f));
    REQUIRE(fx.manaRegenDelayMultiplier == Approx(0.f));
    REQUIRE(fx.maxManaMultiplier == Approx(1.25f));
    REQUIRE(fx.spellManaCostMultiplier == Approx(0.7f));
}

TEST_CASE("Health regeneration is slow and capped at the starting health", "[talents]")
{
    const TalentEffects rogue = computeTalentEffects(withTalents({ TalentId::RogueRegeneration }));
    const TalentEffects wizard = computeTalentEffects(withTalents({ TalentId::WizardVitalSpark }));
    REQUIRE(rogue.lifeRegenIntervalSeconds == Approx(LIFE_REGEN_INTERVAL_SECONDS));
    REQUIRE(wizard.lifeRegenIntervalSeconds == Approx(LIFE_REGEN_INTERVAL_SECONDS));
    REQUIRE(rogue.lifeRegenCap == 3);
}

TEST_CASE("Invulnerability talents lengthen the damage cooldown", "[talents]")
{
    REQUIRE(computeTalentEffects(withTalents({ TalentId::RogueIronWill })).damageCooldownMultiplier == Approx(1.5f));
    REQUIRE(computeTalentEffects(withTalents({ TalentId::WizardWard })).damageCooldownMultiplier == Approx(1.5f));
}

TEST_CASE("Existing talents keep their original effects", "[talents]")
{
    const TalentEffects rogue =
        computeTalentEffects(withTalents({ TalentId::RogueBladeMastery, TalentId::RogueFlurry, TalentId::RogueFleetStep, TalentId::RoguePredator, TalentId::RogueShadowsGift }));
    REQUIRE(rogue.attackSpeedMultiplier == Approx(0.75f * 0.8f));
    REQUIRE(rogue.speedMultiplier == Approx(1.2f));
    REQUIRE(rogue.killsPerExtraLife == 10);
    REQUIRE(rogue.xpMultiplier == Approx(1.5f));

    const TalentEffects wizard =
        computeTalentEffects(withTalents({ TalentId::WizardArcaneFocus, TalentId::WizardSurge, TalentId::WizardArcaneDrain, TalentId::WizardArcaneMastery }));
    REQUIRE(wizard.projectileSpeedMultiplier == Approx(1.35f));
    REQUIRE(wizard.attackSpeedMultiplier == Approx(0.75f));
    REQUIRE(wizard.bonusXpPerKill == 6);
    REQUIRE(wizard.xpMultiplier == Approx(1.75f));
}
