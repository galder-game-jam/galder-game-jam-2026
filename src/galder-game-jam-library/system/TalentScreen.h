//
// Created by andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_TALENTSCREEN_H
#define GALDER_GAME_JAM_2026_TALENTSCREEN_H

#include <string>
#include <vector>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/KeyboardKey.h"
#include "../enums/PlayerPickedCharacter.h"
#include "../data/TalentData.hpp"
#include "../data/TalentState.hpp"
#include "../data/PlayerPersistentState.hpp"
#include "../interfaces/manager/IOptionsManager.h"

namespace ggj
{
    /*!
     * Full-screen overlay that lets the player browse and purchase talents from
     * their class talent tree.
     *
     * The screen is opened by pressing 'N' during gameplay. Navigation uses the
     * standard arrow-key / WASD scheme; Space or Enter purchases the highlighted
     * talent if the player has an unspent talent point and the tier prerequisites
     * are satisfied.  'N' or Escape closes the screen.
     *
     * WorldManager is responsible for syncing talent data between PlayerStats and
     * PlayerPersistentState before opening and after closing this screen.
     */
    class TalentScreen : public IGameObject<raylib::Vector2>
    {
        public:
            explicit TalentScreen(IInputManager<KeyboardKey>& inputManager,
                                  PlayerPersistentState&      persistentState,
                                  IOptionsManager&            options)
                : m_inputManager(inputManager)
                , m_persistentState(persistentState)
                , m_options(options)
            {
                m_isVisible = true;
            }

            void setFont(raylib::Font* font)       { m_font = font; }

            /*!
             * Sets which character class tree to display. Rebuilds the talent list.
             *
             * @param character The character the local player chose.
             */
            void setPickedCharacter(PlayerPickedCharacter character);

            /*!
             * Informs the screen of the player's current level so tier availability
             * can be computed correctly.
             *
             * @param level Current player level.
             */
            void setCurrentPlayerLevel(int level) { m_currentLevel = level; }

            /*! @return true once the player has closed the talent screen. */
            [[nodiscard]] bool isDone() const { return m_isDone; }

            /*! Resets navigation state so the screen is ready to be shown again. */
            void reset();

            void rotate(float) override {}
            void scale(float) override {}
            void move(const raylib::Vector2&) override {}
            void destroy() override { IGameObject::destroy(); }
            void update(float timeDelta) override;
            void draw() override;

        private:
            [[nodiscard]] bool        canUnlock(const TalentDef& def) const;
            [[nodiscard]] std::string statusLabel(const TalentDef& def) const;
            void buildCharacterTalentList();

            IInputManager<KeyboardKey>& m_inputManager;
            PlayerPersistentState&      m_persistentState;
            IOptionsManager&            m_options;
            raylib::Font*               m_font           { nullptr };
            PlayerPickedCharacter       m_pickedCharacter{ PlayerPickedCharacter::Rogue };
            int                         m_currentLevel   { 1 };
            int                         m_selectedIndex  { 0 };
            bool                        m_isDone         { false };

            std::vector<const TalentDef*> m_characterTalents;

            static constexpr float SCREEN_WIDTH    = 800.0f;
            static constexpr float SCREEN_HEIGHT   = 600.0f;
            static constexpr float PANEL_X         = 100.0f;
            static constexpr float PANEL_Y         = 75.0f;
            static constexpr float PANEL_W         = 600.0f;
            static constexpr float PANEL_H         = 450.0f;
            static constexpr int   TITLE_FONT_SIZE = 26;
            static constexpr int   ITEM_FONT_SIZE  = 20;
            static constexpr int   SMALL_FONT_SIZE = 15;
            static constexpr float ITEM_SPACING    = 26.0f;
    };

} // namespace ggj

#endif // GALDER_GAME_JAM_2026_TALENTSCREEN_H
