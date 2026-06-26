//
// Created by robin on 26.06.26.
//

#pragma once

#ifdef GAME_DEV_DEBUG

    #include "rlImGui.h"
    #include "imgui.h"

namespace ggj
{
    /*!
     * Shared debug state written by DebugMenu and read by World/WorldManager each frame.
     */
    struct DebugState
    {
        /*!
         * User-declared default constructor so the type is not an aggregate.
         *
         * Boost.DI aggregate-injects scalar members of aggregates (zero-initializing
         * each one), which would silently override the in-class initializers below.
         * Declaring a constructor forces Boost.DI to call it, preserving the defaults.
         */
        DebugState() = default;

        bool isGameActive { false }; ///< Set to true once character select is dismissed.
        bool showHitboxes { false };
        bool cameraFollowsPlayer { true };
        bool showImGuiDemo { false };
        int requestedStage { -1 }; ///< -1 = no request; consumer resets to -1 after reading.
    };

    /*!
     * Renders the ImGui debug window and updates DebugState in response to user input.
     * Only present in builds where GAME_DEV_DEBUG is defined.
     */
    class DebugMenu
    {
        public:
        /*!
             * Draws the debug menu window.
             *
             * @param state Shared debug state to read from and write to.
             */
        void draw(DebugState& state)
        {
            ImGui::SetNextWindowSize({ 300, 200 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Debug Menu");

            ImGui::Text("Map");
            ImGui::Separator();
            if (ImGui::Button("Map 1"))
                state.requestedStage = 1;
            ImGui::SameLine();
            if (ImGui::Button("Map 2"))
                state.requestedStage = 2;
            ImGui::SameLine();
            if (ImGui::Button("Final"))
                state.requestedStage = 99999;

            ImGui::Spacing();
            ImGui::Text("Options");
            ImGui::Separator();
            ImGui::Checkbox("Show hitboxes", &state.showHitboxes);
            ImGui::Checkbox("Camera follows player", &state.cameraFollowsPlayer);
            ImGui::Checkbox("Show ImGui demo", &state.showImGuiDemo);

            ImGui::End();

            if (state.showImGuiDemo)
                ImGui::ShowDemoWindow(&state.showImGuiDemo);
        }
    };

} // namespace ggj

#endif // GAME_DEV_DEBUG
