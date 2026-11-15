//
// Created by robin on 29.10.22.
//

#include "WorldManager.h"
#include "../enums/SoundName.h"
#include "../game/Hud.h"
#include "../system/MainMenuScreen.h"
#include "../system/NetplayScreen.h"
#include "../system/OptionsScreen.h"
#include "../system/InGameMenuScreen.h"
#include "../system/TalentScreen.h"
#include "ServerInfoMenu.hpp"
#include <cstdlib>
#include <optional>

#ifdef GAME_DEV_DEBUG
    #include "DebugMenu.hpp"
#endif

namespace ggj
{
    void WorldManager::startMusicForStage(int32_t stageIndex)
    {
        MusicName track;
        switch (stageIndex)
        {
            case 1: // Forest level #1
                track = MusicName::Forest1;
                break;
            case 2: // Dungeon level #1
                track = MusicName::Dungeon1;
                break;
            case 3: // Claude Boss level
                track = MusicName::BossBattle1;
                break;
            case 4: // Dungeon level #2
                track = MusicName::Dungeon2;
                break;
            case 5: // Dungeon level #3
                track = MusicName::Dungeon2;
                break;
            case 6: // Forest level #2
                track = MusicName::Forest2;
                break;
            case 7: // Treant Boss level
                track = MusicName::BossBattle2;
                break;
            case 8: // Butcher Boss level
                track = MusicName::BossBattle3;
                break;
            case 99999: // Final diamond map
                track = MusicName::Victory1;
                break;
            default:
                m_logger.debug(fmt::format("Could not set music track for level {0}. Fallback to Forest2", stageIndex));
                track = MusicName::Glitch1;
                break;
        }

        m_soundManager.playMusic(track);
    }

    bool WorldManager::initialize()
    {
        bool projectExists = m_resources.exists(ResourceName::MapProject);
        if (projectExists)
        {
            m_logger.debug("FOUND project!");

            std::string fontPath = m_resources.getResource(ResourceName::FontDroidSansBold);
            m_font.Load(fontPath, 64, nullptr, 250);
            m_font.GetTexture().GenMipmaps();
            // Trilinear blends between mip levels, so the 64 px glyphs stay smooth at every size they are drawn at.
            m_font.GetTexture().SetFilter(TEXTURE_FILTER_TRILINEAR);
            m_debugManager.setFont(&m_font);
            m_hud.setFont(&m_font);
            m_dialog.setFont(&m_font);

            m_mainMenuScreen.initialize();
            m_mainMenuScreen.setFont(&m_font);
            m_netplayScreen.initialize();
            m_optionsScreen.initialize();
            m_inGameMenuScreen.setFont(&m_font);
            m_characterSelectScreen.initialize();
            m_characterSelectScreen.setFont(&m_font);
            m_talentScreen.setFont(&m_font);

            m_regularFont.Load(m_resources.getResource(ResourceName::FontDroidSans), 64, nullptr, 250);
            m_regularFont.GetTexture().GenMipmaps();
            m_regularFont.GetTexture().SetFilter(TEXTURE_FILTER_TRILINEAR);
            m_chatBox.setFonts(&m_font, &m_regularFont);
        }

        return projectExists;
    }

    bool WorldManager::parseProject()
    {
        m_project = std::make_unique<tson::Project>();

        if (!m_project->parse(fs::path(m_resources.getResource(ResourceName::MapProject))))
        {
            m_logger.error("Failed parsing project...");
            return false;
        }
        m_logger.debug("Project parsed SUCCESSFULLY");
        tson::Tileson t { m_project.get() };
        for (const auto& folder : m_project->getFolders())
        {
            for (const auto& file : folder.getFiles())
            {
                if (file.filename().string().ends_with(".json")) //Make sure it's a map!
                {
                    //Stage stage = getStageByFilename(file.filename().generic_string());
                    int32_t stage = getStageIndexByFilename(file.filename().generic_string());
                    if (stage == -1)
                    {
                        m_logger.warning(fmt::format("The filename '{0}' is not recognized as a stage. File is ignored...", file.filename().generic_string()));
                        continue;
                    }
                    std::unique_ptr<tson::Map> map = t.parse(fs::path(folder.getPath() / file.filename()));

                    if (map->getStatus() == tson::ParseStatus::OK)
                    {
                        if (m_startupIndex < 0 && !file.filename().string().starts_with("final") && file.filename().string().starts_with("map"))
                        {
                            bool isStartup = map->get<bool>("is_startup_map");
                            if (isStartup)
                            {
                                m_startupIndex = getStageIndexByFilename(file.filename().string());
                            }
                        }
                        m_worlds[stage] =
                            std::make_unique<World>(m_logger, m_resources, m_mapper, m_textures, m_window, m_input, m_collisionManager, m_debugManager, m_animationManager,
                                                    m_userDataManager, m_soundManager, std::move(map), m_hud, m_dialog, m_optionsManager, m_managers.paletteSwap, m_managers.tint);
                        m_worlds[stage]->setPickedCharacter(m_pickedCharacter);
                        m_worlds[stage]->setPlayerColors(m_pickedColors);
                        m_worlds[stage]->setFont(&m_font);
                        m_worlds[stage]->initialize();
#ifdef GAME_DEV_DEBUG
                        m_worlds[stage]->setDebugState(&m_managers.debugState);
#endif
                        m_logger.debug(fmt::format("World added: {0}", file.filename().generic_string()));
                    }
                    else
                        m_logger.error(fmt::format("Error while parsing map: {0}", file.filename().generic_string()));
                }
            }
        }
#ifdef GAME_DEV_DEBUG
        // Feed the debug menu's map buttons with every loaded map (std::map keeps ids sorted).
        m_managers.debugState.availableStages.clear();
        for (const auto& [stageIndex, world] : m_worlds)
            m_managers.debugState.availableStages.push_back(stageIndex);
#endif
        m_currentStageIndex = m_startupIndex;
        if (m_currentStageIndex < 0)
            m_currentStageIndex = 1;

        //Expect something!
        if (m_worlds.empty())
        {
            m_logger.error("No maps found! initialization of WorldManager failed!");
            return false;
        }
        //        if (m_worlds.count(m_currentStageIndex) > 0)
        //            m_currentStage = Stage::Stage1;

        m_logger.debug("WorldManager initialized SUCCESSFULLY!");

        return true;
    }

    void WorldManager::update(float timeDelta)
    {
        m_soundManager.update(timeDelta);
        updateActiveScreen(timeDelta);
        updateNetplay(timeDelta);
    }

    void WorldManager::updateActiveScreen(float timeDelta)
    {
        if (m_connectionLostPopup.isOpen())
        {
            // The popup itself is drawn by ImGui (drawImGui); keyboard confirms too.
            if (m_input.keyPressed(KeyboardKey::Enter) || m_input.keyPressed(KeyboardKey::Space) || m_input.keyPressed(KeyboardKey::Escape))
                m_connectionLostPopup.dismiss();
            if (m_connectionLostPopup.isDismissed())
                returnToMainMenu();
            return;
        }

        if (m_mainMenuScreenActive)
        {
            m_soundManager.playMusic(MusicName::MainMenu1);
            m_mainMenuScreen.update(timeDelta);
            if (m_mainMenuScreen.isExitGameSelected())
                exit(0);
            if (m_mainMenuScreen.isSinglePlayerSelected())
            {
                m_isNetplay = false;
                m_mainMenuScreenActive = false;
                m_characterSelectScreenActive = true;
            }
            else if (m_mainMenuScreen.isNetplaySelected())
            {
                m_mainMenuScreen.reset();
                m_netplayScreen.reset();
                m_mainMenuScreenActive = false;
                m_netplayScreenActive = true;
                m_network.startDiscovery(); // listen for LAN hosts while browsing
            }
            else if (m_mainMenuScreen.isOptionsSelected())
            {
                m_mainMenuScreen.reset();
                m_optionsScreen.reset();
                m_mainMenuScreenActive = false;
                m_optionsScreenActive = true;
            }
            return;
        }

        if (m_optionsScreenActive)
        {
            if (m_optionsScreen.isBack())
            {
                m_optionsScreen.reset();
                m_optionsScreenActive = false;
                if (m_optionsReturnToInGame)
                {
                    m_optionsReturnToInGame = false;
                    m_inGameMenuActive = true;
                    m_inGameMenuScreen.reset();
                }
                else
                {
                    m_mainMenuScreenActive = true;
                    m_mainMenuScreen.reset();
                }
            }
            return;
        }

        if (m_netplayScreenActive)
        {
            // The dialog itself is driven by ImGui (drawImGui); here we react to its outcome.
            m_network.pollDiscovery(timeDelta);
            m_netplayScreen.setDiscoveredServers(m_network.discoveredServers());

            if (m_netplayScreen.isBack())
            {
                m_network.stopDiscovery();
                m_netplayScreen.reset();
                m_netplayScreenActive = false;
                m_mainMenuScreenActive = true;
            }
            else if (m_netplayScreen.isConfirmed())
            {
                m_network.stopDiscovery();
                m_isNetplay = true;
                m_netplayScreenActive = false;
                m_characterSelectScreenActive = true;
            }
            return;
        }

        if (m_characterSelectScreenActive)
        {
            m_characterSelectScreen.update(timeDelta);
            if (m_characterSelectScreen.isCharacterSelected())
            {
                m_pickedCharacter = m_characterSelectScreen.getPickedCharacter();
                m_pickedColors = m_characterSelectScreen.getPickedColors();
                m_talentScreen.setPickedCharacter(m_pickedCharacter);
                parseProject();
                m_hud.initialize();
                m_hud.setPickedCharacter(m_pickedCharacter, m_pickedColors);
                m_hud.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                m_hud.setWorld(m_worlds[m_currentStageIndex].get());
                m_hud.setMinimap(m_worlds[m_currentStageIndex]->getMinimap());
                m_dialog.initialize();
                m_dialog.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                m_dialog.setWorld(m_worlds[m_currentStageIndex].get());
                m_characterSelectScreenActive = false;

                if (m_isNetplay)
                {
                    const std::string playerName = m_netplayScreen.getPlayerName();
                    const raylib::Color nameColor = m_netplayScreen.getNameColor();
                    m_hud.setPlayerName(playerName);
                    m_hud.setPlayerNameColor(nameColor);
                    if (m_netplayScreen.isHost())
                        m_network.startHost(m_netplayScreen.getPort(), playerName, nameColor, m_netplayScreen.getServerName(), m_pickedCharacter, m_pickedColors);
                    else
                        m_network.startClient(m_netplayScreen.getIpAddress(), m_netplayScreen.getPort(), playerName, nameColor, m_pickedCharacter, m_pickedColors);

                    m_worlds[m_currentStageIndex]->setNetworkManager(&m_network);
                    m_network.setCurrentStage(m_currentStageIndex);
                }

                startMusicForStage(m_currentStageIndex);
#ifdef GAME_DEV_DEBUG
                m_managers.debugState.isGameActive = true;
#endif
            }
        }
        else
        {
            if (m_inGameMenuActive)
            {
                m_inGameMenuScreen.update(timeDelta);
                if (m_inGameMenuScreen.isResumeSelected())
                {
                    m_inGameMenuActive = false;
                    m_inGameMenuScreen.reset();
                }
                else if (m_inGameMenuScreen.isOptionsSelected())
                {
                    m_inGameMenuScreen.reset();
                    m_optionsScreen.reset();
                    m_optionsScreenActive = true;
                    m_optionsReturnToInGame = true;
                }
                else if (m_inGameMenuScreen.isExitSelected())
                {
                    returnToMainMenu();
                    m_inGameMenuActive = false;
                    m_inGameMenuScreen.reset();
                    m_mainMenuScreenActive = true;
                    m_mainMenuScreen.reset();
                    m_characterSelectScreen.reset();
                    m_soundManager.stopMusic();
#ifdef GAME_DEV_DEBUG
                    m_managers.debugState.isGameActive = false;
#endif
                }
                return;
            }

            if (m_talentScreenActive)
            {
                m_talentScreen.update(timeDelta);
                if (m_talentScreen.isDone())
                {
                    m_talentScreenActive = false;
                    if (m_worlds.count(m_currentStageIndex) > 0)
                    {
                        m_worlds[m_currentStageIndex]->updatePlayerTalentState(
                            0,
                            m_managers.persistentState.talentState,
                            m_managers.persistentState.talentPoints);
                    }
                    m_talentScreen.reset();
                }
                return;
            }

            // While a dialog is open, Escape closes the dialog (handled by Dialog) instead of opening the menu.
            const bool isPlayerInDialog = m_worlds.count(m_currentStageIndex) > 0 && m_worlds[m_currentStageIndex]->getPlayer() != nullptr
                && m_worlds[m_currentStageIndex]->getPlayer()->isInDialog();

            // Before anything else reads the keyboard: while open, the chat captures it. It also drains the typed
            // characters every frame, so it must sit out while the kick reason box (ImGui) needs them.
            if (m_isNetplay && !m_isKickReasonCapturingKeyboard)
                m_chatBox.handleInput();

            if (m_input.keyPressed(KeyboardKey::Escape) && !isPlayerInDialog)
            {
                m_inGameMenuActive = true;
                m_inGameMenuScreen.reset();
                return;
            }

            if (m_input.keyPressed(m_optionsManager.getKeybindingOptions().toggleTalentTree) && m_currentStageIndex != NO_MAP_ID
                && m_worlds.count(m_currentStageIndex) > 0)
            {
                // Sync current talent points from the active world before opening
                m_managers.persistentState.talentPoints = m_worlds[m_currentStageIndex]->getPlayerTalentPoints(0);
                m_managers.persistentState.talentState  = m_worlds[m_currentStageIndex]->getPlayerTalentState(0);

                IPlayer* player = m_worlds[m_currentStageIndex]->getPlayer();
                if (player != nullptr)
                    m_talentScreen.setCurrentPlayerLevel(player->getLevel());

                m_talentScreenActive = true;
                m_talentScreen.reset();
                return;
            }

#ifdef GAME_DEV_DEBUG
            if (m_managers.debugState.requestedStage != -1)
            {
                const int32_t requestedStage = m_managers.debugState.requestedStage;
                m_managers.debugState.requestedStage = -1;
                changeStage(requestedStage);
            }

            if (m_managers.debugState.gainXp)
            {
                m_managers.debugState.gainXp = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                    m_worlds[m_currentStageIndex]->addPlayerXp(0, 100);
            }

            if (m_managers.debugState.gainLevel)
            {
                m_managers.debugState.gainLevel = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                {
                    const int xpNeeded = m_worlds[m_currentStageIndex]->getPlayerXpToNextLevel(0);
                    m_worlds[m_currentStageIndex]->addPlayerXp(0, xpNeeded);
                }
            }

            if (m_managers.debugState.killAllEnemies)
            {
                m_managers.debugState.killAllEnemies = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                    m_worlds[m_currentStageIndex]->killAllEnemies();
            }

            if (m_managers.debugState.awardCoin)
            {
                m_managers.debugState.awardCoin = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                {
                    World* world = m_worlds[m_currentStageIndex].get();
                    world->addPlayerScore(0, 1);
                    m_soundManager.playSound(SoundName::CoinPickup);
                    IPlayer* player = world->getPlayer();
                    if (player != nullptr)
                        world->spawnCoinFloatingText(player->getPosition());
                }
            }

            if (m_managers.debugState.unlockAllTalents)
            {
                m_managers.debugState.unlockAllTalents = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                {
                    World* world = m_worlds[m_currentStageIndex].get();
                    const TalentClass talentClass = (m_pickedCharacter == PlayerPickedCharacter::Wizard)
                        ? TalentClass::Wizard
                        : TalentClass::Rogue;
                    TalentState talentState = world->getPlayerTalentState(0);
                    for (const TalentDef& talent : ALL_TALENTS)
                    {
                        if (talent.characterClass == talentClass)
                            talentState.unlockTalent(talent.id);
                    }
                    world->updatePlayerTalentState(0, talentState, world->getPlayerTalentPoints(0));
                    m_managers.persistentState.talentState = talentState;
                }
            }

            if (m_managers.debugState.gainLife)
            {
                m_managers.debugState.gainLife = false;
                if (m_worlds.count(m_currentStageIndex) > 0)
                {
                    World* world = m_worlds[m_currentStageIndex].get();
                    world->addPlayerLives(0, 1);
                    IPlayer* player = world->getPlayer();
                    if (player != nullptr)
                        world->spawnFloatingText(player->getPosition());
                    m_soundManager.playSound(SoundName::ExtraLife);
                }
            }

            if (m_managers.debugState.loseLife)
            {
                m_managers.debugState.loseLife = false;
                if (m_worlds.count(m_currentStageIndex) > 0
                    && m_worlds[m_currentStageIndex]->getPlayerLives(0) > 1)
                {
                    m_worlds[m_currentStageIndex]->addPlayerLives(0, -1);
                }
            }
#endif

            if (m_currentStageIndex == NO_MAP_ID)
                return;

            if (m_worlds.count(m_currentStageIndex) > 0)
            {
                World* world = m_worlds[m_currentStageIndex].get();
                world->update(timeDelta);

                if (world->isLevelCleared())
                {
                    world->setLevelCleared(false);
                    // Netplay clients don't move on by themselves: they wait for the host's
                    // StageChanged so every peer lands on the same map at the same time.
                    if (!m_isNetplay || m_network.role() != NetworkRole::Client)
                        changeStage(getNextStageIndex());
                }
            }
        }
    }

    void WorldManager::updateNetplay(float timeDelta)
    {
        if (!m_isNetplay || !m_network.isActive() || m_connectionLostPopup.isOpen() || m_worlds.count(m_currentStageIndex) == 0)
            return;

        IPlayer* localPlayer = m_worlds[m_currentStageIndex]->getPlayer();
        if (localPlayer != nullptr)
        {
            m_network.setLocalPlayerPosition(localPlayer->getPosition());
            m_network.setLocalPlayerAnimation(localPlayer->getPlayerState(), localPlayer->isFacingLeft());
        }
        m_network.poll(timeDelta);
        m_chatBox.update(timeDelta);

        if (m_network.wasKicked())
        {
            const std::string& reason = m_network.kickReason();
            showConnectionLostPopup(reason.empty() ? "The host removed you from the server." : "The host removed you from the server.\n\nReason: " + reason, "Kicked");
            return;
        }

        if (m_network.hasLostConnection())
        {
            showConnectionLostPopup(m_network.hasEverConnected() ? "Disconnected from server." : "Could not connect to server.");
            return;
        }

        if (const std::optional<int32_t> stage = m_network.takeRequestedStage(); stage.has_value() && *stage != m_currentStageIndex)
            changeStage(*stage);
    }

    void WorldManager::changeStage(int32_t stageIndex)
    {
        if (m_worlds.count(stageIndex) == 0)
        {
            m_logger.warning(fmt::format("Cannot change to stage {0}: no such map loaded", stageIndex));
            return;
        }

        if (m_worlds.count(m_currentStageIndex) > 0)
        {
            World* prevWorld = m_worlds[m_currentStageIndex].get();
            IPlayer* prevPlayer = prevWorld->getPlayer();
            m_managers.persistentState.lives = prevPlayer->getLives();
            m_managers.persistentState.score = prevPlayer->getScore();
            m_managers.persistentState.level = prevPlayer->getLevel();
            m_managers.persistentState.currentXp = prevPlayer->getCurrentXp();
            m_managers.persistentState.totalXp = prevPlayer->getTotalXp();
            m_managers.persistentState.totalEnemiesKilled = prevPlayer->getEnemiesKilled();
            m_managers.persistentState.talentPoints = prevWorld->getPlayerTalentPoints(0);
            m_managers.persistentState.talentState = prevWorld->getPlayerTalentState(0);
            prevWorld->setLevelCleared(false);
        }

        m_currentStageIndex = stageIndex;
        World* world = m_worlds[m_currentStageIndex].get();
        world->getPlayer()->restoreState(m_managers.persistentState);

        startMusicForStage(m_currentStageIndex);

        m_hud.setPlayer(world->getPlayer());
        m_hud.setWorld(world);
        m_hud.setMinimap(world->getMinimap());

        m_dialog.setPlayer(world->getPlayer());
        m_dialog.setWorld(world);

        if (m_isNetplay)
        {
            world->setNetworkManager(&m_network);
            // Resets per-world sync state (ids restart on every map); the host also tells clients to follow.
            m_network.setCurrentStage(m_currentStageIndex);
        }
    }

    int32_t WorldManager::getNextStageIndex() const
    {
        const int32_t nextStageIndex = m_currentStageIndex + 1;
        return m_worlds.count(nextStageIndex) > 0 ? nextStageIndex : FINAL_MAP_ID;
    }

    void WorldManager::returnToMainMenu()
    {
        if (m_isNetplay)
        {
            // Close the session for real: otherwise the connection stays open in the background and
            // the host keeps a ghost of this player around.
            m_network.shutdown();
            m_isNetplay = false;
        }
        m_chatBox.reset();

        m_connectionLostPopup.close();
        m_inGameMenuActive = false;
        m_inGameMenuScreen.reset();
        m_talentScreenActive = false;
        m_talentScreen.reset();
        m_optionsScreenActive = false;
        m_optionsReturnToInGame = false;
        m_optionsScreen.reset();
        m_mainMenuScreenActive = true;
        m_mainMenuScreen.reset();
        m_soundManager.stopMusic();
#ifdef GAME_DEV_DEBUG
        m_managers.debugState.isGameActive = false;
#endif
    }

    void WorldManager::showConnectionLostPopup(const std::string& message, const std::string& title)
    {
        m_logger.information(message);
        m_network.shutdown();
        m_chatBox.close(); // Give the keyboard back, or the popup can't be dismissed.
        m_inGameMenuActive = false;
        m_talentScreenActive = false;
        m_optionsScreenActive = false;
        m_optionsReturnToInGame = false;
        m_connectionLostPopup.open(title, message);
    }

    void WorldManager::draw()
    {
        if (m_mainMenuScreenActive)
        {
            m_mainMenuScreen.draw();
            return;
        }

        if (m_optionsScreenActive)
        {
            m_optionsScreen.draw();
            return;
        }

        if (m_netplayScreenActive)
        {
            m_netplayScreen.draw();
            return;
        }

        if (m_characterSelectScreenActive)
        {
            ClearBackground(BLACK);
            m_characterSelectScreen.draw();
        }
        else
        {
            if (m_currentStageIndex == -1)
                return;

            if (m_worlds.count(m_currentStageIndex) > 0)
            {
                m_worlds[m_currentStageIndex]->draw();
            }

            // Under the menus, so they still cover it.
            if (m_isNetplay)
                m_chatBox.draw();

            if (m_inGameMenuActive)
            {
                m_inGameMenuScreen.draw();
            }

            if (m_talentScreenActive)
            {
                m_talentScreen.draw();
            }
        }
    }

    void WorldManager::setKickReasonKeyboardCapture(bool isCaptured)
    {
        if (m_isKickReasonCapturingKeyboard == isCaptured)
            return;
        m_isKickReasonCapturingKeyboard = isCaptured;
        if (isCaptured)
            m_chatBox.close(); // An open chat already holds the keyboard; close it so releasing it later can't leave the chat open and uncaptured.
        m_input.setKeyboardCaptured(isCaptured);
    }

    void WorldManager::drawImGui()
    {
        // The kick reason box is only drawn in the host window below; if another screen took over, give the keyboard back.
        if (m_optionsScreenActive || m_netplayScreenActive || m_connectionLostPopup.isOpen() || m_characterSelectScreenActive || !m_isNetplay || m_network.role() != NetworkRole::Server)
            setKickReasonKeyboardCapture(false);

        if (m_optionsScreenActive)
        {
            m_optionsScreen.drawImGui();
            return;
        }

        if (m_netplayScreenActive)
        {
            m_netplayScreen.drawImGui();
            return;
        }

        if (m_connectionLostPopup.isOpen())
        {
            m_connectionLostPopup.draw();
            return;
        }

        if (m_characterSelectScreenActive)
        {
            m_characterSelectScreen.drawImGui();
            return;
        }

        // Host gets an in-game window listing connected players and their stats.
        if (m_isNetplay && m_network.role() == NetworkRole::Server)
        {
            const std::optional<KickRequest> kick = m_serverInfoMenu.draw(m_network.connectedPlayers(),
                                                                          m_network.serverName(),
                                                                          m_network.localIpAddress(),
                                                                          m_network.publicIpAddress());
            // Typing the reason must not walk the host around or open menus.
            setKickReasonKeyboardCapture(m_serverInfoMenu.isKickPopupOpen());
            if (kick.has_value())
                m_network.kickPlayer(kick->playerId, kick->reason);
        }
        else if (m_isNetplay && m_network.role() == NetworkRole::Client
                 && m_worlds.count(m_currentStageIndex) > 0)
        {
            IPlayer* p = m_worlds[m_currentStageIndex]->getPlayer();
            if (p != nullptr)
            {
                ClientInfoMenu clientInfo;
                clientInfo.draw(m_network,
                                m_network.localPlayerName(),
                                p->getVisualCenter(),
                                p->getLives(),
                                p->getScore());
            }
        }
    }

    //    Stage WorldManager::getStageByFilename(const std::string &filename)
    //    {
    //        if (filename == "map3.json") return Stage::Stage1;
    //        if (filename == "map2.json") return Stage::Stage2;
    //        if (filename == "map1.json") return Stage::Stage3;
    //        if (filename == "finalmap.json") return Stage::StageFinal;
    //
    //        return Stage::None;
    //    }

    int32_t WorldManager::getStageIndexByFilename(const std::string& filename) const
    {
        if (filename.starts_with("finalmap.json"))
            return FINAL_MAP_ID;

        if (!filename.starts_with("final") && filename.starts_with("map"))
        {
            try
            {
                std::string mapNoStr = filename.substr(3, filename.length() - 3 - 5); // Minus: "map" and ".json"
                return std::stoi(mapNoStr);
            }
            catch (const std::exception& e)
            {
                return -1;
            }
        }
        return -1;
    }
} // namespace ggj