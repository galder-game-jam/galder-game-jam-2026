//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H
#define GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H

#include "../data/DiscoveredServer.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * Lightweight UDP-broadcast LAN discovery. GameNetworkingSockets has no built-in LAN browsing,
     * so this uses a plain (cross-platform) UDP socket: a host periodically broadcasts its name,
     * game port and player count on a well-known port; a browsing client listens and collects the
     * hosts it hears. Non-blocking - poll() is meant to be called once per frame.
     */
    class LanBeacon
    {
        public:
        static constexpr uint16_t DISCOVERY_PORT = 13339;

        LanBeacon() = default;
        ~LanBeacon();

        LanBeacon(const LanBeacon&) = delete;
        LanBeacon& operator=(const LanBeacon&) = delete;

        /*! Host: start broadcasting our presence. */
        bool startAdvertising(const std::string& name, uint16_t gamePort);
        void setPlayerCount(int32_t count) { m_playerCount = count; }

        /*! Client: start listening for host advertisements. */
        bool startListening();

        /*! Pump: broadcast (advertiser, throttled) or drain incoming (listener). */
        void poll(float timeDelta);

        [[nodiscard]] std::vector<DiscoveredServer> discoveredServers() const;

        void stop();
        [[nodiscard]] bool isActive() const { return m_socket != INVALID_SOCKET_VALUE; }

        private:
        void sendBroadcast();
        void receiveBroadcasts(float timeDelta);

#if defined(_WIN32)
        using SocketHandle = uintptr_t; // matches Windows SOCKET
#else
        using SocketHandle = int;
#endif
        static constexpr SocketHandle INVALID_SOCKET_VALUE = static_cast<SocketHandle>(-1);

        SocketHandle m_socket { INVALID_SOCKET_VALUE };
        bool m_isAdvertising { false };
        std::string m_name {};
        uint16_t m_gamePort { 0 };
        int32_t m_playerCount { 0 };
        float m_broadcastTimer { 0.0f };
        std::map<std::string, DiscoveredServer> m_servers; ///< key "ip:port"
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H
