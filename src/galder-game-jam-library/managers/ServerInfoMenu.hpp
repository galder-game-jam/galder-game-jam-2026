//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP

#include "imgui.h"
#include "../data/ConnectedPlayer.hpp"

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
        void draw(const std::vector<ConnectedPlayer>& players)
        {
            ImGui::SetNextWindowSize({ 380, 220 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Server Info");

            ImGui::Text("Connected players: %d", static_cast<int>(players.size()));
            ImGui::Separator();

            constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("players", 6, flags))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Pos");
                ImGui::TableSetupColumn("Coins");
                ImGui::TableSetupColumn("Kills");
                ImGui::TableSetupColumn("Lives");
                ImGui::TableSetupColumn("Ping");
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
                }
                ImGui::EndTable();
            }

            ImGui::End();
        }
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
