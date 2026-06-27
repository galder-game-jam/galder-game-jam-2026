#include "OptionsManager.h"
#include "tileson.hpp"
#include <fstream>
#include <sstream>

namespace ggj
{
    bool OptionsManager::initialize()
    {
        const std::string path = m_resources.getResource(ResourceName::ConfigOptions);
        std::ifstream file(path);
        if (!file.is_open())
        {
            m_logger.error("OptionsManager: could not open '" + path + "', using defaults");
            return false;
        }

        std::ostringstream ss;
        ss << file.rdbuf();

        std::string err;
        json11::Json root = json11::Json::parse(ss.str(), err);
        if (!err.empty() || !root.is_object())
        {
            m_logger.error("OptionsManager: failed to parse options.json: " + err);
            return false;
        }

        const json11::Json& sound = root["sound"];
        if (sound.is_object())
        {
            m_audioOptions.isMusicMuted = sound["musicMuted"].bool_value();
            m_audioOptions.musicVolume  = static_cast<float>(sound["musicVolume"].number_value());
            m_audioOptions.isSoundMuted = sound["soundMuted"].bool_value();
            m_audioOptions.soundVolume  = static_cast<float>(sound["soundVolume"].number_value());
        }

        const json11::Json& hud = root["hud"];
        if (hud.is_object())
        {
            m_hudOptions.hudAlpha = static_cast<float>(hud["alpha"].number_value());
        }

        return true;
    }

    bool OptionsManager::saveAudioOptions()
    {
        const std::string path = m_resources.getResource(ResourceName::ConfigOptions);

        const json11::Json root = json11::Json::object {
            { "sound", json11::Json::object {
                { "musicMuted",  m_audioOptions.isMusicMuted },
                { "musicVolume", static_cast<double>(m_audioOptions.musicVolume) },
                { "soundMuted",  m_audioOptions.isSoundMuted },
                { "soundVolume", static_cast<double>(m_audioOptions.soundVolume) }
            }},
            { "hud", json11::Json::object {
                { "alpha", static_cast<double>(m_hudOptions.hudAlpha) }
            }}
        };

        std::ofstream file(path);
        if (!file.is_open())
        {
            m_logger.error("OptionsManager: could not write '" + path + "'");
            return false;
        }
        file << root.dump();
        return true;
    }
} // namespace ggj
