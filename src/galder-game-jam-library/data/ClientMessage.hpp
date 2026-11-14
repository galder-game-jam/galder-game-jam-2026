//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP

#include "../enums/NetMessageType.h"
#include "CharacterColors.hpp"
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
        ::Color nameColor { RED }; ///< Colour of the player's overhead name label.
        CharacterColors colors {}; ///< Picked sprite colours, see CharacterPalette.
        char localIp[46] {};     ///< Client's LAN IP, sent only in JoinRequest.
        char publicIp[46] {};    ///< Client's public IP, sent only in JoinRequest.

        // Transform / animation (PlayerState)
        raylib::Vector2 position {};
        raylib::Vector2 velocity {};
        uint8_t animationState { 0 };
        uint8_t animationFrame { 0 };
        bool facingLeft { false };

        // Player stats (PlayerState).
        int32_t lives { 0 };
        int32_t score { 0 };
        int32_t kills { 0 };
        int32_t level { 1 };
        float mana { 0.f };
        float maxMana { 0.f };
        float energy { 0.f };
        float maxEnergy { 0.f };

        // Local world event report (GameEvent): something this peer destroyed locally.
        uint32_t objectId { 0 };
        uint8_t eventKind { 0 }; ///< 0 = none, 1 = coin collected, 2 = enemy killed.
        int32_t damage { 0 };    ///< EnemyDamaged: lives the client's hit removes from objectId.

        /*!
         * Stage the sender was on. Object ids restart on every map, so the server drops world
         * events tagged with a stage it has already left instead of applying them to the new one.
         */
        int32_t stageIndex { 0 };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_CLIENTMESSAGE_HPP
