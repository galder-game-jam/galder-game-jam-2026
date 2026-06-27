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
#include <set>
#include <vector>

namespace ggj
{
    class Hud; // Forward declaration

    class World : public ggj::IWorld<int32_t>
    {
        public:
        World(ggj::ILogger& logger, ggj::IResourceManager<ResourceName>& resources, IMapper& mapper, ggj::ITextureManager<TextureName, raylib::Texture>& textures,
              raylib::Window& window, ggj::IInputManager<ggj::KeyboardKey>& input, CollisionManager& collisionManager, IDebugManager& debugManager,
              ggj::IAnimationManager<ggj::Animation, ggj::AnimationName>& animationManager, IUserDataManager<PhysicsObject*>& userDataManager, ggj::ISoundManager& soundManager,
              std::unique_ptr<tson::Map> map, ggj::Hud& hud, Dialog& dialog)
            : m_logger { logger }, m_resources { resources }, m_mapper { mapper }, m_textures { textures }, m_window { window }, m_input { input },
              m_collisionManager { collisionManager }, m_debugManager { debugManager }, m_animationManager { animationManager }, m_userDataManager { userDataManager },
              m_soundManager { soundManager }, m_map { std::move(map) }, m_hud { hud }, m_dialog { dialog }
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
        IPlayer* getPlayer();
        Minimap* getMinimap() { return &m_minimap; }

        /*! When set, remote players synced by the network manager are drawn into this world. */
        void setNetworkManager(INetworkManager* network) { m_network = network; }
        [[nodiscard]] int getEnemies() const;
#ifdef GAME_DEV_DEBUG
        void setDebugState(ggj::DebugState* pState);
        /*! Instantly destroys all remaining enemies and credits the kills to the local player. */
        void killAllEnemies();
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

            /*! Spawns a floating "+1 life" label in world space above the given position. */
        void spawnFloatingText(raylib::Vector2 pos);

    private:
        struct FloatingTextEntry
        {
            raylib::Vector2 position;
            float elapsed { 0.f };
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
        void generateGenericPhysicsObject(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData);
        void destroyMarkedObjects();
        void drawRemotePlayers();

        /*! Assigns a deterministic network id to a syncable object (enemy/coin) and registers it. */
        void registerNetworkObject(PhysicsObject* obj);
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
#ifdef GAME_DEV_DEBUG
        ggj::DebugState* m_pDebugState = nullptr;
#endif

        Portal* m_portal = nullptr;

        Hud& m_hud;

        Dialog& m_dialog;

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

        INetworkManager* m_network = nullptr;
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
