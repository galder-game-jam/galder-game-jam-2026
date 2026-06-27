//
// Created by Andre on 27.06.2026.
//

#include "Minimap.h"
#include "tileson.hpp"
#include <algorithm>
#include <cmath>

namespace ggj
{
    Minimap::~Minimap()
    {
        if (m_isInitialized)
            UnloadRenderTexture(m_mapTexture);
    }

    void Minimap::initialize(tson::Map& map, std::map<int32_t, WorldLayer>& layers)
    {
        tson::Vector2i tileCount = map.getSize();
        tson::Vector2i tileSize  = map.getTileSize();

        m_mapPixelWidth  = tileCount.x * tileSize.x;
        m_mapPixelHeight = tileCount.y * tileSize.y;

        m_mapTexture = LoadRenderTexture(m_mapPixelWidth, m_mapPixelHeight);

        BeginTextureMode(m_mapTexture);
        ClearBackground({ 0, 0, 0, 0 });
        for (auto& [idx, layer] : layers)
        {
            if (layer.isTileLayer())
                layer.draw();
        }
        EndTextureMode();

        m_isInitialized = true;
    }

    void Minimap::draw(raylib::Vector2 playerWorldPos) const
    {
        if (!m_isInitialized)
            return;

        const float minimapX = static_cast<float>(SCREEN_WIDTH - MINIMAP_WIDTH - MINIMAP_MARGIN);
        const float minimapY = static_cast<float>(MINIMAP_MARGIN);

        const float srcX = std::clamp(playerWorldPos.x - WORLD_VIEW_WIDTH  / 2.0f, 0.0f, static_cast<float>(m_mapPixelWidth  - WORLD_VIEW_WIDTH));
        const float srcY = std::clamp(playerWorldPos.y - WORLD_VIEW_HEIGHT / 2.0f, 0.0f, static_cast<float>(m_mapPixelHeight - WORLD_VIEW_HEIGHT));

        // RenderTexture2D is stored Y-flipped in OpenGL; negative height corrects orientation
        Rectangle source { srcX, static_cast<float>(m_mapPixelHeight) - srcY, static_cast<float>(WORLD_VIEW_WIDTH), -static_cast<float>(WORLD_VIEW_HEIGHT) };
        Rectangle dest   { minimapX, minimapY, static_cast<float>(MINIMAP_WIDTH), static_cast<float>(MINIMAP_HEIGHT) };

        DrawRectangle(static_cast<int>(minimapX) - 2, static_cast<int>(minimapY) - 2, MINIMAP_WIDTH + 4, MINIMAP_HEIGHT + 4, BLACK);
        DrawTexturePro(m_mapTexture.texture, source, dest, { 0.f, 0.f }, 0.f, WHITE);

        DrawCircle(static_cast<int>(minimapX) + MINIMAP_WIDTH  / 2,
                   static_cast<int>(minimapY) + MINIMAP_HEIGHT / 2,
                   3.f, RED);

        DrawRectangleLines(static_cast<int>(minimapX), static_cast<int>(minimapY), MINIMAP_WIDTH, MINIMAP_HEIGHT, WHITE);
    }
} // namespace ggj
