#ifndef GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H
#define GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H

#include "../../data/AudioOptions.hpp"
#include "../../data/GraphicsOptions.hpp"
#include "../../data/HudOptions.hpp"
#include "../../data/KeybindingOptions.hpp"

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

            /*!
             * Replaces the in-memory audio options without writing to disk.
             * Call saveAudioOptions() to persist the change.
             * @param options New audio settings to apply.
             */
            virtual void setAudioOptions(const AudioOptions& options) = 0;

            /*!
             * Writes all options (sound and HUD) back to options.json.
             * @return true on success, false if the file could not be written.
             */
            virtual bool saveAudioOptions() = 0;

            /*!
             * Returns the HUD display options parsed from the "hud" object.
             */
            [[nodiscard]] virtual const HudOptions& getHudOptions() const = 0;

            /*!
             * Replaces the in-memory HUD options without writing to disk.
             * Call saveAudioOptions() to persist the change.
             * @param options New HUD settings to apply.
             */
            virtual void setHudOptions(const HudOptions& options) = 0;

            /*!
             * Returns the visual effect options parsed from the "graphics" object.
             */
            [[nodiscard]] virtual const GraphicsOptions& getGraphicsOptions() const = 0;

            /*!
             * Replaces the in-memory graphics options without writing to disk.
             * Call saveAudioOptions() to persist the change.
             * @param options New graphics settings to apply.
             */
            virtual void setGraphicsOptions(const GraphicsOptions& options) = 0;

            /*!
             * Returns the keybinding options parsed from the "keybindings" object.
             */
            [[nodiscard]] virtual const KeybindingOptions& getKeybindingOptions() const = 0;

            /*!
             * Replaces the in-memory keybinding options without writing to disk.
             * Call saveAudioOptions() to persist the change.
             * @param options New keybinding settings to apply.
             */
            virtual void setKeybindingOptions(const KeybindingOptions& options) = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_IOPTIONSMANAGER_H
