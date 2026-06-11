//
// Created by robin on 23.11.2025.
//

#include "CharacterSelectScreen.h"

namespace ggj
{
    void CharacterSelectScreen::initialize()
    {
        m_color = raylib::Color::White();
        m_size = raylib::Vector2{200, 20};
        m_origin = raylib::Vector2{20, 20};
        m_position = raylib::Vector2{GRID_UNIT*4 + TILE_UNIT, GRID_UNIT*3};

        m_hudTexture = m_textures.get(TextureName::HudRpgSubset);
        m_roguePortraitTexture = m_textures.get(TextureName::CharacterSelectRoguePortrait);
        m_wizardPortraitTexture = m_textures.get(TextureName::CharacterSelectWizardPortrait);
        m_currentPortraitTexture = m_roguePortraitTexture;
    }

    bool CharacterSelectScreen::isCharacterSelected()
    {
        return m_isCharacterSelected;
    }

    void CharacterSelectScreen::setFont(raylib::Font* font)
    {
        m_font = font;
    }

    void CharacterSelectScreen::setPlayer(Player* player)
    {
        m_currentPlayer = player;
    }

    void CharacterSelectScreen::setWorld(World* world)
    {
        m_currentWorld = world;
    }

    void CharacterSelectScreen::rotate(float degrees)
    {
    }

    void CharacterSelectScreen::scale(float scale)
    {
    }

    void CharacterSelectScreen::move(const raylib::Vector2& toMove)
    {
    }

    void CharacterSelectScreen::destroy()
    {
        IGameObject::destroy();
    }

    void CharacterSelectScreen::update(float timeDelta)
    {
        if (m_inputManager.keyPressed(KeyboardKey::A) || m_inputManager.keyPressed(KeyboardKey::Left) ||
            m_inputManager.keyPressed(KeyboardKey::D) || m_inputManager.keyPressed(KeyboardKey::Right))
        {
            m_pickedCharacter = (m_pickedCharacter == PlayerPickedCharacter::Rogue) ? PlayerPickedCharacter::Wizard : PlayerPickedCharacter::Rogue;
            m_currentPortraitTexture = (m_pickedCharacter == PlayerPickedCharacter::Rogue) ? m_roguePortraitTexture : m_wizardPortraitTexture;
            m_className = (m_pickedCharacter == PlayerPickedCharacter::Rogue) ? "ROGUE" : "WIZARD";
            m_weaponType = (m_pickedCharacter == PlayerPickedCharacter::Rogue) ? "MELEE" : "RANGED";
            m_name = (m_pickedCharacter == PlayerPickedCharacter::Rogue) ? "GALDER" : "GALDRON" ;

            if (m_pickedCharacter == PlayerPickedCharacter::Rogue)
            {
                m_lore = {
                    "GALDER, THE CUNNING ROGUE,",
                    "HAILS FROM THE SHADOWY ALLEYS",
                    "OF GALDARIA. WITH UNMATCHED",
                    "STEALTH AND AGILITY, HE",
                    "NAVIGATES THE DARKEST",
                    "CORNERS OF THE REALM,",
                    "SEEKING FORTUNE AND JUSTICE."
                };
            }
            else
            {
                m_lore = {
                    "GALDRON, THE WISE WIZARD,",
                    "ORIGINATES FROM THE ANCIENT",
                    "TOWERS OF GALDOR. MASTERING",
                    "THE ARCANE ARTS, HE WIELDS",
                    "POWERFUL SPELLS TO PROTECT",
                    "THE REALM FROM DARK FORCES,",
                    "GUIDED BY KNOWLEDGE AND ",
                    "WISDOM."
                };
            }
        }

        if (m_inputManager.keyPressed(KeyboardKey::Space))
        {
            m_isCharacterSelected = true;
        }

    }

    void CharacterSelectScreen::draw()
    {
        if (!m_isVisible)
            return;

        //Title
        drawDialogBox(m_position + raylib::Vector2{0,0 * GRID_UNIT}, 14, 0, BLACK);
        m_font->DrawText(m_title, m_position + raylib::Vector2{GRID_UNIT * 3 + TILE_UNIT,0 * GRID_UNIT + TILE_UNIT}, FONT_SIZE, 0, WHITE);

        //Portrait
        drawDialogBox(m_position + raylib::Vector2{0,2 * GRID_UNIT}, 4, 4, BLACK);
        m_currentPortraitTexture->Draw(m_position + raylib::Vector2{6,2 * GRID_UNIT + 26});

        //Class
        drawDialogBox(m_position + raylib::Vector2{0,8 * GRID_UNIT}, 4, 0, BLACK);
        m_font->DrawText(m_className, m_position + raylib::Vector2{1 * GRID_UNIT + TILE_UNIT,8 * GRID_UNIT + TILE_UNIT}, FONT_SIZE, 0, WHITE);

        //Weapon type
        drawDialogBox(m_position + raylib::Vector2{0,10 * GRID_UNIT}, 4, 0, BLACK);
        m_font->DrawText(m_weaponType, m_position + raylib::Vector2{1 * GRID_UNIT + TILE_UNIT,10 * GRID_UNIT + TILE_UNIT}, FONT_SIZE, 0, WHITE);


        //Name dialog
        drawDialogBox(m_position + raylib::Vector2{6 * GRID_UNIT,2 * GRID_UNIT}, 1, 0, BLACK);
        m_font->DrawText("NAME", m_position + raylib::Vector2{6 * GRID_UNIT + 10,2 * GRID_UNIT + TILE_UNIT}, FONT_SIZE, 0, WHITE);

        drawDialogBox(m_position + raylib::Vector2{9 * GRID_UNIT,2 * GRID_UNIT}, 5, 0, BLACK);
        m_font->DrawText(m_name, m_position + raylib::Vector2{9 * GRID_UNIT + TILE_UNIT,2 * GRID_UNIT + TILE_UNIT}, FONT_SIZE, 0, WHITE);

        //Lore
        drawDialogBox(m_position + raylib::Vector2{6 * GRID_UNIT,4 * GRID_UNIT}, 8, 6, BLACK);
        for (size_t i = 0; i < m_lore.size(); ++i)
        {
            m_font->DrawText(m_lore[i], m_position + raylib::Vector2{6 * GRID_UNIT + TILE_UNIT, (4 + i) * GRID_UNIT + TILE_UNIT - (i * 4)}, FONT_SIZE - 10, 0, WHITE);
        }
    }

    void CharacterSelectScreen::drawDialogBox(raylib::Vector2 offset, int width, int height,
        const raylib::Color& backgroundColor) const
    {
        float grid = TILE_UNIT * 2;
        raylib::Vector2 shrinkflation = raylib::Vector2(4, 4);

        raylib::Rectangle backdrop = raylib::Rectangle(offset + shrinkflation,
                                                       raylib::Vector2((width + 2) * grid, (height + 2) * grid) -
                                                       shrinkflation * 2);
        DrawRectangle(backdrop.x, backdrop.y, backdrop.width, backdrop.height, backgroundColor);

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
    }
} // ggj