//
// Created by robin on 08.11.24.
//

#ifndef GALDER_GAME_JAM_2024_PROJECT_GALDERSERVER_HPP
#define GALDER_GAME_JAM_2024_PROJECT_GALDERSERVER_HPP

#include "../../gamedev-base/GameDevBase.h"
#include "../data/ClientMessage.hpp"
#include "../data/ServerMessage.hpp"

#include <queue>
#include <utility>
#include <vector>

namespace ggj
{
    /*!
     * Game-side authoritative server transport. Thin wrapper over the non-blocking Server
     * template: it accepts connections, drains inbound ClientMessages into a queue, and records
     * disconnect events. All game logic (id assignment, relaying, world state) lives in
     * NetworkManager, which drains these queues once per frame and calls broadcast()/sendTo().
     */
    class GalderServer : public ggj::Server<ServerMessage, ClientMessage>
    {
        public:
        GalderServer(ILogger& logger, IIpAddressResolver& ipAddressResolver) : ggj::Server<ServerMessage, ClientMessage>(logger, ipAddressResolver) {}

        /*! Sends a message to a single client connection. */
        void sendTo(HSteamNetConnection connection, const ServerMessage& message) { send(connection, message); }

        /*! Drains and returns all ClientMessages received since the last call, with their sender. */
        std::vector<std::pair<HSteamNetConnection, ClientMessage>> takeInbound()
        {
            std::vector<std::pair<HSteamNetConnection, ClientMessage>> result;
            while (!m_inbound.empty())
            {
                result.push_back(m_inbound.front());
                m_inbound.pop();
            }
            return result;
        }

        /*! Drains and returns the connections that dropped since the last call. */
        std::vector<HSteamNetConnection> takeDisconnected()
        {
            std::vector<HSteamNetConnection> result;
            while (!m_disconnected.empty())
            {
                result.push_back(m_disconnected.front());
                m_disconnected.pop();
            }
            return result;
        }

        [[nodiscard]] size_t connectedClientCount() const { return m_mapClients.size(); }

        /*! Round-trip ping in ms for a connection, or -1 if unavailable. */
        [[nodiscard]] int getPing(HSteamNetConnection connection) const
        {
            SteamNetConnectionRealTimeStatus_t status {};
            if (m_netInterface != nullptr && m_netInterface->GetConnectionRealTimeStatus(connection, &status, 0, nullptr) == k_EResultOK)
                return status.m_nPing;
            return -1;
        }

        void onSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) override
        {
            switch (pInfo->m_info.m_eState)
            {
                case k_ESteamNetworkingConnectionState_None:
                    break;

                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                {
                    if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
                    {
                        m_logger.information(fmt::format("Client {0} disconnected", pInfo->m_info.m_szConnectionDescription));
                        m_mapClients.erase(pInfo->m_hConn);
                        m_disconnected.push(pInfo->m_hConn);
                    }

                    m_netInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                    break;
                }

                case k_ESteamNetworkingConnectionState_Connecting:
                {
                    m_logger.information(fmt::format("Connection request from {0}", pInfo->m_info.m_szConnectionDescription));

                    if (m_netInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
                    {
                        m_netInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                        m_logger.information("Can't accept connection.  (It was already closed?)");
                        break;
                    }

                    if (!m_netInterface->SetConnectionPollGroup(pInfo->m_hConn, m_pollGroup))
                    {
                        m_netInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                        m_logger.information("Failed to set poll group?");
                        break;
                    }

                    // Register the client so broadcast() reaches it. The player id / name are
                    // assigned by NetworkManager once the JoinRequest message arrives.
                    m_mapClients[pInfo->m_hConn];
                    break;
                }

                case k_ESteamNetworkingConnectionState_Connected:
                    break;

                default:
                    break;
            }
        }

        protected:
        void tick() override
        {
            pollClientMessages();
            pollConnectionStateChanges();
        }

        void pollClientMessages()
        {
            while (true)
            {
                ISteamNetworkingMessage* incomingMsg = nullptr;
                int numMsgs = m_netInterface->ReceiveMessagesOnPollGroup(m_pollGroup, &incomingMsg, 1);
                if (numMsgs == 0)
                    break;
                if (numMsgs < 0)
                {
                    m_logger.error("Error checking for messages");
                    break;
                }

                if (incomingMsg->m_cbSize == sizeof(ClientMessage))
                {
                    ClientMessage data = *static_cast<ClientMessage*>(incomingMsg->m_pData);
                    m_inbound.emplace(incomingMsg->m_conn, data);
                }

                incomingMsg->Release();
            }
        }

        private:
        std::queue<std::pair<HSteamNetConnection, ClientMessage>> m_inbound;
        std::queue<HSteamNetConnection> m_disconnected;
    };

} // namespace ggj

#endif //GALDER_GAME_JAM_2024_PROJECT_GALDERSERVER_HPP
