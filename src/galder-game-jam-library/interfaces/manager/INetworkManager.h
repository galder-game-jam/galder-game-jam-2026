//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H
#define GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H

#include "../../enums/NetworkRole.h"
#include "../../enums/PlayerPickedCharacter.h"
#include "../../data/RemotePlayer.hpp"
#include "../../../gamedev-base/enums/ConnectionStatus.hpp"
#include "raylib-cpp.hpp"

#include <cstdint>
#include <map>
#include <string>

namespace ggj
{
    /*!
     * Orchestrates netplay for the game. Owns either an authoritative server (host) or a client,
     * drives them non-blockingly once per frame, and exposes the synced state of every other
     * player so the world can render them. The host is also a local player.
     */
    class INetworkManager
    {
        public:
        static constexpr uint16_t DEFAULT_PORT = 13338;
        static constexpr uint32_t HOST_PLAYER_ID = 0;

        virtual ~INetworkManager() = default;

        /*! Begins hosting on the given port. The host is also player HOST_PLAYER_ID. */
        virtual void startHost(uint16_t port, const std::string& name, PlayerPickedCharacter character) = 0;
        /*! Begins (non-blocking) connecting to a host. */
        virtual void startClient(const std::string& ipAddress, uint16_t port, const std::string& name, PlayerPickedCharacter character) = 0;
        /*! Tears down any active server/client and returns to NetworkRole::None. */
        virtual void shutdown() = 0;

        /*! Drives one network tick: send local state, receive + apply remote state, interpolate. */
        virtual void poll(float timeDelta) = 0;

        /*! Feeds the local player's authoritative position in for outbound sync. Call before poll(). */
        virtual void setLocalPlayerPosition(const raylib::Vector2& position) = 0;

        [[nodiscard]] virtual NetworkRole role() const = 0;
        [[nodiscard]] virtual bool isActive() const = 0; ///< role() != None
        [[nodiscard]] virtual ConnectionStatus status() const = 0;
        [[nodiscard]] virtual uint32_t localPlayerId() const = 0;
        [[nodiscard]] virtual const std::string& localPlayerName() const = 0;

        /*! Every player except the local one, keyed by player id, for rendering puppets. */
        [[nodiscard]] virtual const std::map<uint32_t, RemotePlayer>& remotePlayers() const = 0;
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H
