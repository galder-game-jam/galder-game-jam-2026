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
            m_hudOptions.hudAlpha   = static_cast<float>(hud["alpha"].number_value());
            if (hud["imguiScale"].is_number())
                m_hudOptions.imguiScale = static_cast<float>(hud["imguiScale"].number_value());
            if (hud["showMinimap"].is_bool())
                m_hudOptions.showMinimap = hud["showMinimap"].bool_value();
            if (hud["showQuestLog"].is_bool())
                m_hudOptions.showQuestLog = hud["showQuestLog"].bool_value();
        }

        const json11::Json& kb = root["keybindings"];
        if (kb.is_object())
        {
            auto readKey = [&](const char* name, KeyboardKey fallback) -> KeyboardKey
            {
                if (kb[name].is_number())
                    return static_cast<KeyboardKey>(static_cast<int16_t>(kb[name].int_value()));
                return fallback;
            };
            m_keybindingOptions.moveUp    = readKey("moveUp",    KeyboardKey::W);
            m_keybindingOptions.moveDown  = readKey("moveDown",  KeyboardKey::S);
            m_keybindingOptions.moveLeft  = readKey("moveLeft",  KeyboardKey::A);
            m_keybindingOptions.moveRight = readKey("moveRight", KeyboardKey::D);
            m_keybindingOptions.attack         = readKey("attack",         KeyboardKey::C);
            m_keybindingOptions.respawn        = readKey("respawn",        KeyboardKey::Q);
            m_keybindingOptions.toggleQuestLog = readKey("toggleQuestLog", KeyboardKey::L);
            m_keybindingOptions.toggleMinimap  = readKey("toggleMinimap",  KeyboardKey::M);
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
                { "alpha",        static_cast<double>(m_hudOptions.hudAlpha) },
                { "imguiScale",   static_cast<double>(m_hudOptions.imguiScale) },
                { "showMinimap",  m_hudOptions.showMinimap },
                { "showQuestLog", m_hudOptions.showQuestLog }
            }},
            { "keybindings", json11::Json::object {
                { "moveUp",    static_cast<int>(m_keybindingOptions.moveUp) },
                { "moveDown",  static_cast<int>(m_keybindingOptions.moveDown) },
                { "moveLeft",  static_cast<int>(m_keybindingOptions.moveLeft) },
                { "moveRight", static_cast<int>(m_keybindingOptions.moveRight) },
                { "attack",         static_cast<int>(m_keybindingOptions.attack) },
                { "respawn",        static_cast<int>(m_keybindingOptions.respawn) },
                { "toggleQuestLog", static_cast<int>(m_keybindingOptions.toggleQuestLog) },
                { "toggleMinimap",  static_cast<int>(m_keybindingOptions.toggleMinimap) }
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
