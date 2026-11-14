//
// Created by robin on 29.10.22.
//

#include <algorithm>
#include <limits>
#include "World.h"
#include "../game/Hud.h"
#include "../graphics/Sprite.h"
#include "../game/enemies/ClaudeEnemy.h"
#include "../game/enemies/TreantEnemy.h"
#include "../game/enemies/TreantRoot.h"
#include "../game/enemies/Claudling.h"
#include "../game/enemies/Bomb.h"
#include "CollisionCategory.hpp"
#include <numbers>
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
                            else if (key == "idle_time")
                                generatorData.idleTime = value.getValue<float>();
                            else if (key == "bomb_min_time")
                                generatorData.bombMinTime = value.getValue<float>();
                            else if (key == "bomb_max_time")
                                generatorData.bombMaxTime = value.getValue<float>();
                            else if (key == "bomb_spawn_idle_state_time")
                                generatorData.bombSpawnIdleStateTime = value.getValue<float>();
                            else if (key == "bomb_spawn_count")
                                generatorData.bombSpawnCount = value.getValue<int>();
                            else if (key == "root_count")
                                generatorData.rootCount = value.getValue<int>();
                            else if (key == "enraged_root_count")
                                generatorData.enragedRootCount = value.getValue<int>();
                            else if (key == "death_root_count")
                                generatorData.deathRootCount = value.getValue<int>();
                            else if (key == "root_min_spike_delay")
                                generatorData.rootMinSpikeDelay = value.getValue<float>();
                            else if (key == "root_max_spike_delay")
                                generatorData.rootMaxSpikeDelay = value.getValue<float>();
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
                        // Solid static objects are the walls bombs bounce off.
                        if (bodyType == b2_staticBody && !generatorData.isTrigger)
                            CollisionCategory::MarkAsTerrain(*body);

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
                if (Hitbox* hitbox = m_player->getHitbox(); hitbox != nullptr)
                    hitbox->setVisible(m_pDebugState->showHitboxes);
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

    const DialogSpot* World::findDialogSpotNear(const raylib::Vector2& point) const
    {
        const DialogSpot* closest = nullptr;
        float closestDistance = std::numeric_limits<float>::max();
        for (const DialogSpot& spot : m_dialogSpots)
        {
            const float distance = point.Distance(spot.position);
            if (distance <= spot.interactRadius && distance < closestDistance)
            {
                closest = &spot;
                closestDistance = distance;
            }
        }
        return closest;
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

        const Font& worldFont = (m_font != nullptr) ? static_cast<const Font&>(*m_font) : GetFontDefault();
        for (const FloatingTextEntry& e : m_floatingTexts)
        {
            const float progress = e.elapsed / FloatingTextEntry::DURATION;
            const uint8_t alpha = static_cast<uint8_t>(255 * (1.f - progress));
            Color color = e.color;
            color.a = alpha;
            DrawTextEx(worldFont, e.text.c_str(), { e.position.x, e.position.y }, FLOATING_TEXT_FONT_SIZE, 0.f, color);
        }

#ifdef GAME_DEV_DEBUG
        if (m_pDebugState != nullptr && m_pDebugState->showHitboxes)
            drawPhysicsDebug();
#endif
        m_dialog.drawPrompt();
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

        // Matches Projectile: the sprite is centred on the body position.
        constexpr float MAGE_BALL_SIZE = 16.0f;
        const raylib::Vector2 origin { MAGE_BALL_SIZE / 2.0f, MAGE_BALL_SIZE / 2.0f };
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

        constexpr float FONT_SIZE = 11.0f;
        constexpr float NAME_OFFSET_Y = 6.0f; ///< Label sits this far below the top edge of the 64px frame.
        const Font& font = (m_font != nullptr) ? static_cast<const Font&>(*m_font) : GetFontDefault();

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
                const float textWidth = MeasureTextEx(font, remote.name.c_str(), FONT_SIZE, 0.0f).x;
                const float top = remote.position.y - m_playerSpriteOrigin.y + NAME_OFFSET_Y;
                DrawTextEx(font, remote.name.c_str(), { remote.position.x - textWidth * 0.5f, top }, FONT_SIZE, 0.0f, remote.nameColor);
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

        if (generatorData.userData.getObjectType() == ObjectType::Dialog)
        {
            constexpr float INTERACT_MARGIN = 28.f; // Reach beyond the NPC's own extent, in pixels
            const float halfWidth = static_cast<float>(generatorData.originalSize.x) / 2.f;
            const float halfHeight = static_cast<float>(generatorData.originalSize.y) / 2.f;
            m_dialogSpots.push_back(DialogSpot { PhysicsObject::ConvertToVector2(body->GetPosition()), halfHeight,
                                                 std::max(halfWidth, halfHeight) + INTERACT_MARGIN, generatorData.userData.getDialogId() });
        }

        // Enemies and coins are synced over the network. They are generated in the same order on
        // every peer (same map + generators), so a per-spawn counter yields matching ids everywhere.
        if (name == "bat" || name == "snake" || name == "spider" || name == "thing" || name == "ghost" || name == "coin"
            || name == "claude" || name == "claudling" || name == "treant")
            registerNetworkObject(m_userDataManager.getUserData(body));
    }

    uint32_t World::registerNetworkObject(PhysicsObject* obj, uint32_t networkId)
    {
        if (obj == nullptr)
            return 0;

        const uint32_t id = networkId != 0 ? networkId : m_nextNetworkId;
        m_nextNetworkId = std::max(m_nextNetworkId, id + 1);
        obj->setNetworkId(id);
        m_networkObjects[id] = obj;
        if (obj->getBody() != nullptr)
            m_networkObjectSpawns[id] = PhysicsObject::ConvertToVector2(obj->getBody()->GetPosition());
        return id;
    }

    std::vector<uint32_t> World::getActivePlayerIds() const
    {
        std::vector<uint32_t> ids { getLocalPlayerId() };
        if (m_network != nullptr && m_network->isActive())
        {
            for (const auto& [playerId, remote] : m_network->remotePlayers())
                ids.push_back(playerId);
        }
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    uint32_t World::getLocalPlayerId() const
    {
        if (m_network != nullptr && m_network->isActive())
            return m_network->localPlayerId();
        return INetworkManager::HOST_PLAYER_ID;
    }

    std::optional<raylib::Vector2> World::getPlayerPosition(uint32_t playerId) const
    {
        if (playerId == getLocalPlayerId())
        {
            if (m_player == nullptr)
                return std::nullopt;
            return m_player->getPosition();
        }

        if (m_network == nullptr || !m_network->isActive())
            return std::nullopt;

        const std::map<uint32_t, RemotePlayer>& remotes = m_network->remotePlayers();
        const auto found = remotes.find(playerId);
        if (found == remotes.end())
            return std::nullopt;
        return found->second.position;
    }

    bool World::isEnemyAuthority() const
    {
        return m_network == nullptr || !m_network->isActive() || m_network->role() == NetworkRole::Server;
    }

    void World::reportEnemyDamage(const PhysicsObject& enemy, int32_t damage)
    {
        if (m_network != nullptr && m_network->isActive() && enemy.getNetworkId() != 0)
            m_network->notifyLocalEnemyDamaged(enemy.getNetworkId(), damage);
    }

    void World::applyRemoteEnemyDamage()
    {
        for (const EnemyDamageEvent& hit : m_network->takeEnemyDamageEvents())
        {
            const auto found = m_networkObjects.find(hit.objectId);
            if (found == m_networkObjects.end() || found->second->shouldBeDestroyed())
                continue;

            PhysicsObject* enemy = found->second;
            enemy->onNetworkDamage(hit.damage, hit.attackerPlayerId);
            // Enemies that die on the spot; the boss dies later and credits the kill itself.
            if (enemy->shouldBeDestroyed())
                creditRemoteKill(*enemy, hit.attackerPlayerId);
        }
    }

    void World::creditRemoteKill(const PhysicsObject& enemy, uint32_t killerPlayerId)
    {
        const uint32_t id = enemy.getNetworkId();
        if (m_network == nullptr || !m_network->isActive() || id == 0 || m_reportedDestroyed.count(id) > 0)
            return;

        // Drop the object from the registry now, since it is freed before the next prune pass.
        m_reportedDestroyed.insert(id);
        m_network->notifyEnemyKilledByRemote(id, killerPlayerId);
        m_networkObjects.erase(id);
        ++m_networkRemovedEnemyCount;
    }

    void World::spawnBombRing(raylib::Vector2 center, int32_t count, float ringRadius, float slideDistance, float minFuseTime, float maxFuseTime)
    {
        if (!isEnemyAuthority() || count <= 0)
            return;

        // With linear damping d a body launched at speed v slides about v / d before stopping.
        const float speed = Bomb::LINEAR_DAMPING * slideDistance / PhysicsObject::PPM;
        for (int32_t i = 0; i < count; ++i)
        {
            const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(count);
            const float dirX = std::cos(angle);
            const float dirY = std::sin(angle);
            const raylib::Vector2 position { center.x + dirX * ringRadius, center.y + dirY * ringRadius };
            const float fuse = minFuseTime + (maxFuseTime - minFuseTime) * static_cast<float>(GetRandomValue(0, 10000)) / 10000.f;
            m_pendingBombSpawns.push_back(BombSpawnEvent { position, raylib::Vector2 { dirX * speed, dirY * speed }, fuse });
        }
    }

    void World::spawnHostClaudlings()
    {
        for (const ClaudlingSpawnEvent& spawn : m_network->takeClaudlingSpawns())
        {
            // Never create the same Claudling twice (e.g. a replay overlapping a live spawn).
            if (m_networkObjects.count(spawn.objectId) > 0 || m_reportedDestroyed.count(spawn.objectId) > 0)
                continue;
            doSpawnClaudling(PendingClaudlingSpawn { spawn.position, spawn.followSpeed, spawn.knockback, spawn.targetPlayerId, spawn.objectId });
        }
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

    std::vector<const RemotePlayer*> World::getRemotePartyMembers() const
    {
        std::vector<const RemotePlayer*> members;
        if (m_network == nullptr || !m_network->isActive())
            return members;

        for (const auto& [playerId, remote] : m_network->remotePlayers())
            members.push_back(&remote);
        return members;
    }

    std::vector<MinimapMarker> World::getEnemyMarkers() const
    {
        std::vector<MinimapMarker> markers;
        for (const auto& [id, obj] : m_networkObjects)
        {
            if (obj->getUserData()->getObjectType() != ObjectType::Enemy || obj->shouldBeDestroyed() || obj->getBody() == nullptr)
                continue;
            markers.push_back(MinimapMarker { PhysicsObject::ConvertToVector2(obj->getBody()->GetPosition()), raylib::Color { 255, 40, 40 }, false, true });
        }
        return markers;
    }

    std::vector<MinimapMarker> World::getNpcMarkers() const
    {
        std::vector<MinimapMarker> markers;
        markers.reserve(m_dialogSpots.size());
        for (const DialogSpot& spot : m_dialogSpots)
            markers.push_back(MinimapMarker { spot.position, raylib::Color { 255, 215, 0 }, false, false, true });
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
        {
            NetworkPlayerStats stats {};
            stats.lives = m_player->getLives();
            stats.score = m_player->getScore();
            stats.kills = m_player->getEnemiesKilled();
            stats.level = m_player->getLevel();
            stats.mana = m_player->getMana();
            stats.maxMana = m_player->getMaxMana();
            stats.energy = m_player->getEnergy();
            stats.maxEnergy = m_player->getMaxEnergy();
            m_network->setLocalPlayerStats(stats);
        }

        // 1. Apply authoritative removals from the network (objects others destroyed).
        for (uint32_t id : m_network->removedObjectIds())
        {
            auto found = m_networkObjects.find(id);
            if (found != m_networkObjects.end())
            {
                const bool isEnemy = found->second->getUserData()->getObjectType() == ObjectType::Enemy;
                // In netplay the host kills the boss for everyone, so this is where each peer sees it die.
                if (found->second == m_boss)
                    m_soundManager.playSound(SoundName::EnemyDie);
                found->second->destroy();
                m_reportedDestroyed.insert(id);
                m_networkObjects.erase(found);
                if (isEnemy)
                {
                    // The host destroys bosses itself; if our hit killed it, the kill is still ours.
                    if (m_network->isKillCreditedToLocalPlayer(id) && m_player != nullptr)
                        ++m_player->getUserData()->enemiesKilled;
                    else
                        ++m_networkRemovedEnemyCount;
                }
            }
        }

        // 2. Client: create the host's Claudlings, then snap enemies to the host's authoritative
        //    state. The client still runs each enemy's own AI, whose decisions drift from the
        //    host's, so facing (and a boss's state, target and lives) is overwritten too.
        if (m_network->role() == NetworkRole::Client)
        {
            spawnHostClaudlings();
            for (const BombSpawnEvent& bomb : m_network->takeBombSpawns())
                doSpawnBomb(bomb);
            for (const RootSpawnEvent& root : m_network->takeRootSpawns())
                doSpawnTreantRoot(root);
            for (const auto& [id, state] : m_network->enemyStates())
            {
                auto found = m_networkObjects.find(id);
                if (found != m_networkObjects.end())
                    found->second->applyNetworkState(state);
            }
        }

        // 3. Server: apply clients' boss hits, then broadcast enemy states (throttled to ~20 Hz).
        if (m_network->role() == NetworkRole::Server)
        {
            applyRemoteEnemyDamage();
            if (m_enemyStateThrottle == 0)
            {
                for (const auto& [id, obj] : m_networkObjects)
                {
                    if (obj->getUserData()->getObjectType() == ObjectType::Enemy)
                        m_network->publishEnemyState(id, obj->getNetworkState());
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
        // However the boss died (local hit, host removal, debug kill), never keep a pointer to it
        // past this point: the HUD reads its lives every frame.
        if (m_boss != nullptr && m_boss->shouldBeDestroyed())
            m_boss = nullptr;

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

    bool World::tickPlayerLifeRegen(int32_t playerIndex, float timeDelta)
    {
        if (m_playerStats.count(playerIndex) == 0)
            return false;

        PlayerStats& stats = m_playerStats[playerIndex];
        const TalentEffects& fx = stats.computedEffects;
        if (fx.lifeRegenIntervalSeconds <= 0.f || stats.lives <= 0 || stats.lives >= fx.lifeRegenCap)
        {
            stats.lifeRegenTimer = 0.f;
            return false;
        }

        stats.lifeRegenTimer += timeDelta;
        if (stats.lifeRegenTimer < fx.lifeRegenIntervalSeconds)
            return false;

        stats.lifeRegenTimer = 0.f;
        ++stats.lives;
        return true;
    }

    void World::spawnClaudlings(raylib::Vector2 position, float followSpeed, float knockback)
    {
        if (!isEnemyAuthority())
            return;

        // Fan them out side by side so they don't start as one overlapping blob.
        constexpr float SPAWN_SPACING = 36.f;
        const std::vector<uint32_t> playerIds = getActivePlayerIds();
        const float firstOffset = -SPAWN_SPACING * static_cast<float>(playerIds.size() - 1) / 2.f;
        for (size_t i = 0; i < playerIds.size(); ++i)
        {
            const raylib::Vector2 spawnPosition { position.x + firstOffset + SPAWN_SPACING * static_cast<float>(i), position.y };
            m_pendingClaudlingSpawns.push_back(PendingClaudlingSpawn { spawnPosition, followSpeed, knockback, playerIds[i] });
        }
    }

    void World::spawnBombs(raylib::Vector2 origin, int32_t bombsPerPlayer, float minFuseTime, float maxFuseTime)
    {
        if (!isEnemyAuthority())
            return;

        constexpr float MAX_SPREAD_DEGREES = 25.f; ///< Max angle between a bomb's path and the line to its player.
        constexpr float MIN_REACH = 0.8f;          ///< Shortest throw, as a fraction of the distance to the player.
        constexpr float MAX_REACH = 2.f;           ///< Longest throw: well past the player, so they roll through them.
        const auto randomBetween = [](float min, float max)
        {
            constexpr int32_t STEPS = 10000;
            return min + (max - min) * static_cast<float>(GetRandomValue(0, STEPS)) / static_cast<float>(STEPS);
        };

        for (const uint32_t playerId : getActivePlayerIds())
        {
            const std::optional<raylib::Vector2> target = getPlayerPosition(playerId);
            if (!target.has_value())
                continue;

            const float dx = target->x - origin.x;
            const float dy = target->y - origin.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            // Someone standing right on top of the boss: throw in a random direction instead.
            const float baseAngle = distance > 1.f ? std::atan2(dy, dx) : randomBetween(0.f, 2.f * std::numbers::pi_v<float>);
            const float distanceInMetres = std::max(distance, 48.f) / PhysicsObject::PPM;

            for (int32_t i = 0; i < bombsPerPlayer; ++i)
            {
                const float angle = baseAngle + randomBetween(-MAX_SPREAD_DEGREES, MAX_SPREAD_DEGREES) * std::numbers::pi_v<float> / 180.f;
                // With linear damping d a body launched at speed v slides about v / d before stopping,
                // so this makes the bomb come to rest at the chosen fraction of the distance.
                const float speed = Bomb::LINEAR_DAMPING * distanceInMetres * randomBetween(MIN_REACH, MAX_REACH);
                const raylib::Vector2 velocity { std::cos(angle) * speed, std::sin(angle) * speed };
                m_pendingBombSpawns.push_back(BombSpawnEvent { origin, velocity, randomBetween(minFuseTime, maxFuseTime) });
            }
        }
    }

    void World::damageLocalPlayerWithin(const raylib::Vector2& center, float radius)
    {
        if (m_player == nullptr)
            return;

        const raylib::Vector2 playerCenter = m_player->getVisualCenter();
        const float dx = playerCenter.x - center.x;
        const float dy = playerCenter.y - center.y;
        if (dx * dx + dy * dy <= radius * radius)
            m_player->takeHit();
    }

    void World::flushPendingSpawns()
    {
        const bool isHosting = m_network != nullptr && m_network->isActive() && m_network->role() == NetworkRole::Server;
        for (const PendingClaudlingSpawn& spawn : m_pendingClaudlingSpawns)
        {
            const uint32_t networkId = doSpawnClaudling(spawn);
            if (isHosting && networkId != 0)
                m_network->publishClaudlingSpawned(ClaudlingSpawnEvent { networkId, spawn.targetPlayerId, spawn.position, spawn.followSpeed, spawn.knockback });
        }
        m_pendingClaudlingSpawns.clear();

        for (const BombSpawnEvent& bomb : m_pendingBombSpawns)
        {
            doSpawnBomb(bomb);
            if (isHosting)
                m_network->publishBombSpawned(bomb);
        }
        m_pendingBombSpawns.clear();

        for (const RootSpawnEvent& root : m_pendingRootSpawns)
        {
            doSpawnTreantRoot(root);
            if (isHosting)
                m_network->publishRootSpawned(root);
        }
        m_pendingRootSpawns.clear();
    }

    void World::spawnTreantRoot(const raylib::Vector2& origin, const raylib::Vector2& target, float flightTime, float spikeDelay)
    {
        // Clients get the host's roots over the network so every peer sees the same ones.
        if (!isEnemyAuthority())
            return;

        m_pendingRootSpawns.push_back(RootSpawnEvent { origin, target, flightTime, spikeDelay });
    }

    namespace
    {
        /*! Reports whether a point lies inside a solid (non-sensor, static) fixture such as a wall or tree. */
        class SolidPointCallback : public b2QueryCallback
        {
            public:
            explicit SolidPointCallback(const b2Vec2& point) : m_point { point } {}

            bool ReportFixture(b2Fixture* fixture) override
            {
                if (!fixture->IsSensor() && fixture->GetBody()->GetType() == b2_staticBody && fixture->TestPoint(m_point))
                {
                    m_isSolid = true;
                    return false; // One hit is enough.
                }
                return true;
            }

            [[nodiscard]] bool isSolid() const { return m_isSolid; }

            private:
            b2Vec2 m_point;
            bool m_isSolid { false };
        };
    } // namespace

    bool World::isSolidAt(const raylib::Vector2& point) const
    {
        const b2Vec2 position = PhysicsObject::ConvertToB2Vec2(point);
        constexpr float HALF_EXTENT = 0.01f; ///< Metres; just enough for the AABB query to find fixtures at the point.
        SolidPointCallback callback { position };
        m_world.QueryAABB(&callback, b2AABB { position - b2Vec2 { HALF_EXTENT, HALF_EXTENT }, position + b2Vec2 { HALF_EXTENT, HALF_EXTENT } });
        return callback.isSolid();
    }

    std::vector<raylib::Vector2> World::getPlayerCenters() const
    {
        std::vector<raylib::Vector2> centers;
        if (m_player != nullptr)
            centers.push_back(m_player->getVisualCenter());

        if (m_network != nullptr && m_network->isActive())
        {
            for (const auto& [playerId, remote] : m_network->remotePlayers())
            {
                // Same formula as Player::getVisualCenter(), so remote players are aimed at like the local one.
                centers.push_back({ remote.position.x, remote.position.y - m_playerSpriteOrigin.y + m_playerSpriteHeight / 2.f });
            }
        }
        return centers;
    }

    void World::spawnTreantRootsAcrossMap(const raylib::Vector2& origin, int32_t count, float minFlightTime, float maxFlightTime, float minSpikeDelay, float maxSpikeDelay)
    {
        if (count <= 0 || m_map == nullptr)
            return;

        const auto randomBetween = [](float min, float max)
        {
            constexpr int32_t STEPS = 10000;
            return min + (max - min) * static_cast<float>(GetRandomValue(0, STEPS)) / static_cast<float>(STEPS);
        };

        const float mapWidth = static_cast<float>(m_map->getSize().x * m_map->getTileSize().x);
        const float mapHeight = static_cast<float>(m_map->getSize().y * m_map->getTileSize().y);
        if (mapWidth <= 0.f || mapHeight <= 0.f)
            return;

        // Split the map into a grid with at least one cell per root and drop one root in each, in random
        // order. Pure random placement clumps; this covers the whole map.
        const int32_t columns = std::max(1, static_cast<int32_t>(std::ceil(std::sqrt(static_cast<float>(count) * mapWidth / mapHeight))));
        const int32_t rows = std::max(1, (count + columns - 1) / columns);
        const float cellWidth = mapWidth / static_cast<float>(columns);
        const float cellHeight = mapHeight / static_cast<float>(rows);

        std::vector<int32_t> cells(static_cast<size_t>(columns * rows));
        for (int32_t i = 0; i < columns * rows; ++i)
            cells[static_cast<size_t>(i)] = i;
        for (int32_t i = static_cast<int32_t>(cells.size()) - 1; i > 0; --i)
            std::swap(cells[static_cast<size_t>(i)], cells[static_cast<size_t>(GetRandomValue(0, i))]);

        constexpr int32_t ATTEMPTS_PER_CELL = 8;
        for (int32_t i = 0; i < count; ++i)
        {
            const int32_t cell = cells[static_cast<size_t>(i)];
            const float cellX = static_cast<float>(cell % columns) * cellWidth;
            const float cellY = static_cast<float>(cell / columns) * cellHeight;

            for (int32_t attempt = 0; attempt < ATTEMPTS_PER_CELL; ++attempt)
            {
                const raylib::Vector2 target { cellX + randomBetween(0.f, cellWidth), cellY + randomBetween(0.f, cellHeight) };
                if (isSolidAt(target))
                    continue;

                spawnTreantRoot(origin, target, randomBetween(minFlightTime, maxFlightTime), randomBetween(minSpikeDelay, maxSpikeDelay));
                break;
            }
        }
    }

    void World::doSpawnTreantRoot(const RootSpawnEvent& root)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = PhysicsObject::ConvertToB2Vec2(root.origin);
        bodyDef.fixedRotation = true;
        b2Body* body = m_world.CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = TreantRoot::HIT_RADIUS / PhysicsObject::PPM; // Matches the damage check, so "show hitboxes" is honest.
        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f;
        fd.isSensor = true; // Flies over trees and walls; only the spike hurts.
        body->CreateFixture(&fd);

        // Fly in a straight line so it lands on the target exactly when the flight ends.
        const float flightTime = std::max(root.flightTime, 0.01f);
        body->SetLinearVelocity(PhysicsObject::ConvertToB2Vec2({ (root.target.x - root.origin.x) / flightTime, (root.target.y - root.origin.y) / flightTime }));

        const UserData userData { ObjectType::Trap, { 0.f, 0.f }, "", "", "treant_root" };
        PhysicsObject* physicsObject = createObject<TreantRoot>(body, userData, root.target, flightTime, root.spikeDelay, this);
        if (physicsObject == nullptr)
        {
            m_world.DestroyBody(body);
            return;
        }
        m_userDataManager.addUserData(body, physicsObject);
    }

    void World::doSpawnBomb(const BombSpawnEvent& bomb)
    {
        raylib::Texture* tex = m_textures.get(TextureName::EnemyBomb);
        raylib::Texture* explosionTex = m_textures.get(TextureName::EnemyBombExplosion);
        if (tex == nullptr)
            return;

        b2Body* body = Bomb::CreateBody(m_world, bomb);

        const UserData userData { ObjectType::Trap, { 0.f, 0.f }, "", "", "bomb" };
        PhysicsObject* physicsObject = createObject<Bomb>(body, tex, explosionTex, userData, bomb.fuseTime, this);
        if (physicsObject == nullptr)
        {
            m_world.DestroyBody(body);
            return;
        }
        m_userDataManager.addUserData(body, physicsObject);
    }

    uint32_t World::doSpawnClaudling(const PendingClaudlingSpawn& spawn)
    {
        const raylib::Vector2& position = spawn.position;
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
        {
            m_world.DestroyBody(body);
            return 0;
        }

        const raylib::Rectangle drawingRect { 0.f, 0.f, SIZE.x, SIZE.y };
        const UserData userData { ObjectType::Enemy, { 0.f, 0.f }, "", "", "claudling" };

        PhysicsObject* physicsObject = createObject<Claudling>(m_animationManager, m_mapper, m_soundManager, body, SIZE, SIZE, drawingRect, tex, userData, spawn.followSpeed,
                                                               spawn.knockback, this, spawn.targetPlayerId);

        if (physicsObject == nullptr)
            return 0;

        m_userDataManager.addUserData(body, physicsObject);
        ++m_numberOfEnemies;
        return registerNetworkObject(m_userDataManager.getUserData(body), spawn.networkId);
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
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.lives, generatorData.followSpeed, generatorData.knockback, generatorData.idleTime, this, m_paletteSwap,
                    ClaudeBombConfig { generatorData.bombMinTime, generatorData.bombMaxTime, generatorData.bombSpawnIdleStateTime, generatorData.bombSpawnCount });
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
                    generatorData.followSpeed, generatorData.knockback, m_player, this, m_tintShader,
                    TreantRootConfig { generatorData.rootCount, generatorData.enragedRootCount, generatorData.deathRootCount,
                                       SpikeDelayRange { generatorData.rootMinSpikeDelay, generatorData.rootMaxSpikeDelay } });
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
                    m_animationManager, m_mapper, m_soundManager, body, raylib::Vector2((float)generatorData.size.x, (float)generatorData.size.y), spriteSize, r, tex,
                    generatorData.userData, generatorData.followSpeed, generatorData.knockback, this, std::nullopt);
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
