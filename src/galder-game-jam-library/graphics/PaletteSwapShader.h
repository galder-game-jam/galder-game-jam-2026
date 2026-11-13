//
// Created by robin on 13.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PALETTESWAPSHADER_H
#define GALDER_GAME_JAM_2026_PALETTESWAPSHADER_H

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../interfaces/graphics/IPaletteSwapShader.h"
#include "raylib-cpp.hpp"
#include <memory>

namespace ggj
{
    /*!
     * raylib implementation of IPaletteSwapShader, backed by shaders/palette_swap.fs.
     */
    class PaletteSwapShader final : public IPaletteSwapShader
    {
        public:
        PaletteSwapShader(ILogger& logger, IResourceManager<ResourceName>& resources) : m_logger { logger }, m_resources { resources } {}

        bool initialize() override;
        void begin(const PaletteSwap& swap) override;
        void end() override;

        private:
        ILogger& m_logger;
        IResourceManager<ResourceName>& m_resources;

        std::unique_ptr<raylib::Shader> m_shader {};
        int m_fromColorsLocation { -1 };
        int m_toColorsLocation { -1 };
        int m_colorCountLocation { -1 };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_PALETTESWAPSHADER_H
