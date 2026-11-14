//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_TALENTDATA_HPP
#define GALDER_GAME_JAM_2026_TALENTDATA_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace ggj
{
    /*!
     * Unique identifier for every talent in the game.
     *
     * Rogue talents use values 1–99, Wizard talents use 101–199 so the ranges
     * are easy to inspect in saves or logs without the enum name.
     */
    enum class TalentId : uint32_t
    {
        None = 0,

        // ── Rogue ────────────────────────────────────────────────────────────
        RogueBladeMastery = 1,  ///< Tier 1 – attack speed
        RogueFleetStep    = 2,  ///< Tier 1 – movement speed
        RogueFlurry       = 3,  ///< Tier 2 – additional attack speed (req. Blade Mastery)
        RoguePredator     = 4,  ///< Tier 2 – life per 10 kills (req. Fleet Step)
        RogueShadowsGift  = 5,  ///< Tier 3 – +50 % XP (req. Flurry)
        RogueSecondWind   = 6,  ///< Tier 1 – faster energy regeneration
        RogueDeadlyEdge   = 7,  ///< Tier 2 – +1 melee damage (req. Blade Mastery)
        RogueRegeneration = 8,  ///< Tier 2 – slow health regeneration (req. Fleet Step)
        RogueDeepReserves = 9,  ///< Tier 2 – larger energy pool (req. Second Wind)
        RogueTireless     = 10, ///< Tier 3 – cheaper sword swings (req. Deep Reserves)
        RogueIronWill     = 11, ///< Tier 3 – longer invulnerability after a hit (req. Regeneration)

        // ── Wizard ───────────────────────────────────────────────────────────
        WizardArcaneFocus   = 101, ///< Tier 1 – projectile speed
        WizardWindWalk      = 102, ///< Tier 1 – movement speed
        WizardSurge         = 103, ///< Tier 2 – spell cooldown (req. Arcane Focus)
        WizardArcaneDrain   = 104, ///< Tier 2 – bonus XP per kill (req. Wind Walk)
        WizardArcaneMastery = 105, ///< Tier 3 – +75 % XP (req. Surge)
        WizardManaSpring    = 106, ///< Tier 1 – faster mana regeneration
        WizardThriftyCasting = 107, ///< Tier 2 – cheaper fireballs (req. Arcane Focus)
        WizardVitalSpark    = 108, ///< Tier 2 – slow health regeneration (req. Wind Walk)
        WizardDeepWell      = 109, ///< Tier 2 – larger mana pool (req. Mana Spring)
        WizardOverflow      = 110, ///< Tier 3 – mana regenerates without delay (req. Deep Well)
        WizardWard          = 111, ///< Tier 3 – longer invulnerability after a hit (req. Vital Spark)
    };

    /*! Tier within a class talent tree. */
    enum class TalentTier : uint8_t
    {
        Tier1 = 1,
        Tier2 = 2,
        Tier3 = 3,
    };

    /*! Which player class a talent belongs to. */
    enum class TalentClass : uint8_t
    {
        Rogue  = 0,
        Wizard = 1,
    };

    /*!
     * Static definition of a single talent — name, description, tier and
     * unlock conditions.  All instances live in ALL_TALENTS below.
     */
    struct TalentDef
    {
        TalentId         id;
        TalentClass      characterClass;
        TalentTier       tier;
        int              requiredLevel; ///< Minimum player level to unlock.
        TalentId         prerequisite;  ///< Must be unlocked first, or TalentId::None.
        std::string_view name;
        std::string_view description;
        std::string_view icon; ///< Short ASCII label used in the talent screen.
    };

    /*!
     * Computed gameplay modifiers derived from a player's selected talents.
     *
     * All multipliers stack multiplicatively.  Sensible defaults (1.0 / 0)
     * mean "no effect" when no talents are chosen.
     */
    struct TalentEffects
    {
        TalentEffects() = default;

        float speedMultiplier           { 1.0f }; ///< Applied to movement velocity.
        float attackSpeedMultiplier     { 1.0f }; ///< < 1.0 means fewer frames (faster).
        float projectileSpeedMultiplier { 1.0f }; ///< Wizard: applied to mage-ball speed.
        float xpMultiplier              { 1.0f }; ///< Applied to all incoming XP.
        int   bonusXpPerKill            { 0 };    ///< Flat XP added for every kill.
        int   killsPerExtraLife         { 0 };    ///< 0 = disabled; else +1 life every N kills.

        float energyRegenMultiplier       { 1.0f }; ///< Rogue: applied to energy regained per second.
        float maxEnergyBonus              { 0.0f }; ///< Rogue: flat energy added to the energy cap.
        float attackEnergyCostMultiplier  { 1.0f }; ///< Rogue: < 1.0 makes sword swings cheaper.
        int   meleeDamageBonus            { 0 };    ///< Rogue: flat damage added to every sword hit.

        float manaRegenMultiplier         { 1.0f }; ///< Wizard: applied to mana regained per second.
        float manaRegenDelayMultiplier    { 1.0f }; ///< Wizard: < 1.0 shortens the pause after casting before mana returns (0 = none).
        float maxManaMultiplier           { 1.0f }; ///< Wizard: applied to the mana cap.
        float spellManaCostMultiplier     { 1.0f }; ///< Wizard: < 1.0 makes fireballs cheaper.

        float lifeRegenIntervalSeconds    { 0.0f }; ///< 0 = disabled; else +1 life every N seconds while below lifeRegenCap.
        int   lifeRegenCap                { 3 };    ///< Regeneration never heals past the starting health, so it can't stack with extra lives.
        float damageCooldownMultiplier    { 1.0f }; ///< > 1.0 lengthens the invulnerability window after being hit.
    };

    /*!
     * Complete static table of every talent in the game.
     *
     * Ordered by class then tier so the talent screen can iterate it directly.
     */
    inline constexpr std::array<TalentDef, 22> ALL_TALENTS
    {{
        // ── Rogue tier 1 ─────────────────────────────────────────────────────
        {
            TalentId::RogueBladeMastery, TalentClass::Rogue, TalentTier::Tier1,
            2, TalentId::None,
            "Blade Mastery",
            "Your strikes flow faster. -25% attack frames.",
            "[S]"
        },
        {
            TalentId::RogueFleetStep, TalentClass::Rogue, TalentTier::Tier1,
            2, TalentId::None,
            "Fleet Step",
            "Light feet carry you further. +20% movement speed.",
            "[>>]"
        },
        {
            TalentId::RogueSecondWind, TalentClass::Rogue, TalentTier::Tier1,
            2, TalentId::None,
            "Second Wind",
            "Catch your breath faster. +50% energy regeneration.",
            "[E]"
        },

        // ── Rogue tier 2 ─────────────────────────────────────────────────────
        {
            TalentId::RogueFlurry, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueBladeMastery,
            "Flurry",
            "An unstoppable barrage. Requires Blade Mastery. Additional -20% attack frames.",
            "[!!]"
        },
        {
            TalentId::RogueDeadlyEdge, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueBladeMastery,
            "Deadly Edge",
            "A sharpened blade bites deeper. Requires Blade Mastery. +1 melee damage.",
            "[/]"
        },
        {
            TalentId::RoguePredator, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueFleetStep,
            "Predator",
            "Feed on the fallen. Requires Fleet Step. Gain +1 life per 10 kills.",
            "[+]"
        },
        {
            TalentId::RogueRegeneration, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueFleetStep,
            "Regeneration",
            "Wounds close on their own. Requires Fleet Step. +1 life every 30s, up to 3 lives.",
            "[H]"
        },
        {
            TalentId::RogueDeepReserves, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueSecondWind,
            "Deep Reserves",
            "A bottomless well of stamina. Requires Second Wind. +50 maximum energy.",
            "[U]"
        },

        // ── Rogue tier 3 ─────────────────────────────────────────────────────
        {
            TalentId::RogueShadowsGift, TalentClass::Rogue, TalentTier::Tier3,
            10, TalentId::RogueFlurry,
            "Shadow's Gift",
            "Darkness rewards its servants. Requires Flurry. +50% XP from all sources.",
            "[*]"
        },
        {
            TalentId::RogueTireless, TalentClass::Rogue, TalentTier::Tier3,
            10, TalentId::RogueDeepReserves,
            "Tireless",
            "Every swing costs you less. Requires Deep Reserves. -35% energy per sword swing.",
            "[T]"
        },
        {
            TalentId::RogueIronWill, TalentClass::Rogue, TalentTier::Tier3,
            10, TalentId::RogueRegeneration,
            "Iron Will",
            "Shrug off the pain. Requires Regeneration. +50% invulnerability time after a hit.",
            "[I]"
        },

        // ── Wizard tier 1 ────────────────────────────────────────────────────
        {
            TalentId::WizardArcaneFocus, TalentClass::Wizard, TalentTier::Tier1,
            2, TalentId::None,
            "Arcane Focus",
            "Your spells fly true. +35% projectile speed.",
            "[~]"
        },
        {
            TalentId::WizardWindWalk, TalentClass::Wizard, TalentTier::Tier1,
            2, TalentId::None,
            "Wind Walk",
            "The wind carries you. +20% movement speed.",
            "[>>]"
        },
        {
            TalentId::WizardManaSpring, TalentClass::Wizard, TalentTier::Tier1,
            2, TalentId::None,
            "Mana Spring",
            "Power wells up faster. +50% mana regeneration.",
            "[M]"
        },

        // ── Wizard tier 2 ────────────────────────────────────────────────────
        {
            TalentId::WizardSurge, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardArcaneFocus,
            "Surge",
            "Power flows without limit. Requires Arcane Focus. -25% spell cooldown.",
            "[!]"
        },
        {
            TalentId::WizardThriftyCasting, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardArcaneFocus,
            "Thrifty Casting",
            "Waste not, want not. Requires Arcane Focus. -30% mana per fireball.",
            "[$]"
        },
        {
            TalentId::WizardArcaneDrain, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardWindWalk,
            "Arcane Drain",
            "Steal power from your foes. Requires Wind Walk. +6 bonus XP per kill.",
            "[D]"
        },
        {
            TalentId::WizardVitalSpark, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardWindWalk,
            "Vital Spark",
            "A flicker of life returns. Requires Wind Walk. +1 life every 30s, up to 3 lives.",
            "[H]"
        },
        {
            TalentId::WizardDeepWell, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardManaSpring,
            "Deep Well",
            "Your mind holds more. Requires Mana Spring. +25% maximum mana.",
            "[U]"
        },

        // ── Wizard tier 3 ────────────────────────────────────────────────────
        {
            TalentId::WizardArcaneMastery, TalentClass::Wizard, TalentTier::Tier3,
            10, TalentId::WizardSurge,
            "Arcane Mastery",
            "You have transcended mortal limits. Requires Surge. +75% XP from all sources.",
            "[*]"
        },
        {
            TalentId::WizardOverflow, TalentClass::Wizard, TalentTier::Tier3,
            10, TalentId::WizardDeepWell,
            "Overflow",
            "The well never runs dry. Requires Deep Well. Mana regenerates at once, +25% faster.",
            "[O]"
        },
        {
            TalentId::WizardWard, TalentClass::Wizard, TalentTier::Tier3,
            10, TalentId::WizardVitalSpark,
            "Ward",
            "A shimmering barrier lingers. Requires Vital Spark. +50% invulnerability time after a hit.",
            "[W]"
        },
    }};

} // namespace ggj

#endif // GALDER_GAME_JAM_2026_TALENTDATA_HPP
