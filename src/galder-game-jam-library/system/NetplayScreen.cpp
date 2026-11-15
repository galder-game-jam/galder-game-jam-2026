//
// Created by robin on 27.06.2026.
//

#include "NetplayScreen.h"

#include "imgui.h"
#include "ImGuiPlacement.hpp"
#include "fmt/format.h"

#include <cstring>

namespace ggj
{
    void NetplayScreen::initialize()
    {
        m_backgroundTexture = m_textures.get(TextureName::MainMenuBackground);
        m_localIp = m_resolver.getLocalIpAddress();
        m_publicIp = m_resolver.getPublicIpAddress();
    }

    void NetplayScreen::reset()
    {
        m_isConfirmed = false;
        m_isBack = false;
    }

    raylib::Color NetplayScreen::getNameColor() const
    {
        return raylib::Color { ColorFromNormalized({ m_nameColor[0], m_nameColor[1], m_nameColor[2], 1.0f }) };
    }

    void NetplayScreen::draw()
    {
        if (!m_isVisible)
            return;

        m_backgroundTexture->Draw(raylib::Vector2 { 0.0f, 0.0f });
    }

    namespace
    {
        void drawIpRow(const char* label, const std::string& ip)
        {
            ImGui::TextUnformatted(label);
            ImGui::SameLine();
            char buf[46] {};
            std::strncpy(buf, ip.c_str(), sizeof(buf) - 1);
            ImGui::SetNextItemWidth(160.0f);
            ImGui::InputText(fmt::format("##{}", label).c_str(), buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if (ImGui::SmallButton(fmt::format("Copy##{}", label).c_str()))
                ImGui::SetClipboardText(ip.c_str());
        }
    } // namespace

    void NetplayScreen::drawImGui()
    {
        if (!m_isVisible)
            return;

        imgui::prepareWindow({ 400.0f, 380.0f }, imgui::Anchor::Center);
        ImGui::Begin("Netplay", nullptr, ImGuiWindowFlags_NoCollapse);
        imgui::keepWindowOnScreen();

        // Sampled before the widgets run: ImGui uses Esc to cancel a text edit, so if a field is being
        // edited the first Esc should only cancel that edit rather than leave the screen.
        const bool isEscapePressed = IsKeyPressed(KEY_ESCAPE) && !ImGui::IsAnyItemActive();

        ImGui::TextUnformatted("Choose your role");
        ImGui::Separator();

        int role = m_isHost ? 0 : 1;
        ImGui::RadioButton("Host (Server)", &role, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Join (Client)", &role, 1);
        m_isHost = (role == 0);

        ImGui::Spacing();
        drawIpRow("Local IP: ", m_localIp);
        drawIpRow("Public IP:", m_publicIp);

        ImGui::Spacing();
        ImGui::TextUnformatted("Player name");
        ImGui::InputText("##name", m_nameBuffer, sizeof(m_nameBuffer));
        ImGui::SameLine();
        ImGui::ColorEdit3("Name color", m_nameColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

        ImGui::Spacing();
        if (m_isHost)
        {
            ImGui::TextUnformatted("Server name");
            ImGui::InputText("##servername", m_serverNameBuffer, sizeof(m_serverNameBuffer));
            ImGui::TextUnformatted("Port");
            ImGui::InputInt("##port", &m_port);
        }
        else
        {
            ImGui::TextUnformatted("Server IP");
            ImGui::InputText("##ip", m_ipBuffer, sizeof(m_ipBuffer));
            ImGui::TextUnformatted("Port");
            ImGui::InputInt("##port", &m_port);

            ImGui::Spacing();
            ImGui::TextUnformatted("Servers found on LAN:");
            if (m_discoveredServers.empty())
            {
                ImGui::TextDisabled("  searching...");
            }
            else
            {
                for (const DiscoveredServer& server : m_discoveredServers)
                {
                    const std::string label = fmt::format("{0}  ({1}:{2})  {3} player(s)##{1}:{2}", server.name, server.ipAddress, server.port, server.playerCount);
                    if (ImGui::Button(label.c_str()))
                    {
                        std::strncpy(m_ipBuffer, server.ipAddress.c_str(), sizeof(m_ipBuffer) - 1);
                        m_ipBuffer[sizeof(m_ipBuffer) - 1] = '\0';
                        m_port = server.port;
                    }
                }
            }
        }

        // Clamp the port to a valid range.
        if (m_port < 1)
            m_port = 1;
        if (m_port > 65535)
            m_port = 65535;

        ImGui::Spacing();
        ImGui::Separator();

        const bool hasName = m_nameBuffer[0] != '\0';
        if (!hasName)
            ImGui::BeginDisabled();
        if (ImGui::Button(m_isHost ? "Host" : "Connect", { 120.0f, 0.0f }))
            m_isConfirmed = true;
        if (!hasName)
            ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Back", { 120.0f, 0.0f }) || isEscapePressed)
            m_isBack = true;

        ImGui::End();
    }
} // namespace ggj
