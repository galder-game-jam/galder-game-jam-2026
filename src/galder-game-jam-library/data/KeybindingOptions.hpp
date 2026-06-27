#ifndef GALDER_GAME_JAM_2026_KEYBINDINGOPTIONS_HPP
#define GALDER_GAME_JAM_2026_KEYBINDINGOPTIONS_HPP

#include "../enums/KeyboardKey.h"

namespace ggj
{
    /*!
     * Player keybinding settings loaded from the "keybindings" object in options.json.
     *
     * The user-declared constructor prevents Boost.DI aggregate-injection from
     * silently discarding the member initializers (see CLAUDE.md).
     */
    struct KeybindingOptions
    {
        KeybindingOptions() = default;

        KeyboardKey moveUp    { KeyboardKey::W }; ///< Key to move the player upward.
        KeyboardKey moveDown  { KeyboardKey::S }; ///< Key to move the player downward.
        KeyboardKey moveLeft  { KeyboardKey::A }; ///< Key to move the player left.
        KeyboardKey moveRight { KeyboardKey::D }; ///< Key to move the player right.
        KeyboardKey attack    { KeyboardKey::C }; ///< Key to trigger an attack.
        KeyboardKey respawn   { KeyboardKey::Q }; ///< Key to respawn after death.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_KEYBINDINGOPTIONS_HPP
