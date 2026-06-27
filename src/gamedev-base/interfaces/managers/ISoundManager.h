#ifndef GALDER_GAME_JAM_2026_ISOUNDMANAGER_H
#define GALDER_GAME_JAM_2026_ISOUNDMANAGER_H

namespace ggj
{
    enum class SoundName : unsigned short;
    enum class MusicName : unsigned short;

    /*!
     * Manages playback of sound effects and streaming background music.
     */
    class ISoundManager
    {
        public:
            virtual ~ISoundManager() noexcept = default;

            /*!
             * Initialises the audio device and loads all audio assets.
             * @return true on success, false if the audio device could not be opened.
             */
            virtual bool initialize() = 0;

            /*!
             * Must be called every frame to keep streaming music buffers filled.
             * @param timeDelta Seconds since the last frame.
             */
            virtual void update(float timeDelta) = 0;

            /*!
             * Plays a one-shot sound effect.
             * @param id The effect to play.
             */
            virtual void playSound(SoundName id) = 0;

            /*!
             * Starts (or switches to) a looping music track.
             * The previous track, if any, is stopped before the new one begins.
             * @param id The music track to play.
             */
            virtual void playMusic(MusicName id) = 0;

            /*!
             * Stops the currently playing music track, if any.
             */
            virtual void stopMusic() = 0;

            /*!
             * Re-applies the current audio options (volume, mute) to all loaded sounds and
             * music. Call this after modifying options at runtime via IOptionsManager so that
             * volume changes are heard immediately without reloading assets.
             */
            virtual void applyAudioOptions() = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_ISOUNDMANAGER_H
