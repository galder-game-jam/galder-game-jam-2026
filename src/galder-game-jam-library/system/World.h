//
// Created by robin on 29.10.22.
//

#ifndef GAME_DEV_TEMPLATE_WORLD_H
#define GAME_DEV_TEMPLATE_WORLD_H

#ifdef GAME_DEV_DEBUG
namespace ggj
{
    struct DebugState;
}
#endif

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../enums/PlayerPickedCharacter.h"
#include "box2d/box2d.h"
#include "tileson.hpp"
#include "../interfaces/system/IMapper.h"
#include "../interfaces/game/IPlayer.h"
#include "../interfaces/game/IPlayerLocator.h"
#include "raylib-cpp.hpp"
#include "WorldLayer.h"
#include "../game/Minimap.h"
#include "../enums/KeyboardKey.h"
#include "../graphics/PhysicsObject.h"
#include "../GalderGameJamLibrary.h"
#include "ObjectGeneratorData.hpp"
#include "../data/PlayerStats.hpp"
#include "../data/PlayerPersistentState.hpp"
#include "../interfaces/manager/INetworkManager.h"
#include "../interfaces/manager/IOptionsManager.h"
#include "../interfaces/graphics/IPaletteSwapShader.h"
#include "../data/CharacterColors.hpp"
#include <optional>
#include <set>
#include <vector>

namespace ggj
{
    class Hud; // Forward declaration
    class Projectile;

    class World : public ggj::IWorld<int32_t>, public IPlayerLocator
    {
        public:
        World(ggj::ILogger& logger, ggj::IResourceManager<ResourceName>& resources, IMapper& mapper, ggj::ITextureManager<TextureName, raylib::Texture>& textures,
              raylib::Window& window, ggj::IInputManager<ggj::KeyboardKey>& input, CollisionManager& collisionManager, IDebugManager& debugManager,
              ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IUserDataManager<PhysicsObject*>& userDataManager, ggj::ISoundManager& soundManager,
              std::unique_ptr<tson::Map> map, ggj::Hud& hud, Dialog& dialog, IOptionsManager& optionsManager, IPaletteSwapShader& paletteSwap)
            : m_logger { logger }, m_resources { resources }, m_mapper { mapper }, m_textures { textures }, m_window { window }, m_input { input },
              m_collisionManager { collisionManager }, m_debugManager { debugManager }, m_animationManager { animationManager }, m_userDataManager { userDataManager },
              m_soundManager { soundManager }, m_map { std::move(map) }, m_hud { hud }, m_dialog { dialog }, m_optionsManager { optionsManager }, m_paletteSwap { paletteSwap }
        {
            m_camera.target = Vector2 { 0.f, 0.f };
            m_camera.offset = { 0.f, 0.f }; //{8.f, 8.f};//(Vector2){ (float) window.GetWidth() / 2.0f, (float)window.GetHeight() / 2.0f };
            m_camera.rotation = 0.0f;
            m_camera.zoom = 1.0f;
        }

        bool initialize() override;
        void update(float timeDelta) override;
        void draw() override;
        std::string getLeadingPlayer() const;
        void setPickedCharacter(PlayerPickedCharacter character);
        /*!
         * Sets the colours the local player picked on the character select screen. Call after
         * setPickedCharacter(), since the colours are resolved against that character's palette.
         *
         * @param colors Picked colour per palette slot
         */
        void setPlayerColors(const CharacterColors& colors);
        /*! Starts recolouring draws with the local player's picked colours. Pair with endPlayerPalette(). */
        void beginPlayerPalette();
        void endPlayerPalette();
        IPlayer* getPlayer();
        Minimap* getMinimap() { return &m_minimap; }

        /*! When set, remote players synced by the network manager are drawn into this world. */
        void setNetworkManager(INetworkManager* network) { m_network = network; }
        /*! Minimap dots for every remote netplay peer (visual centre + name colour). Empty offline. */
        [[nodiscard]] std::vector<MinimapMarker> getRemotePlayerMarkers() const;
        [[nodiscard]] std::vector<uint32_t> getActivePlayerIds() const override;
        [[nodiscard]] uint32_t getLocalPlayerId() const override;
        [[nodiscard]] std::optional<raylib::Vector2> getPlayerPosition(uint32_t playerId) const override;

        /*!
         * Whether this peer runs the enemies' decisions: offline or hosting. Clients only mirror
         * the host's boss (state, target, health) and must not roll their own.
         */
        [[nodiscard]] bool isEnemyAuthority() const;

        /*!
         * Client: sends a hit on a host-authoritative enemy (a boss) to the host, which applies it.
         *
         * @param enemy The networked enemy that was hit
         * @param damage Lives to remove
         */
        void reportEnemyDamage(const PhysicsObject& enemy, int32_t damage);

        /*! Called by the Portal when the local player walks in, so netplay peers follow through it. */
        void notifyLocalPortalEntered();
        [[nodiscard]] int getEnemies() const;
#ifdef GAME_DEV_DEBUG
        void setDebugState(ggj::DebugState* pState);
        /*! Instantly destroys all remaining enemies and credits the kills to the local player. */
        void killAllEnemies();
        void drawPhysicsDebug();
#endif
        void reduceEnemyKillCountByOne();
        [[nodiscard]] int getNumberOfEnemiesLeft() const;

        // Player stat accessors (playerIndex assigned by World on player creation)
        [[nodiscard]] int getPlayerLives(int32_t playerIndex) const { return m_playerStats.at(playerIndex).lives; }
        [[nodiscard]] int getPlayerScore(int32_t playerIndex) const { return m_playerStats.at(playerIndex).score; }
        [[nodiscard]] int getPlayerLevel(int32_t playerIndex) const { return m_playerStats.at(playerIndex).xpTracker.getLevel(); }
        [[nodiscard]] int getPlayerCurrentXp(int32_t playerIndex) const { return m_playerStats.at(playerIndex).xpTracker.getCurrentXp(); }
        [[nodiscard]] int getPlayerTotalXp(int32_t playerIndex) const { return m_playerStats.at(playerIndex).xpTracker.getTotalXp(); }
        [[nodiscard]] int getPlayerXpToNextLevel(int32_t playerIndex) const
        {
            return m_playerStats.at(playerIndex).xpTracker.getXpToNextLevel(m_playerStats.at(playerIndex).xpConfig);
        }
        [[nodiscard]] const ExperienceConfig& getXpConfig(int32_t playerIndex) const { return m_playerStats.at(playerIndex).xpConfig; }

        // Player stat mutators
        void decrementPlayerLives(int32_t playerIndex) { --m_playerStats.at(playerIndex).lives; }
        void addPlayerLives(int32_t playerIndex, int delta) { m_playerStats.at(playerIndex).lives += delta; }
        void resetPlayerLives(int32_t playerIndex) { m_playerStats.at(playerIndex).lives = 3; }
        void addPlayerScore(int32_t playerIndex, int delta) { m_playerStats.at(playerIndex).score += delta; }

        /*!
         * Awards XP to a player, applying any active XP-multiplier talents.
         * Automatically grants 2 talent points for each level gained.
         */
        void addPlayerXp(int32_t playerIndex, int amount);
        void restorePlayerStats(int32_t playerIndex, const PlayerPersistentState& state);

        // Talent system accessors
        [[nodiscard]] int getPlayerTalentPoints(int32_t playerIndex) const { return m_playerStats.at(playerIndex).talentPoints; }
        [[nodiscard]] const TalentState&   getPlayerTalentState(int32_t playerIndex)   const { return m_playerStats.at(playerIndex).talentState;   }
        [[nodiscard]] const TalentEffects& getPlayerTalentEffects(int32_t playerIndex) const { return m_playerStats.at(playerIndex).computedEffects; }

        /*!
         * Replaces the talent state for a player and recomputes their effects.
         * Called by WorldManager after the talent screen closes.
         */
        void updatePlayerTalentState(int32_t playerIndex, const TalentState& state, int talentPoints);

        template <typename T> T* createProjectile(TextureName texture, raylib::Vector2 size, raylib::Vector2 pos, Vector2 velocity, float timeToLive);

        /*!
         * Registers a projectile fired by the local player so netplay peers can render it. No-op offline.
         *
         * @param projectile Freshly created projectile owned by a world layer
         */
        void registerLocalProjectile(Projectile* projectile);

        /*!
         * Queues one Claudling per active player, each chasing its own player, to be spawned
         * around @p position after the current update loop finishes. Must be queued (not spawned
         * immediately) because calling createGameObject during layer iteration would invalidate
         * the vector iterator. Host/offline only: clients spawn the host's Claudlings when told to.
         *
         * @param position Where the boss stands (pixels)
         * @param followSpeed Chase speed of each Claudling
         * @param knockback Knockback each Claudling applies to players on contact
         */
        void spawnClaudlings(raylib::Vector2 position, float followSpeed, float knockback);

        /*!
         * Queues bombs thrown from @p origin: @p bombsPerPlayer at every active player, each aimed
         * roughly at them with a random spread and speed so they land scattered around the player.
         * Spawned after the current update loop, like spawnClaudlings(). Host/offline only:
         * clients create the host's bombs when told to.
         *
         * @param origin Where the bombs are thrown from (pixels)
         * @param bombsPerPlayer Bombs aimed at each player
         * @param minFuseTime Shortest fuse in seconds
         * @param maxFuseTime Longest fuse in seconds
         */
        void spawnBombs(raylib::Vector2 origin, int32_t bombsPerPlayer, float minFuseTime, float maxFuseTime);

        /*!
         * Hurts the local player if they are within @p radius of @p center. Remote players are
         * hurt by their own peer's copy of the same blast.
         *
         * @param center Blast centre (pixels)
         * @param radius Blast radius (pixels)
         */
        void damageLocalPlayerWithin(const raylib::Vector2& center, float radius);

        void clearBoss() { m_boss = nullptr; }

        /*!
         * Sends every living enemy within @p radius of @p point back to where it spawned.
         * Called on player respawn so the player never reappears underneath an enemy.
         * Enemies whose spawn is itself within @p radius are left alone.
         *
         * @param point  Respawn position (pixels)
         * @param radius Minimum distance (pixels) to keep between enemies and @p point
         */
        void keepEnemiesClearOf(const raylib::Vector2& point, float radius);
        [[nodiscard]] bool        hasBoss()        const { return m_boss != nullptr; }
        [[nodiscard]] std::string getBossName()    const { return m_bossName; }
        [[nodiscard]] int         getBossLives()   const { return m_boss ? m_boss->getLives() : 0; }
        [[nodiscard]] int         getBossMaxLives() const { return m_bossMaxLives; }

            /*! Spawns a floating "+1 life" label in world space above the given position. */
        void spawnFloatingText(raylib::Vector2 pos);

        /*! Spawns a floating yellow "+1" coin label in world space above the given position. */
        void spawnCoinFloatingText(raylib::Vector2 pos);

    private:
        struct FloatingTextEntry
        {
            raylib::Vector2 position;
            float elapsed { 0.f };
            std::string text { "+1 life" };
            Color color { 60, 220, 60, 255 };
            static constexpr float DURATION    = 1.2f;  ///< Total lifetime in seconds.
            static constexpr float RISE_SPEED  = 22.f;  ///< Pixels per second upward drift.
        };

        template <typename T, typename... Args> T* createObject(Args&&... args);
        //Generator logic
        void generatePhysicsObject(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generatePlayer(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateBat(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateSnake(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateCoin(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateSpider(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateThing(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateGhost(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generatePortal(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generatePowerUp(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateClaude(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateTreant(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateClaudling(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateGenericPhysicsObject(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        struct PendingClaudlingSpawn
        {
            raylib::Vector2 position;
            float followSpeed;
            float knockback;
            uint32_t targetPlayerId;
            uint32_t networkId { 0 }; ///< 0 = assign the next free id (host); else the host's id (client).
        };
        /*! Creates the Claudling. @return Its network id, or 0 if it could not be created. */
        uint32_t doSpawnClaudling(const PendingClaudlingSpawn& spawn);
        /*! Creates a bomb (not a networked object and not counted as an enemy). */
        void doSpawnBomb(const BombSpawnEvent& bomb);
        void flushPendingSpawns();
        void destroyMarkedObjects();
        void drawRemotePlayers();
        /*! Steps each remote peer's animation from its synced PlayerState and drops departed peers. */
        void updateRemotePlayerSprites(float timeDelta);
        /*! Reports local projectile impacts and prunes local projectiles about to be freed. */
        void syncLocalProjectiles();
        /*! Spawns/explodes/advances the visual-only projectiles fired by remote peers. */
        void updateRemoteProjectiles(float timeDelta);
        void drawRemoteProjectiles();

        /*!
         * Assigns a deterministic network id to a syncable object (enemy/coin) and registers it.
         *
         * @param obj Object to register
         * @param networkId Id to use, e.g. the host's id for a Claudling it spawned; 0 = next free id
         * @return The id the object was registered under, or 0 if @p obj is null
         */
        uint32_t registerNetworkObject(PhysicsObject* obj, uint32_t networkId = 0);
        /*! Host: applies the boss hits clients reported, crediting any kill to the client. */
        void applyRemoteEnemyDamage();
        /*! Client: creates the Claudlings the host spawned. */
        void spawnHostClaudlings();
        /*! Reconciles networked objects with the network manager (report local kills, apply removals, correct positions). */
        void syncNetworkObjects();

        ggj::ILogger& m_logger;
        ggj::IResourceManager<ResourceName>& m_resources;
        IMapper& m_mapper;
        ggj::ITextureManager<TextureName, raylib::Texture>& m_textures;
        raylib::Window& m_window;
        ggj::IInputManager<ggj::KeyboardKey>& m_input;
        ggj::CollisionManager& m_collisionManager;
        ggj::IDebugManager& m_debugManager;
        ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& m_animationManager;
        IUserDataManager<PhysicsObject*>& m_userDataManager;
        ggj::ISoundManager& m_soundManager;
        raylib::Camera2D m_camera;

        raylib::Vector2 m_cameraMin { 0.f, 0.f };
        raylib::Vector2 m_cameraMax { 0.f, 0.f };
        raylib::Vector2 m_cameraDefault { 0.f, 0.f };

        int m_portalTimer { 5 };

        IPlayer* m_player = nullptr;
        PlayerPickedCharacter m_pickedCharacter { PlayerPickedCharacter::Rogue };
        PaletteSwap m_playerPaletteSwap {}; ///< Resolved from the picked colours; empty = default sprite.
#ifdef GAME_DEV_DEBUG
        ggj::DebugState* m_pDebugState = nullptr;
#endif

        Portal* m_portal = nullptr;

        Hud& m_hud;

        Dialog& m_dialog;

        IOptionsManager& m_optionsManager;
        IPaletteSwapShader& m_paletteSwap;

        b2World m_world { { 0.f, 0.f } };
        std::unique_ptr<tson::Map> m_map { nullptr };

        raylib::Color m_backgroundColor = raylib::Color::White();
        std::map<int32_t, ggj::WorldLayer> m_layers;

        int m_numberOfEnemies { 0 };
        int m_networkRemovedEnemyCount { 0 }; ///< Enemies killed by remote peers (coop quest log).
        Minimap m_minimap {};

        int32_t m_nextPlayerIndex { 0 };
        std::map<int32_t, PlayerStats> m_playerStats;

        std::vector<FloatingTextEntry> m_floatingTexts;

        std::vector<PendingClaudlingSpawn> m_pendingClaudlingSpawns;
        std::vector<BombSpawnEvent> m_pendingBombSpawns;

        PhysicsObject* m_boss        { nullptr };
        std::string    m_bossName    {};
        int            m_bossMaxLives { 0 };

        INetworkManager* m_network = nullptr;

        /*! Locally-animated sprite for a remote netplay peer, driven by its synced PlayerState. */
        struct RemotePlayerSprite
        {
            PlayerState state { PlayerState::Idle };
            Animation animation {};
        };
        std::map<uint32_t, RemotePlayerSprite> m_remotePlayerSprites; ///< By network player id.
        // Local player sprite setup, captured in generatePlayer() so remote peers draw identically.
        raylib::Texture* m_rogueTexture { nullptr };
        raylib::Texture* m_wizardTexture { nullptr };
        raylib::Vector2 m_playerSpriteOrigin {};
        float m_playerSpriteHeight { 0.0f };

        /*! A projectile fired by the local player, tracked so its impact can be reported once. */
        struct LocalProjectile
        {
            Projectile* projectile { nullptr };
            bool hasReportedImpact { false };
        };
        std::map<uint32_t, LocalProjectile> m_localProjectiles; ///< By projectile id.
        uint32_t m_nextProjectileId { 1 };

        /*!
         * Visual-only copy of a remote peer's projectile. No physics body: the shooter resolves hits
         * and reports kills itself, so a real body here would double-count or credit the wrong player.
         */
        struct RemoteProjectile
        {
            uint32_t ownerId { 0 };
            uint32_t projectileId { 0 };
            raylib::Vector2 position {}; ///< Pixels.
            raylib::Vector2 velocity {}; ///< Box2D units (metres/s).
            float timeAlive { 0.0f };
            bool isImpact { false };
            Animation animation {};
        };
        std::vector<RemoteProjectile> m_remoteProjectiles;
        std::map<uint32_t, raylib::Vector2> m_networkObjectSpawns; ///< Spawn position (pixels) by network id.
        std::map<uint32_t, PhysicsObject*> m_networkObjects; ///< Syncable objects (enemies/coins) by network id.
        std::set<uint32_t> m_reportedDestroyed;              ///< Ids already reported destroyed to the network.
        uint32_t m_nextNetworkId { 1 };
        uint32_t m_enemyStateThrottle { 0 };
        static constexpr uint32_t ENEMY_STATE_THROTTLE_FRAMES = 3;
    };

    template <typename T> T* World::createProjectile(TextureName texture, raylib::Vector2 size, raylib::Vector2 pos, Vector2 velocity, float timeToLive)
    {
        b2BodyType bodyType = b2_dynamicBody;
        b2BodyDef bodyDef;
        bodyDef.type = bodyType;
        b2Vec2 b2size = PhysicsObject::ConvertToB2Vec2({ (float)size.x, (float)size.y });
        b2Vec2 b2origin = PhysicsObject::ConvertToB2Vec2({ (float)size.x / 2, (float)size.y / 2 });
        b2Vec2 b2pos = PhysicsObject::ConvertToB2Vec2({ (float)pos.x, (float)pos.y });

        bodyDef.position.Set(b2pos.x + b2origin.x, b2pos.y + b2origin.y);
        b2Body* body = m_world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_p.Set(0.f, 0.f);
        circle.m_radius = b2origin.x;

        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f; //Density must be something to react to gravity.
        fd.friction = 1.0f;

        body->CreateFixture(&fd);

        for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
        {
            f->SetSensor(true);
        }

        raylib::Rectangle rect = { 0.f, 0.f, (float)size.x, (float)size.y };
        raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
        raylib::Vector2 v = { (float)pos.x + origin.x, (float)pos.y + origin.y };
        raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };
        raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

        raylib::Texture* tex = m_textures.get(texture);
        UserData userData { ObjectType::Projectile, { 0.f, 0.f }, "damage", "", "projectile" };

        bool isVisible = true;
        T* obj = createObject<T>(m_animationManager, m_mapper, body, size, size, rect, tex, userData, velocity, this, timeToLive, isVisible);
        m_userDataManager.addUserData(body, obj);
        return obj;
    }

    template <typename T, typename... Args> T* World::createObject(Args&&... args)
    {
        for (auto& layer : m_layers)
        {
            if (layer.second.getName() == "objects")
            {
                return layer.second.createGameObject<T>(args...);
            }
        }
        return nullptr;
    }
} // namespace ggj

#endif //GAME_DEV_TEMPLATE_WORLD_H
