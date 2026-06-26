#ifndef GALDER_GAME_JAM_2026_MUSICNAME_H
#define GALDER_GAME_JAM_2026_MUSICNAME_H

#include <cstdint>

namespace ggj
{
    enum class MusicName : uint16_t
    {
        None = 0,
        Level1 = 1, ///< Background track for the first level set.
        Level2 = 2  ///< Background track for the second level set.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_MUSICNAME_H
