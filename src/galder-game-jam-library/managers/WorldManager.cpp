//
// Created by robin on 29.10.22.
//

#include "WorldManager.h"
#include "../game/Hud.h"

namespace ggj
{
    bool WorldManager::initialize()
    {
        bool projectExists = m_resources.exists(ResourceName::MapProject);
        if (projectExists)
        {
            m_logger.debug("FOUND project!");
            parseProject();
            m_hud.initialize();
            m_hud.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
            m_hud.setWorld(m_worlds[m_currentStageIndex].get());

            m_dialog.initialize();
            m_dialog.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
            m_dialog.setWorld(m_worlds[m_currentStageIndex].get());

            //Load custom font
            std::string fontPath = m_resources.getResource(ResourceName::FontDroidSansBold);
            m_font.Load(fontPath, 64, nullptr, 250);
            m_font.GetTexture().GenMipmaps();
            m_font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);
            m_debugManager.setFont(&m_font);
            m_hud.setFont(&m_font);
            m_dialog.setFont(&m_font);

            m_characterSelectScreen.initialize();
            m_characterSelectScreen.setFont(&m_font);
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
        tson::Tileson t{m_project.get()};
        for (const auto &folder: m_project->getFolders())
        {
            for (const auto &file: folder.getFiles())
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

                        if(m_startupIndex < 0 && !file.filename().string().starts_with("final") && file.filename().string().starts_with("map"))
                        {
                            bool isStartup = map->get<bool>("is_startup_map");
                            if(isStartup)
                            {
                                m_startupIndex = getStageIndexByFilename(file.filename().string());
                            }
                        }
                        m_worlds[stage] = std::make_unique<World>(m_logger, m_resources, m_mapper, m_textures, m_window, m_input, m_collisionManager, m_debugManager, m_animationManager, m_userDataManager, std::move(map), m_hud, m_dialog);
                        m_worlds[stage]->initialize();
                        m_logger.debug(fmt::format("World added: {0}", file.filename().generic_string()));
                    }
                    else
                        m_logger.error(fmt::format("Error while parsing map: {0}", file.filename().generic_string()));
                }
            }
        }
        m_currentStageIndex = m_startupIndex;
        if(m_currentStageIndex < 0)
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
        if (m_characterSelectScreenActive)
        {
            m_characterSelectScreen.update(timeDelta);
            if (m_characterSelectScreen.isCharacterSelected())
            {
                //Do something
                m_characterSelectScreenActive = false;
            }
        }
        else
        {
#ifdef GAME_DEV_DEBUG
            if (m_input.keyPressed(ggj::KeyboardKey::F1)) m_currentStageIndex = 1;
            if (m_input.keyPressed(ggj::KeyboardKey::F2)) m_currentStageIndex = 2;
            if (m_input.keyPressed(ggj::KeyboardKey::F3)) m_currentStageIndex = FINAL_MAP_ID;
            if (m_input.keyPressed(ggj::KeyboardKey::F1) || m_input.keyPressed(ggj::KeyboardKey::F2) || m_input.keyPressed(ggj::KeyboardKey::F3))
            {
                m_hud.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                m_hud.setWorld(m_worlds[m_currentStageIndex].get());

                m_dialog.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                m_dialog.setWorld(m_worlds[m_currentStageIndex].get());
            }
#endif

            if (m_currentStageIndex == NO_MAP_ID)
                return;

            if (m_worlds.count(m_currentStageIndex) > 0)
            {
                m_worlds[m_currentStageIndex]->update(timeDelta);
                if(m_worlds[m_currentStageIndex]->isLevelCleared())
                {
                    m_worlds[m_currentStageIndex]->setLevelCleared(false);
                    ++m_currentStageIndex;

                    if(m_worlds.count(m_currentStageIndex) == 0)
                        m_currentStageIndex = FINAL_MAP_ID;

                    m_hud.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                    m_hud.setWorld(m_worlds[m_currentStageIndex].get());

                    m_dialog.setPlayer(m_worlds[m_currentStageIndex]->getPlayer());
                    m_dialog.setWorld(m_worlds[m_currentStageIndex].get());
                }
            }
        }
    }

    void WorldManager::draw()
    {
        if (m_characterSelectScreenActive)
        {
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

    int32_t WorldManager::getStageIndexByFilename(const std::string &filename) const
    {
        if(filename.starts_with("finalmap.json"))
            return FINAL_MAP_ID;

        if(!filename.starts_with("final") && filename.starts_with("map"))
        {
            try
            {
                std::string mapNoStr = filename.substr(3, filename.length() - 3 - 5); // Minus: "map" and ".json"
                return std::stoi(mapNoStr);
            }
            catch(const std::exception &e)
            {
                return -1;
            }
        }
        return -1;
    }
}