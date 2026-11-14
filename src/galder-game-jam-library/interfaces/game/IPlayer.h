//
// Created by robin on 26.06.2026.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H
#define GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H

#include "raylib-cpp.hpp"
#include <string>
#include "../../data/PlayerPersistentState.hpp"
#include "../../enums/PlayerState.h"
#include "../../data/DialogSpot.hpp"

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
            [[nodiscard]] virtual int getEnemiesKilled() const = 0;
            virtual void restoreState(const PlayerPersistentState& state) = 0;
            /*!
             * @return The character's melee hitbox, or nullptr if it has none (e.g. the Wizard)
             */
            virtual Hitbox* getHitbox() = 0;
            [[nodiscard]] virtual bool cameraShouldFollowPlayer() const = 0;
            virtual void setCameraFollowsPlayer(bool follow) = 0;
            [[nodiscard]] virtual bool hasClearedLevel() const = 0;
            virtual void setHasClearedLevel(bool value) = 0;
            [[nodiscard]] virtual bool isInDialog() const = 0;
            virtual std::string getDialogId() = 0;
            virtual void setHasFinishedDialog() = 0;
            /*! @return The NPC close enough to talk to (Space starts the dialog), or nullptr when none is in range. */
            [[nodiscard]] virtual const DialogSpot* getNearbyDialogSpot() const = 0;

            /*! Current animation state. Mirrored to remote peers so they can animate this player. */
            [[nodiscard]] virtual PlayerState getPlayerState() const = 0;
            /*! True when the sprite is drawn flipped (facing left). */
            [[nodiscard]] virtual bool isFacingLeft() const = 0;

            /*!
             * Hurts the player as if an enemy touched them: costs a life (and a point of score)
             * unless the post-hit damage cooldown is still running or the player is already dead.
             */
            virtual void takeHit() = 0;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2025_PROJECT_IPLAYER_H
