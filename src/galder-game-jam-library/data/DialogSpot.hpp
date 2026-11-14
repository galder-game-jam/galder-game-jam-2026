#ifndef GALDER_GAME_JAM_2026_DIALOGSPOT_HPP
#define GALDER_GAME_JAM_2026_DIALOGSPOT_HPP

#include "raylib-cpp.hpp"
#include <string>

namespace ggj
{
    /*!
     * An NPC the player can talk to. Static for the lifetime of a world, so pointers to a spot stay valid.
     */
    struct DialogSpot
    {
        raylib::Vector2 position {}; ///< NPC centre in world pixels
        float halfHeight {};         ///< Half the NPC's height, so the prompt can sit just above its head
        float interactRadius {};     ///< Max distance (player centre to NPC centre) at which the player can start the dialog
        std::string dialogId;        ///< Dialog started when the player interacts
    };
}

#endif // GALDER_GAME_JAM_2026_DIALOGSPOT_HPP
