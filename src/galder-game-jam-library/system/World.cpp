//
// Created by robin on 29.10.22.
//

#include <algorithm>
#include "World.h"
#include "../game/Hud.h"
#include "../graphics/Sprite.h"
#include "../game/enemies/ClaudeEnemy.h"
#include "../game/enemies/TreantEnemy.h"
#include "../game/enemies/Claudling.h"
#include "../game/projectiles/MageBall.hpp"
#include "../graphics/CharacterPalette.h"

#ifdef GAME_DEV_DEBUG
    #include "../managers/DebugMenu.hpp"
#endif

namespace ggj
{
    bool World::initialize()
    {
        if (m_map == nullptr)
            return false;

        tson::Colori bg = m_map->getBackgroundColor();
        m_backgroundColor = raylib::Color(bg.r, bg.g, bg.b, bg.a);
        m_world.SetContactListener(&m_collisionManager);
        if (m_map->getProp("gravity_x") != nullptr && m_map->getProp("gravity_y") != nullptr)
        {
            m_world.SetGravity({ m_map->get<float>("gravity_x"), m_map->get<float>("gravity_y") });
        }
        if (m_map->getProp("camera_min_x") != nullptr && m_map->getProp("camera_min_y") != nullptr)
        {
            m_cameraMin = raylib::Vector2(m_map->get<float>("camera_min_x"), m_map->get<float>("camera_min_y"));
            m_camera.target = raylib::Vector2(m_cameraMin.GetX(), m_camera.target.y);
        }
        if (m_map->getProp("camera_max_x") != nullptr && m_map->getProp("camera_max_y") != nullptr)
        {
            m_cameraMax = raylib::Vector2(m_map->get<float>("camera_max_x"), m_map->get<float>("camera_max_y"));
            m_camera.target = raylib::Vector2(m_camera.target.x, m_cameraMax.GetY());
        }
        if (m_map->getProp("portal_timer") != nullptr)
        {
            m_portalTimer = m_map->get<int>("portal_timer");
        }
        m_cameraDefault = m_camera.target;

        int32_t layerIndex = 0;
        for (auto& layer : m_map->getLayers())
        {
            if (layer.getType() == tson::LayerType::TileLayer)
            {
                if (m_layers.count(layerIndex) == 0)
                    m_layers[layerIndex] = { layerIndex, true, layer.getName() }; //Tiles are only for drawing and will not be updated

                for (auto& tileObject : layer.getTileObjects())
                {
                    auto pos = tileObject.second.getPosition();
                    auto rect = tileObject.second.getDrawingRect();
                    auto tileset = tileObject.second.getTile()->getTileset()->getImagePath().filename().string();
                    TextureName id = m_mapper.getTextureNameByString(tileset);
                    if (id != TextureName::None)
                    {
                        raylib::Texture* tex = m_textures.get(id);
                        if (tex != nullptr)
                        {
                            raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                            raylib::Vector2 v = { pos.x + origin.x, pos.y + origin.y };
                            raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };
                            m_layers[layerIndex].createGameObject<ggj::Sprite>(v, r, tex);
                        }
                    }
                }
            }
            else if (layer.getType() == tson::LayerType::ObjectGroup)
            {
                if (m_layers.count(layerIndex) == 0)
                    m_layers[layerIndex] = { layerIndex, false, layer.getName() }; //Objects will have their positions updated!

                for (auto& obj : layer.getObjects())
                {
                    tson::TiledClass* c = obj.getClass();
                    if (c == nullptr)
                        continue;

                    if (c->getMember("is_static") != nullptr && c->getMember("is_trigger") != nullptr)
                    {
                        ObjectGeneratorData generatorData {};

                        generatorData.layerIndex = layerIndex;
                        generatorData.obj = obj;

                        std::string name = obj.getName();
                        generatorData.isStatic = c->get<bool>("is_static");
                        generatorData.isTrigger = c->get<bool>("is_trigger");
                        generatorData.isKinematic = c->get<bool>("is_kinematic");
                        generatorData.isPlayer = c->get<bool>("is_player");
                        generatorData.sprite = c->get<std::string>("sprite");
                        generatorData.spriteSize.x = (float)c->get<int>("sprite_size_x");
                        generatorData.spriteSize.y = (float)c->get<int>("sprite_size_y");
                        generatorData.velocity.x = c->get<float>("velocity_x");
                        generatorData.velocity.y = c->get<float>("velocity_y");

                        float userDataForceX = c->get<float>("userdata_force_x");
                        float userDataForceY = c->get<float>("userdata_force_y");
                        int userdataObjectType = c->get<int>("userdata_objecttype");
                        std::string userdataCommand = c->get<std::string>("userdata_command");
                        std::string userdataDialogId = c->get<std::string>("userdata_dialog_id");

                        //RBP: Note to self: Improve Tileson so this was handled automatically in the TiledClass.
                        for (auto& [key, value] : obj.getProperties().getProperties())
                        {
                            if (key == "is_static")
                                generatorData.isStatic = value.getValue<bool>();
                            else if (key == "is_trigger")
                                generatorData.isTrigger = value.getValue<bool>();
                            else if (key == "is_player")
                                generatorData.isPlayer = value.getValue<bool>();
                            else if (key == "sprite")
                                generatorData.sprite = value.getValue<std::string>();
                            else if (key == "sprite_size_x")
                                generatorData.spriteSize.x = (float)value.getValue<int>();
                            else if (key == "sprite_size_y")
                                generatorData.spriteSize.y = (float)value.getValue<int>();
                            else if (key == "velocity_x")
                                generatorData.velocity.x = value.getValue<float>();
                            else if (key == "velocity_y")
                                generatorData.velocity.y = value.getValue<float>();
                            else if (key == "is_kinematic")
                                generatorData.isKinematic = value.getValue<bool>();
                            else if (key == "userdata_force_x")
                                userDataForceX = value.getValue<float>();
                            else if (key == "userdata_force_y")
                                userDataForceY = value.getValue<float>();
                            else if (key == "userdata_objecttype")
                                userdataObjectType = value.getValue<int>();
                            else if (key == "userdata_command")
                                userdataCommand = value.getValue<std::string>();
                            else if (key == "userdata_dialog_id")
                                userdataDialogId = value.getValue<std::string>();
                            else if (key == "lives")
                                generatorData.lives = value.getValue<int>();
                            else if (key == "follow_speed")
                                generatorData.followSpeed = value.getValue<float>();
                            else if (key == "knockback")
                                generatorData.knockback = value.getValue<float>();
                            else if (key == "is_boss")
                                generatorData.isBoss = value.getValue<bool>();
                            else if (key == "boss_name")
                                generatorData.bossName = value.getValue<std::string>();
                        }

                        ObjectType objectType = (ObjectType)userdataObjectType;
                        tson::Vector2i pos = generatorData.obj.getPosition();
                        tson::Vector2i size = generatorData.obj.getSize();
                        tson::Vector2i originalSize = generatorData.obj.getSize();

                        b2BodyType bodyType = (generatorData.isStatic) ? b2_staticBody : b2_dynamicBody;

                        if (generatorData.isKinematic)
                        {
                            bodyType = b2_kinematicBody;
                        }

                        PhysicsShape shape = PhysicsShape::None;
                        if (obj.getObjectType() == tson::ObjectType::Rectangle)
                            shape = PhysicsShape::Rectangle;
                        else if (obj.getObjectType() == tson::ObjectType::Ellipse)
                            shape = PhysicsShape::Circle;

                        b2BodyDef bodyDef;
                        bodyDef.type = bodyType;

                        b2Vec2 b2size = PhysicsObject::ConvertToB2Vec2({ (float)size.x, (float)size.y });
                        b2Vec2 b2origin = PhysicsObject::ConvertToB2Vec2({ (float)size.x / 2, (float)size.y / 2 });
                        b2Vec2 b2pos = PhysicsObject::ConvertToB2Vec2({ (float)pos.x, (float)pos.y });

                        bodyDef.position.Set(b2pos.x + b2origin.x, b2pos.y + b2origin.y);
                        b2Body* body = m_world.CreateBody(&bodyDef);
                        if (shape == PhysicsShape::Rectangle)
                        {
                            b2PolygonShape box;
                            box.SetAsBox(b2size.x / 2 - b2_polygonRadius, b2size.y / 2 - b2_polygonRadius);

                            b2FixtureDef fd;
                            fd.shape = &box;
                            fd.density = 1.0f; //Density must be something to react to gravity.
                            fd.friction = 0.3f;
                            body->CreateFixture(&fd);
                        }
                        else if (shape == PhysicsShape::Circle)
                        {
                            b2CircleShape circle;
                            size = { size.x / 2, size.y / 2 }; //Size is half, as it is radius
                            circle.m_p.Set(0.f, 0.f);
                            circle.m_radius = b2origin.x;

                            b2FixtureDef fd;
                            fd.shape = &circle;
                            fd.density = 1.0f; //Density must be something to react to gravity.
                            fd.friction = 1.0f;

                            body->CreateFixture(&fd);
                        }

                        for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
                        {
                            f->SetSensor(generatorData.isTrigger);
                        }

                        UserData userData { objectType, { userDataForceX, userDataForceY }, userdataCommand, userdataDialogId, name };
                        generatorData.userData = userData;

                        generatorData.pos = pos;
                        generatorData.size = size;
                        generatorData.originalSize = originalSize;

                        generatePhysicsObject(name, body, generatorData);
                    }
                }
            }

            ++layerIndex;
        }

        m_minimap.initialize(*m_map, m_layers);

        return true;
    }

    void World::update(float timeDelta)
    {
        m_world.Step(timeDelta, 8,
                     3); //8 and 3 are numbers suggested by Box2D documentation, higher numbers: more accuracy but costs more performance.
        for (auto& layer : m_layers)
        {
            layer.second.update(timeDelta);
        }

        m_hud.update(timeDelta);
        m_dialog.update(timeDelta);
        updateRemotePlayerSprites(timeDelta);
        updateRemoteProjectiles(timeDelta);

        if (m_player != nullptr)
        {
#ifdef GAME_DEV_DEBUG
            if (m_pDebugState != nullptr)
            {
                m_player->getHitbox()->setVisible(m_pDebugState->showHitboxes);
                m_player->setCameraFollowsPlayer(m_pDebugState->cameraFollowsPlayer);
            }
#endif

            if (m_player->hasClearedLevel())
            {
                setLevelCleared(true);
                m_player->setHasClearedLevel(false);
            }

            m_camera.target = (m_player->cameraShouldFollowPlayer()) ? raylib::Vector2((float)(int)(std::round(m_player->getPosition().x) - (float)800.f / 2),
                                                                                       (float)(int)(std::round(m_player->getPosition().y) - (float)600.f / 2))
                                                                     : m_cameraDefault;
        }

        if (m_camera.target.x > m_cameraMax.x)
            m_camera.target.x = m_cameraMax.x;
        if (m_camera.target.x < m_cameraMin.x)
            m_camera.target.x = m_cameraMin.x;
        if (m_camera.target.y > m_cameraMax.y)
            m_camera.target.y = m_cameraMax.y;
        if (m_camera.target.y < m_cameraMin.y)
            m_camera.target.y = m_cameraMin.y;

        flushPendingSpawns();
        syncNetworkObjects();
        destroyMarkedObjects();

        for (FloatingTextEntry& e : m_floatingTexts)
        {
            e.position.y -= FloatingTextEntry::RISE_SPEED * timeDelta;
            e.elapsed    += timeDelta;
        }
        std::erase_if(m_floatingTexts, [](const FloatingTextEntry& e) {
            return e.elapsed >= FloatingTextEntry::DURATION;
        });
    }

    void World::spawnFloatingText(raylib::Vector2 pos)
    {
        FloatingTextEntry e;
        e.position = raylib::Vector2 { pos.x - 14.f, pos.y - 20.f };
        m_floatingTexts.push_back(e);
    }

    void World::spawnCoinFloatingText(raylib::Vector2 pos)
    {
        FloatingTextEntry e;
        e.position = raylib::Vector2 { pos.x - 6.f, pos.y - 20.f };
        e.text  = "+1";
        e.color = Color { 255, 215, 0, 255 };
        m_floatingTexts.push_back(e);
    }

    void World::draw()
    {
        m_window.ClearBackground(m_backgroundColor);
        m_camera.BeginMode();
        for (auto& layer : m_layers)
        {
            layer.second.draw();
        }
        drawRemotePlayers();
        drawRemoteProjectiles();

        for (const FloatingTextEntry& e : m_floatingTexts)
        {
            const float progress = e.elapsed / FloatingTextEntry::DURATION;
            const uint8_t alpha = static_cast<uint8_t>(255 * (1.f - progress));
            Color color = e.color;
            color.a = alpha;
            DrawTextEx(GetFontDefault(), e.text.c_str(), { e.position.x, e.position.y }, 12, 1.f, color);
        }

#ifdef GAME_DEV_DEBUG
        if (m_pDebugState != nullptr && m_pDebugState->showHitboxes)
            drawPhysicsDebug();
#endif
        m_camera.EndMode();
        m_hud.draw();
        m_dialog.draw();
    }

    void World::updateRemotePlayerSprites(float timeDelta)
    {
        if (m_network == nullptr || !m_network->isActive())
        {
            m_remotePlayerSprites.clear();
            return;
        }

        const std::map<uint32_t, RemotePlayer>& remotes = m_network->remotePlayers();
        std::erase_if(m_remotePlayerSprites, [&remotes](const auto& entry) { return remotes.count(entry.first) == 0; });

        for (const auto& [playerId, remote] : remotes)
        {
            const bool isNew = m_remotePlayerSprites.count(playerId) == 0;
            RemotePlayerSprite& sprite = m_remotePlayerSprites[playerId];
            // Restart the animation on a state change, exactly like Player::setPlayerState().
            if (isNew || sprite.state != remote.state)
            {
                sprite.state = remote.state;
                sprite.animation = m_animationManager.getAnimation(m_mapper.getAnimationNameByPlayerState(remote.state));
            }
            if (sprite.animation.isValid())
                sprite.animation.update(timeDelta);
        }
    }

    void World::registerLocalProjectile(Projectile* projectile)
    {
        if (projectile == nullptr || m_network == nullptr || !m_network->isActive())
            return;

        const uint32_t projectileId = m_nextProjectileId++;
        m_localProjectiles[projectileId] = LocalProjectile { projectile, false };
        // Body position, not getPosition(): m_position is only refreshed on the projectile's first update.
        const raylib::Vector2 position = PhysicsObject::ConvertToVector2(projectile->getBody()->GetPosition());
        m_network->notifyLocalProjectileFired(projectileId, position, projectile->getVelocity());
    }

    void World::syncLocalProjectiles()
    {
        // Runs before destroyMarkedObjects(), so pointers are still valid while shouldBeDestroyed() is set.
        const bool isOnline = m_network != nullptr && m_network->isActive();
        for (auto it = m_localProjectiles.begin(); it != m_localProjectiles.end();)
        {
            Projectile* projectile = it->second.projectile;
            const bool isGone = projectile->shouldBeDestroyed();
            if ((projectile->isExploded() || isGone) && !it->second.hasReportedImpact)
            {
                if (isOnline)
                    m_network->notifyLocalProjectileImpact(it->first, projectile->getPosition());
                it->second.hasReportedImpact = true;
            }

            if (isGone)
                it = m_localProjectiles.erase(it);
            else
                ++it;
        }
    }

    void World::updateRemoteProjectiles(float timeDelta)
    {
        if (m_network == nullptr || !m_network->isActive())
        {
            m_remoteProjectiles.clear();
            return;
        }

        for (const ProjectileEvent& event : m_network->takeProjectileEvents())
        {
            if (!event.isImpact)
            {
                RemoteProjectile projectile;
                projectile.ownerId = event.ownerId;
                projectile.projectileId = event.projectileId;
                projectile.position = event.position;
                projectile.velocity = event.velocity;
                projectile.animation = m_animationManager.getAnimation(AnimationName::MageBallFlightRed);
                m_remoteProjectiles.push_back(projectile);
                continue;
            }

            for (RemoteProjectile& projectile : m_remoteProjectiles)
            {
                if (projectile.ownerId == event.ownerId && projectile.projectileId == event.projectileId && !projectile.isImpact)
                {
                    projectile.position = event.position;
                    projectile.isImpact = true;
                    projectile.timeAlive = 0.0f;
                    projectile.animation = m_animationManager.getAnimation(AnimationName::MageBallImpactRed);
                    break;
                }
            }
        }

        // Same lifecycle as Projectile::update(): fly for TTL, then show the impact for TTL. The
        // owner's impact event normally cuts the flight short; the timer is only the fallback.
        for (RemoteProjectile& projectile : m_remoteProjectiles)
        {
            projectile.timeAlive += timeDelta;
            if (!projectile.isImpact)
            {
                projectile.position.x += projectile.velocity.x * PhysicsObject::PPM * timeDelta;
                projectile.position.y += projectile.velocity.y * PhysicsObject::PPM * timeDelta;
                if (projectile.timeAlive >= MageBall::DEFAULT_TTL)
                {
                    projectile.isImpact = true;
                    projectile.timeAlive = 0.0f;
                    projectile.animation = m_animationManager.getAnimation(AnimationName::MageBallImpactRed);
                }
            }
            if (projectile.animation.isValid())
                projectile.animation.update(timeDelta);
        }

        std::erase_if(m_remoteProjectiles, [](const RemoteProjectile& projectile) { return projectile.isImpact && projectile.timeAlive >= MageBall::DEFAULT_TTL; });
    }

    void World::drawRemoteProjectiles()
    {
        raylib::Texture* texture = m_textures.get(TextureName::MageBall);
        if (texture == nullptr)
            return;

        // Origin matches PhysicsSprite for a 16x16 circle body: (size.x / 2, size.y - physicsSize.y).
        constexpr float MAGE_BALL_SIZE = 16.0f;
        const raylib::Vector2 origin { MAGE_BALL_SIZE / 2.0f, 0.0f };
        for (RemoteProjectile& projectile : m_remoteProjectiles)
        {
            if (!projectile.animation.isValid())
                continue;

            const raylib::Rectangle& frame = projectile.animation.getDrawingRect();
            texture->Draw(frame, raylib::Rectangle { projectile.position.x, projectile.position.y, frame.width, frame.height }, origin, 0.0f, WHITE);
        }
    }

    void World::drawRemotePlayers()
    {
        if (m_network == nullptr || !m_network->isActive())
            return;

        constexpr int FONT_SIZE = 10;
        constexpr float NAME_OFFSET_Y = 6.0f; ///< Label sits this far below the top edge of the 64px frame.

        for (const auto& [playerId, remote] : m_network->remotePlayers())
        {
            raylib::Texture* texture = (remote.character == PlayerPickedCharacter::Wizard) ? m_wizardTexture : m_rogueTexture;
            auto found = m_remotePlayerSprites.find(playerId);
            if (texture == nullptr || found == m_remotePlayerSprites.end() || !found->second.animation.isValid())
                continue;

            // Mirrors PhysicsSprite::draw(): negative source width flips the frame horizontally.
            const raylib::Rectangle& frame = found->second.animation.getDrawingRect();
            const raylib::Rectangle source { frame.x, frame.y, remote.facingLeft ? -frame.width : frame.width, frame.height };
            m_paletteSwap.begin(CharacterPalette::BuildPaletteSwap(remote.character, remote.colors));
            texture->Draw(source, raylib::Rectangle { remote.position.x, remote.position.y, frame.width, frame.height }, m_playerSpriteOrigin, 0.0f, WHITE);
            m_paletteSwap.end();

            if (!remote.name.empty())
            {
                const int textWidth = MeasureText(remote.name.c_str(), FONT_SIZE);
                const float top = remote.position.y - m_playerSpriteOrigin.y + NAME_OFFSET_Y;
                ::DrawText(remote.name.c_str(), static_cast<int>(remote.position.x) - textWidth / 2, static_cast<int>(top), FONT_SIZE, remote.nameColor);
            }
        }
    }

    void World::generatePhysicsObject(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        if (name == "player1")
            generatePlayer(name, body, generatorData);
        else if (name == "bat")
        {
            generateBat(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "snake")
        {
            generateSnake(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "coin")
            generateCoin(name, body, generatorData);
        else if (name == "spider")
        {
            generateSpider(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "thing")
        {
            generateThing(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "ghost")
        {
            generateGhost(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "portal")
            generatePortal(name, body, generatorData);
        else if (name == "powerup")
            generatePowerUp(name, body, generatorData);
        else if (name == "claude")
        {
            generateClaude(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "claudling")
        {
            generateClaudling(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else if (name == "treant")
        {
            generateTreant(name, body, generatorData);
            ++m_numberOfEnemies;
        }
        else
            generateGenericPhysicsObject(name, body, generatorData);

        // Enemies and coins are synced over the network. They are generated in the same order on
        // every peer (same map + generators), so a per-spawn counter yields matching ids everywhere.
        if (name == "bat" || name == "snake" || name == "spider" || name == "thing" || name == "ghost" || name == "coin"
            || name == "claude" || name == "claudling" || name == "treant")
            registerNetworkObject(m_userDataManager.getUserData(body));
    }

    void World::registerNetworkObject(PhysicsObject* obj)
    {
        if (obj == nullptr)
            return;

        const uint32_t id = m_nextNetworkId++;
        obj->setNetworkId(id);
        m_networkObjects[id] = obj;
        if (obj->getBody() != nullptr)
            m_networkObjectSpawns[id] = PhysicsObject::ConvertToVector2(obj->getBody()->GetPosition());
    }

    std::vector<MinimapMarker> World::getRemotePlayerMarkers() const
    {
        std::vector<MinimapMarker> markers;
        if (m_network == nullptr || !m_network->isActive())
            return markers;

        for (const auto& [playerId, remote] : m_network->remotePlayers())
        {
            // Same formula as Player::getVisualCenter(), so remote dots line up with the local one.
            const raylib::Vector2 visualCenter { remote.position.x, remote.position.y - m_playerSpriteOrigin.y + m_playerSpriteHeight / 2.f };
            markers.push_back(MinimapMarker { visualCenter, remote.nameColor, false });
        }
        return markers;
    }

    void World::notifyLocalPortalEntered()
    {
        if (m_network != nullptr && m_network->isActive())
            m_network->notifyLocalPortalEntered();
    }

    void World::syncNetworkObjects()
    {
        // Always prune destroyed objects so raw pointers in m_networkObjects never outlive
        // the unique_ptrs freed by destroyMarkedObjects() — even in single-player where
        // the network-active guard below would otherwise skip this entirely.
        for (auto it = m_networkObjects.begin(); it != m_networkObjects.end();)
        {
            PhysicsObject* obj = it->second;
            if (obj->shouldBeDestroyed())
            {
                if (m_network != nullptr && m_network->isActive())
                {
                    const uint32_t id = it->first;
                    if (m_reportedDestroyed.insert(id).second)
                    {
                        const bool isCoin = obj->getUserData()->getObjectType() == ObjectType::Coin;
                        m_network->notifyLocalObjectDestroyed(id, isCoin);
                    }
                }
                it = m_networkObjects.erase(it);
            }
            else
            {
                ++it;
            }
        }

        syncLocalProjectiles();

        if (m_network == nullptr || !m_network->isActive())
            return;

        // A peer walked into this stage's portal: follow them through it.
        if (m_network->takeRemotePortalEntry() && m_portal != nullptr)
            m_portal->enter();

        // Feed the local player's authoritative stats out for syncing.
        if (m_player != nullptr)
            m_network->setLocalPlayerStats(m_player->getLives(), m_player->getScore(), m_player->getEnemiesKilled());

        // 1. Apply authoritative removals from the network (objects others destroyed).
        for (uint32_t id : m_network->removedObjectIds())
        {
            auto found = m_networkObjects.find(id);
            if (found != m_networkObjects.end())
            {
                const bool isEnemy = found->second->getUserData()->getObjectType() == ObjectType::Enemy;
                found->second->destroy();
                m_reportedDestroyed.insert(id);
                m_networkObjects.erase(found);
                if (isEnemy)
                    ++m_networkRemovedEnemyCount;
            }
        }

        // 2. Client: snap enemies to the host's authoritative position and facing. The client still
        //    runs each enemy's own AI, whose turn decisions drift from the host's, so the facing
        //    has to be overwritten too or sprites end up looking the wrong way.
        if (m_network->role() == NetworkRole::Client)
        {
            for (const auto& [id, state] : m_network->enemyStates())
            {
                auto found = m_networkObjects.find(id);
                if (found != m_networkObjects.end())
                {
                    found->second->setNetworkPosition(state.position);
                    found->second->setFlipped(state.isFlipped);
                }
            }
        }

        // 3. Server: broadcast enemy positions (throttled to ~20 Hz).
        if (m_network->role() == NetworkRole::Server)
        {
            if (m_enemyStateThrottle == 0)
            {
                for (const auto& [id, obj] : m_networkObjects)
                {
                    if (obj->getUserData()->getObjectType() == ObjectType::Enemy)
                        m_network->publishEnemyState(id, obj->getPosition(), obj->isFlipped());
                }
            }
            m_enemyStateThrottle = (m_enemyStateThrottle + 1) % ENEMY_STATE_THROTTLE_FRAMES;
        }
    }

    void World::setPickedCharacter(PlayerPickedCharacter character)
    {
        m_pickedCharacter = character;
    }

    void World::setPlayerColors(const CharacterColors& colors)
    {
        m_playerPaletteSwap = CharacterPalette::BuildPaletteSwap(m_pickedCharacter, colors);
    }

    void World::beginPlayerPalette()
    {
        m_paletteSwap.begin(m_playerPaletteSwap);
    }

    void World::endPlayerPalette()
    {
        m_paletteSwap.end();
    }

    void World::generatePlayer(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);
                raylib::Vector2 physicsSize = raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y);

                // Remember the sprite setup so remote peers can be drawn exactly like a local player.
                // Same origin formula as PhysicsSprite; the map sprite is the Rogue texture.
                const bool isCircle = body->GetFixtureList()->GetShape()->GetType() == b2Shape::e_circle;
                m_playerSpriteOrigin = raylib::Vector2 { spriteSize.x / 2.f, isCircle ? spriteSize.y - physicsSize.y : spriteSize.y - physicsSize.y / 2.f };
                m_playerSpriteHeight = spriteSize.y;
                m_rogueTexture = tex;
                m_wizardTexture = m_textures.get(TextureName::WizardPlayer);

                const int32_t playerIndex = m_nextPlayerIndex++;
                m_playerStats[playerIndex] = PlayerStats {};

                if (m_pickedCharacter == PlayerPickedCharacter::Wizard)
                {
                    if (raylib::Texture* wizardTex = m_textures.get(TextureName::WizardPlayer); wizardTex != nullptr)
                        tex = wizardTex;
                    Player2* p2 = m_layers[generatorData.layerIndex].createGameObject<ggj::Player2>(this, playerIndex, m_input, m_animationManager, m_mapper, m_soundManager, m_optionsManager, body, physicsSize,
                                                                                                    spriteSize, r, tex, generatorData.userData);
                    m_player = p2;
                    m_userDataManager.addUserData(body, p2);
                    m_userDataManager.addUserData(p2->getHitbox()->getBody(), p2->getHitbox());
                }
                else
                {
                    Player* p1 = m_layers[generatorData.layerIndex].createGameObject<ggj::Player>(m_input, m_animationManager, m_mapper, m_soundManager, m_optionsManager, this, playerIndex, body,
                                                                                                  physicsSize, spriteSize, r, tex, generatorData.userData);
                    m_player = p1;
                    m_userDataManager.addUserData(body, p1);
                    m_userDataManager.addUserData(p1->getHitbox()->getBody(), p1->getHitbox());
                }
            }
        }
    }

    void World::generateBat(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Bat>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity, generatorData.lives);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generateGenericPhysicsObject(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        bool isVisible = !generatorData.isStatic;

        PhysicsShape shape = PhysicsShape::None;
        if (generatorData.obj.getObjectType() == tson::ObjectType::Rectangle)
            shape = PhysicsShape::Rectangle;
        else if (generatorData.obj.getObjectType() == tson::ObjectType::Ellipse)
            shape = PhysicsShape::Circle;

        raylib::Color color = generatorData.isStatic ? RED : DARKPURPLE;
        if (generatorData.sprite.empty())
        {
            PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::PhysicsObject>(
                body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), shape, generatorData.userData, color, isVisible);
            m_userDataManager.addUserData(body, physicsObject);
        }
        else
        {
            TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
            if (id != TextureName::None)
            {
                raylib::Texture* tex = m_textures.get(id);
                if (tex != nullptr)
                {
                    raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                    raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                    raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                    if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                    {
                        r.width = generatorData.spriteSize.x;
                        r.height = generatorData.spriteSize.y;
                    }

                    raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                    PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::PhysicsSprite>(
                        body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex, generatorData.userData);
                    m_userDataManager.addUserData(body, physicsObject);
                }
            }
        }
    }

    void World::generateSnake(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Snake>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity, generatorData.lives);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generateCoin(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Coin>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generateSpider(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Spider>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity, generatorData.lives);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generateThing(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Thing>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity, generatorData.lives);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generateGhost(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Ghost>(
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.velocity, generatorData.lives);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::generatePortal(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                m_portal = m_layers[generatorData.layerIndex].createGameObject<ggj::Portal>(m_animationManager, m_mapper, m_soundManager, body,
                                                                                            raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize,
                                                                                            r, tex, generatorData.userData, generatorData.velocity, m_portalTimer, this);
                m_userDataManager.addUserData(body, m_portal);
            }
        }
    }

    void World::generatePowerUp(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Vector2 origin = { (float)rect.width / 2, (float)rect.height / 2 };
                raylib::Vector2 v = { (float)generatorData.pos.x + origin.x, (float)generatorData.pos.y + origin.y };

                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::PowerUp>(
                    m_animationManager, m_mapper, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex, generatorData.userData,
                    generatorData.velocity, m_portalTimer);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

    void World::destroyMarkedObjects()
    {
        for (auto& layer : m_layers)
        {
            layer.second.destroyMarkedObjects();
        }
    }

    int World::getNumberOfEnemiesLeft() const
    {
        // Never report fewer than zero left: kills can never exceed the total in the quest log.
        return std::max(0, m_numberOfEnemies
                               - (m_player->getUserData()->enemiesKilled)
                               - m_networkRemovedEnemyCount);
    }

    std::string World::getLeadingPlayer() const
    {
        //if(m_player->getScore() > m_player2->getScore())
        return m_player->getUserData()->getName();
        //if(m_player->getScore() < m_player2->getScore())
        //  return m_player2->getUserData()->getName();
        //return "both";
    }

    IPlayer* World::getPlayer()
    {
        return m_player;
    }

#ifdef GAME_DEV_DEBUG
    void World::setDebugState(ggj::DebugState* pState)
    {
        m_pDebugState = pState;
    }

    void World::killAllEnemies()
    {
        if (m_player == nullptr)
            return;

        int killed = 0;
        for (auto& [id, obj] : m_networkObjects)
        {
            if (obj->getUserData()->getObjectType() == ObjectType::Enemy
                && !obj->shouldBeDestroyed())
            {
                obj->destroy();
                ++killed;
            }
        }
        m_player->getUserData()->enemiesKilled += killed;
    }
#endif

    int World::getEnemies() const
    {
        return m_numberOfEnemies;
    }

    void World::reduceEnemyKillCountByOne()
    {
        --m_numberOfEnemies;
    }

    void World::restorePlayerStats(int32_t playerIndex, const PlayerPersistentState& state)
    {
        if (m_playerStats.count(playerIndex) > 0)
        {
            PlayerStats& stats = m_playerStats[playerIndex];
            stats.lives        = state.lives;
            stats.score        = state.score;
            stats.xpTracker.restore(state.level, state.currentXp, state.totalXp);
            stats.talentPoints    = state.talentPoints;
            stats.talentState     = state.talentState;
            stats.computedEffects = computeTalentEffects(state.talentState);
        }
    }

    void World::addPlayerXp(int32_t playerIndex, int amount)
    {
        auto& stats = m_playerStats.at(playerIndex);
        const int scaled    = static_cast<int>(static_cast<float>(amount) * stats.computedEffects.xpMultiplier);
        const int prevLevel = stats.xpTracker.getLevel();
        stats.xpTracker.addXp(scaled > 0 ? scaled : amount, stats.xpConfig);
        const int newLevel  = stats.xpTracker.getLevel();
        if (newLevel > prevLevel)
            m_soundManager.playSound(SoundName::LevelUp);
        // Grant 1 talent point for each even level crossed (levels 2, 4, 6 …)
        stats.talentPoints += (newLevel / 2) - (prevLevel / 2);
    }

    void World::updatePlayerTalentState(int32_t playerIndex, const TalentState& state, int talentPoints)
    {
        if (m_playerStats.count(playerIndex) > 0)
        {
            PlayerStats& stats    = m_playerStats[playerIndex];
            stats.talentState     = state;
            stats.talentPoints    = talentPoints;
            stats.computedEffects = computeTalentEffects(state);
        }
    }

    void World::spawnClaudling(raylib::Vector2 position, float followSpeed, float knockback)
    {
        m_pendingClaudlingSpawns.push_back({ position, followSpeed, knockback });
    }

    void World::flushPendingSpawns()
    {
        for (const PendingClaudlingSpawn& spawn : m_pendingClaudlingSpawns)
            doSpawnClaudling(spawn.position, spawn.followSpeed, spawn.knockback);
        m_pendingClaudlingSpawns.clear();
    }

    void World::doSpawnClaudling(raylib::Vector2 position, float followSpeed, float knockback)
    {
        const raylib::Vector2 SIZE { 32.f, 32.f };

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        const b2Vec2 b2origin = PhysicsObject::ConvertToB2Vec2({ SIZE.x / 2.f, SIZE.y / 2.f });
        const b2Vec2 b2pos    = PhysicsObject::ConvertToB2Vec2(position);
        bodyDef.position.Set(b2pos.x + b2origin.x, b2pos.y + b2origin.y);
        b2Body* body = m_world.CreateBody(&bodyDef);

        b2PolygonShape box;
        const b2Vec2 b2size = PhysicsObject::ConvertToB2Vec2(SIZE);
        box.SetAsBox(b2size.x / 2.f - b2_polygonRadius, b2size.y / 2.f - b2_polygonRadius);
        b2FixtureDef fd;
        fd.shape    = &box;
        fd.density  = 1.0f;
        fd.friction = 0.3f;
        fd.isSensor = true;
        body->CreateFixture(&fd);
        body->SetFixedRotation(true);

        raylib::Texture* tex = m_textures.get(TextureName::EnemyClaudling);
        if (tex == nullptr)
            return;

        const raylib::Rectangle drawingRect { 0.f, 0.f, SIZE.x, SIZE.y };
        const UserData userData { ObjectType::Enemy, { 0.f, 0.f }, "", "", "claudling" };

        PhysicsObject* physicsObject = createObject<Claudling>(
            m_animationManager, m_mapper, m_soundManager, body,
            SIZE, SIZE, drawingRect, tex, userData, followSpeed, knockback, m_player);

        if (physicsObject != nullptr)
        {
            m_userDataManager.addUserData(body, physicsObject);
            ++m_numberOfEnemies;
            registerNetworkObject(m_userDataManager.getUserData(body));
        }
    }

    void World::generateClaude(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::ClaudeEnemy>(
                    m_animationManager, m_mapper, m_soundManager, body,
                    raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y),
                    spriteSize, r, tex, generatorData.userData, generatorData.lives,
                    generatorData.followSpeed, generatorData.knockback, m_player, this);
                m_userDataManager.addUserData(body, physicsObject);
                if (generatorData.isBoss && physicsObject != nullptr)
                {
                    m_boss         = physicsObject;
                    m_bossName     = generatorData.bossName;
                    m_bossMaxLives = generatorData.lives;
                }
            }
        }
    }

    void World::generateTreant(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                // The treant sprite is a single 64x64 frame, regardless of the object's physics size.
                const raylib::Rectangle r { 0.f, 0.f, 64.f, 64.f };

                // sprite_size_x/y is the size the frame is scaled up to when drawn.
                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);
                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                    spriteSize = raylib::Vector2(generatorData.spriteSize.x, generatorData.spriteSize.y);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::TreantEnemy>(
                    m_animationManager, m_mapper, m_soundManager, body,
                    raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y),
                    spriteSize, r, tex, generatorData.userData, generatorData.lives,
                    generatorData.followSpeed, generatorData.knockback, m_player, this);
                m_userDataManager.addUserData(body, physicsObject);
                if (generatorData.isBoss && physicsObject != nullptr)
                {
                    m_boss         = physicsObject;
                    m_bossName     = generatorData.bossName;
                    m_bossMaxLives = generatorData.lives;
                }
            }
        }
    }

    void World::keepEnemiesClearOf(const raylib::Vector2& point, float radius)
    {
        const float radiusSq = radius * radius;
        const auto isNear = [&](const raylib::Vector2& pos)
        {
            const float dx = pos.x - point.x;
            const float dy = pos.y - point.y;
            return dx * dx + dy * dy < radiusSq;
        };

        for (const auto& [id, obj] : m_networkObjects)
        {
            if (obj->getUserData()->getObjectType() != ObjectType::Enemy || obj->shouldBeDestroyed() || obj->getBody() == nullptr)
                continue;
            if (m_networkObjectSpawns.count(id) == 0)
                continue;

            const raylib::Vector2 currentPos = PhysicsObject::ConvertToVector2(obj->getBody()->GetPosition());
            const raylib::Vector2& spawnPos  = m_networkObjectSpawns.at(id);
            if (isNear(currentPos) && !isNear(spawnPos))
            {
                obj->setNetworkPosition(spawnPos);
                obj->getBody()->SetLinearVelocity({ 0.f, 0.f });
            }
        }
    }

    void World::generateClaudling(const std::string& name, b2Body* body, const ObjectGeneratorData& generatorData)
    {
        TextureName id = m_mapper.getTextureNameByString(generatorData.sprite);
        if (id != TextureName::None)
        {
            raylib::Texture* tex = m_textures.get(id);
            if (tex != nullptr)
            {
                raylib::Rectangle rect = { 0.f, 0.f, (float)generatorData.originalSize.x, (float)generatorData.originalSize.y };
                raylib::Rectangle r = { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height };

                if (generatorData.spriteSize.x != 0 && generatorData.spriteSize.y != 0)
                {
                    r.width = generatorData.spriteSize.x;
                    r.height = generatorData.spriteSize.y;
                }

                raylib::Vector2 spriteSize = raylib::Vector2(r.width, r.height);

                PhysicsObject* physicsObject = m_layers[generatorData.layerIndex].createGameObject<ggj::Claudling>(
                    m_animationManager, m_mapper, m_soundManager, body,
                    raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y),
                    spriteSize, r, tex, generatorData.userData, generatorData.followSpeed, generatorData.knockback, m_player);
                m_userDataManager.addUserData(body, physicsObject);
            }
        }
    }

#ifdef GAME_DEV_DEBUG
    void World::drawPhysicsDebug()
    {
        static constexpr Color DYNAMIC_COLOR { 0, 100, 255, 120 };
        static constexpr Color STATIC_COLOR { 180, 0, 255, 120 };
        static constexpr Color SENSOR_COLOR { 255, 0, 0, 120 };
        static constexpr float LINE_THICK = 1.5f;

        for (const b2Body* body = m_world.GetBodyList(); body != nullptr; body = body->GetNext())
        {
            const Color bodyColor = (body->GetType() == b2_staticBody) ? STATIC_COLOR : DYNAMIC_COLOR;

            for (const b2Fixture* fix = body->GetFixtureList(); fix != nullptr; fix = fix->GetNext())
            {
                const Color color = fix->IsSensor() ? SENSOR_COLOR : bodyColor;

                const b2Shape* shape = fix->GetShape();
                if (shape->GetType() == b2Shape::e_polygon)
                {
                    const auto* poly = static_cast<const b2PolygonShape*>(shape);
                    const int32 n = poly->m_count;
                    for (int32 i = 0; i < n; ++i)
                    {
                        const b2Vec2 w1 = body->GetWorldPoint(poly->m_vertices[i]);
                        const b2Vec2 w2 = body->GetWorldPoint(poly->m_vertices[(i + 1) % n]);
                        DrawLineEx({ w1.x * PhysicsObject::PPM, w1.y * PhysicsObject::PPM },
                                   { w2.x * PhysicsObject::PPM, w2.y * PhysicsObject::PPM }, LINE_THICK, color);
                    }
                }
                else if (shape->GetType() == b2Shape::e_circle)
                {
                    const auto* circle = static_cast<const b2CircleShape*>(shape);
                    const b2Vec2 center = body->GetWorldPoint(circle->m_p);
                    DrawCircleLines(
                        static_cast<int>(center.x * PhysicsObject::PPM),
                        static_cast<int>(center.y * PhysicsObject::PPM),
                        circle->m_radius * PhysicsObject::PPM,
                        color);
                }
            }
        }
    }
#endif

} // namespace ggj
