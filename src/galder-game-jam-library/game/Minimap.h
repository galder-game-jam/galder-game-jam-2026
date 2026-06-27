//
// Created by Andre on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_MINIMAP_H
#define GALDER_GAME_JAM_2026_MINIMAP_H

#include "raylib-cpp.hpp"
#include "../system/WorldLayer.h"
#include <map>
#include <cstdint>

namespace tson { class Map; }

namespace ggj
{
    /*!
     * Pre-renders all tile layers to an offscreen texture once on initialize,
     * then draws a scaled window of that texture centred on the player each frame.
     */
    class Minimap
    {
        public:
            Minimap() = default;
            ~Minimap();

            void initialize(tson::Map& map, std::map<int32_t, WorldLayer>& layers);
            void draw(raylib::Vector2 playerWorldPos) const;

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
