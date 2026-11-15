//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
#define GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H

#include "../interfaces/manager/INetworkManager.h"
#include "../network/GalderServer.hpp"
#include "../network/GalderClient.hpp"
#include "../network/LanBeacon.h"
#include "../../gamedev-base/GameDevBase.h"

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
        static constexpr uint32_t INVALID_PLAYER_ID = NO_PLAYER_ID;

        NetworkManager(ILogger& logger, IIpAddressResolver& ipAddressResolver) : m_logger { logger }, m_resolver { ipAddressResolver } {}
        /*! Closes any open session gracefully so peers drop us right away when the game quits. */
        ~NetworkManager() override { shutdown(); }

        void startHost(uint16_t port, const std::string& playerName, const raylib::Color& nameColor, const std::string& serverName, PlayerPickedCharacter character,
                       const CharacterColors& colors) override;
        void startClient(const std::string& ipAddress, uint16_t port, const std::string& name, const raylib::Color& nameColor, PlayerPickedCharacter character,
                         const CharacterColors& colors) override;
        void shutdown() override;

        void poll(float timeDelta) override;
        void setLocalPlayerPosition(const raylib::Vector2& position) override { m_localPosition = position; }
        void setLocalPlayerAnimation(PlayerState state, bool isFacingLeft) override
        {
            m_localPlayerState = state;
            m_isLocalFacingLeft = isFacingLeft;
        }

        [[nodiscard]] NetworkRole role() const override { return m_role; }
        [[nodiscard]] bool isActive() const override { return m_role != NetworkRole::None; }
        [[nodiscard]] ConnectionStatus status() const override;
        [[nodiscard]] uint32_t localPlayerId() const override { return m_localPlayerId; }
        [[nodiscard]] const std::string& localPlayerName() const override { return m_localName; }
        [[nodiscard]] const raylib::Color& localPlayerNameColor() const override { return m_localNameColor; }
        [[nodiscard]] const std::string& serverName() const override { return m_serverName; }
        [[nodiscard]] std::string localIpAddress() const override { return m_cachedLocalIp; }
        [[nodiscard]] std::string publicIpAddress() const override { return m_cachedPublicIp; }
        [[nodiscard]] int32_t pingToServer() const override;
        [[nodiscard]] bool hasLostConnection() const override;
        [[nodiscard]] bool wasKicked() const override { return m_wasKicked; }
        [[nodiscard]] const std::string& kickReason() const override { return m_kickReason; }
        [[nodiscard]] bool hasEverConnected() const override { return m_hasSentJoinRequest; }
        [[nodiscard]] const std::map<uint32_t, RemotePlayer>& remotePlayers() const override { return m_remotePlayers; }

        void publishEnemyState(uint32_t objectId, const EnemyNetworkState& state) override;
        void notifyLocalObjectDestroyed(uint32_t objectId, bool isCoin) override;
        void notifyEnemyKilledByRemote(uint32_t objectId, uint32_t killerPlayerId) override;
        [[nodiscard]] bool isKillCreditedToLocalPlayer(uint32_t objectId) const override { return m_locallyCreditedKills.count(objectId) > 0; }
        void notifyLocalEnemyDamaged(uint32_t objectId, int32_t damage) override;
        [[nodiscard]] std::vector<EnemyDamageEvent> takeEnemyDamageEvents() override;
        void publishClaudlingSpawned(const ClaudlingSpawnEvent& spawn) override;
        [[nodiscard]] std::vector<ClaudlingSpawnEvent> takeClaudlingSpawns() override;
        void publishBombSpawned(const BombSpawnEvent& bomb) override;
        [[nodiscard]] std::vector<BombSpawnEvent> takeBombSpawns() override;
        void publishRootSpawned(const RootSpawnEvent& root) override;
        [[nodiscard]] std::vector<RootSpawnEvent> takeRootSpawns() override;
        void publishCleaverSpawned(const CleaverSpawnEvent& cleaver) override;
        [[nodiscard]] std::vector<CleaverSpawnEvent> takeCleaverSpawns() override;
        void publishSpiderSpawned(const SpiderSpawnEvent& spider) override;
        [[nodiscard]] std::vector<SpiderSpawnEvent> takeSpiderSpawns() override;
        void publishIceCircleSpawned(const IceCircleSpawnEvent& circle) override;
        [[nodiscard]] std::vector<IceCircleSpawnEvent> takeIceCircleSpawns() override;
        void publishFireLineSpawned(const FireLineSpawnEvent& fire) override;
        [[nodiscard]] std::vector<FireLineSpawnEvent> takeFireLineSpawns() override;
        void publishCrystalSpawned(const CrystalSpawnEvent& crystal) override;
        [[nodiscard]] std::vector<CrystalSpawnEvent> takeCrystalSpawns() override;
        void setLocalPlayerStats(const NetworkPlayerStats& stats) override { m_localStats = stats; }
        void setCurrentStage(int32_t stageIndex) override;
        [[nodiscard]] std::optional<int32_t> takeRequestedStage() override;
        void notifyLocalPortalEntered() override;
        [[nodiscard]] bool takeRemotePortalEntry() override;
        [[nodiscard]] const std::map<uint32_t, EnemyNetworkState>& enemyStates() const override { return m_enemyStates; }
        void notifyLocalProjectileFired(uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity) override;
        void notifyLocalProjectileImpact(uint32_t projectileId, const raylib::Vector2& position) override;
        [[nodiscard]] std::vector<ProjectileEvent> takeProjectileEvents() override;
        void notifyLocalSound(SoundName sound) override;
        [[nodiscard]] std::vector<SoundEvent> takeSoundEvents() override;
        [[nodiscard]] const std::set<uint32_t>& removedObjectIds() const override { return m_removedObjectIds; }

        [[nodiscard]] std::vector<ConnectedPlayer> connectedPlayers() const override;
        bool kickPlayer(uint32_t playerId, const std::string& reason) override;

        void sendChatMessage(const std::string& text) override;
        [[nodiscard]] std::vector<ChatEntry> takeChatMessages() override;

        void startDiscovery() override { m_beacon.startListening(); }
        void stopDiscovery() override { m_beacon.stop(); }
        void pollDiscovery(float timeDelta) override { m_beacon.poll(timeDelta); }
        [[nodiscard]] std::vector<DiscoveredServer> discoveredServers() const override { return m_beacon.discoveredServers(); }

        private:
        void pollServer(float timeDelta);
        void pollClient(float timeDelta);
        void applyPlayerSnapshot(const ServerMessage& message);
        void applyPlayerSnapshot(uint32_t playerId, const ClientMessage& message);
        void interpolateRemotePlayers(float timeDelta);
        void broadcastObjectRemoved(uint32_t objectId, bool isCoin, uint32_t killerPlayerId);
        void recordObjectRemoved(uint32_t objectId, bool isCoin, uint32_t killerPlayerId = NO_PLAYER_ID);
        /*! Server: brings a just-joined client up to date with the stage it is dropped into. */
        void sendWorldStateTo(HSteamNetConnection connection);
        /*! Client: follows the host to the given stage (on join or when the host moves on). */
        void followHostStage(int32_t stageIndex);
        /*! Clears per-world synced state; object ids restart on every map. */
        void clearWorldState();
        void sendProjectileEvent(NetMessageType type, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity);
        /*! Queues a chat line for takeChatMessages(), dropping the oldest when nobody drains. */
        void queueChatMessage(const ChatMessage& message);
        /*! Queues a remote sound for takeSoundEvents(); ignores ids that are not networked sounds. */
        void queueSoundEvent(uint32_t ownerId, uint32_t soundId);
        void queueProjectileEvent(NetMessageType type, uint32_t ownerId, uint32_t projectileId, const raylib::Vector2& position, const raylib::Vector2& velocity);

        ILogger& m_logger;
        IIpAddressResolver& m_resolver;
        std::unique_ptr<GalderServer> m_server;
        std::unique_ptr<GalderClient> m_client;

        NetworkRole m_role { NetworkRole::None };
        uint32_t m_localPlayerId { HOST_PLAYER_ID };
        std::string m_localName {};
        raylib::Color m_localNameColor { RED };
        std::string m_serverName {};
        std::string m_cachedLocalIp {};
        std::string m_cachedPublicIp {};
        PlayerPickedCharacter m_localCharacter { PlayerPickedCharacter::Rogue };
        CharacterColors m_localColors {};
        raylib::Vector2 m_localPosition {};
        PlayerState m_localPlayerState { PlayerState::Idle };
        bool m_isLocalFacingLeft { false };
        NetworkPlayerStats m_localStats {};

        std::map<uint32_t, RemotePlayer> m_remotePlayers;

        // World-state sync (Phase 2)
        std::map<uint32_t, EnemyNetworkState> m_enemyStates;  ///< Client: latest enemy position + flip.
        std::vector<ProjectileEvent> m_projectileEvents;       ///< Remote projectile events awaiting the World.
        static constexpr size_t MAX_QUEUED_PROJECTILE_EVENTS = 256; ///< Guards against growth while no World drains.
        std::vector<SoundEvent> m_soundEvents;                 ///< Remote sound events awaiting the World.
        static constexpr size_t MAX_QUEUED_SOUND_EVENTS = 64;  ///< Guards against growth while no World drains.
        std::set<uint32_t> m_removedObjectIds;                ///< Authoritative removed object ids.
        std::set<uint32_t> m_locallyCreditedKills;            ///< Client: removed enemies the host credited to us.
        std::vector<EnemyDamageEvent> m_enemyDamageEvents;    ///< Server: client boss hits awaiting the World.
        std::vector<ClaudlingSpawnEvent> m_claudlingSpawns;   ///< Client: host Claudling spawns awaiting the World.
        std::vector<BombSpawnEvent> m_bombSpawns;             ///< Client: host bomb throws awaiting the World.
        std::vector<RootSpawnEvent> m_rootSpawns;             ///< Client: host root throws awaiting the World.
        std::vector<CleaverSpawnEvent> m_cleaverSpawns;       ///< Client: host cleaver throws awaiting the World.
        std::vector<SpiderSpawnEvent> m_spiderSpawns;         ///< Client: host spider throws awaiting the World.
        std::vector<IceCircleSpawnEvent> m_iceCircleSpawns;   ///< Client: host ice circles awaiting the World.
        std::vector<FireLineSpawnEvent> m_fireLineSpawns;     ///< Client: host lines of fire awaiting the World.
        std::vector<CrystalSpawnEvent> m_crystalSpawns;       ///< Client: host crystals awaiting the World.
        static constexpr size_t MAX_QUEUED_WORLD_EVENTS = 256; ///< Guards against growth while no World drains.
        std::vector<ChatEntry> m_chatMessages;                 ///< Chat lines awaiting the chat box.
        static constexpr size_t MAX_QUEUED_CHAT_MESSAGES = 64; ///< Guards against growth while no chat box drains.

        // Shared progression
        static constexpr int32_t NO_STAGE = -1;
        int32_t m_currentStage { NO_STAGE };          ///< Stage the local peer is on.
        std::optional<int32_t> m_requestedStage {};   ///< Client: host stage not yet applied by the WorldManager.
        bool m_isPortalEntered { false };             ///< Someone already went through this stage's portal.
        bool m_hasPendingRemotePortalEntry { false }; ///< A remote portal entry the World has not consumed yet.

        // Server-only bookkeeping
        std::map<HSteamNetConnection, uint32_t> m_connectionToPlayerId;
        uint32_t m_nextPlayerId { HOST_PLAYER_ID + 1 };
        std::set<uint32_t> m_removedCoinIds; ///< Subset of m_removedObjectIds that are coins, for late joiners.
        std::map<uint32_t, ClaudlingSpawnEvent> m_spawnedClaudlings; ///< Every Claudling spawned this stage, replayed to late joiners.

        // Client-only bookkeeping
        bool m_hasSentJoinRequest { false };
        bool m_wasKicked { false }; ///< The host sent us a Kicked message.
        std::string m_kickReason {}; ///< Reason that came with it; empty if none.

        // LAN discovery / advertising
        LanBeacon m_beacon;
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETWORKMANAGER_H
