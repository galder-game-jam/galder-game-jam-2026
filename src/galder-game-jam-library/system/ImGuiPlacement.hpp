#ifndef GALDER_GAME_JAM_2026_PROJECT_IMGUIPLACEMENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_IMGUIPLACEMENT_HPP

#include "imgui.h"

#include <algorithm>

namespace ggj::imgui
{
    /*!
     * Screen corner/edge a window is anchored to when it first appears.
     */
    enum class Anchor
    {
        Center,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    constexpr float SCREEN_MARGIN = 10.0f; ///< Gap kept between a window and the screen edge.

    /*!
     * Prepares the next ImGui window so it is visible on any window size. Call right before ImGui::Begin().
     *
     * ImGui is rendered at the real (resizable) window resolution, not the 800x600 virtual game
     * resolution, so placement must be derived from the main viewport rather than hardcoded numbers.
     * The window's size is capped to the viewport (minus margin) every frame, so it can never be
     * larger than a small screen; its default size and anchor position apply on first appearance.
     *
     * @param desiredSize Preferred size on first use (ignored for auto-resizing windows)
     * @param anchor      Where the window is placed when it appears
     * @param setSize     False for AlwaysAutoResize windows, which size themselves
     */
    inline void prepareWindow(const ImVec2& desiredSize, const Anchor anchor, const bool setSize = true)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 maxSize { std::max(viewport->WorkSize.x - SCREEN_MARGIN * 2.0f, 50.0f), std::max(viewport->WorkSize.y - SCREEN_MARGIN * 2.0f, 50.0f) };

        ImGui::SetNextWindowSizeConstraints({ 0.0f, 0.0f }, maxSize);
        if (setSize)
        {
            ImGui::SetNextWindowSize({ std::min(desiredSize.x, maxSize.x), std::min(desiredSize.y, maxSize.y) }, ImGuiCond_FirstUseEver);
        }

        const float left = viewport->WorkPos.x + SCREEN_MARGIN;
        const float top = viewport->WorkPos.y + SCREEN_MARGIN;
        const float right = viewport->WorkPos.x + viewport->WorkSize.x - SCREEN_MARGIN;
        const float bottom = viewport->WorkPos.y + viewport->WorkSize.y - SCREEN_MARGIN;

        switch (anchor)
        {
            case Anchor::Center:
                ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, { 0.5f, 0.5f });
                break;
            case Anchor::TopLeft:
                ImGui::SetNextWindowPos({ left, top }, ImGuiCond_Appearing, { 0.0f, 0.0f });
                break;
            case Anchor::TopRight:
                ImGui::SetNextWindowPos({ right, top }, ImGuiCond_Appearing, { 1.0f, 0.0f });
                break;
            case Anchor::BottomLeft:
                ImGui::SetNextWindowPos({ left, bottom }, ImGuiCond_Appearing, { 0.0f, 1.0f });
                break;
            case Anchor::BottomRight:
                ImGui::SetNextWindowPos({ right, bottom }, ImGuiCond_Appearing, { 1.0f, 1.0f });
                break;
        }
    }

    /*!
     * Moves the current window back inside the viewport if it ended up (partly) off screen, e.g. after
     * the game window was shrunk or the user dragged it out. Call right after ImGui::Begin().
     */
    inline void keepWindowOnScreen()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 size = ImGui::GetWindowSize();
        const ImVec2 pos = ImGui::GetWindowPos();

        const float minX = viewport->WorkPos.x;
        const float minY = viewport->WorkPos.y;
        const float maxX = std::max(minX, viewport->WorkPos.x + viewport->WorkSize.x - size.x);
        const float maxY = std::max(minY, viewport->WorkPos.y + viewport->WorkSize.y - size.y);

        const ImVec2 clamped { std::clamp(pos.x, minX, maxX), std::clamp(pos.y, minY, maxY) };
        if (clamped.x != pos.x || clamped.y != pos.y)
            ImGui::SetWindowPos(clamped);
    }
} // namespace ggj::imgui

#endif // GALDER_GAME_JAM_2026_PROJECT_IMGUIPLACEMENT_HPP
