//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP

#include "imgui.h"
#include "../data/ConnectedPlayer.hpp"

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * Host-only in-game ImGui window listing every connected player and their live stats. Built
     * from INetworkManager::connectedPlayers() each frame. Each client has a Kick button that asks
     * for confirmation first; the menu must outlive a frame so that popup survives.
     */
    class ServerInfoMenu
    {
        public:
        /*!
         * Draws the window.
         *
         * @param players    Everyone on the server, host included
         * @param serverName Name shown in the header
         * @param localIp    Host LAN IP
         * @param publicIp   Host public IP
         * @return           Id of the player the host confirmed kicking this frame, if any
         */
        [[nodiscard]] std::optional<uint32_t> draw(const std::vector<ConnectedPlayer>& players,
                                                   const std::string& serverName,
                                                   const std::string& localIp,
                                                   const std::string& publicIp)
        {
            std::optional<uint32_t> kickRequest {};
            ImGui::SetNextWindowSize({ 460, 360 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Server Info");

            ImGui::Text("Server: %s", serverName.c_str());

            drawIpRow("Local IP: ", localIp);
            drawIpRow("Public IP:", publicIp);

            ImGui::Separator();
            ImGui::Text("Connected players: %d", static_cast<int>(players.size()));
            ImGui::Separator();

            constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("players", 7, flags))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Pos");
                ImGui::TableSetupColumn("Coins");
                ImGui::TableSetupColumn("Kills");
                ImGui::TableSetupColumn("Lives");
                ImGui::TableSetupColumn("Ping");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();

                for (const ConnectedPlayer& player : players)
                {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    drawPlayerName(player);
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
                    drawKickButton(player);
                }
                ImGui::EndTable();
            }

            // Addresses live in their own table so the stats table above stays narrow.
            ImGui::Spacing();
            ImGui::TextUnformatted("Network");
            if (ImGui::BeginTable("addresses", 3, flags))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Local IP");
                ImGui::TableSetupColumn("Public IP");
                ImGui::TableHeadersRow();

                for (const ConnectedPlayer& player : players)
                {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    drawPlayerName(player);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(player.localIp.empty() ? "-" : player.localIp.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(player.publicIp.empty() ? "-" : player.publicIp.c_str());
                }
                ImGui::EndTable();
            }

            kickRequest = drawKickConfirmation();

            ImGui::End();
            return kickRequest;
        }

        private:
        static constexpr const char* KICK_POPUP_TITLE = "Kick player?";

        /*! Shows a Kick button for clients; the host cannot kick themselves. */
        void drawKickButton(const ConnectedPlayer& player)
        {
            if (player.isHost)
                return;

            ImGui::PushID(static_cast<int>(player.playerId));
            if (ImGui::SmallButton("Kick"))
            {
                m_pendingKickId = player.playerId;
                m_pendingKickName = player.name;
                m_isKickPopupRequested = true; // Opened at window level: a popup's id depends on the id stack it is opened from.
            }
            ImGui::PopID();
        }

        /*! @return The player the host confirmed kicking, once they press Kick in the popup. */
        [[nodiscard]] std::optional<uint32_t> drawKickConfirmation()
        {
            if (m_isKickPopupRequested)
            {
                ImGui::OpenPopup(KICK_POPUP_TITLE);
                m_isKickPopupRequested = false;
            }

            std::optional<uint32_t> confirmed {};
            if (ImGui::BeginPopupModal(KICK_POPUP_TITLE, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Kick %s from the server?", m_pendingKickName.c_str());
                ImGui::TextUnformatted("They can join again unless you stop hosting.");
                ImGui::Spacing();
                if (ImGui::Button("Kick", { 100, 0 }))
                {
                    confirmed = m_pendingKickId;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", { 100, 0 }))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            return confirmed;
        }

        static void drawPlayerName(const ConnectedPlayer& player)
        {
            ImGui::Text("%s%s", player.name.c_str(), player.isHost ? " (host)" : "");
        }

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

        uint32_t m_pendingKickId { 0 };      ///< Player the open kick popup asks about.
        std::string m_pendingKickName {};
        bool m_isKickPopupRequested { false }; ///< A Kick button was pressed; open the popup next.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
