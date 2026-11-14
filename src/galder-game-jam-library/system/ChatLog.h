//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CHATLOG_H
#define GALDER_GAME_JAM_2026_PROJECT_CHATLOG_H

#include "../data/ChatEntry.hpp"
#include "../data/ChatMessage.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * State behind the chat box, without any drawing or input: the line being typed and the
     * received history. Kept separate from ChatBox so the editing rules can be unit tested.
     */
    class ChatLog
    {
        public:
        static constexpr size_t MAX_DRAFT_LENGTH = ChatMessage::MAX_TEXT_LENGTH; ///< Characters per line.
        static constexpr size_t MAX_ENTRIES = 50;                                ///< Oldest lines are dropped beyond this.
        static constexpr float VISIBLE_SECONDS = 8.f;                            ///< How long a line stays up while chat is closed.
        static constexpr float FADE_SECONDS = 1.f;                               ///< Last part of VISIBLE_SECONDS spent fading out.

        ChatLog() = default;

        /*!
         * Adds a typed character to the draft.
         *
         * @param codepoint Unicode codepoint
         * @return False if it was rejected (unprintable, or the draft is full)
         */
        bool appendCodepoint(int32_t codepoint);
        /*! Removes the last character of the draft (a whole character, even if multi-byte). */
        void eraseLast();
        /*! Throws away the draft. */
        void clearDraft();
        /*!
         * Takes the draft for sending and clears it.
         *
         * @return The trimmed UTF-8 text, or std::nullopt if there is nothing but whitespace
         */
        [[nodiscard]] std::optional<std::string> takeDraft();
        [[nodiscard]] std::string draftUtf8() const;
        [[nodiscard]] size_t draftLength() const { return m_draft.size(); } ///< In characters.

        /*! Adds a received line; drops the oldest beyond MAX_ENTRIES. */
        void addEntry(const ChatEntry& entry);
        /*! Ages every line by timeDelta seconds. */
        void update(float timeDelta);
        [[nodiscard]] const std::deque<ChatEntry>& entries() const { return m_entries; } ///< Oldest first.

        /*!
         * Opacity of a line shown while chat is closed.
         *
         * @param age Seconds since the line was received
         * @return 1 while fresh, fading linearly to 0 at VISIBLE_SECONDS
         */
        [[nodiscard]] static float FadeAlpha(float age);

        /*! Clears the draft and the history. */
        void reset();

        private:
        std::vector<int32_t> m_draft {}; ///< Codepoints, so length and backspace are per character.
        std::deque<ChatEntry> m_entries {};
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CHATLOG_H
