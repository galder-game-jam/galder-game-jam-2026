//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CLIENTINFOMENU_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CLIENTINFOMENU_HPP

#include "imgui.h"
#include "../system/ImGuiPlacement.hpp"
#include "../interfaces/manager/INetworkManager.h"
#include "raylib-cpp.hpp"

#include <cstring>
#include <string>

namespace ggj
{
    /*!
     * Client-only in-game ImGui window showing the connected server name, this client's IPs,
     * and a table of all players (local + remote) with Name, Pos, Coins, Lives, Ping.
     */
    class ClientInfoMenu
    {
        public:
        void draw(const INetworkManager& network,
                  const std::string& localPlayerName,
                  const raylib::Vector2& localPlayerPos,
                  int32_t localLives,
                  int32_t localScore)
        {
            imgui::prepareWindow({ 480, 300 }, imgui::Anchor::BottomRight);
            ImGui::Begin("Client Info");
            imgui::keepWindowOnScreen();

            ImGui::Text("Connected to: %s", network.serverName().c_str());

            drawIpRow("Local IP: ", network.localIpAddress());
            drawIpRow("Public IP:", network.publicIpAddress());

            ImGui::Separator();

            constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("players", 5, flags))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Pos");
                ImGui::TableSetupColumn("Coins");
                ImGui::TableSetupColumn("Lives");
                ImGui::TableSetupColumn("Ping");
                ImGui::TableHeadersRow();

                // Local player row
                {
                    const int32_t myPing = network.pingToServer();
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s (me)", localPlayerName.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d, %d", static_cast<int>(localPlayerPos.x), static_cast<int>(localPlayerPos.y));
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", localScore);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", localLives);
                    ImGui::TableNextColumn();
                    if (myPing >= 0)
                        ImGui::Text("%d ms", myPing);
                    else
                        ImGui::TextUnformatted("-");
                }

                // Remote players
                for (const auto& [id, remote] : network.remotePlayers())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(remote.name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d, %d", static_cast<int>(remote.position.x), static_cast<int>(remote.position.y));
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", remote.score);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", remote.lives);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("-");
                }

                ImGui::EndTable();
            }

            ImGui::End();
        }

        private:
        static void drawIpRow(const char* label, const std::string& ip)
        {
            ImGui::TextUnformatted(label);
            ImGui::SameLine();
            char buf[46] {};
            std::strncpy(buf, ip.c_str(), sizeof(buf) - 1);
            ImGui::SetNextItemWidth(160.0f);
            ImGui::InputText(label, buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            const std::string copyLabel = std::string("Copy##") + label;
            if (ImGui::SmallButton(copyLabel.c_str()))
                ImGui::SetClipboardText(ip.c_str());
        }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CLIENTINFOMENU_HPP
