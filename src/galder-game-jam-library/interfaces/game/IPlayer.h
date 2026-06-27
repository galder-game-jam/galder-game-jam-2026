//
// Created by robin on 26.06.2026.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H
#define GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H

#include "raylib-cpp.hpp"
#include <string>

namespace ggj
{
    class Hitbox;
    class UserData;

    /*!
     * Abstract interface for any player-controlled character.
     *
     * Both Player (Rogue) and Player2 (Wizard) implement this so the rest of
     * the engine (World, Hud, Dialog) can work with either without knowing the
     * concrete type.
     */
    class IPlayer
    {
        public:
            virtual ~IPlayer() = default;

            [[nodiscard]] virtual raylib::Vector2 getPosition() const = 0;
            [[nodiscard]] virtual raylib::Vector2 getVisualCenter() const = 0;
            virtual UserData* getUserData() = 0;
            [[nodiscard]] virtual int getLives() const = 0;
            [[nodiscard]] virtual int getScore() const = 0;
            [[nodiscard]] virtual int getLevel() const = 0;
            [[nodiscard]] virtual int getCurrentXp() const = 0;
            [[nodiscard]] virtual int getTotalXp() const = 0;
            [[nodiscard]] virtual int getXpToNextLevel() const = 0;
            virtual Hitbox* getHitbox() = 0;
            [[nodiscard]] virtual bool cameraShouldFollowPlayer() const = 0;
            virtual void setCameraFollowsPlayer(bool follow) = 0;
            [[nodiscard]] virtual bool hasClearedLevel() const = 0;
            virtual void setHasClearedLevel(bool value) = 0;
            [[nodiscard]] virtual bool isInDialog() const = 0;
            virtual std::string getDialogId() = 0;
            virtual void setHasFinishedDialog() = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H
