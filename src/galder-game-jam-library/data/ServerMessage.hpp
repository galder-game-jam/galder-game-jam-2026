//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP

#include "../enums/NetMessageType.h"
#include "CharacterColors.hpp"
#include "raylib-cpp.hpp"
#include <cstdint>

namespace ggj
{
    /*!
     * Server -> Client message envelope. Trivially copyable POD sent as raw bytes (sizeof).
     * See ClientMessage for the endianness/layout caveat.
     *
     * For JoinAccept, playerId holds the id the server assigned to this client, name holds the
     * server name and stageIndex the stage the host is currently on.
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
        ::Color nameColor { RED }; ///< Colour of the player's overhead name label.
        CharacterColors colors {}; ///< Picked sprite colours, see CharacterPalette.

        // Transform / animation (PlayerState)
        raylib::Vector2 position {};
        raylib::Vector2 velocity {};
        uint8_t animationState { 0 };
        uint8_t animationFrame { 0 };
        bool facingLeft { false };
        bool isConnected { true };

        // Player stats (PlayerState) - authoritative life/score/kills for HUD & ServerInfo.
        int32_t lives { 0 };
        int32_t score { 0 };
        int32_t kills { 0 };

        // World object sync (EnemyState / CoinState).
        uint32_t objectId { 0 }; ///< Networked object id this message refers to.
        bool flag { false };     ///< EnemyState: isAlive. CoinState: collected.

        int32_t stageIndex { 0 }; ///< JoinAccept / StageChanged: the host's stage. PortalEntered: stage of that portal.
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP
