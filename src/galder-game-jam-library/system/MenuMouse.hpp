#ifndef GGJ_MENUMOUSE_HPP
#define GGJ_MENUMOUSE_HPP

#include <algorithm>
#include "raylib-cpp.hpp"

namespace ggj
{
    /*!
     * Converts the current OS mouse position into the fixed virtual (render texture) coordinate
     * space the menus are laid out in. GameManager letterboxes the render texture into the
     * resizable window, so raw window coordinates have to be un-scaled and un-offset first.
     *
     * @param virtualWidth Width of the virtual screen the menus are drawn on
     * @param virtualHeight Height of the virtual screen the menus are drawn on
     * @return Mouse position in virtual coordinates (may lie outside the virtual screen when over the letterbox bars)
     */
    [[nodiscard]] inline raylib::Vector2 getVirtualMousePosition(const float virtualWidth, const float virtualHeight)
    {
        const float windowWidth = static_cast<float>(GetScreenWidth());
        const float windowHeight = static_cast<float>(GetScreenHeight());
        const float scale = std::min(windowWidth / virtualWidth, windowHeight / virtualHeight);
        const float offsetX = (windowWidth - virtualWidth * scale) * 0.5f;
        const float offsetY = (windowHeight - virtualHeight * scale) * 0.5f;

        const Vector2 mouse = GetMousePosition();
        return raylib::Vector2 { (mouse.x - offsetX) / scale, (mouse.y - offsetY) / scale };
    }
} // namespace ggj

#endif // GGJ_MENUMOUSE_HPP
