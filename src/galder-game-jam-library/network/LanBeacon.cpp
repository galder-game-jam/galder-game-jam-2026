//
// Created by robin on 27.06.2026.
//

#include "LanBeacon.h"

#include <cstring>
#include <string>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

namespace ggj
{
    namespace
    {
        constexpr float BROADCAST_INTERVAL = 1.0f;
        constexpr float SERVER_EXPIRY_SECONDS = 5.0f;
        constexpr char BEACON_MAGIC[4] = { 'G', 'G', 'J', 'B' };

        // Fixed-layout discovery datagram. Sent/received as raw bytes on the LAN (same arch assumed).
        struct BeaconPacket
        {
            char magic[4];
            uint16_t port;
            int32_t playerCount;
            char name[21];
        };

#if defined(_WIN32)
        // GameNetworkingSockets already initialises Winsock, but refcount our own use to be safe.
        struct WinsockGuard
        {
            WinsockGuard()
            {
                WSADATA data;
                WSAStartup(MAKEWORD(2, 2), &data);
            }
            ~WinsockGuard() { WSACleanup(); }
        };
        WinsockGuard g_winsockGuard;

        void closeSocketHandle(uintptr_t socketHandle)
        {
            closesocket(socketHandle);
        }
        void makeNonBlocking(uintptr_t socketHandle)
        {
            u_long mode = 1;
            ioctlsocket(socketHandle, FIONBIO, &mode);
        }
#else
        void closeSocketHandle(int socketHandle)
        {
            ::close(socketHandle);
        }
        void makeNonBlocking(int socketHandle)
        {
            const int flags = fcntl(socketHandle, F_GETFL, 0);
            fcntl(socketHandle, F_SETFL, flags | O_NONBLOCK);
        }
#endif
    } // namespace

    LanBeacon::~LanBeacon()
    {
        stop();
    }

    bool LanBeacon::startAdvertising(const std::string& name, uint16_t gamePort)
    {
        stop();

        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket == INVALID_SOCKET_VALUE)
            return false;

        int enable = 1;
        setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&enable), sizeof(enable));
        makeNonBlocking(m_socket);

        m_isAdvertising = true;
        m_name = name;
        m_gamePort = gamePort;
        m_broadcastTimer = BROADCAST_INTERVAL; // broadcast on the next poll immediately
        return true;
    }

    bool LanBeacon::startListening()
    {
        stop();

        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket == INVALID_SOCKET_VALUE)
            return false;

        int enable = 1;
        setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(DISCOVERY_PORT);
        if (bind(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
        {
            stop();
            return false;
        }

        makeNonBlocking(m_socket);
        m_isAdvertising = false;
        return true;
    }

    void LanBeacon::poll(float timeDelta)
    {
        if (m_socket == INVALID_SOCKET_VALUE)
            return;

        if (m_isAdvertising)
        {
            m_broadcastTimer += timeDelta;
            if (m_broadcastTimer >= BROADCAST_INTERVAL)
            {
                m_broadcastTimer = 0.0f;
                sendBroadcast();
            }
        }
        else
        {
            receiveBroadcasts(timeDelta);
        }
    }

    void LanBeacon::sendBroadcast()
    {
        BeaconPacket packet {};
        std::memcpy(packet.magic, BEACON_MAGIC, sizeof(packet.magic));
        packet.port = m_gamePort;
        packet.playerCount = m_playerCount;
        std::strncpy(packet.name, m_name.c_str(), sizeof(packet.name) - 1);

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        address.sin_port = htons(DISCOVERY_PORT);

        sendto(m_socket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    }

    void LanBeacon::receiveBroadcasts(float timeDelta)
    {
        // Age out servers we have not heard from recently.
        for (auto it = m_servers.begin(); it != m_servers.end();)
        {
            it->second.secondsSinceSeen += timeDelta;
            if (it->second.secondsSinceSeen > SERVER_EXPIRY_SECONDS)
                it = m_servers.erase(it);
            else
                ++it;
        }

        while (true)
        {
            BeaconPacket packet {};
            sockaddr_in from {};
            socklen_t fromLength = sizeof(from);
            const int received = static_cast<int>(recvfrom(m_socket, reinterpret_cast<char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&from), &fromLength));

            if (received < static_cast<int>(sizeof(packet)))
                break; // nothing left (or a partial/foreign packet)

            if (std::memcmp(packet.magic, BEACON_MAGIC, sizeof(BEACON_MAGIC)) != 0)
                continue;

            char ipText[INET_ADDRSTRLEN] {};
            inet_ntop(AF_INET, &from.sin_addr, ipText, sizeof(ipText));
            packet.name[sizeof(packet.name) - 1] = '\0';

            DiscoveredServer server;
            server.ipAddress = ipText;
            server.port = packet.port;
            server.name = packet.name;
            server.playerCount = packet.playerCount;
            server.secondsSinceSeen = 0.0f;

            m_servers[server.ipAddress + ":" + std::to_string(server.port)] = server;
        }
    }

    std::vector<DiscoveredServer> LanBeacon::discoveredServers() const
    {
        std::vector<DiscoveredServer> result;
        result.reserve(m_servers.size());
        for (const auto& [key, server] : m_servers)
            result.push_back(server);
        return result;
    }

    void LanBeacon::stop()
    {
        if (m_socket != INVALID_SOCKET_VALUE)
        {
            closeSocketHandle(m_socket);
            m_socket = INVALID_SOCKET_VALUE;
        }
        m_isAdvertising = false;
        m_servers.clear();
    }
} // namespace ggj
