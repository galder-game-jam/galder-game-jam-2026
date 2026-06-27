//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP

#include "../enums/NetMessageType.h"
#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * Server -> Client message envelope. Trivially copyable POD sent as raw bytes (sizeof).
     * See ClientMessage for the endianness/layout caveat.
     *
     * For JoinAccept, playerId holds the id the server assigned to this client.
     * For PlayerState / PlayerLeft, playerId identifies which player the snapshot describes.
     *
     * @note No std::string / containers allowed here - it must stay trivially copyable.
     */
    struct ServerMessage
    {
        NetMessageType type { NetMessageType::None };
        uint32_t playerId { 0 };

        // Identity
        char name[21] {};
        uint8_t character { 0 }; ///< PlayerPickedCharacter value.

        // Transform / animation (PlayerState)
        raylib::Vector2 position {};
        raylib::Vector2 velocity {};
        uint8_t animationState { 0 };
        uint8_t animationFrame { 0 };
        bool facingLeft { false };
        bool isConnected { true };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP
