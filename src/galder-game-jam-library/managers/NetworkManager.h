//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
#define GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H

#include "../interfaces/manager/INetworkManager.h"
#include "../network/GalderServer.hpp"
#include "../network/GalderClient.hpp"
#include "../network/LanBeacon.h"
#include "../../gamedev-base/GameDevBase.h"

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * Concrete netplay orchestrator. Client-authoritative model: each peer owns its own player and
     * sends its position; the host owns world state and relays every player's snapshot to all
     * clients. Remote players are interpolated toward their latest received position.
     */
    class NetworkManager : public INetworkManager
    {
        public:
        static constexpr uint32_t INVALID_PLAYER_ID = std::numeric_limits<uint32_t>::max();

        NetworkManager(ILogger& logger, IIpAddressResolver& ipAddressResolver) : m_logger { logger }, m_resolver { ipAddressResolver } {}

        void startHost(uint16_t port, const std::string& playerName, const raylib::Color& nameColor, const std::string& serverName, PlayerPickedCharacter character) override;
        void startClient(const std::string& ipAddress, uint16_t port, const std::string& name, const raylib::Color& nameColor, PlayerPickedCharacter character) override;
        void shutdown() override;

        void poll(float timeDelta) override;
        void setLocalPlayerPosition(const raylib::Vector2& position) override { m_localPosition = position; }
        void setLocalPlayerAnimation(PlayerState state, bool isFacingLeft) override
        {
            m_localPlayerState = state;
            m_isLocalFacingLeft = isFacingLeft;
        }

        [[nodiscard]] NetworkRole role() const override { return m_role; }
        [[nodiscard]] bool isActive() const override { return m_role != NetworkRole::None; }
        [[nodiscard]] ConnectionStatus status() const override;
        [[nodiscard]] uint32_t localPlayerId() const override { return m_localPlayerId; }
        [[nodiscard]] const std::string& localPlayerName() const override { return m_localName; }
        [[nodiscard]] const std::string& serverName() const override { return m_serverName; }
        [[nodiscard]] std::string localIpAddress() const override { return m_cachedLocalIp; }
        [[nodiscard]] std::string publicIpAddress() const override { return m_cachedPublicIp; }
        [[nodiscard]] int32_t pingToServer() const override;
        [[nodiscard]] const std::map<uint32_t, RemotePlayer>& remotePlayers() const override { return m_remotePlayers; }

        void publishEnemyState(uint32_t objectId, const raylib::Vector2& position, bool isFlipped) override;
        void notifyLocalObjectDestroyed(uint32_t objectId, bool isCoin) override;
        void setLocalPlayerStats(int32_t lives, int32_t score, int32_t kills) override
        {
            m_localLives = lives;
            m_localScore = score;
            m_localKills = kills;
        }
        void clearWorldState() override
        {
            m_enemyStates.clear();
            m_removedObjectIds.clear();
            m_projectileEvents.clear();
        }
        [[nodiscard]] const std::map<uint32_t, EnemyNetworkState>& enemyStates() const override { return m_enemyStates; }
        void notifyLocalProjectileFired(uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity) override;
        void notifyLocalProjectileImpact(uint32_t projectileId, const raylib::Vector2& position) override;
        [[nodiscard]] std::vector<ProjectileEvent> takeProjectileEvents() override;
        [[nodiscard]] const std::set<uint32_t>& removedObjectIds() const override { return m_removedObjectIds; }

        [[nodiscard]] std::vector<ConnectedPlayer> connectedPlayers() const override;

        void startDiscovery() override { m_beacon.startListening(); }
        void stopDiscovery() override { m_beacon.stop(); }
        void pollDiscovery(float timeDelta) override { m_beacon.poll(timeDelta); }
        [[nodiscard]] std::vector<DiscoveredServer> discoveredServers() const override { return m_beacon.discoveredServers(); }

        private:
        void pollServer(float timeDelta);
        void pollClient(float timeDelta);
        void applyPlayerSnapshot(const ServerMessage& message);
        void applyPlayerSnapshot(uint32_t playerId, const ClientMessage& message);
        void interpolateRemotePlayers(float timeDelta);
        void broadcastObjectRemoved(uint32_t objectId, bool isCoin);
        void sendProjectileEvent(NetMessageType type, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity);
        void queueProjectileEvent(NetMessageType type, uint32_t ownerId, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity);

        ILogger& m_logger;
        IIpAddressResolver& m_resolver;
        std::unique_ptr<GalderServer> m_server;
        std::unique_ptr<GalderClient> m_client;

        NetworkRole m_role { NetworkRole::None };
        uint32_t m_localPlayerId { HOST_PLAYER_ID };
        std::string m_localName {};
        raylib::Color m_localNameColor { RED };
        std::string m_serverName {};
        std::string m_cachedLocalIp {};
        std::string m_cachedPublicIp {};
        PlayerPickedCharacter m_localCharacter { PlayerPickedCharacter::Rogue };
        raylib::Vector2 m_localPosition {};
        PlayerState m_localPlayerState { PlayerState::Idle };
        bool m_isLocalFacingLeft { false };
        int32_t m_localLives { 0 };
        int32_t m_localScore { 0 };
        int32_t m_localKills { 0 };

        std::map<uint32_t, RemotePlayer> m_remotePlayers;

        // World-state sync (Phase 2)
        std::map<uint32_t, EnemyNetworkState> m_enemyStates;  ///< Client: latest enemy position + flip.
        std::vector<ProjectileEvent> m_projectileEvents;       ///< Remote projectile events awaiting the World.
        static constexpr size_t MAX_QUEUED_PROJECTILE_EVENTS = 256; ///< Guards against growth while no World drains.
        std::set<uint32_t> m_removedObjectIds;                ///< Authoritative removed object ids.

        // Server-only bookkeeping
        std::map<HSteamNetConnection, uint32_t> m_connectionToPlayerId;
        uint32_t m_nextPlayerId { HOST_PLAYER_ID + 1 };

        // Client-only bookkeeping
        bool m_hasSentJoinRequest { false };

        // LAN discovery / advertising
        LanBeacon m_beacon;
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
