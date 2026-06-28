//
// Created by robin on 26.06.26.
//

#pragma once

#ifdef GAME_DEV_DEBUG

    #include "rlImGui.h"
    #include "imgui.h"
    #include "raylib.h"
    #include "../interfaces/manager/IOptionsManager.h"
    #include "../data/KeybindingOptions.hpp"
    #include "../../gamedev-base/interfaces/managers/ISoundManager.h"

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
        bool gainLife       { false }; ///< Consumer grants one extra life to player 0 and resets to false.
        bool loseLife       { false }; ///< Consumer removes one life from player 0 and resets to false.
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
        void draw(DebugState& state, IOptionsManager& options, ISoundManager& sound)
        {
            // Capture the next key press when a rebind is in progress.
            if (m_rebindingAction >= 0)
            {
                const int rawKey = GetKeyPressed();
                if (rawKey != 0 && rawKey != static_cast<int>(KeyboardKey::Escape))
                {
                    KeybindingOptions kb = options.getKeybindingOptions();
                    KeyboardKey* targets[] = {
                        &kb.moveUp, &kb.moveDown, &kb.moveLeft,
                        &kb.moveRight, &kb.attack, &kb.respawn,
                        &kb.toggleQuestLog,
                        &kb.toggleMinimap,
                        &kb.toggleTalentTree,
                        &kb.toggleInventory,
                    };
                    *targets[m_rebindingAction] = static_cast<KeyboardKey>(rawKey);
                    options.setKeybindingOptions(kb);
                    m_rebindingAction = -1;
                }
                else if (rawKey == static_cast<int>(KeyboardKey::Escape))
                {
                    m_rebindingAction = -1;
                }
            }
            ImGui::SetNextWindowSize({ 300, 660 }, ImGuiCond_FirstUseEver);
            ImGui::Begin("Debug Menu");

            ImGui::Text("UI Scale");
            ImGui::Separator();
            {
                constexpr float SCALES[] = { 1.0f, 1.5f, 2.0f, 4.0f };
                constexpr const char* LABELS[] = { "1x", "1.5x", "2x", "4x" };
                HudOptions scaleOpts = options.getHudOptions();
                for (int i = 0; i < 4; ++i)
                {
                    if (i > 0) ImGui::SameLine();
                    const bool active = (scaleOpts.imguiScale == SCALES[i]);
                    if (active)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    if (ImGui::Button(LABELS[i], { 0.0f, 0.0f }))
                    {
                        scaleOpts.imguiScale = SCALES[i];
                        options.setHudOptions(scaleOpts);
                    }
                    if (active)
                        ImGui::PopStyleColor();
                }
            }

            ImGui::Spacing();
            ImGui::Text("Map");
            ImGui::Separator();
            if (ImGui::Button("Map 1"))
                state.requestedStage = 1;
            ImGui::SameLine();
            if (ImGui::Button("Map 2"))
                state.requestedStage = 2;
            ImGui::SameLine();
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

                if (ImGui::Button("Award Coin"))
                    state.awardCoin = true;

                if (ImGui::Button("Kill All Enemies"))
                    state.killAllEnemies = true;

                if (ImGui::Button("+1 Life"))
                    state.gainLife = true;
                ImGui::SameLine();
                if (ImGui::Button("-1 Life"))
                    state.loseLife = true;
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
            bool hudChanged = false;
            hudChanged |= ImGui::SliderFloat("HUD Alpha##hud", &hudOpts.hudAlpha, 0.0f, 1.0f);
            hudChanged |= ImGui::Checkbox("Show Minimap##hud", &hudOpts.showMinimap);
            hudChanged |= ImGui::Checkbox("Show Quest Log##hud", &hudOpts.showQuestLog);
            hudChanged |= ImGui::Checkbox("Show Inventory##hud", &hudOpts.showInventory);
            if (hudChanged)
                options.setHudOptions(hudOpts);

            ImGui::Spacing();
            ImGui::Text("Audio");
            ImGui::Separator();
            AudioOptions audioOpts = options.getAudioOptions();
            bool audioChanged = false;
            audioChanged |= ImGui::Checkbox("Mute Music", &audioOpts.isMusicMuted);
            audioChanged |= ImGui::SliderFloat("Music Volume", &audioOpts.musicVolume, 0.0f, 1.0f);
            audioChanged |= ImGui::Checkbox("Mute Sounds", &audioOpts.isSoundMuted);
            audioChanged |= ImGui::SliderFloat("Sound Volume", &audioOpts.soundVolume, 0.0f, 1.0f);
            if (audioChanged)
            {
                options.setAudioOptions(audioOpts);
                sound.applyAudioOptions();
            }

            ImGui::Spacing();
            ImGui::Text("Keybindings");
            ImGui::Separator();

            struct KeybindRow { const char* label; KeyboardKey key; int index; };
            const KeybindingOptions& kb = options.getKeybindingOptions();
            const KeybindRow rows[] = {
                { "Move Up",    kb.moveUp,    0 },
                { "Move Down",  kb.moveDown,  1 },
                { "Move Left",  kb.moveLeft,  2 },
                { "Move Right", kb.moveRight, 3 },
                { "Attack",           kb.attack,          4 },
                { "Respawn",          kb.respawn,         5 },
                { "Toggle Quest Log",  kb.toggleQuestLog,   6 },
                { "Toggle Minimap",    kb.toggleMinimap,    7 },
                { "Toggle Talent Tree", kb.toggleTalentTree, 8 },
                { "Toggle Inventory",   kb.toggleInventory,    9 },
            };
            if (ImGui::BeginTable("##dktable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                for (const KeybindRow& row : rows)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(row.label);
                    ImGui::TableSetColumnIndex(1);
                    char btnLabel[32];
                    if (m_rebindingAction == row.index)
                        snprintf(btnLabel, sizeof(btnLabel), "Press a key...##dk%d", row.index);
                    else
                        snprintf(btnLabel, sizeof(btnLabel), "%-10s##dk%d", keyName(row.key), row.index);
                    if (ImGui::Button(btnLabel, { 0.0f, 0.0f }))
                        m_rebindingAction = row.index;
                }
                ImGui::EndTable();
            }
            if (ImGui::Button("Reset to default##dk"))
            {
                m_rebindingAction = -1;
                options.setKeybindingOptions(KeybindingOptions {});
            }

            ImGui::End();

            if (state.showImGuiDemo)
                ImGui::ShowDemoWindow(&state.showImGuiDemo);
        }

        private:
        int m_rebindingAction { -1 }; ///< Index of the action being rebound, or -1 when idle.
    };

} // namespace ggj

#endif // GAME_DEV_DEBUG
