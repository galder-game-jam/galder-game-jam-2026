//
// Created by robin on 22.11.2025.
//

#include "Hud.h"

namespace ggj
{
    void Hud::initialize()
    {
        m_color = raylib::Color::White();
        m_size = raylib::Vector2{200, 20};
        m_origin = raylib::Vector2{20, 20};
        m_position = raylib::Vector2{5, 0};

        m_portraitOffset = raylib::Vector2{m_position.x + 0, m_position.y + 50};
        m_coinOffset = raylib::Vector2{m_position.x + 0, m_position.y + 120};
        m_killsOffset = raylib::Vector2{m_position.x + 0, m_position.y + 152};
        m_playerPositionOffset = raylib::Vector2{m_position.x + 570, m_position.y + 570};
        m_questLogOffset = raylib::Vector2{m_position.x + 0, m_position.y + 400};
        m_playerPortraitOffset = raylib::Vector2{m_portraitOffset.x + 3, m_portraitOffset.y + 3};


        m_hudTexture = m_textures.get(TextureName::HudRpgSubset);
        m_playerPortraitTexture = m_textures.get(TextureName::HudRpgRoguePortrait);
    }

    void Hud::setFont(raylib::Font* font)
    {
        m_font = font;
    }

    void Hud::setPlayer(Player* player)
    {
        m_currentPlayer = player;
    }

    void Hud::setWorld(World* world)
    {
        m_currentWorld = world;
    }

    void Hud::rotate(float degrees)
    {
    }

    void Hud::scale(float scale)
    {
    }

    void Hud::move(const raylib::Vector2& toMove)
    {
    }

    void Hud::destroy()
    {
        IGameObject::destroy();
    }

    void Hud::update(float timeDelta)
    {
        if (m_inputManager.keyPressed(KeyboardKey::F8))
        {
            setVisible(!m_isVisible);
        }

        m_health = static_cast<float>(m_currentPlayer->getLives());
        m_mana = 2; // Implement mana and then: static_cast<float>(m_currentPlayer->getMana());
        m_energy = 3; // Implement energy and then: static_cast<float>(m_currentPlayer->getEnergy());
        m_coins = m_currentPlayer->getScore();
        m_kills = m_currentPlayer->getUserData()->enemiesKilled;
        m_playerPosition = raylib::Vector2{m_currentPlayer->getPosition()};
    }

    void Hud::draw()
    {
        if (!m_isVisible) return;

        drawPlayerPortrait();
        drawCoinBar(m_coinOffset);
        drawPlayerPositionBar(m_playerPositionOffset);
        drawKillsBar(m_killsOffset);

        drawQuestLog(m_questLogOffset, 3, 1);
    }

    void Hud::drawQuestLog(raylib::Vector2 offset, int width, int height) const
    {
        float grid = TILE_UNIT * 2;
        raylib::Vector2 shrinkflation = raylib::Vector2(4, 4);

        raylib::Rectangle backdrop = raylib::Rectangle(offset + shrinkflation,
                                                       raylib::Vector2((width + 2) * grid, (height + 2) * grid) -
                                                       shrinkflation * 2);
        DrawRectangle(backdrop.x, backdrop.y, backdrop.width, backdrop.height, raylib::Color(143, 86, 59, HUD_ALPHA));

        raylib::Rectangle tiles[3][3] =
        {
            {m_boxTopLeftRect, m_boxTopMiddleRect, m_boxTopRightRect},
            {m_boxMiddleLeftRect, m_boxMiddleMiddleRect, m_boxMiddleRightRect},
            {m_boxBottomLeftRect, m_boxBottomMiddleRect, m_boxBottomRightRect}
        };

        for (int y = 0; y < height + 2; ++y)
        {
            for (int x = 0; x < width + 2; ++x)
            {
                int tx = (x == 0) ? 0 : (x == width + 1 ? 2 : 1); // left/mid/right tile
                int ty = (y == 0) ? 0 : (y == height + 1 ? 2 : 1); // top/mid/bottom tile

                const raylib::Rectangle& src = tiles[ty][tx];
                raylib::Vector2 pos = offset + raylib::Vector2{grid * x, grid * y};

                m_hudTexture->Draw(src, pos);
            }
        }

        int enemies = m_currentWorld->getEnemies();
        int enemiesLeft = m_currentWorld->getNumberOfEnemiesLeft();

        int enemiesKilled = enemies - enemiesLeft;
        bool isOpen = enemiesKilled == enemies;

        m_font->DrawText(TextFormat("Quest Log"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 7}, FONT_SIZE, 0,YELLOW);
        m_font->DrawText(TextFormat("Kill enemies %i/%i", enemiesKilled, enemies),raylib::Vector2{offset.x + TILE_UNIT, offset.y + 27}, FONT_SIZE, 0, BLACK);
        m_font->DrawText(TextFormat("Portal is %s", isOpen ? "open" : "closed"),raylib::Vector2{offset.x + TILE_UNIT, offset.y + 67}, FONT_SIZE, 0, isOpen ? GREEN : RED);
    }

    void Hud::DrawResourceBar(const float counter, raylib::Rectangle rectResource, raylib::Rectangle rectEnd,
                              float barNo) const
    {
        float barTypeOffset = 20 * barNo;
        for (int i = 0; i < counter; ++i)
        {
            m_hudTexture->Draw(rectResource, raylib::Vector2{m_portraitRect.width  + rectResource.width * i + m_portraitOffset.x,6 + barTypeOffset + m_portraitOffset.y});
        }
        m_hudTexture->Draw(rectEnd, raylib::Vector2{m_portraitRect.width + rectResource.width * counter + m_portraitOffset.x,2 + barTypeOffset + m_portraitOffset.y});
    }


    void Hud::drawPlayerPortrait() const
    {
        m_playerPortraitTexture->Draw(m_playerPortraitOffset);
        m_hudTexture->Draw(m_portraitRect, m_portraitOffset);

        DrawResourceBar(m_health, m_lifeRect, m_lifeEnd, 0);
        DrawResourceBar(m_mana, m_manaRect, m_manaEnd, 1);
        DrawResourceBar(m_energy, m_energyRect, m_energyEnd, 2);
    }

    void Hud::drawBar(raylib::Vector2 offset, int middleParts) const
    {
        m_hudTexture->Draw(m_barRectLeft, offset);
        for (int i = 0; i < middleParts; ++i)
        {
            m_hudTexture->Draw(m_barRectMiddle, offset + raylib::Vector2{TILE_UNIT * 2 + TILE_UNIT * 2 * i,0});
        }

        m_hudTexture->Draw(m_barRectRight, offset + raylib::Vector2{TILE_UNIT * 2 + TILE_UNIT * 2 * middleParts,0});
    }

    void Hud::drawCoinBar(raylib::Vector2 offset) const
    {
        drawBar(offset, 1);
        m_hudTexture->Draw(m_coinRect, offset + raylib::Vector2{TILE_UNIT * 1,4});

        m_font->DrawText(TextFormat("%i", static_cast<int>(m_coins)), raylib::Vector2{offset.x + TILE_UNIT * 3, offset.y + 5}, FONT_SIZE, 0, BLACK);
    }

    void Hud::drawKillsBar(raylib::Vector2 offset) const
    {
        drawBar(offset, 1);
        m_font->DrawText(TextFormat("Kills: %i", static_cast<int>(m_kills)), raylib::Vector2{offset.x + TILE_UNIT * 1, offset.y + 5}, FONT_SIZE, 0, BLACK);
    }

    void Hud::drawPlayerPositionBar(raylib::Vector2 offset) const
    {
        drawBar(offset, 5);
        m_font->DrawText(TextFormat("Player position: (%i, %i)", static_cast<int>(m_playerPosition.x), static_cast<int>(m_playerPosition.y)), raylib::Vector2{offset.x + TILE_UNIT * 1, offset.y + 5}, FONT_SIZE, 0, BLACK);
    }
} // ggj
