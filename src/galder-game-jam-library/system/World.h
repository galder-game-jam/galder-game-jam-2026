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
#include "../graphics/BloodSplatter.h"
#include "../GalderGameJamLibrary.h"
#include "ObjectGeneratorData.hpp"
#include "CollisionCategory.hpp"
#include "../data/PlayerStats.hpp"
#include "../data/PlayerPersistentState.hpp"
#include "../interfaces/manager/INetworkManager.h"
#include "../interfaces/manager/IOptionsManager.h"
#include "../interfaces/graphics/IPaletteSwapShader.h"
#include "../interfaces/graphics/ITintShader.h"
#include "../data/CharacterColors.hpp"
#include "../data/DialogSpot.hpp"
#include "../data/CleaverSpawnEvent.hpp"
#include "../data/SpiderSpawnEvent.hpp"
#include "../data/IceCircleSpawnEvent.hpp"
#include "../data/FireLineSpawnEvent.hpp"
#include "../data/CrystalSpawnEvent.hpp"
#include "../data/QuestProgress.hpp"
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
              std::unique_ptr<tson::Map> map, ggj::Hud& hud, Dialog& dialog, IOptionsManager& optionsManager, IPaletteSwapShader& paletteSwap, ITintShader& tintShader)
            : m_logger { logger }, m_resources { resources }, m_mapper { mapper }, m_textures { textures }, m_window { window }, m_input { input },
              m_collisionManager { collisionManager }, m_debugManager { debugManager }, m_animationManager { animationManager }, m_userDataManager { userDataManager },
              m_soundManager { soundManager }, m_map { std::move(map) }, m_hud { hud }, m_dialog { dialog }, m_optionsManager { optionsManager }, m_paletteSwap { paletteSwap }, m_tintShader { tintShader }
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
        /*!
         * Sets the font used for text drawn in the world (floating texts, remote player names).
         * Falls back to raylib's built-in bitmap font while none is set.
         *
         * @param font Font to draw with; must outlive the world
         */
        void setFont(raylib::Font* font) { m_font = font; }
        /*! Starts recolouring draws with the local player's picked colours. Pair with endPlayerPalette(). */
        void beginPlayerPalette();
        void endPlayerPalette();
        IPlayer* getPlayer();
        Minimap* getMinimap() { return &m_minimap; }

        /*! When set, remote players synced by the network manager are drawn into this world. */
        void setNetworkManager(INetworkManager* network) { m_network = network; }
        /*! Minimap dots for every remote netplay peer (visual centre + name colour). Empty offline. */
        [[nodiscard]] std::vector<MinimapMarker> getRemotePlayerMarkers() const;
        /*!
         * The other players in the session, for the HUD party bars.
         *
         * @return Pointers into the network manager's state, valid until the next network update.
         *         Ordered by player id; empty outside netplay.
         */
        [[nodiscard]] std::vector<const RemotePlayer*> getRemotePartyMembers() const;
        /*! Minimap dots for every living enemy in the world (world-pixel position). */
        [[nodiscard]] std::vector<MinimapMarker> getEnemyMarkers() const;
        /*! Minimap dots for every NPC the player can talk to (world-pixel position). */
        [[nodiscard]] std::vector<MinimapMarker> getNpcMarkers() const;
        [[nodiscard]] std::vector<uint32_t> getActivePlayerIds() const override;
        [[nodiscard]] uint32_t getLocalPlayerId() const override;
        [[nodiscard]] std::optional<raylib::Vector2> getPlayerPosition(uint32_t playerId) const override;

        /*!
         * Where a player is, as the point attacks should aim at (the centre of their sprite). Remote
         * netplay peers are included.
         *
         * @param playerId Network player id
         * @return The player's centre, or nothing when they are not in the session
         */
        [[nodiscard]] std::optional<raylib::Vector2> getPlayerCenter(uint32_t playerId) const;

        /*!
         * Whether this peer runs the enemies' decisions: offline or hosting. Clients only mirror
         * the host's boss (state, target, health) and must not roll their own.
         */
        [[nodiscard]] bool isEnemyAuthority() const;

        /*!
         * Where every player is, as the point attacks should aim at (the centre of their sprite).
         * Includes remote netplay peers.
         */
        [[nodiscard]] std::vector<raylib::Vector2> getPlayerCenters() const;

        /*!
         * Whether a point lies inside something solid and immovable such as a wall or a tree.
         *
         * @param point Position in pixels
         */
        [[nodiscard]] bool isSolidAt(const raylib::Vector2& point) const;

        /*!
         * Client: sends a hit on a host-authoritative enemy (a boss) to the host, which applies it.
         *
         * @param enemy The networked enemy that was hit
         * @param damage Lives to remove
         */
        void reportEnemyDamage(const PhysicsObject& enemy, int32_t damage);

        /*!
         * Host: a networked enemy died from a client's hit. Removes it everywhere and credits the
         * kill to that client instead of the host. Call right after destroying the enemy.
         *
         * @param enemy The enemy that just died
         * @param killerPlayerId Network player id of the client that landed the killing blow
         */
        void creditRemoteKill(const PhysicsObject& enemy, uint32_t killerPlayerId);

        /*!
         * Queues @p count bombs evenly spaced on a circle around @p center, each pushed gently
         * outwards. Host/offline only, like spawnBombs().
         *
         * @param center Circle centre (pixels)
         * @param count Number of bombs
         * @param ringRadius Distance from @p center each bomb starts at (pixels)
         * @param slideDistance How far each bomb slides outwards before stopping (pixels)
         * @param minFuseTime Shortest fuse in seconds
         * @param maxFuseTime Longest fuse in seconds
         */
        void spawnBombRing(raylib::Vector2 center, int32_t count, float ringRadius, float slideDistance, float minFuseTime, float maxFuseTime);

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
        /*! Coins on the map that nobody has picked up yet, counting pickups by netplay peers. */
        [[nodiscard]] int getNumberOfCoinsLeft() const;
        /*! The stage's quest goals and progress; its isComplete() decides when the portal opens. */
        [[nodiscard]] QuestProgress getQuestProgress() const;

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

        /*!
         * Advances the slow health-regeneration talents and grants a life when the interval elapses.
         * The clock only runs while the player is alive and below the regeneration cap, so it never
         * stacks with extra lives from other sources.
         *
         * @param playerIndex Player to tick
         * @param timeDelta   Time since last frame in seconds
         * @return            true if a life was regenerated this call
         */
        [[nodiscard]] bool tickPlayerLifeRegen(int32_t playerIndex, float timeDelta);

        template <typename T> T* createProjectile(TextureName texture, raylib::Vector2 size, raylib::Vector2 pos, Vector2 velocity, float timeToLive);

        /*!
         * Registers a projectile fired by the local player so netplay peers can render it. No-op offline.
         *
         * @param projectile Freshly created projectile owned by a world layer
         */
        void registerLocalProjectile(Projectile* projectile);

        /*!
         * Plays a sound the local player caused and, in netplay, has the other peers play it too.
         * Use this instead of ISoundManager::playSound() for player actions (attack, hurt, ...).
         *
         * @param sound The sound to play
         */
        void playPlayerSound(SoundName sound);

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
         * @return True if the local player was in range, whether or not the hit cost them a life
         *         (they may be invulnerable after a recent hit)
         */
        bool damageLocalPlayerWithin(const raylib::Vector2& center, float radius);

        /*!
         * Hurts the local player if the lower half of their body (middle of the body down to the feet) touches a spiking treant root (see
         * TreantRoot::IsInsideSpikeArea()). Unlike damageLocalPlayerWithin() this ignores how tall
         * the player sprite is, so stepping on the root with your feet hurts and standing clearly beside it does not.
         *
         * @param root Where the root lies on the ground (pixels)
         * @return True if the local player was on the root, whether or not the hit cost them a life
         */
        bool damageLocalPlayerOnRoot(const raylib::Vector2& root);

        /*!
         * Stuns the local player if they are within @p radius of @p center. Remote players are
         * stunned by their own peer's copy of the same thing.
         *
         * @param center Centre of the effect (pixels)
         * @param radius Radius of the effect (pixels)
         * @param seconds How long the stun lasts
         * @return True if the local player was in range
         */
        bool stunLocalPlayerWithin(const raylib::Vector2& center, float radius, float seconds);

        /*!
         * Whether any player, local or remote, is within @p radius of @p center. Lets thrown objects
         * react to a hit on a remote player too, even though only that player's own peer applies it.
         *
         * @param center Point to test around (pixels)
         * @param radius Radius (pixels)
         */
        [[nodiscard]] bool isAnyPlayerWithin(const raylib::Vector2& center, float radius) const;

        /*!
         * Freezes the local player if they are within @p radius of @p center. Remote players are
         * frozen by their own peer's copy of the same thing.
         *
         * @param center Centre of the effect (pixels)
         * @param radius Radius of the effect (pixels)
         * @param seconds How long the freeze lasts
         * @return True if the local player was in range
         */
        bool freezeLocalPlayerWithin(const raylib::Vector2& center, float radius, float seconds);

        /*!
         * Hurts the local player if they are within @p halfWidth of the line from @p start to @p end,
         * as if an enemy touched them. Remote players are hurt by their own peer's copy of the same thing.
         *
         * @param start One end of the line (pixels)
         * @param end The other end of the line (pixels)
         * @param halfWidth How far either side of the line hurts (pixels)
         * @return True if the local player was in range
         */
        bool damageLocalPlayerAlongSegment(const raylib::Vector2& start, const raylib::Vector2& end, float halfWidth);

        /*!
         * Queues ice circles for the mage: the first ones land on players (so the spell is a real
         * threat), the rest anywhere on the map. Each circle freezes whoever is still inside after its
         * own random delay. Spawned after the current update loop. Host/offline only: clients create
         * the host's circles when told to.
         *
         * @param count Number of circles
         * @param radius Radius of each circle (pixels)
         * @param minDelay Shortest time before a circle freezes (seconds)
         * @param maxDelay Longest time before a circle freezes (seconds)
         * @param freezeSeconds How long a frozen player stays frozen
         */
        void spawnIceCircles(int32_t count, float radius, float minDelay, float maxDelay, float freezeSeconds);

        /*!
         * Queues lines of fire from @p origin, one per angle, each running in a straight line until
         * it meets a wall or the edge of the map. Host/offline only: the host tells the clients, which
         * each create their copy and only hurt their own player.
         *
         * @param origin Where every line starts (pixels)
         * @param angles Direction of each line (radians)
         * @param warningSeconds Telegraph time before they ignite
         * @param burnSeconds How long they burn once ignited
         * @param igniteSpeed How fast the flames race along each line (pixels per second)
         */
        void spawnFireLines(const raylib::Vector2& origin, const std::vector<float>& angles, float warningSeconds, float burnSeconds, float igniteSpeed);

        /*!
         * Queues a fan of pink crystals shot from @p origin. Host/offline only.
         *
         * @param origin Where the crystals leave from (pixels)
         * @param aimAngle Direction of the middle of the fan (radians)
         * @param count Number of crystals
         * @param spreadDegrees Total width of the fan
         * @param speed Crystal speed in pixels per second
         */
        void spawnCrystalFan(const raylib::Vector2& origin, float aimAngle, int32_t count, float spreadDegrees, float speed);

        /*!
         * Queues a ring of pink crystals flying outwards from @p origin in every direction. Host/offline only.
         *
         * @param origin Centre of the ring (pixels)
         * @param count Number of crystals
         * @param startAngle Direction of the first crystal (radians)
         * @param speed Crystal speed in pixels per second
         * @param ringRadius Distance from @p origin each crystal starts at (pixels)
         */
        void spawnCrystalRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius);

        /*!
         * Queues a ring of meat cleavers flying outwards from @p origin, evenly spaced in every
         * direction. Spawned after the current update loop, like spawnBombs(). Host/offline only:
         * clients create the host's cleavers when told to.
         *
         * @param origin Centre of the ring (pixels)
         * @param count Number of cleavers
         * @param startAngle Direction of the first cleaver (radians)
         * @param speed Cleaver speed in pixels per second
         * @param ringRadius Distance from @p origin each cleaver starts at (pixels)
         */
        void spawnCleaverRing(const raylib::Vector2& origin, int32_t count, float startAngle, float speed, float ringRadius);

        /*!
         * Queues a spider thrown from @p origin to land on @p target. It is created after this
         * frame's update pass. Host/offline only: the host tells the clients, which each create their
         * copy and only stun their own player.
         *
         * @param origin Where the spider is thrown from (pixels)
         * @param target Where it lands (pixels)
         * @param flightTime Seconds spent in the air
         */
        void spawnThrownSpider(const raylib::Vector2& origin, const raylib::Vector2& target, float flightTime);

        /*!
         * Queues a root thrown by the treant. It is created after this frame's update pass. Host/offline
         * only: the host tells the clients, which each create their copy and only hurt their own player.
         *
         * @param origin Where the root is thrown from (pixels)
         * @param target Where it lands (pixels)
         * @param flightTime Seconds spent in the air
         * @param spikeDelay Seconds between landing and spiking
         */
        void spawnTreantRoot(const raylib::Vector2& origin, const raylib::Vector2& target, float flightTime, float spikeDelay);

        /*!
         * Queues @p count roots thrown from @p origin that land spread evenly across the whole map,
         * never inside walls or trees. Each root gets its own random flight and spike delay, so they
         * rain down and spike in waves.
         *
         * @param origin Where the roots are thrown from (pixels)
         * @param count Number of roots; fewer if the map has no free spot for some of them
         * @param minFlightTime Shortest flight (seconds)
         * @param maxFlightTime Longest flight (seconds)
         * @param minSpikeDelay Shortest wait between landing and spiking (seconds)
         * @param maxSpikeDelay Longest wait between landing and spiking (seconds)
         */
        void spawnTreantRootsAcrossMap(const raylib::Vector2& origin, int32_t count, float minFlightTime, float maxFlightTime, float minSpikeDelay, float maxSpikeDelay);

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

        /*! Spawns a floating red "Ability not ready" label in world space above the given position. */
        void spawnAbilityNotReadyText(raylib::Vector2 pos);

        /*!
         * Finds the NPC closest to the given point that is within its own interaction radius.
         *
         * @param point Position to test from, in world pixels
         * @return The matching spot, or nullptr when no NPC is in range
         */
        [[nodiscard]] const DialogSpot* findDialogSpotNear(const raylib::Vector2& point) const;

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
        static constexpr float FLOATING_TEXT_FONT_SIZE = 12.f;

        template <typename T, typename... Args> T* createObject(Args&&... args);
        /*! Like createObject(), but in the layer drawn underneath the characters (falls back to the objects layer), for decals on the floor. */
        template <typename T, typename... Args> T* createGroundObject(Args&&... args);
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
        void generateButcher(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void generateMageBoss(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
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

        /*! Creates a treant root (not a networked object and not counted as an enemy). */
        void doSpawnTreantRoot(const RootSpawnEvent& root);

        /*! Creates a meat cleaver (not a networked object and not counted as an enemy). */
        void doSpawnCleaver(const CleaverSpawnEvent& cleaver);

        /*! Creates a thrown spider (not a networked object and not counted as an enemy). */
        void doSpawnSpider(const SpiderSpawnEvent& spider);

        /*! Creates an ice circle on the floor (not a networked object and not counted as an enemy). */
        void doSpawnIceCircle(const IceCircleSpawnEvent& circle);

        /*! Creates a line of fire on the floor (not a networked object and not counted as an enemy). */
        void doSpawnFireLine(const FireLineSpawnEvent& fire);

        /*! Creates a pink crystal (not a networked object and not counted as an enemy). */
        void doSpawnCrystal(const CrystalSpawnEvent& crystal);

        /*!
         * Where a line of fire from @p origin ends: the first wall it meets, or the edge of the map.
         *
         * @param origin Where the line starts (pixels)
         * @param angle Direction of the line (radians)
         * @return The end of the line (pixels)
         */
        [[nodiscard]] raylib::Vector2 fireLineEnd(const raylib::Vector2& origin, float angle);
        void flushPendingSpawns();
        void destroyMarkedObjects();
        void drawRemotePlayers();
        /*! Steps each remote peer's animation from its synced PlayerState and drops departed peers. */
        void updateRemotePlayerSprites(float timeDelta);
        /*! Reports local projectile impacts and prunes local projectiles about to be freed. */
        void syncLocalProjectiles();
        /*! Spawns/explodes/advances the visual-only projectiles fired by remote peers. */
        void updateRemoteProjectiles(float timeDelta);
        /*! Plays the sound effects the remote peers triggered since the last frame. */
        void playRemoteSounds();
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
        ITintShader& m_tintShader;

        b2World m_world { { 0.f, 0.f } };
        std::unique_ptr<tson::Map> m_map { nullptr };

        raylib::Color m_backgroundColor = raylib::Color::White();
        std::map<int32_t, ggj::WorldLayer> m_layers;

        int m_numberOfEnemies { 0 };
        int m_networkRemovedEnemyCount { 0 }; ///< Enemies killed by remote peers (coop quest log).
        int m_numberOfCoins { 0 };
        bool m_isAllEnemiesRequired { true }; ///< Map property require_all_monsters_killed.
        bool m_isAllCoinsRequired { false };  ///< Map property require_all_coins_collected.
        Minimap m_minimap {};

        int32_t m_nextPlayerIndex { 0 };
        std::map<int32_t, PlayerStats> m_playerStats;

        std::vector<FloatingTextEntry> m_floatingTexts;
        BloodSplatter m_blood { m_optionsManager };
        raylib::Font* m_font { nullptr };
        std::vector<DialogSpot> m_dialogSpots;

        std::vector<PendingClaudlingSpawn> m_pendingClaudlingSpawns;
        std::vector<BombSpawnEvent> m_pendingBombSpawns;
        std::vector<RootSpawnEvent> m_pendingRootSpawns;
        std::vector<CleaverSpawnEvent> m_pendingCleaverSpawns;
        std::vector<SpiderSpawnEvent> m_pendingSpiderSpawns;
        std::vector<IceCircleSpawnEvent> m_pendingIceCircleSpawns;
        std::vector<FireLineSpawnEvent> m_pendingFireLineSpawns;
        std::vector<CrystalSpawnEvent> m_pendingCrystalSpawns;

        PhysicsObject* m_boss        { nullptr };
        std::string    m_bossName    {};
        int            m_bossMaxLives { 0 };

        INetworkManager* m_network = nullptr;

        /*! Locally-animated sprite for a remote netplay peer, driven by its synced PlayerState. */
        struct RemotePlayerSprite
        {
            PlayerState state { PlayerState::Idle };
            Animation animation {};
            int32_t lives { 0 }; ///< Lives seen last frame; a drop means the peer was hurt.
        };
        std::map<uint32_t, RemotePlayerSprite> m_remotePlayerSprites; ///< By network player id.
        // Local player sprite setup, captured in generatePlayer() so remote peers draw identically.
        raylib::Texture* m_rogueTexture { nullptr };
        raylib::Texture* m_wizardTexture { nullptr };
        raylib::Vector2 m_playerSpriteOrigin {};
        float m_playerSpriteHeight { 0.0f };
        float m_playerBodyHalfWidth { 0.0f }; ///< Half the width of the local player's physics body (pixels).

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
        fd.filter.categoryBits = CollisionCategory::PROJECTILE; // Mask stays "everything"; only passthrough walls filter it out.

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

    template <typename T, typename... Args> T* World::createGroundObject(Args&&... args)
    {
        for (auto& layer : m_layers)
        {
            if (layer.second.getName() == "background_objects")
            {
                return layer.second.createGameObject<T>(args...);
            }
        }
        return createObject<T>(args...);
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
