//
// Created by Andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_MINIMAP_H
#define GALDER_GAME_JAM_2026_MINIMAP_H

#include "raylib-cpp.hpp"
#include "../system/WorldLayer.h"
#include <map>
#include <cstdint>
#include <vector>

namespace tson { class Map; }

namespace ggj
{
    /*! A player or enemy dot on the minimap. */
    struct MinimapMarker
    {
        raylib::Vector2 worldPosition {}; ///< Visual centre of the player in world pixels.
        raylib::Color color { RED };      ///< The player's chosen name colour.
        bool isLocalPlayer { false };     ///< Drawn on top with a white ring so you can find yourself.
        bool isEnemy { false };           ///< Drawn as a small dot underneath the player dots.
    };

    /*!
     * Pre-renders all tile layers to an offscreen texture once on initialize,
     * then draws a scaled window of that texture with a dot for every player each frame.
     */
    class Minimap
    {
        public:
            Minimap() = default;
            ~Minimap();

            void initialize(tson::Map& map, std::map<int32_t, WorldLayer>& layers);
            /*!
             * Draws the map and a dot per player and enemy.
             *
             * @param markers Players and enemies to mark; enemies are drawn first and the local player last so it stays on top
             * @param alpha HUD opacity in 0..1
             */
            void draw(const std::vector<MinimapMarker>& markers, float alpha = 1.0f) const;

        private:
            RenderTexture2D m_mapTexture {};
            int m_mapPixelWidth  { 0 };
            int m_mapPixelHeight { 0 };
            bool m_isInitialized { false };

            static constexpr int MINIMAP_MARGIN { 10 };
            static constexpr int MINIMAP_WIDTH  { 200 };
            static constexpr int MINIMAP_HEIGHT { 150 };
            static constexpr int SCREEN_WIDTH   { 800 };
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_MINIMAP_H
