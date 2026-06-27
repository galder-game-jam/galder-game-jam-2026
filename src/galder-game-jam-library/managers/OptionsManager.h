#ifndef GALDER_GAME_JAM_2026_OPTIONSMANAGER_H
#define GALDER_GAME_JAM_2026_OPTIONSMANAGER_H

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../interfaces/manager/IOptionsManager.h"

namespace ggj
{
    /*!
     * Reads options.json from the content directory and provides the parsed values.
     */
    class OptionsManager : public IOptionsManager
    {
        public:
            OptionsManager(ILogger& logger, IResourceManager<ResourceName>& resources)
                : m_logger { logger }, m_resources { resources }
            {
            }

            bool initialize() override;

            [[nodiscard]] const AudioOptions& getAudioOptions() const override
            {
                return m_audioOptions;
            }

            void setAudioOptions(const AudioOptions& options) override
            {
                m_audioOptions = options;
            }

            bool saveAudioOptions() override;

        private:
            ILogger& m_logger;
            IResourceManager<ResourceName>& m_resources;

            AudioOptions m_audioOptions {};
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_OPTIONSMANAGER_H
