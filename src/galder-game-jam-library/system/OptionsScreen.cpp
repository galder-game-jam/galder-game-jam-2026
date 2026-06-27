#include "OptionsScreen.h"

#include "imgui.h"
#include "raylib.h"

namespace ggj
{
    void OptionsScreen::initialize()
    {
        m_backgroundTexture = m_textures.get(TextureName::MainMenuBackground);
    }

    void OptionsScreen::reset()
    {
        m_isBack = false;
        m_rebindingAction = -1;
        m_localOptions = m_options.getAudioOptions();
        m_localHudOptions = m_options.getHudOptions();
        m_localKeybindingOptions = m_options.getKeybindingOptions();
    }

    void OptionsScreen::draw()
    {
        if (!m_isVisible)
            return;

        m_backgroundTexture->Draw(raylib::Vector2 { 0.0f, 0.0f });
    }

    void OptionsScreen::drawImGui()
    {
        if (!m_isVisible)
            return;

        const ImVec2 size { 360.0f, 490.0f };
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowPos(
            { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f },
            ImGuiCond_Always,
            { 0.5f, 0.5f });
        ImGui::Begin("Options", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        bool audioChanged = false;

        ImGui::TextUnformatted("Music");
        ImGui::Separator();
        audioChanged |= ImGui::Checkbox("Mute Music", &m_localOptions.isMusicMuted);
        ImGui::BeginDisabled(m_localOptions.isMusicMuted);
        audioChanged |= ImGui::SliderFloat("Music Volume##music", &m_localOptions.musicVolume, 0.0f, 1.0f);
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::TextUnformatted("Sound Effects");
        ImGui::Separator();
        audioChanged |= ImGui::Checkbox("Mute Sound Effects", &m_localOptions.isSoundMuted);
        ImGui::BeginDisabled(m_localOptions.isSoundMuted);
        audioChanged |= ImGui::SliderFloat("Sound Volume##sound", &m_localOptions.soundVolume, 0.0f, 1.0f);
        ImGui::EndDisabled();

        if (audioChanged)
        {
            m_options.setAudioOptions(m_localOptions);
            m_sound.applyAudioOptions();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("HUD");
        ImGui::Separator();
        if (ImGui::SliderFloat("HUD Alpha##hud", &m_localHudOptions.hudAlpha, 0.0f, 1.0f))
            m_options.setHudOptions(m_localHudOptions);

        ImGui::Spacing();
        ImGui::TextUnformatted("Keybindings");
        ImGui::Separator();

        // Capture next key press when a rebind is in progress.
        if (m_rebindingAction >= 0)
        {
            const int rawKey = GetKeyPressed();
            if (rawKey != 0 && rawKey != static_cast<int>(KeyboardKey::Escape))
            {
                KeyboardKey* targets[] = {
                    &m_localKeybindingOptions.moveUp,
                    &m_localKeybindingOptions.moveDown,
                    &m_localKeybindingOptions.moveLeft,
                    &m_localKeybindingOptions.moveRight,
                    &m_localKeybindingOptions.attack,
                    &m_localKeybindingOptions.respawn,
                };
                *targets[m_rebindingAction] = static_cast<KeyboardKey>(rawKey);
                m_options.setKeybindingOptions(m_localKeybindingOptions);
                m_rebindingAction = -1;
            }
            else if (rawKey == static_cast<int>(KeyboardKey::Escape))
            {
                m_rebindingAction = -1;
            }
        }

        struct KeybindRow { const char* label; KeyboardKey key; int index; };
        const KeybindRow rows[] = {
            { "Move Up",    m_localKeybindingOptions.moveUp,    0 },
            { "Move Down",  m_localKeybindingOptions.moveDown,  1 },
            { "Move Left",  m_localKeybindingOptions.moveLeft,  2 },
            { "Move Right", m_localKeybindingOptions.moveRight, 3 },
            { "Attack",     m_localKeybindingOptions.attack,    4 },
            { "Respawn",    m_localKeybindingOptions.respawn,   5 },
        };

        for (const KeybindRow& row : rows)
        {
            ImGui::TextUnformatted(row.label);
            ImGui::SameLine(120.0f);

            char btnLabel[32];
            if (m_rebindingAction == row.index)
                snprintf(btnLabel, sizeof(btnLabel), "Press a key...##kb%d", row.index);
            else
                snprintf(btnLabel, sizeof(btnLabel), "%-10s##kb%d", keyName(row.key), row.index);

            if (ImGui::Button(btnLabel, { 160.0f, 0.0f }))
                m_rebindingAction = row.index;
        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Back", { 120.0f, 0.0f }))
        {
            m_rebindingAction = -1;
            m_options.saveAudioOptions();
            m_isBack = true;
        }

        ImGui::End();
    }
} // namespace ggj
