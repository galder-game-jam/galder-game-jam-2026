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
     * Rogue talents use values 1–9, Wizard talents use 11–19 so the ranges
     * are easy to inspect in saves or logs without the enum name.
     */
    enum class TalentId : uint32_t
    {
        None = 0,

        // ── Rogue ────────────────────────────────────────────────────────────
        RogueBladeMastery = 1, ///< Tier 1 – attack speed
        RogueFleetStep    = 2, ///< Tier 1 – movement speed
        RogueFlurry       = 3, ///< Tier 2 – additional attack speed (req. Blade Mastery)
        RoguePredator     = 4, ///< Tier 2 – life per 10 kills (req. Fleet Step)
        RogueShadowsGift  = 5, ///< Tier 3 – +50 % XP (req. Flurry)

        // ── Wizard ───────────────────────────────────────────────────────────
        WizardArcaneFocus   = 11, ///< Tier 1 – projectile speed
        WizardWindWalk      = 12, ///< Tier 1 – movement speed
        WizardSurge         = 13, ///< Tier 2 – spell cooldown (req. Arcane Focus)
        WizardArcaneDrain   = 14, ///< Tier 2 – bonus XP per kill (req. Wind Walk)
        WizardArcaneMastery = 15, ///< Tier 3 – +75 % XP (req. Surge)
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
    };

    /*!
     * Complete static table of every talent in the game.
     *
     * Ordered by class then tier so the talent screen can iterate it directly.
     */
    inline constexpr std::array<TalentDef, 10> ALL_TALENTS
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

        // ── Rogue tier 2 ─────────────────────────────────────────────────────
        {
            TalentId::RogueFlurry, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueBladeMastery,
            "Flurry",
            "An unstoppable barrage. Requires Blade Mastery. Additional -20% attack frames.",
            "[!!]"
        },
        {
            TalentId::RoguePredator, TalentClass::Rogue, TalentTier::Tier2,
            5, TalentId::RogueFleetStep,
            "Predator",
            "Feed on the fallen. Requires Fleet Step. Gain +1 life per 10 kills.",
            "[+]"
        },

        // ── Rogue tier 3 ─────────────────────────────────────────────────────
        {
            TalentId::RogueShadowsGift, TalentClass::Rogue, TalentTier::Tier3,
            10, TalentId::RogueFlurry,
            "Shadow's Gift",
            "Darkness rewards its servants. Requires Flurry. +50% XP from all sources.",
            "[*]"
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

        // ── Wizard tier 2 ────────────────────────────────────────────────────
        {
            TalentId::WizardSurge, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardArcaneFocus,
            "Surge",
            "Power flows without limit. Requires Arcane Focus. -25% spell cooldown.",
            "[!]"
        },
        {
            TalentId::WizardArcaneDrain, TalentClass::Wizard, TalentTier::Tier2,
            5, TalentId::WizardWindWalk,
            "Arcane Drain",
            "Steal power from your foes. Requires Wind Walk. +6 bonus XP per kill.",
            "[D]"
        },

        // ── Wizard tier 3 ────────────────────────────────────────────────────
        {
            TalentId::WizardArcaneMastery, TalentClass::Wizard, TalentTier::Tier3,
            10, TalentId::WizardSurge,
            "Arcane Mastery",
            "You have transcended mortal limits. Requires Surge. +75% XP from all sources.",
            "[*]"
        },
    }};

} // namespace ggj

#endif // GALDER_GAME_JAM_2026_TALENTDATA_HPP
