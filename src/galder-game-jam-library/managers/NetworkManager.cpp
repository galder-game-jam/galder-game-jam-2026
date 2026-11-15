//
// Created by robin on 27.06.2026.
//

#include "NetworkManager.h"
#include "../system/ChatText.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

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

        ServerMessage makeObjectRemovedMessage(uint32_t objectId, bool isCoin, uint32_t killerPlayerId)
        {
            ServerMessage state {};
            state.objectId = objectId;
            state.killerPlayerId = killerPlayerId;
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
            return state;
        }

        ServerMessage makeClaudlingSpawnedMessage(const ClaudlingSpawnEvent& spawn)
        {
            ServerMessage message {};
            message.type = NetMessageType::ClaudlingSpawned;
            message.objectId = spawn.objectId;
            message.targetPlayerId = spawn.targetPlayerId;
            message.position = spawn.position;
            message.followSpeed = spawn.followSpeed;
            message.knockback = spawn.knockback;
            return message;
        }
    } // namespace

    void NetworkManager::startHost(uint16_t port, const std::string& playerName, const raylib::Color& nameColor, const std::string& serverName, PlayerPickedCharacter character,
                                   const CharacterColors& colors)
    {
        shutdown();

        m_role = NetworkRole::Server;
        m_localName = playerName;
        m_localNameColor = nameColor;
        m_serverName = serverName;
        m_localCharacter = character;
        m_localColors = colors;
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
        if (!m_beacon.startAdvertising(serverName, port))
            m_logger.warning(fmt::format("LAN discovery unavailable: could not bind UDP port {0} (clients can still join by IP)", LanBeacon::DISCOVERY_PORT));
        m_logger.information(fmt::format("Hosting on port {0} as '{1}' (server: '{2}')", port, playerName, serverName));
    }

    void NetworkManager::startClient(const std::string& ipAddress, uint16_t port, const std::string& name, const raylib::Color& nameColor, PlayerPickedCharacter character,
                                     const CharacterColors& colors)
    {
        shutdown();

        m_role = NetworkRole::Client;
        m_localName = name;
        m_localNameColor = nameColor;
        m_localCharacter = character;
        m_localColors = colors;
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

    bool NetworkManager::hasLostConnection() const
    {
        return m_role == NetworkRole::Client && m_client && m_client->status() == ConnectionStatus::Error;
    }

    void NetworkManager::shutdown()
    {
        if (m_server)
        {
            // Close every client explicitly so they get "closed by host" now instead of timing out.
            m_server->closeAllConnections();
            m_server->stop();
            m_server.reset();
        }
        if (m_client)
        {
            m_client->disconnect();
            m_client.reset();
        }
        m_beacon.stop();

        m_role = NetworkRole::None;
        m_localPlayerId = HOST_PLAYER_ID;
        m_serverName.clear();
        m_cachedLocalIp.clear();
        m_cachedPublicIp.clear();
        m_localStats = NetworkPlayerStats {};
        m_remotePlayers.clear();
        m_connectionToPlayerId.clear();
        m_nextPlayerId = HOST_PLAYER_ID + 1;
        m_hasSentJoinRequest = false;
        m_wasKicked = false;
        m_kickReason.clear();
        m_currentStage = NO_STAGE;
        m_requestedStage.reset();
        m_chatMessages.clear();
        clearWorldState();
    }

    void NetworkManager::clearWorldState()
    {
        m_enemyStates.clear();
        m_removedObjectIds.clear();
        m_removedCoinIds.clear();
        m_locallyCreditedKills.clear();
        m_enemyDamageEvents.clear();
        m_claudlingSpawns.clear();
        m_bombSpawns.clear();
        m_rootSpawns.clear();
        m_cleaverSpawns.clear();
        m_spiderSpawns.clear();
        m_spawnedClaudlings.clear();
        m_projectileEvents.clear();
        m_isPortalEntered = false;
        m_hasPendingRemotePortalEntry = false;
    }

    void NetworkManager::setCurrentStage(int32_t stageIndex)
    {
        if (stageIndex == m_currentStage)
            return;

        clearWorldState();
        m_currentStage = stageIndex;

        if (m_role == NetworkRole::Server && m_server)
        {
            ServerMessage changed {};
            changed.type = NetMessageType::StageChanged;
            changed.stageIndex = stageIndex;
            m_server->broadcast(changed);
            m_logger.information(fmt::format("Host moved to stage {0}", stageIndex));
        }
    }

    std::optional<int32_t> NetworkManager::takeRequestedStage()
    {
        return std::exchange(m_requestedStage, std::nullopt);
    }

    void NetworkManager::followHostStage(int32_t stageIndex)
    {
        if (stageIndex == m_currentStage)
            return;

        // Clear right away rather than when the WorldManager applies the change: messages for the
        // new stage that arrive in the same poll must not be wiped afterwards.
        m_logger.information(fmt::format("Following host to stage {0}", stageIndex));
        clearWorldState();
        m_currentStage = stageIndex;
        m_requestedStage = stageIndex;
    }

    void NetworkManager::notifyLocalPortalEntered()
    {
        if (m_isPortalEntered)
            return;
        m_isPortalEntered = true;

        if (m_role == NetworkRole::Server && m_server)
        {
            ServerMessage entered {};
            entered.type = NetMessageType::PortalEntered;
            entered.playerId = HOST_PLAYER_ID;
            entered.stageIndex = m_currentStage;
            m_server->broadcast(entered);
        }
        else if (m_role == NetworkRole::Client && m_client && m_client->status() == ConnectionStatus::Connected)
        {
            ClientMessage entered {};
            entered.type = NetMessageType::PortalEntered;
            entered.playerId = m_localPlayerId;
            entered.stageIndex = m_currentStage;
            m_client->sendToServer(entered);
        }
    }

    bool NetworkManager::takeRemotePortalEntry()
    {
        return std::exchange(m_hasPendingRemotePortalEntry, false);
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
                    const bool isNewPlayer = it == m_connectionToPlayerId.end();
                    if (isNewPlayer)
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
                    accept.stageIndex = m_currentStage;
                    copyName(accept.name, m_serverName);
                    m_server->sendTo(connection, accept);
                    if (isNewPlayer)
                        sendWorldStateTo(connection);

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
                case NetMessageType::ProjectileFired:
                case NetMessageType::ProjectileImpact:
                {
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it == m_connectionToPlayerId.end() || message.stageIndex != m_currentStage)
                        break;

                    queueProjectileEvent(message.type, it->second, message.objectId, message.position, message.velocity);

                    ServerMessage relay {};
                    relay.type = message.type;
                    relay.playerId = it->second;
                    relay.objectId = message.objectId;
                    relay.position = message.position;
                    relay.velocity = message.velocity;
                    m_server->broadcast(relay);
                    break;
                }
                case NetMessageType::GameEvent:
                {
                    // A client picked up a coin / killed an enemy. Reconcile and tell everyone.
                    // Ids restart on every map, so an event from a stage we already left would
                    // remove the wrong object here.
                    if (message.stageIndex != m_currentStage)
                        break;
                    recordObjectRemoved(message.objectId, message.eventKind == 1);
                    break;
                }
                case NetMessageType::EnemyDamaged:
                {
                    // A client hit a boss: the host owns its health, so queue the hit for the World.
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it == m_connectionToPlayerId.end() || message.stageIndex != m_currentStage || message.damage <= 0)
                        break;
                    if (m_enemyDamageEvents.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_enemyDamageEvents.push_back(EnemyDamageEvent { message.objectId, it->second, message.damage });
                    break;
                }
                case NetMessageType::PortalEntered:
                {
                    auto it = m_connectionToPlayerId.find(connection);
                    if (it == m_connectionToPlayerId.end() || message.stageIndex != m_currentStage || m_isPortalEntered)
                        break;

                    // Send the host through, and relay so every other client follows too.
                    m_isPortalEntered = true;
                    m_hasPendingRemotePortalEntry = true;

                    ServerMessage relay {};
                    relay.type = NetMessageType::PortalEntered;
                    relay.playerId = it->second;
                    relay.stageIndex = m_currentStage;
                    m_server->broadcast(relay);
                    m_logger.information(fmt::format("Player id {0} entered the portal of stage {1}", it->second, m_currentStage));
                    break;
                }
                default:
                    break;
            }
        }

        // Relay chat. Who sent it comes from the connection, and the name / colour from our own
        // record of that player, so a client can't speak as somebody else.
        for (auto& [connection, chat] : m_server->takeInboundChat())
        {
            const auto it = m_connectionToPlayerId.find(connection);
            if (it == m_connectionToPlayerId.end())
                continue;

            chat.playerId = it->second;
            if (const auto player = m_remotePlayers.find(it->second); player != m_remotePlayers.end())
            {
                copyName(chat.name, player->second.name);
                chat.nameColor = player->second.nameColor;
            }
            queueChatMessage(chat);
            m_server->broadcastChat(chat);
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
        hostState.colors = m_localColors;
        hostState.position = m_localPosition;
        hostState.animationState = static_cast<uint8_t>(m_localPlayerState);
        hostState.facingLeft = m_isLocalFacingLeft;
        hostState.lives = m_localStats.lives;
        hostState.score = m_localStats.score;
        hostState.kills = m_localStats.kills;
        hostState.level = m_localStats.level;
        hostState.mana = m_localStats.mana;
        hostState.maxMana = m_localStats.maxMana;
        hostState.energy = m_localStats.energy;
        hostState.maxEnergy = m_localStats.maxEnergy;
        m_server->broadcast(hostState);

        for (const auto& [playerId, remote] : m_remotePlayers)
        {
            ServerMessage state {};
            state.type = NetMessageType::PlayerState;
            state.playerId = playerId;
            copyName(state.name, remote.name);
            state.nameColor = remote.nameColor;
            state.character = static_cast<uint8_t>(remote.character);
            state.colors = remote.colors;
            state.position = remote.targetPosition;
            state.animationState = static_cast<uint8_t>(remote.state);
            state.facingLeft = remote.facingLeft;
            state.lives = remote.lives;
            state.score = remote.score;
            state.kills = remote.kills;
            state.level = remote.level;
            state.mana = remote.mana;
            state.maxMana = remote.maxMana;
            state.energy = remote.energy;
            state.maxEnergy = remote.maxEnergy;
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
                join.colors = m_localColors;
                join.position = m_localPosition;
                join.stageIndex = m_currentStage;
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
            state.colors = m_localColors;
            state.position = m_localPosition;
            state.animationState = static_cast<uint8_t>(m_localPlayerState);
            state.facingLeft = m_isLocalFacingLeft;
            state.lives = m_localStats.lives;
            state.score = m_localStats.score;
            state.kills = m_localStats.kills;
            state.level = m_localStats.level;
            state.mana = m_localStats.mana;
            state.maxMana = m_localStats.maxMana;
            state.energy = m_localStats.energy;
            state.maxEnergy = m_localStats.maxEnergy;
            state.stageIndex = m_currentStage;
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
                    followHostStage(message.stageIndex);
                    break;
                case NetMessageType::StageChanged:
                    followHostStage(message.stageIndex);
                    break;
                case NetMessageType::PortalEntered:
                    // The host relays to everyone, including the player who walked in; skip our own.
                    if (message.playerId != m_localPlayerId && message.stageIndex == m_currentStage && !m_isPortalEntered)
                    {
                        m_isPortalEntered = true;
                        m_hasPendingRemotePortalEntry = true;
                    }
                    break;
                case NetMessageType::PlayerState:
                    applyPlayerSnapshot(message);
                    break;
                case NetMessageType::PlayerLeft:
                    m_remotePlayers.erase(message.playerId);
                    break;
                case NetMessageType::EnemyState:
                    if (message.flag) // alive: update position, facing and boss state
                        m_enemyStates[message.objectId] = EnemyNetworkState { message.position, message.facingLeft, message.lives, message.aiState, message.targetPlayerId };
                    else // dead: remove
                    {
                        m_enemyStates.erase(message.objectId);
                        m_removedObjectIds.insert(message.objectId);
                        if (message.killerPlayerId != NO_PLAYER_ID && message.killerPlayerId == m_localPlayerId)
                            m_locallyCreditedKills.insert(message.objectId);
                    }
                    break;
                case NetMessageType::BombSpawned:
                    if (m_bombSpawns.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_bombSpawns.push_back(BombSpawnEvent { message.position, message.velocity, message.fuseTime });
                    break;
                case NetMessageType::RootSpawned:
                    if (m_rootSpawns.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_rootSpawns.push_back(RootSpawnEvent { message.position, message.target, message.flightTime, message.spikeDelay });
                    break;
                case NetMessageType::CleaverSpawned:
                    if (m_cleaverSpawns.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_cleaverSpawns.push_back(CleaverSpawnEvent { message.position, message.velocity });
                    break;
                case NetMessageType::SpiderSpawned:
                    if (m_spiderSpawns.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_spiderSpawns.push_back(SpiderSpawnEvent { message.position, message.target, message.flightTime });
                    break;
                case NetMessageType::ClaudlingSpawned:
                    if (m_claudlingSpawns.size() < MAX_QUEUED_WORLD_EVENTS)
                        m_claudlingSpawns.push_back(ClaudlingSpawnEvent { message.objectId, message.targetPlayerId, message.position, message.followSpeed, message.knockback });
                    break;
                case NetMessageType::CoinState:
                    m_removedObjectIds.insert(message.objectId);
                    break;
                case NetMessageType::ProjectileFired:
                case NetMessageType::ProjectileImpact:
                    // The server relays to everyone, including the shooter; skip our own.
                    if (message.playerId != m_localPlayerId)
                        queueProjectileEvent(message.type, message.playerId, message.objectId, message.position, message.velocity);
                    break;
                default:
                    break;
            }
        }

        // The host relays to everyone, including the sender; ours is already queued.
        for (const ChatMessage& chat : m_client->takeInboundChat())
        {
            if (chat.type == NetMessageType::Kicked)
            {
                m_wasKicked = true;
                m_kickReason = chat.text;
                m_logger.information(m_kickReason.empty() ? "Kicked by the host" : fmt::format("Kicked by the host: {0}", m_kickReason));
                continue;
            }

            if (chat.playerId != m_localPlayerId)
                queueChatMessage(chat);
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
        remote.colors = message.colors;
        remote.targetPosition = message.position;
        remote.state = static_cast<PlayerState>(message.animationState);
        remote.facingLeft = message.facingLeft;
        remote.lives = message.lives;
        remote.score = message.score;
        remote.kills = message.kills;
        remote.level = message.level;
        remote.mana = message.mana;
        remote.maxMana = message.maxMana;
        remote.energy = message.energy;
        remote.maxEnergy = message.maxEnergy;

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
        remote.colors = message.colors;
        remote.targetPosition = message.position;
        remote.state = static_cast<PlayerState>(message.animationState);
        remote.facingLeft = message.facingLeft;
        remote.lives = message.lives;
        remote.score = message.score;
        remote.kills = message.kills;
        remote.level = message.level;
        remote.mana = message.mana;
        remote.maxMana = message.maxMana;
        remote.energy = message.energy;
        remote.maxEnergy = message.maxEnergy;

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
        host.lives = m_localStats.lives;
        host.score = m_localStats.score;
        host.kills = m_localStats.kills;
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

    bool NetworkManager::kickPlayer(uint32_t playerId, const std::string& reason)
    {
        if (m_role != NetworkRole::Server || !m_server || playerId == HOST_PLAYER_ID)
            return false;

        const auto it = std::find_if(m_connectionToPlayerId.begin(), m_connectionToPlayerId.end(), [playerId](const auto& entry) { return entry.second == playerId; });
        if (it == m_connectionToPlayerId.end())
            return false;

        ChatMessage notice {};
        notice.type = NetMessageType::Kicked;
        notice.playerId = playerId;
        ChatText::CopyUtf8Truncated(notice.text, std::min(sizeof(notice.text), MAX_KICK_REASON_BYTES + 1), reason);
        if (!m_server->kickClient(it->first, notice))
            return false;

        // The connection is closed on our side, so no disconnect event will arrive: do the same cleanup as a drop.
        m_remotePlayers.erase(playerId);
        m_connectionToPlayerId.erase(it);

        ServerMessage left {};
        left.type = NetMessageType::PlayerLeft;
        left.playerId = playerId;
        m_server->broadcast(left);
        m_logger.information(reason.empty() ? fmt::format("Player id {0} was kicked", playerId) : fmt::format("Player id {0} was kicked: {1}", playerId, reason));
        return true;
    }

    void NetworkManager::publishEnemyState(uint32_t objectId, const EnemyNetworkState& state)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        ServerMessage message {};
        message.type = NetMessageType::EnemyState;
        message.objectId = objectId;
        message.position = state.position;
        message.facingLeft = state.isFlipped;
        message.lives = state.lives;
        message.aiState = state.aiState;
        message.targetPlayerId = state.targetPlayerId;
        message.flag = true; // alive
        m_server->broadcast(message);
    }

    void NetworkManager::broadcastObjectRemoved(uint32_t objectId, bool isCoin, uint32_t killerPlayerId)
    {
        if (!m_server)
            return;

        m_server->broadcast(makeObjectRemovedMessage(objectId, isCoin, killerPlayerId));
    }

    void NetworkManager::recordObjectRemoved(uint32_t objectId, bool isCoin, uint32_t killerPlayerId)
    {
        if (!m_removedObjectIds.insert(objectId).second)
            return;

        if (isCoin)
            m_removedCoinIds.insert(objectId);
        broadcastObjectRemoved(objectId, isCoin, killerPlayerId);
    }

    void NetworkManager::notifyEnemyKilledByRemote(uint32_t objectId, uint32_t killerPlayerId)
    {
        if (m_role == NetworkRole::Server)
            recordObjectRemoved(objectId, false, killerPlayerId);
    }

    void NetworkManager::notifyLocalEnemyDamaged(uint32_t objectId, int32_t damage)
    {
        if (m_role != NetworkRole::Client || !m_client || m_client->status() != ConnectionStatus::Connected)
            return;

        ClientMessage event {};
        event.type = NetMessageType::EnemyDamaged;
        event.playerId = m_localPlayerId;
        event.objectId = objectId;
        event.damage = damage;
        event.stageIndex = m_currentStage;
        m_client->sendToServer(event);
    }

    std::vector<EnemyDamageEvent> NetworkManager::takeEnemyDamageEvents()
    {
        std::vector<EnemyDamageEvent> events;
        events.swap(m_enemyDamageEvents);
        return events;
    }

    void NetworkManager::publishClaudlingSpawned(const ClaudlingSpawnEvent& spawn)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        m_spawnedClaudlings[spawn.objectId] = spawn;
        m_server->broadcast(makeClaudlingSpawnedMessage(spawn));
    }

    std::vector<ClaudlingSpawnEvent> NetworkManager::takeClaudlingSpawns()
    {
        std::vector<ClaudlingSpawnEvent> spawns;
        spawns.swap(m_claudlingSpawns);
        return spawns;
    }

    void NetworkManager::publishBombSpawned(const BombSpawnEvent& bomb)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        // Not kept for late joiners: a bomb is gone within a few seconds.
        ServerMessage message {};
        message.type = NetMessageType::BombSpawned;
        message.position = bomb.position;
        message.velocity = bomb.velocity;
        message.fuseTime = bomb.fuseTime;
        m_server->broadcast(message);
    }

    std::vector<BombSpawnEvent> NetworkManager::takeBombSpawns()
    {
        std::vector<BombSpawnEvent> bombs;
        bombs.swap(m_bombSpawns);
        return bombs;
    }

    void NetworkManager::publishRootSpawned(const RootSpawnEvent& root)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        // Not kept for late joiners: a root is gone within a few seconds.
        ServerMessage message {};
        message.type = NetMessageType::RootSpawned;
        message.position = root.origin;
        message.target = root.target;
        message.flightTime = root.flightTime;
        message.spikeDelay = root.spikeDelay;
        m_server->broadcast(message);
    }

    std::vector<RootSpawnEvent> NetworkManager::takeRootSpawns()
    {
        std::vector<RootSpawnEvent> roots;
        roots.swap(m_rootSpawns);
        return roots;
    }

    void NetworkManager::publishCleaverSpawned(const CleaverSpawnEvent& cleaver)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        // Not kept for late joiners: a cleaver is gone within a few seconds.
        ServerMessage message {};
        message.type = NetMessageType::CleaverSpawned;
        message.position = cleaver.position;
        message.velocity = cleaver.velocity;
        m_server->broadcast(message);
    }

    std::vector<CleaverSpawnEvent> NetworkManager::takeCleaverSpawns()
    {
        std::vector<CleaverSpawnEvent> cleavers;
        cleavers.swap(m_cleaverSpawns);
        return cleavers;
    }

    void NetworkManager::publishSpiderSpawned(const SpiderSpawnEvent& spider)
    {
        if (m_role != NetworkRole::Server || !m_server)
            return;

        // Not kept for late joiners: a spider is gone within a couple of seconds.
        ServerMessage message {};
        message.type = NetMessageType::SpiderSpawned;
        message.position = spider.origin;
        message.target = spider.target;
        message.flightTime = spider.flightTime;
        m_server->broadcast(message);
    }

    std::vector<SpiderSpawnEvent> NetworkManager::takeSpiderSpawns()
    {
        std::vector<SpiderSpawnEvent> spiders;
        spiders.swap(m_spiderSpawns);
        return spiders;
    }

    void NetworkManager::sendWorldStateTo(HSteamNetConnection connection)
    {
        // Claudlings the boss already spawned: the late joiner's map never had them, so without
        // this their ids would be unknown there and they would be invisible yet still deadly.
        for (const auto& [objectId, spawn] : m_spawnedClaudlings)
        {
            if (m_removedObjectIds.count(objectId) == 0)
                m_server->sendTo(connection, makeClaudlingSpawnedMessage(spawn));
        }

        // Without this a late joiner sees coins/enemies others already took, and its enemy count
        // (and so its portal) never reaches zero.
        for (const uint32_t objectId : m_removedObjectIds)
            m_server->sendTo(connection, makeObjectRemovedMessage(objectId, m_removedCoinIds.count(objectId) > 0, NO_PLAYER_ID));

        if (m_isPortalEntered)
        {
            ServerMessage entered {};
            entered.type = NetMessageType::PortalEntered;
            entered.playerId = HOST_PLAYER_ID;
            entered.stageIndex = m_currentStage;
            m_server->sendTo(connection, entered);
        }
    }

    void NetworkManager::notifyLocalObjectDestroyed(uint32_t objectId, bool isCoin)
    {
        if (m_role == NetworkRole::Server)
        {
            // Host is authoritative: record + tell all clients.
            recordObjectRemoved(objectId, isCoin);
        }
        else if (m_role == NetworkRole::Client && m_client)
        {
            // Report to the host, which reconciles and rebroadcasts.
            ClientMessage event {};
            event.type = NetMessageType::GameEvent;
            event.objectId = objectId;
            event.eventKind = isCoin ? 1 : 2;
            event.stageIndex = m_currentStage;
            m_client->sendToServer(event);
            m_removedObjectIds.insert(objectId);
        }
    }

    void NetworkManager::notifyLocalProjectileFired(uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity)
    {
        sendProjectileEvent(NetMessageType::ProjectileFired, projectileId, position, velocity);
    }

    void NetworkManager::notifyLocalProjectileImpact(uint32_t projectileId, const raylib::Vector2& position)
    {
        sendProjectileEvent(NetMessageType::ProjectileImpact, projectileId, position, raylib::Vector2 { 0.0f, 0.0f });
    }

    std::vector<ProjectileEvent> NetworkManager::takeProjectileEvents()
    {
        std::vector<ProjectileEvent> events;
        events.swap(m_projectileEvents);
        return events;
    }

    void NetworkManager::sendChatMessage(const std::string& text)
    {
        ChatMessage chat {};
        chat.type = NetMessageType::Chat;
        chat.playerId = m_localPlayerId;
        copyName(chat.name, m_localName);
        chat.nameColor = m_localNameColor;
        ChatText::CopyUtf8Truncated(chat.text, sizeof(chat.text), text);
        if (chat.text[0] == '\0')
            return;

        if (m_role == NetworkRole::Server && m_server)
        {
            m_server->broadcastChat(chat);
            queueChatMessage(chat);
        }
        // A client only has a player id (and so can be relayed) once the host accepted the join.
        else if (m_role == NetworkRole::Client && m_client && m_client->status() == ConnectionStatus::Connected && m_localPlayerId != HOST_PLAYER_ID)
        {
            m_client->sendChatToServer(chat);
            queueChatMessage(chat);
        }
    }

    std::vector<ChatEntry> NetworkManager::takeChatMessages()
    {
        std::vector<ChatEntry> messages;
        messages.swap(m_chatMessages);
        return messages;
    }

    void NetworkManager::queueChatMessage(const ChatMessage& message)
    {
        if (m_chatMessages.size() >= MAX_QUEUED_CHAT_MESSAGES)
            m_chatMessages.erase(m_chatMessages.begin());

        ChatEntry entry {};
        entry.name = std::string(message.name, strnlen(message.name, sizeof(message.name)));
        entry.nameColor = message.nameColor;
        entry.text = std::string(message.text, strnlen(message.text, sizeof(message.text)));
        m_chatMessages.push_back(entry);
    }

    void NetworkManager::sendProjectileEvent(NetMessageType type, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity)
    {
        if (m_role == NetworkRole::Server && m_server)
        {
            ServerMessage message {};
            message.type = type;
            message.playerId = HOST_PLAYER_ID;
            message.objectId = projectileId;
            message.position = position;
            message.velocity = velocity;
            m_server->broadcast(message);
        }
        else if (m_role == NetworkRole::Client && m_client && m_client->status() == ConnectionStatus::Connected)
        {
            ClientMessage message {};
            message.type = type;
            message.playerId = m_localPlayerId;
            message.objectId = projectileId;
            message.position = position;
            message.velocity = velocity;
            message.stageIndex = m_currentStage;
            m_client->sendToServer(message);
        }
    }

    void NetworkManager::queueProjectileEvent(NetMessageType type, uint32_t ownerId, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity)
    {
        if (m_projectileEvents.size() >= MAX_QUEUED_PROJECTILE_EVENTS)
            return;

        m_projectileEvents.push_back(ProjectileEvent { ownerId, projectileId, type == NetMessageType::ProjectileImpact, position, velocity });
    }

    void NetworkManager::interpolateRemotePlayers(float timeDelta)
    {
        const float t = std::clamp(timeDelta * INTERPOLATION_RATE, 0.0f, 1.0f);
        for (auto& [playerId, remote] : m_remotePlayers)
        {
            const float dx = remote.targetPosition.x - remote.position.x;
            const float dy = remote.targetPosition.y - remote.position.y;

            remote.isMoving = (dx * dx + dy * dy) > MOVING_THRESHOLD_SQUARED;

            remote.position.x += dx * t;
            remote.position.y += dy * t;
        }
    }
} // namespace ggj
