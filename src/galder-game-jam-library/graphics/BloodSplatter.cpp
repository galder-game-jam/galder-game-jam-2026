#include "BloodSplatter.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ggj
{
    namespace
    {
        constexpr float PI_F = 3.14159265358979f;
        constexpr float SPRAY_HALF_ANGLE = 1.0f;     ///< Radians either side of the blow direction that most droplets fall in.
        constexpr float OMNI_DROPLET_CHANCE = 0.25f; ///< Share of droplets that ignore the blow direction (the "splash").

        [[nodiscard]] int32_t clampAmount(int32_t bloodAmount)
        {
            return std::clamp(bloodAmount, GraphicsOptions::MIN_BLOOD_AMOUNT, GraphicsOptions::MAX_BLOOD_AMOUNT);
        }
    } // namespace

    int32_t BloodSplatter::dropletCountFor(int32_t bloodAmount, float strength)
    {
        const float amount = static_cast<float>(clampAmount(bloodAmount));
        // Quadratic on purpose: the low settings stay tasteful, the top one is a gorefest.
        const float count = std::max(strength, 0.f) * (3.f + amount * amount * 0.55f);
        return std::clamp(static_cast<int32_t>(std::lround(count)), 1, MAX_DROPLETS_PER_BURST);
    }

    float BloodSplatter::stainLifetimeFor(int32_t bloodAmount)
    {
        return 15.f + static_cast<float>(clampAmount(bloodAmount)) * 4.f;
    }

    float BloodSplatter::randomFloat(float min, float max)
    {
        return std::uniform_real_distribution<float> { min, max }(m_random);
    }

    void BloodSplatter::emitBlood(const raylib::Vector2& position, const raylib::Vector2& direction, float strength)
    {
        const int32_t bloodAmount = clampAmount(m_options.getGraphicsOptions().bloodAmount);
        const int32_t count = dropletCountFor(bloodAmount, strength);

        const float directionLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        const bool hasDirection = directionLength > 0.001f;
        const float baseAngle = hasDirection ? std::atan2(direction.y, direction.x) : 0.f;
        const float speedScale = (0.7f + static_cast<float>(bloodAmount) * 0.06f) * std::clamp(0.8f + strength * 0.2f, 0.8f, 1.6f);

        constexpr std::array<Color, 4> SHADES { { { 200, 20, 20, 255 }, { 170, 10, 10, 255 }, { 140, 0, 0, 255 }, { 110, 0, 5, 255 } } };

        for (int32_t i = 0; i < count && m_droplets.size() < MAX_DROPLETS; ++i)
        {
            const bool isOmni = !hasDirection || randomFloat(0.f, 1.f) < OMNI_DROPLET_CHANCE;
            const float angle = isOmni ? randomFloat(0.f, 2.f * PI_F) : baseAngle + randomFloat(-SPRAY_HALF_ANGLE, SPRAY_HALF_ANGLE);
            const float speed = randomFloat(50.f, 260.f) * speedScale * (isOmni ? 0.6f : 1.f);

            Droplet droplet;
            droplet.position = raylib::Vector2 { position.x + randomFloat(-3.f, 3.f), position.y + randomFloat(-3.f, 3.f) };
            droplet.velocity = raylib::Vector2 { std::cos(angle) * speed, std::sin(angle) * speed };
            droplet.timeLeft = randomFloat(0.2f, 0.55f);
            droplet.size = randomFloat(1.2f, 2.0f + static_cast<float>(bloodAmount) * 0.12f);
            droplet.color = SHADES[static_cast<size_t>(randomFloat(0.f, static_cast<float>(SHADES.size()) - 0.001f))];
            m_droplets.push_back(droplet);
        }

        // A pool right under the wound so even the lowest setting leaves a mark.
        Stain pool;
        pool.position = position;
        pool.radius = (3.f + static_cast<float>(bloodAmount) * 0.9f) * std::clamp(0.7f + strength * 0.3f, 0.7f, 1.8f);
        pool.lifetime = stainLifetimeFor(bloodAmount);
        pool.color = SHADES[2];
        m_stains.push_back(pool);
        while (m_stains.size() > MAX_STAINS)
            m_stains.pop_front();
    }

    void BloodSplatter::land(const Droplet& droplet)
    {
        Stain stain;
        stain.position = droplet.position;
        stain.radius = droplet.size * randomFloat(0.9f, 1.8f);
        stain.lifetime = stainLifetimeFor(clampAmount(m_options.getGraphicsOptions().bloodAmount)) * randomFloat(0.7f, 1.f);
        stain.color = droplet.color;
        stain.color.r = static_cast<uint8_t>(stain.color.r * 0.8f); // Dried blood is darker than blood in the air.
        m_stains.push_back(stain);
        while (m_stains.size() > MAX_STAINS)
            m_stains.pop_front();
    }

    void BloodSplatter::update(float timeDelta)
    {
        const float drag = std::exp(-DROPLET_DRAG * timeDelta);
        for (Droplet& droplet : m_droplets)
        {
            droplet.position.x += droplet.velocity.x * timeDelta;
            droplet.position.y += droplet.velocity.y * timeDelta;
            droplet.velocity.x *= drag;
            droplet.velocity.y *= drag;
            droplet.timeLeft -= timeDelta;
        }

        for (const Droplet& droplet : m_droplets)
        {
            if (droplet.timeLeft <= 0.f)
                land(droplet);
        }
        std::erase_if(m_droplets, [](const Droplet& droplet) { return droplet.timeLeft <= 0.f; });

        for (Stain& stain : m_stains)
            stain.age += timeDelta;
        // Stains are pushed oldest first but lifetimes vary, so expired ones can sit anywhere.
        std::erase_if(m_stains, [](const Stain& stain) { return stain.age >= stain.lifetime; });
    }

    void BloodSplatter::drawStains() const
    {
        for (const Stain& stain : m_stains)
        {
            const float timeLeft = stain.lifetime - stain.age;
            Color color = stain.color;
            if (timeLeft < FADE_SECONDS)
                color.a = static_cast<uint8_t>(255.f * std::max(timeLeft, 0.f) / FADE_SECONDS);
            DrawCircleV(stain.position, stain.radius, color);
        }
    }

    void BloodSplatter::drawDroplets() const
    {
        for (const Droplet& droplet : m_droplets)
            DrawCircleV(droplet.position, droplet.size, droplet.color);
    }

    void BloodSplatter::clear()
    {
        m_droplets.clear();
        m_stains.clear();
    }
} // namespace ggj
