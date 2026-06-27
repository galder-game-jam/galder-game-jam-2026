#ifndef GALDER_GAME_JAM_2026_SOUNDMANAGER_H
#define GALDER_GAME_JAM_2026_SOUNDMANAGER_H

#include <map>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../enums/SoundName.h"
#include "../enums/MusicName.h"
#include "../interfaces/manager/IOptionsManager.h"

namespace ggj
{
    class SoundManager : public ISoundManager
    {
        public:
            SoundManager(ILogger& logger, IResourceManager<ResourceName>& resources, IOptionsManager& options)
                : m_logger { logger }, m_resources { resources }, m_options { options }
            {
            }

            ~SoundManager() override;

            bool initialize() override;
            void update(float timeDelta) override;
            void playSound(SoundName id) override;
            void playMusic(MusicName id) override;
            void stopMusic() override;
            void applyAudioOptions() override;

        private:
            bool loadSound(SoundName id, ResourceName resource);
            bool loadMusic(MusicName id, ResourceName resource);

            ILogger& m_logger;
            IResourceManager<ResourceName>& m_resources;
            IOptionsManager& m_options;

            std::map<SoundName, Sound> m_sounds;
            std::map<MusicName, Music> m_music;

            MusicName m_currentMusic { MusicName::None };
            MusicName m_pendingMusic { MusicName::None }; ///< Track requested via playMusic(), regardless of mute state.
            bool m_audioReady { false };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_SOUNDMANAGER_H
