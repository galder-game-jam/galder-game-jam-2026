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
     * For EnemyState, lives carries the enemy's remaining lives (used by bosses).
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
        int32_t level { 1 };   ///< Level and resources feed the party bars of the HUD.
        float mana { 0.f };
        float maxMana { 0.f };
        float energy { 0.f };
        float maxEnergy { 0.f };

        // World object sync (EnemyState / CoinState).
        uint32_t objectId { 0 }; ///< Networked object id this message refers to.
        bool flag { false };     ///< EnemyState: isAlive. CoinState: collected.
        uint8_t aiState { 0 };   ///< EnemyState: enemy-specific AI state (e.g. ClaudeState).
        uint32_t targetPlayerId { 0 }; ///< EnemyState / ClaudlingSpawned: player id the enemy chases.
        /*!
         * EnemyState with flag == false: player id credited with the kill, or
         * INetworkManager::NO_PLAYER_ID when the killer's own peer already counted it.
         */
        uint32_t killerPlayerId { 0 };
        float followSpeed { 0.f }; ///< ClaudlingSpawned: chase speed.
        float knockback { 0.f };   ///< ClaudlingSpawned: knockback applied to players on contact.
        float fuseTime { 0.f };    ///< BombSpawned: seconds until the bomb explodes.
        raylib::Vector2 target {}; ///< RootSpawned / SpiderSpawned: where it lands (position holds where it was thrown from). FireLineSpawned: where the line ends.
        float flightTime { 0.f };  ///< RootSpawned / SpiderSpawned: seconds it spends in the air.
        float spikeDelay { 0.f };  ///< RootSpawned: seconds between landing and spiking.
        float radius { 0.f };      ///< IceCircleSpawned: radius of the circle (pixels).
        float delay { 0.f };       ///< IceCircleSpawned: seconds until it freezes. FireLineSpawned: seconds of warning before it ignites.
        float speed { 0.f };       ///< FireLineSpawned: how fast the flames race along the line (pixels per second).
        float duration { 0.f };    ///< IceCircleSpawned: seconds a frozen player stays frozen. FireLineSpawned: seconds it burns.

        int32_t stageIndex { 0 }; ///< JoinAccept / StageChanged: the host's stage. PortalEntered: stage of that portal.
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_SERVERMESSAGE_HPP
