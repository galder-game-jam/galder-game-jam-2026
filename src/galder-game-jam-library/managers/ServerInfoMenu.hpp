//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP

#include "imgui.h"
#include "../data/ConnectedPlayer.hpp"

#include <cstring>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * Host-only in-game ImGui window listing every connected player and their live stats. Built
     * from INetworkManager::connectedPlayers() each frame.
     */
    class ServerInfoMenu
    {
        public:
        void draw(const std::vector<ConnectedPlayer>& players,
                  const std::string& serverName,
                  const std::string& localIp,
                  const std::string& publicIp)
        {
            ImGui::SetNextWindowSize({ 560, 280 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Server Info");

            ImGui::Text("Server: %s", serverName.c_str());

            drawIpRow("Local IP: ", localIp);
            drawIpRow("Public IP:", publicIp);

            ImGui::Separator();
            ImGui::Text("Connected players: %d", static_cast<int>(players.size()));
            ImGui::Separator();

            constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("players", 8, flags))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Pos");
                ImGui::TableSetupColumn("Coins");
                ImGui::TableSetupColumn("Kills");
                ImGui::TableSetupColumn("Lives");
                ImGui::TableSetupColumn("Ping");
                ImGui::TableSetupColumn("Local IP");
                ImGui::TableSetupColumn("Public IP");
                ImGui::TableHeadersRow();

                for (const ConnectedPlayer& player : players)
                {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s%s", player.name.c_str(), player.isHost ? " (host)" : "");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d, %d", static_cast<int>(player.position.x), static_cast<int>(player.position.y));
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", player.score);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", player.kills);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", player.lives);
                    ImGui::TableNextColumn();
                    if (player.pingMs >= 0)
                        ImGui::Text("%d ms", player.pingMs);
                    else
                        ImGui::TextUnformatted("-");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(player.localIp.empty() ? "-" : player.localIp.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(player.publicIp.empty() ? "-" : player.publicIp.c_str());
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

#endif // GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
