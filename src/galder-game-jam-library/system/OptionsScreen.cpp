#include "OptionsScreen.h"

#include "imgui.h"

namespace ggj
{
    void OptionsScreen::initialize()
    {
        m_backgroundTexture = m_textures.get(TextureName::MainMenuBackground);
    }

    void OptionsScreen::reset()
    {
        m_isBack = false;
        m_localOptions = m_options.getAudioOptions();
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

        const ImVec2 size { 360.0f, 230.0f };
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowPos(
            { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f },
            ImGuiCond_Always,
            { 0.5f, 0.5f });
        ImGui::Begin("Sound Options", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        bool changed = false;

        ImGui::TextUnformatted("Music");
        ImGui::Separator();
        changed |= ImGui::Checkbox("Mute Music", &m_localOptions.isMusicMuted);
        ImGui::BeginDisabled(m_localOptions.isMusicMuted);
        changed |= ImGui::SliderFloat("Music Volume##music", &m_localOptions.musicVolume, 0.0f, 1.0f);
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::TextUnformatted("Sound Effects");
        ImGui::Separator();
        changed |= ImGui::Checkbox("Mute Sound Effects", &m_localOptions.isSoundMuted);
        ImGui::BeginDisabled(m_localOptions.isSoundMuted);
        changed |= ImGui::SliderFloat("Sound Volume##sound", &m_localOptions.soundVolume, 0.0f, 1.0f);
        ImGui::EndDisabled();

        if (changed)
        {
            m_options.setAudioOptions(m_localOptions);
            m_sound.applyAudioOptions();
        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Back", { 120.0f, 0.0f }))
        {
            m_options.saveAudioOptions();
            m_isBack = true;
        }

        ImGui::End();
    }
} // namespace ggj
