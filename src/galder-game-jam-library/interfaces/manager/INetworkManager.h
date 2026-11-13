//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H
#define GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H

#include "../../enums/NetworkRole.h"
#include "../../enums/PlayerPickedCharacter.h"
#include "../../data/RemotePlayer.hpp"
#include "../../data/ConnectedPlayer.hpp"
#include "../../data/DiscoveredServer.hpp"
#include "../../../gamedev-base/enums/ConnectionStatus.hpp"
#include "raylib-cpp.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

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
        virtual void startHost(uint16_t port, const std::string& playerName, const raylib::Color& nameColor, const std::string& serverName, PlayerPickedCharacter character) = 0;
        /*! Begins (non-blocking) connecting to a host. */
        virtual void startClient(const std::string& ipAddress, uint16_t port, const std::string& name, const raylib::Color& nameColor, PlayerPickedCharacter character) = 0;
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
        [[nodiscard]] virtual const std::string& serverName() const = 0;
        [[nodiscard]] virtual std::string localIpAddress() const = 0;
        [[nodiscard]] virtual std::string publicIpAddress() const = 0;
        [[nodiscard]] virtual int32_t pingToServer() const = 0;

        /*! Every player except the local one, keyed by player id, for rendering puppets. */
        [[nodiscard]] virtual const std::map<uint32_t, RemotePlayer>& remotePlayers() const = 0;

        // --- World-state sync (Phase 2) ---

        /*! Server: broadcast an enemy's authoritative position. No-op when not hosting. */
        virtual void publishEnemyState(uint32_t objectId, const raylib::Vector2& position, bool facingLeft) = 0;
        /*! Reports a networked object destroyed locally (coin picked up / enemy killed) so the server can reconcile. */
        virtual void notifyLocalObjectDestroyed(uint32_t objectId, bool isCoin) = 0;
        /*! Feeds the local player's authoritative stats (lives/score/kills) for outbound sync. */
        virtual void setLocalPlayerStats(int32_t lives, int32_t score, int32_t kills) = 0;
        /*! Clears per-world synced state. Call when (re)entering a world to avoid id reuse bleed. */
        virtual void clearWorldState() = 0;

        /*! Client: latest authoritative enemy positions keyed by object id. */
        [[nodiscard]] virtual const std::map<uint32_t, raylib::Vector2>& enemyPositions() const = 0;
        /*! Object ids the server has declared gone (collected coins / dead enemies). */
        [[nodiscard]] virtual const std::set<uint32_t>& removedObjectIds() const = 0;

        /*! Host's view of all connected players (host + clients) for the ServerInfo window. Empty off-server. */
        [[nodiscard]] virtual std::vector<ConnectedPlayer> connectedPlayers() const = 0;

        // --- LAN discovery (Phase 3) ---

        /*! Begin listening for LAN host advertisements (call while browsing on the netplay screen). */
        virtual void startDiscovery() = 0;
        /*! Stop listening for LAN advertisements. */
        virtual void stopDiscovery() = 0;
        /*! Pump the discovery listener; call each frame while browsing. */
        virtual void pollDiscovery(float timeDelta) = 0;
        /*! Hosts found on the LAN since discovery started. */
        [[nodiscard]] virtual std::vector<DiscoveredServer> discoveredServers() const = 0;
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_INETWORKMANAGER_H
