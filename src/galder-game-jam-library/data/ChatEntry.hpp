//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CHATENTRY_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CHATENTRY_HPP

#include "raylib-cpp.hpp"

#include <string>

namespace ggj
{
    /*!
     * A received chat line, ready for display.
     */
    struct ChatEntry
    {
        std::string name {};             ///< Sender name.
        raylib::Color nameColor { RED }; ///< Sender's name colour, used for the whole line.
        std::string text {};             ///< UTF-8 message text.
        float age { 0.f };               ///< Seconds since it was received; drives the fade-out.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_CHATENTRY_HPP
