#include "SoundManager.h"
#include "fmt/format.h"

namespace ggj
{
    SoundManager::~SoundManager()
    {
        if (!m_audioReady)
            return;

        stopMusic();
        for (auto& [id, sound] : m_sounds)
            UnloadSound(sound);
        for (auto& [id, music] : m_music)
            UnloadMusicStream(music);
        CloseAudioDevice();
    }

    bool SoundManager::initialize()
    {
        InitAudioDevice();
        if (!IsAudioDeviceReady())
        {
            m_logger.error("SoundManager: audio device could not be opened");
            return false;
        }
        m_audioReady = true;

        bool ok = true;
        ok &= loadSound(SoundName::CoinPickup, ResourceName::SoundCoinPickup);
        ok &= loadSound(SoundName::EnemyDie, ResourceName::SoundEnemyDie);
        ok &= loadSound(SoundName::PortalUse, ResourceName::SoundPortalUse);
        ok &= loadSound(SoundName::PlayerAttack, ResourceName::SoundPlayerAttack);
        ok &= loadSound(SoundName::DialogMumble, ResourceName::SoundDialogMumble);
        ok &= loadSound(SoundName::DialogTalking1, ResourceName::SoundDialogTalking1);
        ok &= loadSound(SoundName::DialogTalking2, ResourceName::SoundDialogTalking2);
        ok &= loadSound(SoundName::DialogTalking3, ResourceName::SoundDialogTalking3);
        ok &= loadSound(SoundName::DialogTalking4, ResourceName::SoundDialogTalking4);
        ok &= loadSound(SoundName::DialogTalking5, ResourceName::SoundDialogTalking5);
        ok &= loadSound(SoundName::DialogTalking6, ResourceName::SoundDialogTalking6);
        ok &= loadSound(SoundName::DialogTalking7, ResourceName::SoundDialogTalking7);
        ok &= loadMusic(MusicName::Level1, ResourceName::MusicLevel1);
        ok &= loadMusic(MusicName::Level2, ResourceName::MusicLevel2);

        return ok;
    }

    void SoundManager::update(float /*timeDelta*/)
    {
        if (m_audioReady && m_currentMusic != MusicName::None)
        {
            if (m_music.count(m_currentMusic) > 0)
                UpdateMusicStream(m_music[m_currentMusic]);
        }
    }

    void SoundManager::playSound(SoundName id)
    {
        if (!m_audioReady || m_sounds.count(id) == 0)
            return;
        PlaySound(m_sounds[id]);
    }

    void SoundManager::playMusic(MusicName id)
    {
        if (!m_audioReady || id == MusicName::None)
            return;
        if (id == m_currentMusic)
            return;

        stopMusic();

        if (m_music.count(id) == 0)
            return;

        m_currentMusic = id;
        PlayMusicStream(m_music[id]);
    }

    void SoundManager::stopMusic()
    {
        if (!m_audioReady || m_currentMusic == MusicName::None)
            return;
        if (m_music.count(m_currentMusic) > 0)
            StopMusicStream(m_music[m_currentMusic]);
        m_currentMusic = MusicName::None;
    }

    bool SoundManager::loadSound(SoundName id, ResourceName resource)
    {
        const std::string path = m_resources.getResource(resource);
        Sound sound = LoadSound(path.c_str());
        if (!IsSoundValid(sound))
        {
            m_logger.error(fmt::format("SoundManager: failed to load sound '{}' from '{}'", (int)id, path));
            return false;
        }
        m_sounds[id] = sound;
        return true;
    }

    bool SoundManager::loadMusic(MusicName id, ResourceName resource)
    {
        const std::string path = m_resources.getResource(resource);
        Music music = LoadMusicStream(path.c_str());
        if (!IsMusicValid(music))
        {
            m_logger.error(fmt::format("SoundManager: failed to load music '{}' from '{}'", (int)id, path));
            return false;
        }
        music.looping = true;
        m_music[id] = music;
        return true;
    }
} // namespace ggj
