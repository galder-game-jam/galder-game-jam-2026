#include "TintShader.h"
#include "fmt/format.h"
#include "rlgl.h"
#include <algorithm>
#include <array>

namespace ggj
{
    bool TintShader::initialize()
    {
        const std::string path = m_resources.getResource(ResourceName::ShaderTint);
        std::unique_ptr<raylib::Shader> shader = std::make_unique<raylib::Shader>(nullptr, path.c_str());
        // raylib falls back to the default shader when compiling fails, which still counts as valid.
        if (!shader->IsValid() || shader->id == rlGetShaderIdDefault())
        {
            m_logger.error(fmt::format("Failed to load tint shader. Path: '{0}'", path));
            return false;
        }

        m_tintColorLocation = shader->GetLocation("tintColor");
        m_tintStrengthLocation = shader->GetLocation("tintStrength");
        m_shader = std::move(shader);
        return true;
    }

    void TintShader::begin(const ::Color& tint, const float strength)
    {
        if (m_shader == nullptr)
            return;

        const std::array<float, 3> color { static_cast<float>(tint.r) / 255.f, static_cast<float>(tint.g) / 255.f, static_cast<float>(tint.b) / 255.f };
        const float clampedStrength = std::clamp(strength, 0.f, 1.f);

        // Begin first: switching shader flushes what was batched under the previous one, so the new
        // uniform values only apply to what is drawn from here on.
        m_shader->BeginMode();
        m_shader->SetValue(m_tintColorLocation, color.data(), SHADER_UNIFORM_VEC3);
        m_shader->SetValue(m_tintStrengthLocation, &clampedStrength, SHADER_UNIFORM_FLOAT);
    }

    void TintShader::end()
    {
        if (m_shader == nullptr)
            return;

        m_shader->EndMode();
    }
} // namespace ggj
