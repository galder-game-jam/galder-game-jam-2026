//
// Created by Claude on 15.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_SOUNDEVENT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_SOUNDEVENT_HPP

#include "../enums/SoundName.h"
#include <cstdint>

namespace ggj
{
    /*!
     * A sound effect another player triggered, to be replayed locally by the World so everyone
     * hears each other's attacks, hits and level-ups.
     */
    struct SoundEvent
    {
        uint32_t ownerId { 0 }; ///< Network player id of the player who triggered the sound.
        SoundName sound { SoundName::None };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_SOUNDEVENT_HPP
