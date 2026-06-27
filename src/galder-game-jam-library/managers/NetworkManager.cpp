//
// Created by robin on 27.06.2026.
//

#include "NetworkManager.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace ggj
{
    namespace
    {
        // How quickly remote players catch up to their latest received position (per second).
        constexpr float INTERPOLATION_RATE = 12.0f;
        // Squared distance below which a remote player is considered "standing still" (idle anim).
        constexpr float MOVING_THRESHOLD_SQUARED = 0.25f;

        void copyName(char (&destination)[21], const std::string& source)
        {
            std::strncpy(destination, source.c_str(), sizeof(destination) - 1);
            destination[sizeof(destination) - 1] = '\0';
        }
    } // namespace

    void NetworkManager::startHost(uint16_t port, const std::string& name, PlayerPickedCharacter character)
    {
        shutdown();

        m_role = NetworkRole::Server;
        m_localName = name;
        m_localCharacter = character;
        m_localPlayerId = HOST_PLAYER_ID;

        m_server = std::make_unique<GalderServer>(m_logger, m_resolver);
        if (!m_server->initialize(port, name) || !m_server->startListening())
        {
            m_logger.error("Failed to start host");
            shutdown();
            return;
        }
        m_logger.information(fmt::format("Hosting on port {0} as '{1}'", port, name));
    }

    void NetworkManager::startClient(const std::string& ipAddress, uint16_t port, const std::string& name, PlayerPickedCharacter character)
    {
        shutdown();

        m_role = NetworkRole::Client;
        m_localName = name;
        m_localCharacter = character;
        m_localPlayerId = INVALID_PLAYER_ID; // assigned by the server via JoinAccept
        m_hasSentJoinRequest = false;

        m_client = std::make_unique<GalderClient>(m_logger, m_resolver);
        if (!m_client->initialize())
        {
            m_logger.error("Failed to initialize client");
            shutdown();
            return;
        }
        m_client->beginConnect(port, ipAddress);
        m_logger.information(fmt::format("Connecting to {0}:{1} as '{2}'", ipAddress, port, name));
    }

    void NetworkManager::shutdown()
    {
        if (m_server)
        {
            m_server->stop();
            m_server.reset();
        }
        m_client.reset();

        m_role = NetworkRole::None;
        m_localPlayerId = HOST_PLAYER_ID;
        m_remotePlayers.clear();
        m_connectionToPlayerId.clear();
        m_nextPlayerId = HOST_PLAYER_ID + 1;
        m_hasSentJoinRequest = false;
    }

    ConnectionStatus NetworkManager::status() const
    {
        switch (m_role)
        {
            case NetworkRole::Server:
                return m_server ? ConnectionStatus::Connected : ConnectionStatus::None;
            case NetworkRole::Client:
                return m_client ? m_client->status() : ConnectionStatus::None;
            default:
                return ConnectionStatus::None;
        }
    }

    void NetworkManager::poll(float timeDelta)
    {
        switch (m_role)
        {
            case NetworkRole::Server:
                pollServer(timeDelta);
                break;
            case NetworkRole::Client:
                pollClient(timeDelta);
                break;
            default:
                break;
        }
    }

    void NetworkManager::pollServer(float timeDelta)
    {
        m_server->poll();

        // Apply inbound client messages.
        for (const auto& [connection, message] : m_server->takeInbound())
        {
            switch (message.type)
            {
                case NetMessageType::JoinRequest:
                {
                    uint32_t playerId;
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it == m_connectionToPlayerId.end())
                    {
                        playerId = m_nextPlayerId++;
                        m_connectionToPlayerId[connection] = playerId;
                        m_logger.information(fmt::format("Player '{0}' joined as id {1}", message.name, playerId));
                    }
                    else
                    {
                        playerId = it->second;
                    }

                    ServerMessage accept {};
                    accept.type = NetMessageType::JoinAccept;
                    accept.playerId = playerId;
                    m_server->sendTo(connection, accept);

                    applyPlayerSnapshot(playerId, message.name, message.character, message.position);
                    break;
                }
                case NetMessageType::PlayerState:
                {
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it != m_connectionToPlayerId.end())
                        applyPlayerSnapshot(it->second, message.name, message.character, message.position);
                    break;
                }
                default:
                    break;
            }
        }

        // Handle drops: remove the player and tell everyone.
        for (HSteamNetConnection connection : m_server->takeDisconnected())
        {
            auto it = m_connectionToPlayerId.find(connection);
            if (it == m_connectionToPlayerId.end())
                continue;

            const uint32_t playerId = it->second;
            m_remotePlayers.erase(playerId);
            m_connectionToPlayerId.erase(it);

            ServerMessage left {};
            left.type = NetMessageType::PlayerLeft;
            left.playerId = playerId;
            m_server->broadcast(left);
            m_logger.information(fmt::format("Player id {0} left", playerId));
        }

        // Broadcast the authoritative state of every player (host + clients) to all clients.
        ServerMessage hostState {};
        hostState.type = NetMessageType::PlayerState;
        hostState.playerId = HOST_PLAYER_ID;
        copyName(hostState.name, m_localName);
        hostState.character = static_cast<uint8_t>(m_localCharacter);
        hostState.position = m_localPosition;
        m_server->broadcast(hostState);

        for (const auto& [playerId, remote] : m_remotePlayers)
        {
            ServerMessage state {};
            state.type = NetMessageType::PlayerState;
            state.playerId = playerId;
            copyName(state.name, remote.name);
            state.character = static_cast<uint8_t>(remote.character);
            state.position = remote.targetPosition;
            m_server->broadcast(state);
        }

        interpolateRemotePlayers(timeDelta);
    }

    void NetworkManager::pollClient(float timeDelta)
    {
        m_client->poll();

        // Once connected, announce ourselves, then stream our position every tick.
        if (m_client->status() == ConnectionStatus::Connected)
        {
            if (!m_hasSentJoinRequest)
            {
                ClientMessage join {};
                join.type = NetMessageType::JoinRequest;
                copyName(join.name, m_localName);
                join.character = static_cast<uint8_t>(m_localCharacter);
                join.position = m_localPosition;
                m_client->sendToServer(join);
                m_hasSentJoinRequest = true;
            }

            ClientMessage state {};
            state.type = NetMessageType::PlayerState;
            state.playerId = m_localPlayerId;
            copyName(state.name, m_localName);
            state.character = static_cast<uint8_t>(m_localCharacter);
            state.position = m_localPosition;
            m_client->sendToServer(state);
        }

        for (const ServerMessage& message : m_client->takeInbound())
        {
            switch (message.type)
            {
                case NetMessageType::JoinAccept:
                    m_localPlayerId = message.playerId;
                    m_logger.information(fmt::format("Joined as player id {0}", message.playerId));
                    break;
                case NetMessageType::PlayerState:
                    applyPlayerSnapshot(message.playerId, message.name, message.character, message.position);
                    break;
                case NetMessageType::PlayerLeft:
                    m_remotePlayers.erase(message.playerId);
                    break;
                default:
                    break;
            }
        }

        interpolateRemotePlayers(timeDelta);
    }

    void NetworkManager::applyPlayerSnapshot(uint32_t playerId, const char* name, uint8_t character, const raylib::Vector2& position)
    {
        if (playerId == m_localPlayerId)
            return; // never track ourselves as a remote player

        RemotePlayer& remote = m_remotePlayers[playerId];
        remote.playerId = playerId;
        remote.name = name;
        remote.character = static_cast<PlayerPickedCharacter>(character);
        remote.targetPosition = position;

        // First time we see this player, snap rather than interpolate from origin.
        if (remote.position.x == 0.0f && remote.position.y == 0.0f)
            remote.position = position;
    }

    void NetworkManager::interpolateRemotePlayers(float timeDelta)
    {
        const float t = std::clamp(timeDelta * INTERPOLATION_RATE, 0.0f, 1.0f);
        for (auto& [playerId, remote] : m_remotePlayers)
        {
            const float dx = remote.targetPosition.x - remote.position.x;
            const float dy = remote.targetPosition.y - remote.position.y;

            remote.isMoving = (dx * dx + dy * dy) > MOVING_THRESHOLD_SQUARED;
            if (std::fabs(dx) > 0.01f)
                remote.facingLeft = dx < 0.0f;

            remote.position.x += dx * t;
            remote.position.y += dy * t;
        }
    }
} // namespace ggj
