//
// Created by Andre on 27.06.2026.
//

#include "Minimap.h"
#include "tileson.hpp"
#include <algorithm>

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

    void Minimap::draw(const std::vector<MinimapMarker>& markers, float alpha) const
    {
        if (!m_isInitialized)
            return;

        const auto a = static_cast<unsigned char>(alpha * 255.0f);

        const float minimapX = static_cast<float>(SCREEN_WIDTH - MINIMAP_WIDTH - MINIMAP_MARGIN);
        const float minimapY = static_cast<float>(MINIMAP_MARGIN);

        // Full map: srcY=0, viewH=H → pre-adjusted y = H-0-H = 0, height = -H (y-flip convention).
        const Rectangle source { 0.f, 0.f, static_cast<float>(m_mapPixelWidth), -static_cast<float>(m_mapPixelHeight) };
        const Rectangle dest   { minimapX, minimapY, static_cast<float>(MINIMAP_WIDTH), static_cast<float>(MINIMAP_HEIGHT) };

        DrawRectangle(static_cast<int>(minimapX) - 2, static_cast<int>(minimapY) - 2, MINIMAP_WIDTH + 4, MINIMAP_HEIGHT + 4, Color { 0, 0, 0, a });
        DrawTexturePro(m_mapTexture.texture, source, dest, { 0.f, 0.f }, 0.f, Color { 255, 255, 255, a });

        // Enemies and NPCs first, then remote players, then the local player, so players are never hidden underneath other dots.
        for (const int32_t drawPass : { 0, 1, 2 })
        {
            for (const MinimapMarker& marker : markers)
            {
                const int32_t markerPass = (marker.isEnemy || marker.isNpc) ? 0 : (marker.isLocalPlayer ? 2 : 1);
                if (markerPass != drawPass)
                    continue;

                const float dotX = minimapX + (marker.worldPosition.x / static_cast<float>(m_mapPixelWidth)) * static_cast<float>(MINIMAP_WIDTH);
                const float dotY = minimapY + (marker.worldPosition.y / static_cast<float>(m_mapPixelHeight)) * static_cast<float>(MINIMAP_HEIGHT);
                if (marker.isNpc)
                {
                    DrawCircle(static_cast<int>(dotX), static_cast<int>(dotY), 3.f, Color { 0, 0, 0, a });
                    DrawCircle(static_cast<int>(dotX), static_cast<int>(dotY), 2.f, Color { 255, 215, 0, a });
                    continue;
                }
                if (marker.isEnemy)
                {
                    DrawCircle(static_cast<int>(dotX), static_cast<int>(dotY), 2.f, Color { 255, 40, 40, a });
                    continue;
                }
                // A ring keeps dark/map-coloured name colours readable; white marks "you".
                const Color ringColor = marker.isLocalPlayer ? Color { 255, 255, 255, a } : Color { 0, 0, 0, a };
                DrawCircle(static_cast<int>(dotX), static_cast<int>(dotY), 4.f, ringColor);
                DrawCircle(static_cast<int>(dotX), static_cast<int>(dotY), 3.f, Color { marker.color.r, marker.color.g, marker.color.b, a });
            }
        }

        DrawRectangleLines(static_cast<int>(minimapX), static_cast<int>(minimapY), MINIMAP_WIDTH, MINIMAP_HEIGHT, Color { 255, 255, 255, a });
    }
} // namespace ggj
