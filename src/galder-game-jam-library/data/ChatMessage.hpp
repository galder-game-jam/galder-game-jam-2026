//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CHATMESSAGE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CHATMESSAGE_HPP

#include "../enums/NetMessageType.h"
#include "raylib-cpp.hpp"

#include <cstddef>
#include <cstdint>

namespace ggj
{
    /*!
     * One chat line on the wire, in either direction (type Chat), or the host's kick notice to a
     * client (type Kicked, text = optional reason). Kept out of ClientMessage / ServerMessage on
     * purpose: those are sent every tick, and the text buffer would bloat every PlayerState. The
     * receivers tell the three apart by their (distinct) sizeof.
     *
     * @note Trivially copyable POD, sent as raw bytes - no std::string / containers.
     */
    struct ChatMessage
    {
        static constexpr size_t MAX_TEXT_LENGTH = 300; ///< Max characters (codepoints) in one line.
        /*!
         * Only codepoints below U+0800 are accepted (see ChatText::IsAcceptedCodepoint), which take
         * at most 2 bytes in UTF-8. Plus the null terminator.
         */
        static constexpr size_t MAX_TEXT_BYTES = MAX_TEXT_LENGTH * 2 + 1;

        NetMessageType type { NetMessageType::Chat };
        uint32_t playerId { 0 };      ///< Sender. Set by the server from the connection, never trusted from a client.
        char name[21] {};             ///< Sender name, up to 20 chars + null terminator.
        ::Color nameColor { RED };    ///< Sender's chosen name colour; the whole line is drawn in it.
        char text[MAX_TEXT_BYTES] {}; ///< UTF-8, null-terminated.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CHATMESSAGE_HPP
