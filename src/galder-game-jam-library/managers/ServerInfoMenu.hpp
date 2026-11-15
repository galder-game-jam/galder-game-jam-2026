//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP

#include "imgui.h"
#include "../system/ImGuiPlacement.hpp"
#include "../data/ConnectedPlayer.hpp"
#include "../interfaces/manager/INetworkManager.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace ggj
{
    /*! The host's confirmed decision to kick someone, as returned by ServerInfoMenu::draw(). */
    struct KickRequest
    {
        uint32_t playerId { 0 };
        std::string reason {}; ///< Optional free text for the kicked player; empty for none.
    };

    /*!
     * Host-only in-game ImGui window listing every connected player and their live stats. Built
     * from INetworkManager::connectedPlayers() each frame. Each client has a Kick button that asks
     * for confirmation first, with an optional reason the kicked player gets to see; the menu must
     * outlive a frame so that popup survives.
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
         * @return           The kick the host confirmed this frame, if any
         */
        [[nodiscard]] std::optional<KickRequest> draw(const std::vector<ConnectedPlayer>& players,
                                                   const std::string& serverName,
                                                   const std::string& localIp,
                                                   const std::string& publicIp)
        {
            std::optional<KickRequest> kickRequest {};
            imgui::prepareWindow({ 460, 360 }, imgui::Anchor::BottomRight);
            ImGui::Begin("Server Info");
            imgui::keepWindowOnScreen();

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

        /*!
         * @return true while the kick confirmation is open. Its reason box takes text, so the game
         *         should not react to key presses meanwhile.
         */
        [[nodiscard]] bool isKickPopupOpen() const { return m_isKickPopupOpen; }

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
                m_kickReasonBuffer[0] = '\0';
                m_isKickPopupRequested = true; // Opened at window level: a popup's id depends on the id stack it is opened from.
            }
            ImGui::PopID();
        }

        /*! @return The kick the host confirmed, once they press Kick in the popup. */
        [[nodiscard]] std::optional<KickRequest> drawKickConfirmation()
        {
            if (m_isKickPopupRequested)
            {
                ImGui::OpenPopup(KICK_POPUP_TITLE);
                m_isKickPopupRequested = false;
            }

            std::optional<KickRequest> confirmed {};
            m_isKickPopupOpen = false;
            if (ImGui::BeginPopupModal(KICK_POPUP_TITLE, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                m_isKickPopupOpen = true;
                ImGui::Text("Kick %s from the server?", m_pendingKickName.c_str());
                ImGui::TextUnformatted("They can join again unless you stop hosting.");
                ImGui::Spacing();

                ImGui::TextUnformatted("Reason (optional, shown to the player):");
                if (ImGui::IsWindowAppearing())
                    ImGui::SetKeyboardFocusHere();
                ImGui::SetNextItemWidth(360.0f);
                const bool isEnterPressed = ImGui::InputText("##kickReason", m_kickReasonBuffer, sizeof(m_kickReasonBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::Spacing();

                if (ImGui::Button("Kick", { 100, 0 }) || isEnterPressed)
                {
                    confirmed = KickRequest { m_pendingKickId, trimmed(m_kickReasonBuffer) };
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", { 100, 0 }))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            return confirmed;
        }

        /*! @return text without leading / trailing whitespace, so a blank reason counts as no reason. */
        [[nodiscard]] static std::string trimmed(const std::string& text)
        {
            constexpr const char* WHITESPACE = " \t\r\n";
            const size_t first = text.find_first_not_of(WHITESPACE);
            if (first == std::string::npos)
                return {};
            return text.substr(first, text.find_last_not_of(WHITESPACE) - first + 1);
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
        bool m_isKickPopupOpen { false };      ///< The confirmation was open as of the last frame drawn.
        char m_kickReasonBuffer[INetworkManager::MAX_KICK_REASON_BYTES + 1] {}; ///< ImGui edits this in place.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_SERVERINFOMENU_HPP
