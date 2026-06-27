//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP

#include "../enums/NetMessageType.h"
#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * Client -> Server message envelope. Trivially copyable POD: it is sent as raw bytes
     * (sizeof) over the wire, which assumes both peers share the same endianness/layout
     * (fine for a LAN game jam, same compiler/arch on both ends).
     *
     * @note No std::string / containers allowed here - it must stay trivially copyable.
     */
    struct ClientMessage
    {
        NetMessageType type { NetMessageType::None };
        uint32_t playerId { 0 }; ///< Filled in by the server-assigned id once known.

        // Identity (JoinRequest)
        char name[21] {};        ///< Up to 20 chars + null terminator.
        uint8_t character { 0 }; ///< PlayerPickedCharacter value.

        // Transform / animation (PlayerState)
        raylib::Vector2 position {};
        raylib::Vector2 velocity {};
        uint8_t animationState { 0 };
        uint8_t animationFrame { 0 };
        bool facingLeft { false };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP
