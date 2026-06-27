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
        KeyboardKey attack    { KeyboardKey::Space }; ///< Key to trigger an attack.
        KeyboardKey respawn   { KeyboardKey::Q }; ///< Key to respawn after death.
    };

    /*! Returns a short display name for a keyboard key. */
    inline const char* keyName(KeyboardKey k)
    {
        switch (k)
        {
            case KeyboardKey::A: return "A";
            case KeyboardKey::B: return "B";
            case KeyboardKey::C: return "C";
            case KeyboardKey::D: return "D";
            case KeyboardKey::E: return "E";
            case KeyboardKey::F: return "F";
            case KeyboardKey::G: return "G";
            case KeyboardKey::H: return "H";
            case KeyboardKey::I: return "I";
            case KeyboardKey::J: return "J";
            case KeyboardKey::K: return "K";
            case KeyboardKey::L: return "L";
            case KeyboardKey::M: return "M";
            case KeyboardKey::N: return "N";
            case KeyboardKey::O: return "O";
            case KeyboardKey::P: return "P";
            case KeyboardKey::Q: return "Q";
            case KeyboardKey::R: return "R";
            case KeyboardKey::S: return "S";
            case KeyboardKey::T: return "T";
            case KeyboardKey::U: return "U";
            case KeyboardKey::V: return "V";
            case KeyboardKey::W: return "W";
            case KeyboardKey::X: return "X";
            case KeyboardKey::Y: return "Y";
            case KeyboardKey::Z: return "Z";
            case KeyboardKey::Space:        return "Space";
            case KeyboardKey::Enter:        return "Enter";
            case KeyboardKey::Escape:       return "Escape";
            case KeyboardKey::Tab:          return "Tab";
            case KeyboardKey::Backspace:    return "Backspace";
            case KeyboardKey::Up:           return "Up";
            case KeyboardKey::Down:         return "Down";
            case KeyboardKey::Left:         return "Left";
            case KeyboardKey::Right:        return "Right";
            case KeyboardKey::LeftShift:    return "L.Shift";
            case KeyboardKey::RightShift:   return "R.Shift";
            case KeyboardKey::LeftControl:  return "L.Ctrl";
            case KeyboardKey::RightControl: return "R.Ctrl";
            case KeyboardKey::LeftAlt:      return "L.Alt";
            case KeyboardKey::RightAlt:     return "R.Alt";
            case KeyboardKey::Zero:  return "0";
            case KeyboardKey::One:   return "1";
            case KeyboardKey::Two:   return "2";
            case KeyboardKey::Three: return "3";
            case KeyboardKey::Four:  return "4";
            case KeyboardKey::Five:  return "5";
            case KeyboardKey::Six:   return "6";
            case KeyboardKey::Seven: return "7";
            case KeyboardKey::Eight: return "8";
            case KeyboardKey::Nine:  return "9";
            default: return "?";
        }
    }
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_KEYBINDINGOPTIONS_HPP
