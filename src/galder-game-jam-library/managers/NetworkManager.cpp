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

    void NetworkManager::startHost(uint16_t port, const std::string& playerName, const raylib::Color& nameColor, const std::string& serverName, PlayerPickedCharacter character)
    {
        shutdown();

        m_role = NetworkRole::Server;
        m_localName = playerName;
        m_localNameColor = nameColor;
        m_serverName = serverName;
        m_localCharacter = character;
        m_localPlayerId = HOST_PLAYER_ID;
        m_cachedLocalIp = m_resolver.getLocalIpAddress();
        m_cachedPublicIp = m_resolver.getPublicIpAddress();

        m_server = std::make_unique<GalderServer>(m_logger, m_resolver);
        if (!m_server->initialize(port, serverName) || !m_server->startListening())
        {
            m_logger.error("Failed to start host");
            shutdown();
            return;
        }
        m_beacon.startAdvertising(serverName, port);
        m_logger.information(fmt::format("Hosting on port {0} as '{1}' (server: '{2}')", port, playerName, serverName));
    }

    void NetworkManager::startClient(const std::string& ipAddress, uint16_t port, const std::string& name, const raylib::Color& nameColor, PlayerPickedCharacter character)
    {
        shutdown();

        m_role = NetworkRole::Client;
        m_localName = name;
        m_localNameColor = nameColor;
        m_localCharacter = character;
        m_localPlayerId = INVALID_PLAYER_ID; // assigned by the server via JoinAccept
        m_hasSentJoinRequest = false;
        m_cachedLocalIp = m_resolver.getLocalIpAddress();
        m_cachedPublicIp = m_resolver.getPublicIpAddress();

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

    int32_t NetworkManager::pingToServer() const
    {
        if (m_role == NetworkRole::Client && m_client)
            return m_client->getPing();
        return -1;
    }

    void NetworkManager::shutdown()
    {
        if (m_server)
        {
            m_server->stop();
            m_server.reset();
        }
        m_client.reset();
        m_beacon.stop();

        m_role = NetworkRole::None;
        m_localPlayerId = HOST_PLAYER_ID;
        m_serverName.clear();
        m_cachedLocalIp.clear();
        m_cachedPublicIp.clear();
        m_localLives = 0;
        m_localScore = 0;
        m_localKills = 0;
        m_remotePlayers.clear();
        m_connectionToPlayerId.clear();
        m_nextPlayerId = HOST_PLAYER_ID + 1;
        m_hasSentJoinRequest = false;
        m_enemyPositions.clear();
        m_removedObjectIds.clear();
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

        // Keep advertising our presence on the LAN with an up-to-date player count.
        m_beacon.setPlayerCount(static_cast<int32_t>(m_server->connectedClientCount()) + 1);
        m_beacon.poll(timeDelta);

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
                    copyName(accept.name, m_serverName);
                    m_server->sendTo(connection, accept);

                    applyPlayerSnapshot(playerId, message);
                    m_remotePlayers[playerId].localIp = message.localIp;
                    m_remotePlayers[playerId].publicIp = message.publicIp;
                    break;
                }
                case NetMessageType::PlayerState:
                {
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it != m_connectionToPlayerId.end())
                        applyPlayerSnapshot(it->second, message);
                    break;
                }
                case NetMessageType::GameEvent:
                {
                    // A client picked up a coin / killed an enemy. Reconcile and tell everyone.
                    const bool isCoin = message.eventKind == 1;
                    if (m_removedObjectIds.insert(message.objectId).second)
                        broadcastObjectRemoved(message.objectId, isCoin);
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
        hostState.nameColor = m_localNameColor;
        hostState.character = static_cast<uint8_t>(m_localCharacter);
        hostState.position = m_localPosition;
        hostState.lives = m_localLives;
        hostState.score = m_localScore;
        hostState.kills = m_localKills;
        m_server->broadcast(hostState);

        for (const auto& [playerId, remote] : m_remotePlayers)
        {
            ServerMessage state {};
            state.type = NetMessageType::PlayerState;
            state.playerId = playerId;
            copyName(state.name, remote.name);
            state.nameColor = remote.nameColor;
            state.character = static_cast<uint8_t>(remote.character);
            state.position = remote.targetPosition;
            state.lives = remote.lives;
            state.score = remote.score;
            state.kills = remote.kills;
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
                join.nameColor = m_localNameColor;
                join.character = static_cast<uint8_t>(m_localCharacter);
                join.position = m_localPosition;
                std::strncpy(join.localIp, m_cachedLocalIp.c_str(), sizeof(join.localIp) - 1);
                std::strncpy(join.publicIp, m_cachedPublicIp.c_str(), sizeof(join.publicIp) - 1);
                m_client->sendToServer(join);
                m_hasSentJoinRequest = true;
            }

            ClientMessage state {};
            state.type = NetMessageType::PlayerState;
            state.playerId = m_localPlayerId;
            copyName(state.name, m_localName);
            state.nameColor = m_localNameColor;
            state.character = static_cast<uint8_t>(m_localCharacter);
            state.position = m_localPosition;
            state.lives = m_localLives;
            state.score = m_localScore;
            state.kills = m_localKills;
            m_client->sendToServer(state);
        }

        for (const ServerMessage& message : m_client->takeInbound())
        {
            switch (message.type)
            {
                case NetMessageType::JoinAccept:
                    m_localPlayerId = message.playerId;
                    m_serverName = std::string(message.name, strnlen(message.name, sizeof(message.name)));
                    m_logger.information(fmt::format("Joined '{0}' as player id {1}", m_serverName, message.playerId));
                    break;
                case NetMessageType::PlayerState:
                    applyPlayerSnapshot(message);
                    break;
                case NetMessageType::PlayerLeft:
                    m_remotePlayers.erase(message.playerId);
                    break;
                case NetMessageType::EnemyState:
                    if (message.flag) // alive: update position
                        m_enemyPositions[message.objectId] = message.position;
                    else // dead: remove
                    {
                        m_enemyPositions.erase(message.objectId);
                        m_removedObjectIds.insert(message.objectId);
                    }
                    break;
                case NetMessageType::CoinState:
                    m_removedObjectIds.insert(message.objectId);
                    break;
                default:
                    break;
            }
        }

        interpolateRemotePlayers(timeDelta);
    }

    void NetworkManager::applyPlayerSnapshot(const ServerMessage& message)
    {
        if (message.playerId == m_localPlayerId)
            return; // never track ourselves as a remote player

        RemotePlayer& remote = m_remotePlayers[message.playerId];
        remote.playerId = message.playerId;
        remote.name = message.name;
        remote.nameColor = message.nameColor;
        remote.character = static_cast<PlayerPickedCharacter>(message.character);
        remote.targetPosition = message.position;
        remote.lives = message.lives;
        remote.score = message.score;
        remote.kills = message.kills;

        // First time we see this player, snap rather than interpolate from origin.
        if (remote.position.x == 0.0f && remote.position.y == 0.0f)
            remote.position = message.position;
    }

    void NetworkManager::applyPlayerSnapshot(uint32_t playerId, const ClientMessage& message)
    {
        if (playerId == m_localPlayerId)
            return;

        RemotePlayer& remote = m_remotePlayers[playerId];
        remote.playerId = playerId;
        remote.name = message.name;
        remote.nameColor = message.nameColor;
        remote.character = static_cast<PlayerPickedCharacter>(message.character);
        remote.targetPosition = message.position;
        remote.lives = message.lives;
        remote.score = message.score;
        remote.kills = message.kills;

        if (remote.position.x == 0.0f && remote.position.y == 0.0f)
            remote.position = message.position;
    }

    std::vector<ConnectedPlayer> NetworkManager::connectedPlayers() const
    {
        std::vector<ConnectedPlayer> result;
        if (m_role != NetworkRole::Server)
            return result;

        ConnectedPlayer host;
        host.playerId = HOST_PLAYER_ID;
        host.name = m_localName;
        host.isHost = true;
        host.position = m_localPosition;
        host.lives = m_localLives;
        host.score = m_localScore;
        host.kills = m_localKills;
        host.pingMs = -1;
        host.localIp = m_cachedLocalIp;
        host.publicIp = m_cachedPublicIp;
        result.push_back(host);

        for (const auto& [playerId, remote] : m_remotePlayers)
        {
            ConnectedPlayer player;
            player.playerId = playerId;
            player.name = remote.name;
            player.isHost = false;
            player.position = remote.targetPosition;
            player.lives = remote.lives;
            player.score = remote.score;
            player.kills = remote.kills;
            player.localIp = remote.localIp;
            player.publicIp = remote.publicIp;

            if (m_server)
            {
                for (const auto& [connection, mappedId] : m_connectionToPlayerId)
                {
                    if (mappedId == playerId)
                    {
                        player.pingMs = m_server->getPing(connection);
                        break;
                    }
                }
            }
            result.push_back(player);
        }
        return result;
    }

    void NetworkManager::publishEnemyState(uint32_t objectId, const raylib::Vector2& position, bool facingLeft)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        ServerMessage state {};
        state.type = NetMessageType::EnemyState;
        state.objectId = objectId;
        state.position = position;
        state.facingLeft = facingLeft;
        state.flag = true; // alive
        m_server->broadcast(state);
    }

    void NetworkManager::broadcastObjectRemoved(uint32_t objectId, bool isCoin)
    {
        if (!m_server)
            return;

        ServerMessage state {};
        state.objectId = objectId;
        if (isCoin)
        {
            state.type = NetMessageType::CoinState;
            state.flag = true; // collected
        }
        else
        {
            state.type = NetMessageType::EnemyState;
            state.flag = false; // dead
        }
        m_server->broadcast(state);
    }

    void NetworkManager::notifyLocalObjectDestroyed(uint32_t objectId, bool isCoin)
    {
        if (m_role == NetworkRole::Server)
        {
            // Host is authoritative: record + tell all clients.
            if (m_removedObjectIds.insert(objectId).second)
                broadcastObjectRemoved(objectId, isCoin);
        }
        else if (m_role == NetworkRole::Client && m_client)
        {
            // Report to the host, which reconciles and rebroadcasts.
            ClientMessage event {};
            event.type = NetMessageType::GameEvent;
            event.objectId = objectId;
            event.eventKind = isCoin ? 1 : 2;
            m_client->sendToServer(event);
            m_removedObjectIds.insert(objectId);
        }
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
