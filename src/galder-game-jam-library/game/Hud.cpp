//
// Created by robin on 22.11.2025.
//

#include "Hud.h"
#include "Minimap.h"
#include "../graphics/CharacterPalette.h"

namespace ggj
{
    void Hud::initialize()
    {
        m_color = raylib::Color::White();
        m_size = raylib::Vector2 { 200, 20 };
        m_origin = raylib::Vector2 { 20, 20 };
        m_position = raylib::Vector2 { 5, 0 };

        m_portraitOffset = raylib::Vector2 { m_position.x + 0, m_position.y + 50 };
        m_playerPositionOffset = raylib::Vector2 { 704.0f, 556.0f }; // 96 px wide (3×32), right-aligned, just above XP bar
        // Placed below the minimap (right side): minimap is at x=590, y=10, 200×150.
        m_questLogOffset = raylib::Vector2 { 590.0f, 10.0f + 150.0f + 6.0f };
        m_playerPortraitOffset = raylib::Vector2 { m_portraitOffset.x + 3, m_portraitOffset.y + 3 };

        m_hudTexture = m_textures.get(TextureName::HudRpgSubset);
        m_playerPortraitTexture = m_textures.get(TextureName::HudRpgRoguePortrait);
        m_playerPortraitMaskTexture = m_textures.get(TextureName::HudRpgRoguePortraitMask);
    }

    void Hud::setFont(raylib::Font* font)
    {
        m_font = font;
    }

    void Hud::setPlayer(IPlayer* player)
    {
        m_currentPlayer = player;
    }

    void Hud::setPlayerName(const std::string& name)
    {
        m_playerName = name;
    }

    void Hud::setPickedCharacter(PlayerPickedCharacter character, const CharacterColors& colors)
    {
        const bool isWizard = character == PlayerPickedCharacter::Wizard;
        m_playerPortraitTexture = m_textures.get(isWizard ? TextureName::HudRpgWizardPortrait : TextureName::HudRpgRoguePortrait);
        m_playerPortraitMaskTexture = m_textures.get(isWizard ? TextureName::HudRpgWizardPortraitMask : TextureName::HudRpgRoguePortraitMask);
        m_playerPortraitRecolor = CharacterPalette::BuildMaskRecolor(isWizard ? PlayerPickedCharacter::Wizard : PlayerPickedCharacter::Rogue, colors);
    }

    void Hud::setWorld(World* world)
    {
        m_currentWorld = world;
    }

    void Hud::setMinimap(Minimap* minimap)
    {
        m_minimap = minimap;
    }

    void Hud::rotate(float degrees) {}

    void Hud::scale(float scale) {}

    void Hud::move(const raylib::Vector2& toMove) {}

    void Hud::destroy()
    {
        IGameObject::destroy();
    }

    void Hud::update(float timeDelta)
    {
        if (m_inputManager.keyPressed(KeyboardKey::F8))
            setVisible(!m_isVisible);

        HudOptions hudOpts = m_options.getHudOptions();
        bool hudOptsChanged = false;
        if (m_inputManager.keyPressed(m_options.getKeybindingOptions().toggleQuestLog))
        {
            hudOpts.showQuestLog = !hudOpts.showQuestLog;
            hudOptsChanged = true;
        }
        if (m_inputManager.keyPressed(m_options.getKeybindingOptions().toggleMinimap))
        {
            hudOpts.showMinimap = !hudOpts.showMinimap;
            hudOptsChanged = true;
        }
        if (hudOptsChanged)
            m_options.setHudOptions(hudOpts);

        if (m_inputManager.keyPressed(m_options.getKeybindingOptions().toggleInventory))
        {
            HudOptions inv = m_options.getHudOptions();
            inv.showInventory = !inv.showInventory;
            m_options.setHudOptions(inv);
        }

        if (m_currentPlayer == nullptr)
            return;

        m_health = static_cast<float>(m_currentPlayer->getLives());
        m_mana = 2;   // Implement mana and then: static_cast<float>(m_currentPlayer->getMana());
        m_energy = 3; // Implement energy and then: static_cast<float>(m_currentPlayer->getEnergy());
        m_coins = m_currentPlayer->getScore();
        m_level       = m_currentPlayer->getLevel();
        m_currentXp   = m_currentPlayer->getCurrentXp();
        m_totalXp     = m_currentPlayer->getTotalXp();
        m_xpToNextLevel = m_currentPlayer->getXpToNextLevel();
        m_playerPosition = m_currentPlayer->getVisualCenter();
    }

    raylib::Color Hud::tinted(raylib::Color color) const
    {
        const float alpha = m_options.getHudOptions().hudAlpha;
        return raylib::Color { color.r, color.g, color.b, static_cast<unsigned char>(alpha * static_cast<float>(color.a)) };
    }

    void Hud::draw()
    {
        if (!m_isVisible)
            return;

        drawPlayerPortrait();
        if (m_options.getHudOptions().showInventory)
            drawPlayerPositionBar(m_playerPositionOffset);
        drawXpBar();
        if (m_options.getHudOptions().showQuestLog)
            drawQuestLog(m_questLogOffset, 4, 1);

        if (!m_playerName.empty() && m_font != nullptr)
            m_font->DrawText(m_playerName, raylib::Vector2 { m_position.x + 8, m_position.y + 6 }, 24, 0, tinted(m_playerNameColor));

        if (m_minimap != nullptr && m_options.getHudOptions().showMinimap)
        {
            std::vector<MinimapMarker> markers = m_currentWorld != nullptr ? m_currentWorld->getRemotePlayerMarkers() : std::vector<MinimapMarker> {};
            markers.push_back(MinimapMarker { m_playerPosition, m_playerNameColor, true });
            m_minimap->draw(markers, m_options.getHudOptions().hudAlpha);

            if (m_font != nullptr)
            {
                static constexpr float MINIMAP_RIGHT        = 800.0f - 10.0f - 2.0f;  // right edge minus padding
                static constexpr float MINIMAP_BOTTOM = 10.0f + 150.0f - 15.0f - 2.0f; // inside bottom edge
                const char* posText = TextFormat("Pos: (%i,%i)", static_cast<int>(m_playerPosition.x), static_cast<int>(m_playerPosition.y));
                const raylib::Vector2 textSize = MeasureTextEx(*m_font, posText, static_cast<float>(FONT_SIZE), 0.0f);
                m_font->DrawText(posText, raylib::Vector2 { MINIMAP_RIGHT - textSize.x, MINIMAP_BOTTOM }, FONT_SIZE, 0, tinted(WHITE));
            }
        }

        if (m_currentWorld != nullptr && m_currentWorld->hasBoss())
            drawBossBar();
    }

    void Hud::drawQuestLog(raylib::Vector2 offset, int width, int height) const
    {
        float grid = TILE_UNIT * 2;
        raylib::Vector2 shrinkflation = raylib::Vector2(4, 4);

        raylib::Rectangle backdrop = raylib::Rectangle(offset + shrinkflation, raylib::Vector2((width + 2) * grid, (height + 2) * grid) - shrinkflation * 2);
        DrawRectangle(backdrop.x, backdrop.y, backdrop.width, backdrop.height, tinted(raylib::Color(143, 86, 59, 180)));

        const raylib::Color texTint = tinted(WHITE);
        raylib::Rectangle tiles[3][3] = { { m_boxTopLeftRect, m_boxTopMiddleRect, m_boxTopRightRect },
                                          { m_boxMiddleLeftRect, m_boxMiddleMiddleRect, m_boxMiddleRightRect },
                                          { m_boxBottomLeftRect, m_boxBottomMiddleRect, m_boxBottomRightRect } };

        for (int y = 0; y < height + 2; ++y)
        {
            for (int x = 0; x < width + 2; ++x)
            {
                int tx = (x == 0) ? 0 : (x == width + 1 ? 2 : 1);  // left/mid/right tile
                int ty = (y == 0) ? 0 : (y == height + 1 ? 2 : 1); // top/mid/bottom tile

                const raylib::Rectangle& src = tiles[ty][tx];
                raylib::Vector2 pos = offset + raylib::Vector2 { grid * x, grid * y };

                m_hudTexture->Draw(src, pos, texTint);
            }
        }

        int enemies = m_currentWorld->getEnemies();
        int enemiesLeft = m_currentWorld->getNumberOfEnemiesLeft();

        int enemiesKilled = enemies - enemiesLeft;
        bool isOpen = enemiesKilled == enemies;

        m_font->DrawText(TextFormat("Quest Log"), raylib::Vector2 { offset.x + TILE_UNIT, offset.y + 7 }, FONT_SIZE, 0, tinted(YELLOW));
        m_font->DrawText(TextFormat("Kill enemies %i/%i", enemiesKilled, enemies), raylib::Vector2 { offset.x + TILE_UNIT, offset.y + 27 }, FONT_SIZE, 0, tinted(BLACK));
        m_font->DrawText(TextFormat("Portal is %s", isOpen ? "open" : "closed"), raylib::Vector2 { offset.x + TILE_UNIT, offset.y + 67 }, FONT_SIZE, 0, tinted(isOpen ? GREEN : RED));
    }

    void Hud::DrawResourceBar(const float counter, raylib::Rectangle rectResource, raylib::Rectangle rectEnd, float barNo) const
    {
        const raylib::Color texTint = tinted(WHITE);
        float barTypeOffset = 20 * barNo;
        for (int i = 0; i < counter; ++i)
        {
            m_hudTexture->Draw(rectResource, raylib::Vector2 { m_portraitRect.width + rectResource.width * i + m_portraitOffset.x, 6 + barTypeOffset + m_portraitOffset.y }, texTint);
        }
        m_hudTexture->Draw(rectEnd, raylib::Vector2 { m_portraitRect.width + rectResource.width * counter + m_portraitOffset.x, 2 + barTypeOffset + m_portraitOffset.y }, texTint);
    }

    void Hud::drawPlayerPortrait() const
    {
        const raylib::Color texTint = tinted(WHITE);
        // Without a mask the portrait is still drawn, just in its painted colours.
        if (m_playerPortraitMaskTexture != nullptr)
            m_maskRecolor.begin(m_playerPortraitRecolor, *m_playerPortraitMaskTexture);
        m_playerPortraitTexture->Draw(m_playerPortraitOffset, texTint);
        if (m_playerPortraitMaskTexture != nullptr)
            m_maskRecolor.end();
        m_hudTexture->Draw(m_portraitRect, m_portraitOffset, texTint);

        DrawResourceBar(m_health, m_lifeRect, m_lifeEnd, 0);
        DrawResourceBar(m_mana, m_manaRect, m_manaEnd, 1);
        DrawResourceBar(m_energy, m_energyRect, m_energyEnd, 2);

        static constexpr float BADGE_RADIUS = 12.0f;
        const float cx = m_portraitOffset.x + m_portraitRect.width - BADGE_RADIUS - 20.0f;
        const float cy = m_portraitOffset.y + m_portraitRect.height - BADGE_RADIUS;
        DrawCircle(static_cast<int>(cx), static_cast<int>(cy), BADGE_RADIUS + 1.5f, tinted(BLACK));
        DrawCircle(static_cast<int>(cx), static_cast<int>(cy), BADGE_RADIUS, tinted(Color { 70, 40, 10, 255 }));
        const char* levelText = TextFormat("%i", m_level);
        const raylib::Vector2 textSize = MeasureTextEx(*m_font, levelText, static_cast<float>(FONT_SIZE), 0.0f);
        m_font->DrawText(levelText,
                         raylib::Vector2 { cx - textSize.x * 0.5f, cy - textSize.y * 0.5f },
                         FONT_SIZE, 0, tinted(GOLD));
    }

    void Hud::drawBar(raylib::Vector2 offset, int middleParts) const
    {
        const raylib::Color texTint = tinted(WHITE);
        m_hudTexture->Draw(m_barRectLeft, offset, texTint);
        for (int i = 0; i < middleParts; ++i)
        {
            m_hudTexture->Draw(m_barRectMiddle, offset + raylib::Vector2 { TILE_UNIT * 2 + TILE_UNIT * 2 * i, 0 }, texTint);
        }

        m_hudTexture->Draw(m_barRectRight, offset + raylib::Vector2 { TILE_UNIT * 2 + TILE_UNIT * 2 * middleParts, 0 }, texTint);
    }

    void Hud::drawPlayerPositionBar(raylib::Vector2 offset) const
    {
        drawBar(offset, 1); // (1+2)×32 = 96 px — compact coin widget
        m_hudTexture->Draw(m_coinRect, offset + raylib::Vector2 { TILE_UNIT * 1, 4 }, tinted(WHITE));
        m_font->DrawText(TextFormat("%i", static_cast<int>(m_coins)),
                         raylib::Vector2 { offset.x + TILE_UNIT * 3, offset.y + 5 }, FONT_SIZE, 0, tinted(BLACK));
    }

    void Hud::drawXpBar() const
    {
        if (m_font == nullptr)
            return;

        static constexpr float BAR_Y      = 582.0f;
        static constexpr float BAR_HEIGHT = 18.0f;
        static constexpr float BAR_WIDTH  = 800.0f;
        static constexpr int   SEPARATORS = 12;

        DrawRectangle(0, static_cast<int>(BAR_Y), static_cast<int>(BAR_WIDTH), static_cast<int>(BAR_HEIGHT),
                      tinted(Color { 0, 0, 0, 160 }));

        const float ratio      = m_xpToNextLevel > 0
                                 ? static_cast<float>(m_currentXp) / static_cast<float>(m_xpToNextLevel)
                                 : 0.0f;
        const float safeRatio  = ratio < 0.0f ? 0.0f : (ratio > 1.0f ? 1.0f : ratio);
        const int   fillWidth  = static_cast<int>(BAR_WIDTH * safeRatio);
        if (fillWidth > 0)
        {
            DrawRectangle(0, static_cast<int>(BAR_Y), fillWidth, static_cast<int>(BAR_HEIGHT),
                          tinted(Color { 100, 0, 180, 230 }));
            DrawRectangle(0, static_cast<int>(BAR_Y), fillWidth, 3,
                          tinted(Color { 170, 80, 255, 180 }));
        }

        for (int i = 1; i <= SEPARATORS; ++i)
        {
            const int x = static_cast<int>(BAR_WIDTH * static_cast<float>(i) / static_cast<float>(SEPARATORS + 1));
            DrawRectangle(x, static_cast<int>(BAR_Y), 2, static_cast<int>(BAR_HEIGHT),
                          tinted(Color { 30, 30, 30, 200 }));
        }

        const char* xpText         = TextFormat("%i / %i XP", m_currentXp, m_xpToNextLevel);
        const raylib::Vector2 size = MeasureTextEx(*m_font, xpText, static_cast<float>(FONT_SIZE), 0.0f);
        m_font->DrawText(xpText,
                         raylib::Vector2 { (BAR_WIDTH - size.x) * 0.5f,
                                           BAR_Y + (BAR_HEIGHT - size.y) * 0.5f },
                         FONT_SIZE, 0, tinted(WHITE));
    }

    void Hud::drawBossBar() const
    {
        const int maxLives = m_currentWorld->getBossMaxLives();
        const int curLives = m_currentWorld->getBossLives();
        if (maxLives <= 0)
            return;

        // All HUD drawing happens inside a fixed-size render texture (800×600).
        // GetScreenWidth() returns the actual window size and must not be used here.
        const int RENDER_W  = 800;
        const int BAR_W     = 400;
        const int BAR_H     = 20;
        const int BAR_X     = (RENDER_W - BAR_W) / 2;
        const int NAME_Y    = 8;
        const int BAR_Y     = NAME_Y + FONT_SIZE + 4;

        DrawRectangle(BAR_X, BAR_Y, BAR_W, BAR_H, tinted(raylib::Color { 40, 40, 40, 200 }));

        const int fillW = (curLives * BAR_W) / maxLives;
        DrawRectangle(BAR_X, BAR_Y, fillW, BAR_H, tinted(raylib::Color { 200, 30, 30, 220 }));

        DrawRectangleLines(BAR_X, BAR_Y, BAR_W, BAR_H, tinted(raylib::Color::White()));

        const std::string& name = m_currentWorld->getBossName();
        if (m_font != nullptr && !name.empty())
        {
            const raylib::Vector2 nameSize = m_font->MeasureText(name, FONT_SIZE, 0);
            const float nameX = static_cast<float>((RENDER_W - static_cast<int>(nameSize.x)) / 2);
            m_font->DrawText(name, raylib::Vector2 { nameX, static_cast<float>(NAME_Y) }, FONT_SIZE, 0, tinted(raylib::Color::White()));
        }
    }
} // namespace ggj
