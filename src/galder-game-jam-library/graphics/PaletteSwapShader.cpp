//
// Created by robin on 13.11.2026.
//

#include "PaletteSwapShader.h"
#include "fmt/format.h"
#include "rlgl.h"
#include <array>

namespace ggj
{
    bool PaletteSwapShader::initialize()
    {
        const std::string path = m_resources.getResource(ResourceName::ShaderPaletteSwap);
        std::unique_ptr<raylib::Shader> shader = std::make_unique<raylib::Shader>(nullptr, path.c_str());
        // raylib falls back to the default shader when compiling fails, which still counts as valid.
        if (!shader->IsValid() || shader->id == rlGetShaderIdDefault())
        {
            m_logger.error(fmt::format("Failed to load palette swap shader. Path: '{0}'", path));
            return false;
        }

        m_fromColorsLocation = shader->GetLocation("fromColors");
        m_toColorsLocation = shader->GetLocation("toColors");
        m_colorCountLocation = shader->GetLocation("colorCount");
        m_shader = std::move(shader);
        return true;
    }

    void PaletteSwapShader::begin(const PaletteSwap& swap)
    {
        if (m_shader == nullptr)
            return;

        std::array<float, PaletteSwap::MAX_COLORS * 3> from {};
        std::array<float, PaletteSwap::MAX_COLORS * 3> to {};
        for (int32_t i = 0; i < swap.count; ++i)
        {
            from[i * 3 + 0] = static_cast<float>(swap.from[i].r) / 255.0f;
            from[i * 3 + 1] = static_cast<float>(swap.from[i].g) / 255.0f;
            from[i * 3 + 2] = static_cast<float>(swap.from[i].b) / 255.0f;
            to[i * 3 + 0] = static_cast<float>(swap.to[i].r) / 255.0f;
            to[i * 3 + 1] = static_cast<float>(swap.to[i].g) / 255.0f;
            to[i * 3 + 2] = static_cast<float>(swap.to[i].b) / 255.0f;
        }

        // Begin first: switching shader flushes what was batched under the previous one, so the new
        // uniform values only apply to what is drawn from here on.
        m_shader->BeginMode();
        m_shader->SetValue(m_fromColorsLocation, from.data(), SHADER_UNIFORM_VEC3, static_cast<int>(PaletteSwap::MAX_COLORS));
        m_shader->SetValue(m_toColorsLocation, to.data(), SHADER_UNIFORM_VEC3, static_cast<int>(PaletteSwap::MAX_COLORS));
        m_shader->SetValue(m_colorCountLocation, &swap.count, SHADER_UNIFORM_INT);
    }

    void PaletteSwapShader::end()
    {
        if (m_shader == nullptr)
            return;

        m_shader->EndMode();
    }
} // namespace ggj
