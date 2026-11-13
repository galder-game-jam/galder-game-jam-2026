//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_MESSAGEPOPUP_HPP
#define GALDER_GAME_JAM_2026_PROJECT_MESSAGEPOPUP_HPP

#include "imgui.h"

#include <string>

namespace ggj
{
    /*!
     * Centered modal ImGui message box with a single OK button, e.g. "Disconnected from server".
     * Keeps its own open/dismissed state across frames; the owner polls isDismissed() and calls
     * close() once it has reacted.
     */
    class MessagePopup
    {
        public:
        /*!
         * Shows the popup from the next ImGui frame on.
         *
         * @param title Window title, also used as the ImGui popup id
         * @param message Body text
         */
        void open(const std::string& title, const std::string& message)
        {
            m_title = title;
            m_message = message;
            m_isOpen = true;
            m_isDismissed = false;
            m_hasRequestedOpen = false;
        }

        /*! Hides the popup and clears the dismissed flag. */
        void close()
        {
            m_isOpen = false;
            m_isDismissed = false;
            m_hasRequestedOpen = false;
        }

        /*! Marks the popup as acknowledged, as if OK was clicked (e.g. from a keyboard shortcut). */
        void dismiss() { m_isDismissed = true; }

        [[nodiscard]] bool isOpen() const { return m_isOpen; }
        [[nodiscard]] bool isDismissed() const { return m_isDismissed; }

        /*! Draws the popup. Call inside an ImGui frame. */
        void draw()
        {
            if (!m_isOpen)
                return;

            // OpenPopup() must only be called once, re-opening every frame resets the modal.
            if (!m_hasRequestedOpen)
            {
                ImGui::OpenPopup(m_title.c_str());
                m_hasRequestedOpen = true;
            }

            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, { 0.5f, 0.5f });
            if (ImGui::BeginPopupModal(m_title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::TextUnformatted(m_message.c_str());
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("OK", { 120.0f, 0.0f }))
                    m_isDismissed = true;
                ImGui::EndPopup();
            }
        }

        private:
        std::string m_title {};
        std::string m_message {};
        bool m_isOpen { false };
        bool m_isDismissed { false };
        bool m_hasRequestedOpen { false };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_MESSAGEPOPUP_HPP
