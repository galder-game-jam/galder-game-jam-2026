//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_MASKRECOLORSHADER_H
#define GALDER_GAME_JAM_2026_MASKRECOLORSHADER_H

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../interfaces/graphics/IMaskRecolorShader.h"
#include "raylib-cpp.hpp"
#include <memory>

namespace ggj
{
    /*!
     * raylib implementation of IMaskRecolorShader, backed by shaders/mask_recolor.fs.
     */
    class MaskRecolorShader final : public IMaskRecolorShader
    {
        public:
        MaskRecolorShader(ILogger& logger, IResourceManager<ResourceName>& resources) : m_logger { logger }, m_resources { resources } {}

        bool initialize() override;
        void begin(const MaskRecolor& recolor, const ::Texture2D& mask) override;
        void end() override;

        private:
        ILogger& m_logger;
        IResourceManager<ResourceName>& m_resources;

        std::unique_ptr<raylib::Shader> m_shader {};
        int m_maskTextureLocation { -1 };
        int m_baseColorsLocation { -1 };
        int m_targetColorsLocation { -1 };
        int m_slotWeightsLocation { -1 };
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_MASKRECOLORSHADER_H
