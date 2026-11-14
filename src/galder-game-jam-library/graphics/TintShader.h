#ifndef GGJ_TINTSHADER_H
#define GGJ_TINTSHADER_H

#include "../../gamedev-base/GameDevBase.h"
#include "../enums/ResourceName.h"
#include "../interfaces/graphics/ITintShader.h"
#include "raylib-cpp.hpp"
#include <memory>

namespace ggj
{
    /*!
     * raylib implementation of ITintShader, backed by shaders/tint.fs.
     */
    class TintShader final : public ITintShader
    {
        public:
        TintShader(ILogger& logger, IResourceManager<ResourceName>& resources) : m_logger { logger }, m_resources { resources } {}

        bool initialize() override;
        void begin(const ::Color& tint, float strength) override;
        void end() override;

        private:
        ILogger& m_logger;
        IResourceManager<ResourceName>& m_resources;

        std::unique_ptr<raylib::Shader> m_shader {};
        int m_tintColorLocation { -1 };
        int m_tintStrengthLocation { -1 };
    };
} // namespace ggj

#endif // GGJ_TINTSHADER_H
