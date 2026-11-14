//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_IPLAYERLOCATOR_H
#define GALDER_GAME_JAM_2026_PROJECT_IPLAYERLOCATOR_H

#include "raylib-cpp.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace ggj
{
    /*!
     * Finds every player in the current stage, local or remote, by network player id. Lets enemies
     * chase any player in netplay rather than only the one on the machine they run on.
     */
    class IPlayerLocator
    {
        public:
        virtual ~IPlayerLocator() = default;

        /*!
         * Every player in the session: the local player plus each remote peer. Offline this is
         * just the local player.
         *
         * @return Network player ids, in ascending order
         */
        [[nodiscard]] virtual std::vector<uint32_t> getActivePlayerIds() const = 0;

        /*!
         * Network player id of the player on this machine.
         */
        [[nodiscard]] virtual uint32_t getLocalPlayerId() const = 0;

        /*!
         * Where a player currently is.
         *
         * @param playerId Network player id from getActivePlayerIds()
         * @return Position in pixels, or std::nullopt if that player has left
         */
        [[nodiscard]] virtual std::optional<raylib::Vector2> getPlayerPosition(uint32_t playerId) const = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PROJECT_IPLAYERLOCATOR_H
