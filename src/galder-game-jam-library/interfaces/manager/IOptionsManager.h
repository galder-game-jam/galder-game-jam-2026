#ifndef GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H
#define GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H

#include "../../data/AudioOptions.hpp"

namespace ggj
{
    /*!
     * Loads and exposes application options read from options.json at startup.
     */
    class IOptionsManager
    {
        public:
            virtual ~IOptionsManager() noexcept = default;

            /*!
             * Reads options.json and populates all option values.
             * Falls back to defaults if the file is missing or malformed.
             * @return true on success, false if the file could not be parsed.
             */
            virtual bool initialize() = 0;

            /*!
             * Returns the audio options parsed from the "sound" object.
             */
            [[nodiscard]] virtual const AudioOptions& getAudioOptions() const = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H
