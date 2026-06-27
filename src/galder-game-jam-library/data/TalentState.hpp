//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_TALENTSTATE_HPP
#define GALDER_GAME_JAM_2026_TALENTSTATE_HPP

#include "TalentData.hpp"
#include <unordered_set>

namespace ggj
{
    /*!
     * The set of talents a player has unlocked.
     *
     * Stored as raw uint32_t values (the underlying type of TalentId) so the
     * set works without a custom hash specialization.
     */
    struct TalentState
    {
        TalentState() = default;

        std::unordered_set<uint32_t> unlockedTalents {}; ///< Raw TalentId values.

        [[nodiscard]] bool hasTalent(TalentId id) const
        {
            return unlockedTalents.count(static_cast<uint32_t>(id)) > 0;
        }

        void unlockTalent(TalentId id)
        {
            unlockedTalents.insert(static_cast<uint32_t>(id));
        }
    };

    /*!
     * Derives the active gameplay effects from a player's unlocked talents.
     *
     * Effects from multiple talents are composed multiplicatively where it
     * makes sense (speed, XP, attack speed) and additively for flat bonuses.
     *
     * @param state The player's current talent selection.
     * @return      Composed TalentEffects ready to apply each frame.
     */
    [[nodiscard]] inline TalentEffects computeTalentEffects(const TalentState& state)
    {
        TalentEffects fx {};

        // ── Rogue ─────────────────────────────────────────────────────────────
        if (state.hasTalent(TalentId::RogueBladeMastery))
            fx.attackSpeedMultiplier *= 0.75f;    // -25 % frames

        if (state.hasTalent(TalentId::RogueFleetStep))
            fx.speedMultiplier *= 1.20f;

        if (state.hasTalent(TalentId::RogueFlurry))
            fx.attackSpeedMultiplier *= 0.80f;    // additional -20 % frames

        if (state.hasTalent(TalentId::RoguePredator))
            fx.killsPerExtraLife = 10;

        if (state.hasTalent(TalentId::RogueShadowsGift))
            fx.xpMultiplier *= 1.50f;

        // ── Wizard ────────────────────────────────────────────────────────────
        if (state.hasTalent(TalentId::WizardArcaneFocus))
            fx.projectileSpeedMultiplier *= 1.35f;

        if (state.hasTalent(TalentId::WizardWindWalk))
            fx.speedMultiplier *= 1.20f;

        if (state.hasTalent(TalentId::WizardSurge))
            fx.attackSpeedMultiplier *= 0.75f;    // -25 % cooldown

        if (state.hasTalent(TalentId::WizardArcaneDrain))
            fx.bonusXpPerKill = 6;

        if (state.hasTalent(TalentId::WizardArcaneMastery))
            fx.xpMultiplier *= 1.75f;

        return fx;
    }

} // namespace ggj

#endif // GALDER_GAME_JAM_2026_TALENTSTATE_HPP
