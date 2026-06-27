#ifndef GALDER_GAME_JAM_2026_AUDIOOPTIONS_HPP
#define GALDER_GAME_JAM_2026_AUDIOOPTIONS_HPP

namespace ggj
{
    /*!
     * Audio playback settings loaded from the "sound" object in options.json.
     *
     * The user-declared constructor prevents Boost.DI aggregate-injection from
     * silently discarding the member initializers (see CLAUDE.md).
     */
    struct AudioOptions
    {
        AudioOptions() = default;

        bool isMusicMuted { false }; ///< When true, no music is played.
        float musicVolume  { 1.0f }; ///< Master music volume in the range [0.0, 1.0].
        bool isSoundMuted { false }; ///< When true, no sound effects are played.
        float soundVolume  { 1.0f }; ///< Master sound-effect volume in the range [0.0, 1.0].
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_AUDIOOPTIONS_HPP
