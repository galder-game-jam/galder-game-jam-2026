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
        ok &= loadSound(SoundName::LevelUp, ResourceName::SoundLevelUp);
        ok &= loadSound(SoundName::PlayerHurt, ResourceName::SoundPlayerHurt);
        ok &= loadMusic(MusicName::Forest1, ResourceName::MusicForest1);
        ok &= loadMusic(MusicName::Forest2, ResourceName::MusicForest2);
        ok &= loadMusic(MusicName::Dungeon1, ResourceName::MusicDungeon1);
        ok &= loadMusic(MusicName::Dungeon2, ResourceName::MusicDungeon2);
        ok &= loadMusic(MusicName::BossBattle1, ResourceName::MusicBossBattle1);
        ok &= loadMusic(MusicName::BossBattle2, ResourceName::MusicBossBattle2);
        ok &= loadMusic(MusicName::BossBattle3, ResourceName::MusicBossBattle3);
        ok &= loadMusic(MusicName::BossBattle4, ResourceName::MusicBossBattle4);

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
        if (m_options.getAudioOptions().isSoundMuted)
            return;
        PlaySound(m_sounds[id]);
    }

    void SoundManager::playMusic(MusicName id)
    {
        if (!m_audioReady || id == MusicName::None)
            return;
        m_pendingMusic = id;
        if (m_options.getAudioOptions().isMusicMuted)
            return;
        if (id == m_currentMusic)
            return;

        stopMusic();

        if (m_music.count(id) == 0)
            return;

        m_currentMusic = id;
        PlayMusicStream(m_music[id]);
    }

    void SoundManager::applyAudioOptions()
    {
        if (!m_audioReady)
            return;

        const AudioOptions& opts = m_options.getAudioOptions();

        for (auto& [id, sound] : m_sounds)
            SetSoundVolume(sound, opts.soundVolume);

        for (auto& [id, music] : m_music)
            SetMusicVolume(music, opts.musicVolume);

        if (opts.isMusicMuted && m_currentMusic != MusicName::None)
        {
            stopMusic();
        }
        else if (!opts.isMusicMuted && m_currentMusic == MusicName::None && m_pendingMusic != MusicName::None)
        {
            if (m_music.count(m_pendingMusic) > 0)
            {
                m_currentMusic = m_pendingMusic;
                PlayMusicStream(m_music[m_currentMusic]);
            }
        }
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
        SetSoundVolume(sound, m_options.getAudioOptions().soundVolume);
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
        SetMusicVolume(music, m_options.getAudioOptions().musicVolume);
        m_music[id] = music;
        return true;
    }
} // namespace ggj
