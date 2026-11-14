//
// Created by robin on 08.11.24.
//

#ifndef GALDER_GAME_JAM_2024_PROJECT_GALDERCLIENT_HPP
#define GALDER_GAME_JAM_2024_PROJECT_GALDERCLIENT_HPP

#include "../../gamedev-base/GameDevBase.h"
#include "../data/ChatMessage.hpp"
#include "../data/ClientMessage.hpp"
#include "../data/ServerMessage.hpp"

#include <cstring>
#include <queue>
#include <type_traits>
#include <vector>

namespace ggj
{
    static_assert(std::is_trivially_copyable_v<ChatMessage>, "ChatMessage is sent as raw bytes");
    static_assert(sizeof(ChatMessage) != sizeof(ClientMessage) && sizeof(ChatMessage) != sizeof(ServerMessage), "Receivers tell ChatMessage apart from the envelopes by its size");

    /*!
     * Game-side client transport. Thin wrapper over the non-blocking Client template: it drains
     * inbound ServerMessages into a queue and lets NetworkManager send ClientMessages. Connection
     * state (and status()) is handled by the base class.
     */
    class GalderClient : public ggj::Client<ClientMessage, ServerMessage>
    {
        public:
        GalderClient(ILogger& logger, IIpAddressResolver& ipAddressResolver) : ggj::Client<ClientMessage, ServerMessage>(logger, ipAddressResolver) {}

        /*! Sends a message to the server. */
        void sendToServer(const ClientMessage& message) { send(message); }

        /*! Sends a chat line to the server, which relays it to everyone. */
        void sendChatToServer(const ChatMessage& message)
        {
            if (m_netInterface != nullptr)
                m_netInterface->SendMessageToConnection(m_connection, &message, sizeof(message), k_nSteamNetworkingSend_Reliable, nullptr);
        }

        /*! Drains and returns all chat lines received since the last call. */
        std::vector<ChatMessage> takeInboundChat()
        {
            std::vector<ChatMessage> result;
            while (!m_inboundChat.empty())
            {
                result.push_back(m_inboundChat.front());
                m_inboundChat.pop();
            }
            return result;
        }

        /*! Round-trip time to the server in milliseconds, or -1 if unavailable. */
        int32_t getPing() const
        {
            SteamNetConnectionRealTimeStatus_t status {};
            if (m_netInterface && m_netInterface->GetConnectionRealTimeStatus(m_connection, &status, 0, nullptr) == k_EResultOK)
                return status.m_nPing;
            return -1;
        }

        /*! Drains and returns all ServerMessages received since the last call. */
        std::vector<ServerMessage> takeInbound()
        {
            std::vector<ServerMessage> result;
            while (!m_inbound.empty())
            {
                result.push_back(m_inbound.front());
                m_inbound.pop();
            }
            return result;
        }

        protected:
        void tick() override
        {
            pollServerMessages();
            pollConnectionStateChanges();
        }

        void pollServerMessages()
        {
            while (!m_quit)
            {
                ISteamNetworkingMessage* incomingMsg = nullptr;
                int numMsgs = m_netInterface->ReceiveMessagesOnConnection(m_connection, &incomingMsg, 1);
                if (numMsgs == 0)
                    break;
                if (numMsgs < 0)
                {
                    m_logger.error("Error checking for messages");
                    break;
                }

                if (incomingMsg->m_cbSize == sizeof(ServerMessage))
                {
                    ServerMessage data = *static_cast<ServerMessage*>(incomingMsg->m_pData);
                    m_inbound.push(data);
                }
                else if (incomingMsg->m_cbSize == sizeof(ChatMessage))
                {
                    ChatMessage chat {};
                    std::memcpy(&chat, incomingMsg->m_pData, sizeof(chat));
                    if (chat.type == NetMessageType::Chat || chat.type == NetMessageType::Kicked)
                    {
                        // Never trust a peer to have terminated its strings.
                        chat.name[sizeof(chat.name) - 1] = '\0';
                        chat.text[sizeof(chat.text) - 1] = '\0';
                        m_inboundChat.push(chat);
                    }
                }

                incomingMsg->Release();
            }
        }

        private:
        std::queue<ServerMessage> m_inbound;
        std::queue<ChatMessage> m_inboundChat;
    };

} // namespace ggj

#endif //GALDER_GAME_JAM_2024_PROJECT_GALDERCLIENT_HPP
