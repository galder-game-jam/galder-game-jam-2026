//
// Created by robin on 28.10.22.
//

#ifndef GAME_DEV_TEMPLATE_IINPUTMANAGER_H
#define GAME_DEV_TEMPLATE_IINPUTMANAGER_H

#include <vector>
#include <cstdint>

namespace ggj
{
    template <class T>
    class IInputManager
    {
        public:
            /*!
             * Pure virtual class needs virtual destructor so derived classes can call their own destructors
            */
            virtual ~IInputManager() noexcept = default;

            virtual bool keyPressed(T key) = 0;
            virtual bool keysPressed(const std::vector<T> &keys) = 0;
            virtual bool keyPressedCode(int16_t key) = 0;
            virtual bool keysPressedCode(const std::vector<int16_t> &keys) = 0;

            virtual bool keyReleased(T key) = 0;
            virtual bool keysReleased(const std::vector<T> &keys) = 0;
            virtual bool keyReleasedCode(int16_t key) = 0;
            virtual bool keysReleasedCode(const std::vector<int16_t> &keys) = 0;

            virtual bool keyDown(T key) = 0;
            virtual bool keysDown(const std::vector<T> &keys) = 0;
            virtual bool keyDownCode(int16_t key) = 0;
            virtual bool keysDownCode(const std::vector<int16_t> &keys) = 0;

            virtual bool keyUp(T key) = 0;
            virtual bool keysUp(const std::vector<T> &keys) = 0;
            virtual bool keyUpCode(int16_t key) = 0;
            virtual bool keysUpCode(const std::vector<int16_t> &keys) = 0;

            virtual std::vector<int16_t> getScanCodeOfKeysPressed() = 0;

            /*!
             * Routes the keyboard to text entry (e.g. a chat box). While captured, every key query
             * above reports no key pressed / down, so gameplay ignores what the user types.
             *
             * @param isCaptured True to capture the keyboard, false to give it back to the game
             */
            virtual void setKeyboardCaptured(bool isCaptured) = 0;
            [[nodiscard]] virtual bool isKeyboardCaptured() const = 0;

            /*!
             * Text-entry key query: true on the initial press and on OS key repeat. Ignores the capture,
             * since it is meant for whoever holds it.
             *
             * @param key Key to check
             * @return True if the key was pressed or repeated this frame
             */
            virtual bool textKeyPressed(T key) = 0;

            /*!
             * Drains the unicode codepoints typed since the last call. Ignores the capture.
             *
             * @return Typed codepoints in order
             */
            virtual std::vector<int32_t> takeCharsPressed() = 0;
    };
}

#endif //GAME_DEV_TEMPLATE_IINPUTMANAGER_H
