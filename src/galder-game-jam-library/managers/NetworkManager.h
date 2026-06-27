//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
#define GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H

#include "../interfaces/manager/INetworkManager.h"
#include "../network/GalderServer.hpp"
#include "../network/GalderClient.hpp"
#include "../../gamedev-base/GameDevBase.h"

#include <limits>
#include <map>
#include <memory>
#include <string>

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

        void startHost(uint16_t port, const std::string& name, PlayerPickedCharacter character) override;
        void startClient(const std::string& ipAddress, uint16_t port, const std::string& name, PlayerPickedCharacter character) override;
        void shutdown() override;

        void poll(float timeDelta) override;
        void setLocalPlayerPosition(const raylib::Vector2& position) override { m_localPosition = position; }

        [[nodiscard]] NetworkRole role() const override { return m_role; }
        [[nodiscard]] bool isActive() const override { return m_role != NetworkRole::None; }
        [[nodiscard]] ConnectionStatus status() const override;
        [[nodiscard]] uint32_t localPlayerId() const override { return m_localPlayerId; }
        [[nodiscard]] const std::string& localPlayerName() const override { return m_localName; }
        [[nodiscard]] const std::map<uint32_t, RemotePlayer>& remotePlayers() const override { return m_remotePlayers; }

        private:
        void pollServer(float timeDelta);
        void pollClient(float timeDelta);
        void applyPlayerSnapshot(uint32_t playerId, const char* name, uint8_t character, const raylib::Vector2& position);
        void interpolateRemotePlayers(float timeDelta);

        ILogger& m_logger;
        IIpAddressResolver& m_resolver;
        std::unique_ptr<GalderServer> m_server;
        std::unique_ptr<GalderClient> m_client;

        NetworkRole m_role { NetworkRole::None };
        uint32_t m_localPlayerId { HOST_PLAYER_ID };
        std::string m_localName {};
        PlayerPickedCharacter m_localCharacter { PlayerPickedCharacter::Rogue };
        raylib::Vector2 m_localPosition {};

        std::map<uint32_t, RemotePlayer> m_remotePlayers;

        // Server-only bookkeeping
        std::map<HSteamNetConnection, uint32_t> m_connectionToPlayerId;
        uint32_t m_nextPlayerId { HOST_PLAYER_ID + 1 };

        // Client-only bookkeeping
        bool m_hasSentJoinRequest { false };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
