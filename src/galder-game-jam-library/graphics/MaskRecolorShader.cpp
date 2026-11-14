//
// Created by robin on 14.11.2026.
//

#include "MaskRecolorShader.h"
#include "fmt/format.h"
#include "rlgl.h"
#include <array>

namespace ggj
{
    namespace
    {
        void writeRgb(std::array<float, MaskRecolor::MAX_SLOTS * 3>& out, const size_t slot, const ::Color& color)
        {
            out[slot * 3 + 0] = static_cast<float>(color.r) / 255.0f;
            out[slot * 3 + 1] = static_cast<float>(color.g) / 255.0f;
            out[slot * 3 + 2] = static_cast<float>(color.b) / 255.0f;
        }
    } // namespace

    bool MaskRecolorShader::initialize()
    {
        const std::string path = m_resources.getResource(ResourceName::ShaderMaskRecolor);
        std::unique_ptr<raylib::Shader> shader = std::make_unique<raylib::Shader>(nullptr, path.c_str());
        // raylib falls back to the default shader when compiling fails, which still counts as valid.
        if (!shader->IsValid() || shader->id == rlGetShaderIdDefault())
        {
            m_logger.error(fmt::format("Failed to load mask recolor shader. Path: '{0}'", path));
            return false;
        }

        m_maskTextureLocation = shader->GetLocation("maskTexture");
        m_baseColorsLocation = shader->GetLocation("baseColors");
        m_targetColorsLocation = shader->GetLocation("targetColors");
        m_slotWeightsLocation = shader->GetLocation("slotWeights");
        m_shader = std::move(shader);
        return true;
    }

    void MaskRecolorShader::begin(const MaskRecolor& recolor, const ::Texture2D& mask)
    {
        if (m_shader == nullptr)
            return;

        std::array<float, MaskRecolor::MAX_SLOTS * 3> base {};
        std::array<float, MaskRecolor::MAX_SLOTS * 3> target {};
        std::array<float, MaskRecolor::MAX_SLOTS> weights {};
        for (size_t i = 0; i < MaskRecolor::MAX_SLOTS; ++i)
        {
            writeRgb(base, i, recolor.base[i]);
            writeRgb(target, i, recolor.target[i]);
            weights[i] = recolor.isRecolored[i] ? 1.0f : 0.0f;
        }

        // Begin first: switching shader flushes what was batched under the previous one, so the new
        // uniform values (and the mask, which raylib only binds until the next batch flush) apply to
        // what is drawn from here on.
        m_shader->BeginMode();
        m_shader->SetValue(m_baseColorsLocation, base.data(), SHADER_UNIFORM_VEC3, static_cast<int>(MaskRecolor::MAX_SLOTS));
        m_shader->SetValue(m_targetColorsLocation, target.data(), SHADER_UNIFORM_VEC3, static_cast<int>(MaskRecolor::MAX_SLOTS));
        m_shader->SetValue(m_slotWeightsLocation, weights.data(), SHADER_UNIFORM_FLOAT, static_cast<int>(MaskRecolor::MAX_SLOTS));
        SetShaderValueTexture(*m_shader, m_maskTextureLocation, mask);
    }

    void MaskRecolorShader::end()
    {
        if (m_shader == nullptr)
            return;

        m_shader->EndMode();
    }
} // namespace ggj
