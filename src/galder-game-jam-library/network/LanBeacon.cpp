//
// Created by robin on 27.06.2026.
//

#include "LanBeacon.h"

#include <algorithm>
#include <cstring>
#include <set>
#include <string>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

namespace ggj
{
    namespace
    {
        constexpr float QUERY_INTERVAL = 1.0f;        ///< Broadcast queries.
        constexpr float SWEEP_INTERVAL = 3.0f;        ///< Unicast sweep rounds (~ARP timeout, see class docs).
        constexpr size_t SWEEP_QUERIES_PER_POLL = 32; ///< Spreads a /24 sweep over ~8 frames.
        constexpr float SERVER_EXPIRY_SECONDS = 8.0f; ///< > SWEEP_INTERVAL so sweep-only hosts don't flicker.
        constexpr char REPLY_MAGIC[4] = { 'G', 'G', 'J', 'B' };
        constexpr char QUERY_MAGIC[4] = { 'G', 'G', 'J', 'Q' };
        constexpr uint32_t SWEEP_NETMASK = 0xFFFFFF00u; ///< Unicast-sweep at most a /24 (254 hosts) per interface.

        // Fixed-layout discovery datagrams. Sent/received as raw bytes on the LAN (same arch assumed).
        struct ReplyPacket
        {
            char magic[4];
            uint16_t port;
            int32_t playerCount;
            char name[21];
        };

        struct QueryPacket
        {
            char magic[4];
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
        void disableConnectionResetErrors(uintptr_t socketHandle)
        {
            // Windows turns ICMP "port unreachable" (any LAN machine not running a host answers our
            // sweep with one) into WSAECONNRESET on the next recvfrom, which would stall draining.
    #ifndef SIO_UDP_CONNRESET
        #define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
    #endif
            BOOL isEnabled = FALSE;
            DWORD bytesReturned = 0;
            WSAIoctl(socketHandle, SIO_UDP_CONNRESET, &isEnabled, sizeof(isEnabled), nullptr, 0, &bytesReturned, nullptr, nullptr);
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
        void disableConnectionResetErrors(int) {}
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
        disableConnectionResetErrors(m_socket);
        m_isAdvertising = true;
        m_name = name;
        m_gamePort = gamePort;
        return true;
    }

    LanBeacon::SocketHandle LanBeacon::openClientSocket()
    {
        const SocketHandle socketHandle = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketHandle == INVALID_SOCKET_VALUE)
            return INVALID_SOCKET_VALUE;

        int enable = 1;
        setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&enable), sizeof(enable));

        // Ephemeral port: replies come back to whatever port the OS picks for our queries.
        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = 0;
        if (bind(socketHandle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
        {
            closeSocketHandle(socketHandle);
            return INVALID_SOCKET_VALUE;
        }

        makeNonBlocking(socketHandle);
        disableConnectionResetErrors(socketHandle);
        return socketHandle;
    }

    bool LanBeacon::startListening()
    {
        stop();

        m_socket = openClientSocket();
        if (m_socket == INVALID_SOCKET_VALUE)
            return false;
        // Optional: broadcasts alone still work if these fail.
        for (SocketHandle& sweepSocket : m_sweepSockets)
            sweepSocket = openClientSocket();

        m_isAdvertising = false;
        m_interfaces = enumerateInterfaces();
        m_queryTimer = QUERY_INTERVAL; // query on the next poll immediately
        m_sweepTimer = SWEEP_INTERVAL;
        return true;
    }

    void LanBeacon::poll(float timeDelta)
    {
        if (m_socket == INVALID_SOCKET_VALUE)
            return;

        if (m_isAdvertising)
        {
            answerQueries();
            return;
        }

        m_queryTimer += timeDelta;
        if (m_queryTimer >= QUERY_INTERVAL)
        {
            m_queryTimer = 0.0f;
            sendBroadcastQueries();
        }

        m_sweepTimer += timeDelta;
        if (m_sweepTimer >= SWEEP_INTERVAL && m_sweepIndex >= m_sweepTargets.size())
        {
            m_sweepTimer = 0.0f;
            startSweep();
        }
        continueSweep();

        expireServers(timeDelta);
        receiveReplies(m_socket);
        for (const SocketHandle sweepSocket : m_sweepSockets)
        {
            if (sweepSocket != INVALID_SOCKET_VALUE)
                receiveReplies(sweepSocket);
        }
    }

    void LanBeacon::answerQueries()
    {
        ReplyPacket reply {};
        std::memcpy(reply.magic, REPLY_MAGIC, sizeof(reply.magic));
        reply.port = m_gamePort;
        reply.playerCount = m_playerCount;
        std::strncpy(reply.name, m_name.c_str(), sizeof(reply.name) - 1);

        while (true)
        {
            QueryPacket query {};
            sockaddr_in from {};
            socklen_t fromLength = sizeof(from);
            const int received = static_cast<int>(recvfrom(m_socket, reinterpret_cast<char*>(&query), sizeof(query), 0, reinterpret_cast<sockaddr*>(&from), &fromLength));
            if (received < 0)
                break; // drained

            if (received != static_cast<int>(sizeof(query)) || std::memcmp(query.magic, QUERY_MAGIC, sizeof(QUERY_MAGIC)) != 0)
                continue;

            sendto(m_socket, reinterpret_cast<const char*>(&reply), sizeof(reply), 0, reinterpret_cast<sockaddr*>(&from), fromLength);
        }
    }

    void LanBeacon::sendBroadcastQueries()
    {
        // A set, so interfaces sharing a subnet never query the same address twice.
        std::set<uint32_t> targets { INADDR_BROADCAST };
        for (const Ipv4Interface& networkInterface : m_interfaces)
            targets.insert(networkInterface.address | ~networkInterface.netmask); // directed broadcast

        for (const uint32_t target : targets)
            static_cast<void>(sendQueryTo(m_socket, target));
    }

    void LanBeacon::startSweep()
    {
        std::set<uint32_t> targets;
        for (const Ipv4Interface& networkInterface : m_interfaces)
        {
            const uint32_t sweepMask = networkInterface.netmask | SWEEP_NETMASK;
            const uint32_t network = networkInterface.address & sweepMask;
            const uint32_t broadcast = network | ~sweepMask;
            for (uint32_t host = network + 1; host < broadcast; ++host)
                targets.insert(host);
        }

        m_sweepTargets.assign(targets.begin(), targets.end());
        m_sweepIndex = 0;
    }

    void LanBeacon::continueSweep()
    {
        const size_t end = std::min(m_sweepIndex + SWEEP_QUERIES_PER_POLL, m_sweepTargets.size());
        while (m_sweepIndex < end)
        {
            const SocketHandle sweepSocket = m_sweepSockets[m_sweepIndex % SWEEP_SOCKET_COUNT];
            if (sweepSocket != INVALID_SOCKET_VALUE && !sendQueryTo(sweepSocket, m_sweepTargets[m_sweepIndex]))
                return; // send buffer full: retry this address next poll
            ++m_sweepIndex;
        }
    }

    bool LanBeacon::sendQueryTo(SocketHandle socketHandle, uint32_t hostOrderAddress)
    {
        QueryPacket query {};
        std::memcpy(query.magic, QUERY_MAGIC, sizeof(query.magic));

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(hostOrderAddress);
        address.sin_port = htons(DISCOVERY_PORT);
        return sendto(socketHandle, reinterpret_cast<const char*>(&query), sizeof(query), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address)) >= 0;
    }

    void LanBeacon::expireServers(float timeDelta)
    {
        for (auto it = m_servers.begin(); it != m_servers.end();)
        {
            it->second.secondsSinceSeen += timeDelta;
            if (it->second.secondsSinceSeen > SERVER_EXPIRY_SECONDS)
                it = m_servers.erase(it);
            else
                ++it;
        }
    }

    void LanBeacon::receiveReplies(SocketHandle socketHandle)
    {
        while (true)
        {
            ReplyPacket packet {};
            sockaddr_in from {};
            socklen_t fromLength = sizeof(from);
            const int received = static_cast<int>(recvfrom(socketHandle, reinterpret_cast<char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&from), &fromLength));
            if (received < 0)
                break; // drained

            if (received != static_cast<int>(sizeof(packet)) || std::memcmp(packet.magic, REPLY_MAGIC, sizeof(REPLY_MAGIC)) != 0)
                continue; // foreign or partial datagram

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

    std::vector<LanBeacon::Ipv4Interface> LanBeacon::enumerateInterfaces() const
    {
        std::vector<Ipv4Interface> result;
#if defined(_WIN32)
        INTERFACE_INFO interfaces[32] {};
        DWORD bytesReturned = 0;
        if (WSAIoctl(m_socket, SIO_GET_INTERFACE_LIST, nullptr, 0, interfaces, sizeof(interfaces), &bytesReturned, nullptr, nullptr) != 0)
            return result;

        const size_t count = bytesReturned / sizeof(INTERFACE_INFO);
        for (size_t i = 0; i < count; ++i)
        {
            const INTERFACE_INFO& info = interfaces[i];
            if ((info.iiFlags & IFF_UP) == 0 || (info.iiFlags & IFF_LOOPBACK) != 0)
                continue;
            result.push_back({ ntohl(info.iiAddress.AddressIn.sin_addr.s_addr), ntohl(info.iiNetmask.AddressIn.sin_addr.s_addr) });
        }
#else
        ifaddrs* addresses = nullptr;
        if (getifaddrs(&addresses) != 0)
            return result;

        for (const ifaddrs* entry = addresses; entry != nullptr; entry = entry->ifa_next)
        {
            if (entry->ifa_addr == nullptr || entry->ifa_netmask == nullptr || entry->ifa_addr->sa_family != AF_INET)
                continue;
            if ((entry->ifa_flags & IFF_UP) == 0 || (entry->ifa_flags & IFF_LOOPBACK) != 0)
                continue;

            const uint32_t address = ntohl(reinterpret_cast<const sockaddr_in*>(entry->ifa_addr)->sin_addr.s_addr);
            const uint32_t netmask = ntohl(reinterpret_cast<const sockaddr_in*>(entry->ifa_netmask)->sin_addr.s_addr);
            result.push_back({ address, netmask });
        }
        freeifaddrs(addresses);
#endif
        return result;
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
        for (SocketHandle& sweepSocket : m_sweepSockets)
        {
            if (sweepSocket != INVALID_SOCKET_VALUE)
                closeSocketHandle(sweepSocket);
            sweepSocket = INVALID_SOCKET_VALUE;
        }
        m_isAdvertising = false;
        m_interfaces.clear();
        m_sweepTargets.clear();
        m_sweepIndex = 0;
        m_servers.clear();
    }
} // namespace ggj
