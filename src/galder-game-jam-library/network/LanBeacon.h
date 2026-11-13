//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H
#define GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H

#include "../data/DiscoveredServer.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ggj
{
    /*!
     * Minimal LAN server discovery. GameNetworkingSockets has no LAN discovery of its own, so this
     * uses a plain (cross-platform) UDP socket with a client-initiated query/response:
     *
     * - The host binds the well-known DISCOVERY_PORT and answers every query with a unicast reply
     *   carrying its name, game port and player count.
     * - A browsing client periodically sends queries from an ephemeral port: a limited broadcast,
     *   a directed broadcast per network interface, and a unicast sweep of each interface's /24.
     *
     * Client-initiated is the point: an unsolicited host broadcast is dropped by the client's
     * firewall (Windows "Public" profile, ufw/firewalld), whereas replies to the client's own
     * outbound unicast queries pass any stateful firewall - the same reason connecting by IP works.
     * The per-interface broadcasts cover machines with several adapters (VPN, Hyper-V, WSL, ...),
     * where 255.255.255.255 only leaves through one of them. Non-blocking - poll() once per frame.
     *
     * The sweep uses its own sockets and is paced across frames: each unicast to an absent address
     * sits in the kernel's ARP queue for ~3 s, pinning ~1 KB of that socket's send buffer (Linux
     * fills its default 208 KB after ~220 queries). Several sockets give a /24 room to go out in
     * one round, and keep the broadcasts from ever being starved.
     */
    class LanBeacon
    {
        public:
        static constexpr uint16_t DISCOVERY_PORT = 13339;

        LanBeacon() { m_sweepSockets.fill(INVALID_SOCKET_VALUE); }
        ~LanBeacon();
        LanBeacon(const LanBeacon&) = delete;
        LanBeacon& operator=(const LanBeacon&) = delete;

        /*! Host: start answering discovery queries on DISCOVERY_PORT. */
        bool startAdvertising(const std::string& name, uint16_t gamePort);
        void setPlayerCount(int32_t count) { m_playerCount = count; }

        /*! Client: start querying the LAN for hosts. */
        bool startListening();

        /*! Pump: answer queries (host) or send throttled queries and collect replies (client). */
        void poll(float timeDelta);

        [[nodiscard]] std::vector<DiscoveredServer> discoveredServers() const;

        void stop();
        [[nodiscard]] bool isActive() const { return m_socket != INVALID_SOCKET_VALUE; }

        private:
#if defined(_WIN32)
        using SocketHandle = uintptr_t; // matches Windows SOCKET
#else
        using SocketHandle = int;
#endif
        static constexpr SocketHandle INVALID_SOCKET_VALUE = static_cast<SocketHandle>(-1);

        /*! An IPv4 interface address + netmask, both in host byte order. */
        struct Ipv4Interface
        {
            uint32_t address { 0 };
            uint32_t netmask { 0 };
        };

        void answerQueries();
        void sendBroadcastQueries();
        void startSweep();
        void continueSweep();
        void expireServers(float timeDelta);
        void receiveReplies(SocketHandle socketHandle);
        [[nodiscard]] static bool sendQueryTo(SocketHandle socketHandle, uint32_t hostOrderAddress);
        [[nodiscard]] static SocketHandle openClientSocket();
        [[nodiscard]] std::vector<Ipv4Interface> enumerateInterfaces() const;

        SocketHandle m_socket { INVALID_SOCKET_VALUE };      ///< Host: query listener. Client: broadcast queries.
        static constexpr size_t SWEEP_SOCKET_COUNT = 4;
        std::array<SocketHandle, SWEEP_SOCKET_COUNT> m_sweepSockets {}; ///< Client: unicast sweep queries (round-robin).
        bool m_isAdvertising { false };
        std::string m_name {};
        uint16_t m_gamePort { 0 };
        int32_t m_playerCount { 0 };
        float m_queryTimer { 0.0f };
        float m_sweepTimer { 0.0f };
        std::vector<uint32_t> m_sweepTargets {}; ///< Client: addresses of the current sweep round.
        size_t m_sweepIndex { 0 };               ///< Next entry of m_sweepTargets to send.
        std::vector<Ipv4Interface> m_interfaces {}; ///< Client: captured in startListening().
        std::map<std::string, DiscoveredServer> m_servers; ///< key "ip:port"
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_LANBEACON_H
