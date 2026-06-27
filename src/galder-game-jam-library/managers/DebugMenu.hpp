//
// Created by robin on 26.06.26.
//

#pragma once

#ifdef GAME_DEV_DEBUG

    #include "rlImGui.h"
    #include "imgui.h"
    #include "../interfaces/manager/IOptionsManager.h"

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
        int  requestedStage { -1 };  ///< -1 = no request; consumer resets to -1 after reading.
        bool gainXp         { false }; ///< Consumer awards 100 XP to player 0 and resets to false.
        bool gainLevel      { false }; ///< Consumer awards exactly enough XP to level up player 0, then resets to false.
        bool killAllEnemies { false }; ///< Consumer destroys all remaining enemies in the current level and resets to false.
        bool awardCoin      { false }; ///< Consumer adds one coin to player 0's score and resets to false.
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
             * @param state   Shared debug state to read from and write to.
             * @param options Options manager for reading and tweaking runtime settings.
             */
        void draw(DebugState& state, IOptionsManager& options)
        {
            ImGui::SetNextWindowSize({ 300, 260 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Debug Menu");

            ImGui::Text("Map");
            ImGui::Separator();
            if (ImGui::Button("Map 1"))
                state.requestedStage = 1;
            ImGui::SameLine();
            if (ImGui::Button("Map 2"))
                state.requestedStage = 2;
            if (ImGui::Button("Map 3"))
                state.requestedStage = 3;
            ImGui::SameLine();
            if (ImGui::Button("Map 4"))
                state.requestedStage = 4;
            if (ImGui::Button("Map 5"))
                state.requestedStage = 5;
            ImGui::SameLine();
            if (ImGui::Button("Final"))
                state.requestedStage = 99999;

            ImGui::Spacing();
            ImGui::Text("Player");
            ImGui::Separator();
            if (state.isGameActive)
            {
                if (ImGui::Button("Gain 100 XP"))
                    state.gainXp = true;
                ImGui::SameLine();
                if (ImGui::Button("Increase Level"))
                    state.gainLevel = true;
                if (ImGui::Button("Kill All Enemies"))
                    state.killAllEnemies = true;
                ImGui::SameLine();
                if (ImGui::Button("Award Coin"))
                    state.awardCoin = true;
            }
            else
            {
                ImGui::TextDisabled("(start a game first)");
            }

            ImGui::Spacing();
            ImGui::Text("Options");
            ImGui::Separator();
            ImGui::Checkbox("Show hitboxes", &state.showHitboxes);
            ImGui::Checkbox("Camera follows player", &state.cameraFollowsPlayer);
            ImGui::Checkbox("Show ImGui demo", &state.showImGuiDemo);

            ImGui::Spacing();
            ImGui::Text("HUD");
            ImGui::Separator();
            HudOptions hudOpts = options.getHudOptions();
            if (ImGui::SliderFloat("HUD Alpha##hud", &hudOpts.hudAlpha, 0.0f, 1.0f))
                options.setHudOptions(hudOpts);

            ImGui::End();

            if (state.showImGuiDemo)
                ImGui::ShowDemoWindow(&state.showImGuiDemo);
        }
    };

} // namespace ggj

#endif // GAME_DEV_DEBUG
