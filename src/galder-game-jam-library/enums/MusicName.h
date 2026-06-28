#ifndef GALDER_GAME_JAM_2026_MUSICNAME_H
#define GALDER_GAME_JAM_2026_MUSICNAME_H

#include <cstdint>

namespace ggj
{
    enum class MusicName : uint16_t
    {
        None = 0,
        Forest1 = 1,
        Forest2 = 2,

        Dungeon1 = 10,
        Dungeon2 = 11,

        BossBattle1 = 20,
        BossBattle2 = 21,
        BossBattle3 = 22,
        BossBattle4 = 23,

        MainMenu1 = 30,
        MainMenu2 = 31,

        Glitch1 = 40,
        Glitch2 = 41,

        Victory1 = 50,
        Victory2 = 51,
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_MUSICNAME_H
