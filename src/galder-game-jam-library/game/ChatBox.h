//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CHATBOX_H
#define GALDER_GAME_JAM_2026_PROJECT_CHATBOX_H

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/KeyboardKey.h"
#include "../interfaces/manager/INetworkManager.h"
#include "../system/ChatLog.h"
#include "raylib-cpp.hpp"

#include <string>
#include <vector>

namespace ggj
{
    /*!
     * In-game netplay chat, drawn in the bottom-left corner of the 800x600 render target.
     *
     * Tab opens and closes it. While open it captures the keyboard (see
     * IInputManager::setKeyboardCaptured), so typing never moves the player or opens menus:
     * Enter sends, Escape throws the unsent line away and Backspace erases. While closed, new
     * lines are shown for a few seconds and then fade out.
     */
    class ChatBox
    {
        public:
        static constexpr KeyboardKey TOGGLE_KEY = KeyboardKey::Tab;

        ChatBox(IInputManager<KeyboardKey>& input, INetworkManager& network) : m_input { input }, m_network { network } {}

        /*!
         * Sets the fonts used for drawing. Nothing is drawn until both are set.
         *
         * @param bold Font for the "<name>: " prefix
         * @param regular Font for the message text
         */
        void setFonts(raylib::Font* bold, raylib::Font* regular);

        /*! Reads the keyboard. Call once per gameplay frame, before the game reads its own input. */
        void handleInput();
        /*!
         * Pulls newly received lines from the network and ages the history. Call every frame
         * netplay runs, also while a menu is open, so nothing piles up.
         *
         * @param timeDelta Time since last frame in seconds
         */
        void update(float timeDelta);
        void draw() const;

        /*! Closes the chat and gives the keyboard back to the game. Keeps the draft and history. */
        void close();
        /*! Closes the chat and forgets the draft and history (e.g. when leaving the session). */
        void reset();

        [[nodiscard]] bool isOpen() const { return m_isOpen; }
        [[nodiscard]] const ChatLog& log() const { return m_log; }

        private:
        /*! A wrapped, ready-to-draw line: the first line of an entry carries the bold name prefix. */
        struct VisualLine
        {
            std::string prefix {};
            std::string text {};
            raylib::Color color { WHITE };
        };

        void open();
        /*!
         * Wraps one chat line to the panel width.
         *
         * @param prefix Bold "<name>: " part (drawn on the first line only)
         * @param text Regular message part
         * @param color Colour of the whole line
         * @return The visual lines, top to bottom
         */
        [[nodiscard]] std::vector<VisualLine> layout(const std::string& prefix, const std::string& text, const raylib::Color& color) const;
        void drawLine(const VisualLine& line, float y, float alpha) const;

        static constexpr float PANEL_X = 8.f;
        static constexpr float PANEL_BOTTOM = 576.f; ///< Just above the XP bar (y = 582).
        static constexpr float PANEL_WIDTH = 400.f;
        static constexpr float PADDING = 4.f;
        static constexpr float FONT_SIZE = 18.f;
        static constexpr float SMALL_FONT_SIZE = 14.f;
        static constexpr float LINE_HEIGHT = 20.f;
        static constexpr size_t MAX_OPEN_LINES = 10;  ///< History lines shown while open.
        static constexpr size_t MAX_CLOSED_LINES = 6; ///< Fading lines shown while closed.

        IInputManager<KeyboardKey>& m_input;
        INetworkManager& m_network;
        ChatLog m_log {};
        raylib::Font* m_boldFont { nullptr };
        raylib::Font* m_regularFont { nullptr };
        bool m_isOpen { false };
        float m_caretTimer { 0.f };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CHATBOX_H
