//
// Created by andre on 27.06.2026.
//

#include "TalentScreen.h"

namespace ggj
{
    // ── helpers ──────────────────────────────────────────────────────────────────

    void TalentScreen::setPickedCharacter(PlayerPickedCharacter character)
    {
        m_pickedCharacter = character;
        buildCharacterTalentList();
    }

    void TalentScreen::buildCharacterTalentList()
    {
        m_characterTalents.clear();
        const TalentClass cls = (m_pickedCharacter == PlayerPickedCharacter::Rogue)
            ? TalentClass::Rogue
            : TalentClass::Wizard;

        for (const TalentDef& def : ALL_TALENTS)
        {
            if (def.characterClass == cls)
                m_characterTalents.push_back(&def);
        }
    }

    bool TalentScreen::canUnlock(const TalentDef& def) const
    {
        if (m_persistentState.talentState.hasTalent(def.id))
            return false;
        if (m_persistentState.talentPoints <= 0)
            return false;
        if (m_currentLevel < def.requiredLevel)
            return false;
        if (def.prerequisite != TalentId::None
            && !m_persistentState.talentState.hasTalent(def.prerequisite))
            return false;
        return true;
    }

    std::string TalentScreen::statusLabel(const TalentDef& def) const
    {
        if (m_persistentState.talentState.hasTalent(def.id))
            return "UNLOCKED";
        if (m_persistentState.talentPoints <= 0)
            return "NO POINTS";
        if (m_currentLevel < def.requiredLevel)
            return "NEED LVL " + std::to_string(def.requiredLevel);
        if (def.prerequisite != TalentId::None
            && !m_persistentState.talentState.hasTalent(def.prerequisite))
        {
            // Find prerequisite name to show a helpful message
            for (const TalentDef& other : ALL_TALENTS)
            {
                if (other.id == def.prerequisite)
                    return "NEED: " + std::string(other.name);
            }
            return "NEED PREREQ";
        }
        return "AVAILABLE";
    }

    std::vector<std::string> TalentScreen::wrapText(const std::string& text, float maxWidth) const
    {
        std::vector<std::string> lines;
        std::string current;
        size_t pos = 0;
        while (pos < text.size())
        {
            size_t end = text.find(' ', pos);
            if (end == std::string::npos)
                end = text.size();
            const std::string word = text.substr(pos, end - pos);
            pos = end + 1;

            const std::string candidate = current.empty() ? word : current + " " + word;
            const Vector2 size = MeasureTextEx(*m_font, candidate.c_str(), static_cast<float>(SMALL_FONT_SIZE), 0);
            if (size.x > maxWidth && !current.empty())
            {
                lines.push_back(current);
                current = word;
            }
            else
                current = candidate;
        }
        if (!current.empty())
            lines.push_back(current);
        return lines;
    }

    void TalentScreen::reset()
    {
        m_selectedIndex = 0;
        m_isDone        = false;
    }

    // ── update ────────────────────────────────────────────────────────────────

    void TalentScreen::update(float /*timeDelta*/)
    {
        if (m_characterTalents.empty())
            return;

        const int count = static_cast<int>(m_characterTalents.size());

        if (m_inputManager.keyPressed(KeyboardKey::Up) || m_inputManager.keyPressed(KeyboardKey::W))
            m_selectedIndex = (m_selectedIndex - 1 + count) % count;

        if (m_inputManager.keyPressed(KeyboardKey::Down) || m_inputManager.keyPressed(KeyboardKey::S))
            m_selectedIndex = (m_selectedIndex + 1) % count;

        if (m_inputManager.keyPressed(KeyboardKey::Escape) || m_inputManager.keyPressed(m_options.getKeybindingOptions().toggleTalentTree))
        {
            m_isDone = true;
            return;
        }

        if (m_inputManager.keyPressed(KeyboardKey::Space) || m_inputManager.keyPressed(KeyboardKey::Enter))
        {
            const TalentDef* selected = m_characterTalents[m_selectedIndex];
            if (canUnlock(*selected))
            {
                m_persistentState.talentState.unlockTalent(selected->id);
                --m_persistentState.talentPoints;
            }
        }
    }

    // ── draw ──────────────────────────────────────────────────────────────────

    void TalentScreen::draw()
    {
        if (!m_isVisible || m_font == nullptr)
            return;

        // Dim the world behind the panel
        DrawRectangle(0, 0,
            static_cast<int>(SCREEN_WIDTH),
            static_cast<int>(SCREEN_HEIGHT),
            Color { 0, 0, 0, 170 });

        // Panel background
        DrawRectangle(
            static_cast<int>(PANEL_X), static_cast<int>(PANEL_Y),
            static_cast<int>(PANEL_W), static_cast<int>(PANEL_H),
            Color { 10, 10, 30, 220 });

        // Panel border
        DrawRectangleLines(
            static_cast<int>(PANEL_X), static_cast<int>(PANEL_Y),
            static_cast<int>(PANEL_W), static_cast<int>(PANEL_H),
            DARKGRAY);

        // Title
        const std::string className = (m_pickedCharacter == PlayerPickedCharacter::Rogue)
            ? "ROGUE" : "WIZARD";
        const std::string title = "TALENT TREE  -  " + className;
        const Vector2 titleSize = MeasureTextEx(*m_font, title.c_str(),
            static_cast<float>(TITLE_FONT_SIZE), 0);
        const float titleX = PANEL_X + (PANEL_W - titleSize.x) * 0.5f;
        m_font->DrawText(title, { titleX, PANEL_Y + 14.0f },
            static_cast<float>(TITLE_FONT_SIZE), 0, GOLD);

        // Talent points
        const std::string pts = "Talent Points: " + std::to_string(m_persistentState.talentPoints);
        const Vector2 ptsSize = MeasureTextEx(*m_font, pts.c_str(),
            static_cast<float>(SMALL_FONT_SIZE), 0);
        const float ptsX = PANEL_X + (PANEL_W - ptsSize.x) * 0.5f;
        const raylib::Color ptsColor = (m_persistentState.talentPoints > 0) ? YELLOW : LIGHTGRAY;
        m_font->DrawText(pts, { ptsX, PANEL_Y + 46.0f },
            static_cast<float>(SMALL_FONT_SIZE), 0, ptsColor);

        // Talent list
        float itemY = PANEL_Y + 78.0f;
        TalentTier lastTier = static_cast<TalentTier>(0);

        for (int i = 0; i < static_cast<int>(m_characterTalents.size()); ++i)
        {
            const TalentDef& def = *m_characterTalents[i];
            const bool isSelected = (i == m_selectedIndex);

            // Tier header when tier changes
            if (def.tier != lastTier)
            {
                lastTier = def.tier;
                const std::string tierLabel = "  Tier " + std::to_string(static_cast<int>(def.tier));
                m_font->DrawText(tierLabel, { PANEL_X + 14.0f, itemY },
                    static_cast<float>(SMALL_FONT_SIZE), 0, DARKGRAY);
                itemY += ITEM_SPACING - 4.0f;
            }

            // Cursor
            const std::string cursor = isSelected ? ">" : " ";

            // Status-driven color
            raylib::Color nameColor = DARKGRAY;
            if (m_persistentState.talentState.hasTalent(def.id))
                nameColor = GREEN;
            else if (canUnlock(def))
                nameColor = isSelected ? YELLOW : LIGHTGRAY;

            // Build label: "  [S] Blade Mastery"
            const std::string label = cursor + " " + std::string(def.icon)
                + " " + std::string(def.name);
            m_font->DrawText(label, { PANEL_X + 20.0f, itemY },
                static_cast<float>(ITEM_FONT_SIZE), 0, nameColor);

            // Status on the right
            const std::string status = statusLabel(def);
            const Vector2 statusSize = MeasureTextEx(*m_font, status.c_str(),
                static_cast<float>(SMALL_FONT_SIZE), 0);
            const float statusX = PANEL_X + PANEL_W - statusSize.x - 18.0f;
            raylib::Color statusColor = DARKGRAY;
            if (m_persistentState.talentState.hasTalent(def.id))
                statusColor = GREEN;
            else if (canUnlock(def))
                statusColor = YELLOW;
            m_font->DrawText(status, { statusX, itemY + 3.0f },
                static_cast<float>(SMALL_FONT_SIZE), 0, statusColor);

            itemY += ITEM_SPACING;
        }

        // Description of selected talent
        if (!m_characterTalents.empty())
        {
            const TalentDef& sel = *m_characterTalents[m_selectedIndex];
            const float descY = PANEL_Y + PANEL_H - 100.0f;

            DrawLine(
                static_cast<int>(PANEL_X + 14),  static_cast<int>(descY - 6),
                static_cast<int>(PANEL_X + PANEL_W - 14), static_cast<int>(descY - 6),
                DARKGRAY);

            const std::vector<std::string> lines = wrapText(std::string(sel.description), PANEL_W - 40.0f);
            for (int line = 0; line < static_cast<int>(lines.size()) && line < DESC_MAX_LINES; ++line)
            {
                m_font->DrawText(lines[line],
                    { PANEL_X + 20.0f, descY + static_cast<float>(line) * DESC_LINE_HEIGHT },
                    static_cast<float>(SMALL_FONT_SIZE), 0, LIGHTGRAY);
            }
        }

        // Footer
        const std::string footer = "Space/Enter: Unlock     N / Esc: Close";
        const Vector2 footerSize = MeasureTextEx(*m_font, footer.c_str(),
            static_cast<float>(SMALL_FONT_SIZE), 0);
        const float footerX = PANEL_X + (PANEL_W - footerSize.x) * 0.5f;
        m_font->DrawText(footer,
            { footerX, PANEL_Y + PANEL_H - 40.0f },
            static_cast<float>(SMALL_FONT_SIZE), 0, DARKGRAY);
    }

} // namespace ggj
