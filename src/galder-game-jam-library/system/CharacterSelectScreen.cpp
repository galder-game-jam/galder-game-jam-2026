//
// Created by robin on 23.11.2025.
//

#include "CharacterSelectScreen.h"
#include "../graphics/CharacterPalette.h"
#include "imgui.h"
#include "rlImGui.h"

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
        m_roguePortraitMaskTexture = m_textures.get(TextureName::CharacterSelectRoguePortraitMask);
        m_wizardPortraitMaskTexture = m_textures.get(TextureName::CharacterSelectWizardPortraitMask);

        m_rogueSpriteTexture = m_textures.get(TextureName::RoguePlayer);
        m_wizardSpriteTexture = m_textures.get(TextureName::WizardPlayer);
        // Both sprite sheets share the Rogue animation layout.
        m_previewAnimation = m_animations.getAnimation(AnimationName::RogueIdle);
        if (m_previewTexture == nullptr)
            m_previewTexture = std::make_unique<raylib::RenderTexture>(PREVIEW_SPRITE_SIZE, PREVIEW_SPRITE_SIZE);

        m_rogueColors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Rogue);
        m_wizardColors = CharacterPalette::GetDefaultColors(PlayerPickedCharacter::Wizard);
    }

    bool CharacterSelectScreen::isCharacterSelected() const
    {
        return m_isCharacterSelected;
    }

    PlayerPickedCharacter CharacterSelectScreen::getPickedCharacter() const
    {
        return m_pickedCharacter;
    }

    const CharacterColors& CharacterSelectScreen::getPickedColors() const
    {
        return (m_pickedCharacter == PlayerPickedCharacter::Wizard) ? m_wizardColors : m_rogueColors;
    }

    CharacterColors& CharacterSelectScreen::pickedColors()
    {
        return (m_pickedCharacter == PlayerPickedCharacter::Wizard) ? m_wizardColors : m_rogueColors;
    }

    void CharacterSelectScreen::reset()
    {
        m_isCharacterSelected = false;
    }

    void CharacterSelectScreen::setFont(raylib::Font* font)
    {
        m_font = font;
    }

    void CharacterSelectScreen::setPlayer(IPlayer* player)
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
        m_previewAnimation.update(timeDelta);

        // Typing in a colour picker's text field must not switch or confirm the character.
        if (ImGui::GetIO().WantCaptureKeyboard)
            return;

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

        if (m_inputManager.keyPressed(KeyboardKey::Space) || m_inputManager.keyPressed(KeyboardKey::Enter))
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
        drawPortrait(m_position + raylib::Vector2{6,2 * GRID_UNIT + 26});

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

    void CharacterSelectScreen::drawPortrait(const raylib::Vector2& position) const
    {
        const raylib::Texture* mask = (m_pickedCharacter == PlayerPickedCharacter::Wizard) ? m_wizardPortraitMaskTexture : m_roguePortraitMaskTexture;
        // Rebuilt every frame so the portrait follows the colour pickers live, like the idle preview.
        // Without a mask the portrait is still drawn, just in its painted colours.
        if (mask != nullptr)
            m_maskRecolor.begin(CharacterPalette::BuildMaskRecolor(m_pickedCharacter, getPickedColors()), *mask);
        m_currentPortraitTexture->Draw(position);
        if (mask != nullptr)
            m_maskRecolor.end();
    }

    void CharacterSelectScreen::renderPreview()
    {
        raylib::Texture* texture = (m_pickedCharacter == PlayerPickedCharacter::Wizard) ? m_wizardSpriteTexture : m_rogueSpriteTexture;
        if (texture == nullptr || m_previewTexture == nullptr)
            return;

        m_previewTexture->BeginMode();
        ClearBackground(BLANK);
        m_paletteSwap.begin(CharacterPalette::BuildPaletteSwap(m_pickedCharacter, getPickedColors()));
        texture->Draw(m_previewAnimation.getDrawingRect(), raylib::Vector2 { 0.0f, 0.0f });
        m_paletteSwap.end();
        m_previewTexture->EndMode();
    }

    void CharacterSelectScreen::drawImGui()
    {
        if (!m_isVisible)
            return;

        // Rendered here rather than in draw(): draw() runs inside the game's render texture, and
        // raylib texture modes can't be nested.
        renderPreview();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2 { viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f }, ImGuiCond_Appearing, ImVec2 { 1.0f, 0.0f });
        if (ImGui::Begin("Customize colors", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_previewTexture != nullptr)
            {
                constexpr float PREVIEW_DRAW_SIZE = static_cast<float>(PREVIEW_SPRITE_SIZE) * PREVIEW_SCALE;
                const float previewX = (ImGui::GetContentRegionAvail().x - PREVIEW_DRAW_SIZE) * 0.5f;
                if (previewX > 0.0f)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + previewX);

                // Render textures are stored upside down, hence the negative source height.
                const Rectangle source { 0.0f, 0.0f, static_cast<float>(PREVIEW_SPRITE_SIZE), -static_cast<float>(PREVIEW_SPRITE_SIZE) };
                rlImGuiImageRect(&m_previewTexture->texture, static_cast<int>(PREVIEW_DRAW_SIZE), static_cast<int>(PREVIEW_DRAW_SIZE), source);
            }

            CharacterColors& colors = pickedColors();
            const std::vector<PaletteSlot>& slots = CharacterPalette::GetSlots(m_pickedCharacter);
            for (size_t i = 0; i < slots.size() && i < CharacterColors::MAX_SLOTS; ++i)
            {
                ::Color& color = colors.slots[i];
                float rgb[3] { static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f, static_cast<float>(color.b) / 255.0f };
                // Swatch only (the picker opens on click) keeps the window narrow enough to sit beside the dialog.
                if (ImGui::ColorEdit3(slots[i].label, rgb, ImGuiColorEditFlags_NoInputs))
                {
                    color.r = static_cast<unsigned char>(rgb[0] * 255.0f + 0.5f);
                    color.g = static_cast<unsigned char>(rgb[1] * 255.0f + 0.5f);
                    color.b = static_cast<unsigned char>(rgb[2] * 255.0f + 0.5f);
                    color.a = 255;
                }
            }

            if (ImGui::Button("Reset to default"))
                colors = CharacterPalette::GetDefaultColors(m_pickedCharacter);
        }
        ImGui::End();
    }
} // namespace ggj
