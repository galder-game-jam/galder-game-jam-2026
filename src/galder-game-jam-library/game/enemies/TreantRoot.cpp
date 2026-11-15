#include "TreantRoot.h"
#include "../../system/World.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace ggj
{
    namespace
    {
        constexpr int32_t TENDRIL_COUNT = 7;
        constexpr int32_t SPIKE_COUNT = 9;
        constexpr ::Color ROOT_DARK { 58, 36, 22, 255 };
        constexpr ::Color ROOT_LIGHT { 112, 72, 40, 255 };
        constexpr ::Color WARNING_COLOR { 190, 70, 255, 255 };
        constexpr ::Color SPIKE_COLOR { 150, 104, 60, 255 };
        constexpr ::Color SPIKE_TIP_COLOR { 226, 196, 140, 255 };
    } // namespace

    TreantRoot::TreantRoot(b2Body* body, const UserData& userData, const raylib::Vector2& target, float flightTime, float spikeDelay, World* world)
        : PhysicsObject(body, { HIT_RADIUS, HIT_RADIUS }, PhysicsShape::Circle, userData), m_target { target }, m_world { world }, m_flightTime { std::max(flightTime, 0.01f) },
          m_spikeDelay { std::max(spikeDelay, 0.f) }
    {
        m_body->SetFixedRotation(true);
        // Spawned after this frame's update pass, so it is drawn once before update() syncs this.
        m_position = ConvertToVector2(m_body->GetPosition());
        m_wigglePhase = static_cast<float>(GetRandomValue(0, 628)) / 100.f;
    }

    float TreantRoot::ArcFraction(float progress)
    {
        const float t = std::clamp(progress, 0.f, 1.f);
        return 4.f * t * (1.f - t);
    }

    bool TreantRoot::IsInsideSpikeArea(const raylib::Vector2& root, const raylib::Vector2& footMin, const raylib::Vector2& footMax)
    {
        // The point of the player's lower body closest to the root decides: if it is inside the ellipse, they overlap.
        const float closestX = std::clamp(root.x, footMin.x, footMax.x);
        const float closestY = std::clamp(root.y, footMin.y, footMax.y);
        const float dx = (closestX - root.x) / HIT_RADIUS;
        const float dy = (closestY - root.y) / (HIT_RADIUS * GROUND_SQUASH);
        return dx * dx + dy * dy <= 1.f;
    }

    float TreantRoot::SpikeFraction(float spikeElapsed)
    {
        if (spikeElapsed <= 0.f || spikeElapsed >= SPIKE_DURATION)
            return 0.f;
        if (spikeElapsed < SPIKE_RISE_DURATION)
            return spikeElapsed / SPIKE_RISE_DURATION;

        // Hold fully risen for a beat, then sink during the last third.
        const float sinkStart = SPIKE_DURATION * 0.66f;
        if (spikeElapsed < sinkStart)
            return 1.f;
        return 1.f - (spikeElapsed - sinkStart) / (SPIKE_DURATION - sinkStart);
    }

    void TreantRoot::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        m_elapsed += timeDelta;
        if (m_phase == RootPhase::Flying)
        {
            if (m_elapsed >= m_flightTime)
                land();
        }
        else if (m_phase == RootPhase::Landed)
        {
            if (m_elapsed >= m_spikeDelay)
                spike();
        }
        else if (m_elapsed >= SPIKE_DURATION)
        {
            destroy();
            return;
        }

        PhysicsObject::update(timeDelta);
    }

    void TreantRoot::land()
    {
        m_phase = RootPhase::Landed;
        m_elapsed = 0.f;
        m_body->SetLinearVelocity({ 0.f, 0.f });
        // Snap to the target so frame-rate hiccups cannot leave it short or past the mark.
        m_body->SetTransform(ConvertToB2Vec2(m_target), 0.f);
    }

    void TreantRoot::spike()
    {
        m_phase = RootPhase::Spiking;
        m_elapsed = 0.f;

        // Every peer runs its own copy and only hurts its own player, like bombs do.
        if (m_world != nullptr)
            m_world->damageLocalPlayerOnRoot(m_target);
    }

    void TreantRoot::draw()
    {
        if (m_body == nullptr || !m_isVisible)
            return;

        if (m_phase == RootPhase::Spiking)
        {
            drawRoot();
            drawSpikes();
        }
        else
        {
            drawRoot();
        }
    }

    void TreantRoot::drawRoot() const
    {
        const float x = m_position.x;
        const float y = m_position.y;

        if (m_phase == RootPhase::Flying)
        {
            const float progress = std::clamp(m_elapsed / m_flightTime, 0.f, 1.f);
            const float lift = ArcFraction(progress) * ARC_HEIGHT;
            const float spin = m_elapsed * 14.f;

            // Shadow on the ground, shrinking as the root climbs.
            DrawEllipse(static_cast<int>(x), static_cast<int>(y), 10.f - 4.f * ArcFraction(progress), 5.f - 2.f * ArcFraction(progress), Fade(BLACK, 0.35f));
            // The root itself: a knobbly chunk with a few trailing tendrils.
            for (int32_t i = 0; i < 3; ++i)
            {
                const float angle = spin + static_cast<float>(i) * 2.1f;
                const raylib::Vector2 tip { x + std::cos(angle) * 13.f, y - lift + std::sin(angle) * 13.f };
                DrawLineEx({ x, y - lift }, tip, 3.f, ROOT_LIGHT);
            }
            DrawCircle(static_cast<int>(x), static_cast<int>(y - lift), 8.f, ROOT_DARK);
            return;
        }

        // Landing squash: the root splats flat and spreads out.
        const float squash = (m_phase == RootPhase::Landed) ? std::clamp(m_elapsed / LANDING_SQUASH_DURATION, 0.f, 1.f) : 1.f;
        const float spread = 0.55f + 0.45f * squash;
        const float radius = ROOT_RADIUS * spread;

        DrawEllipse(static_cast<int>(x), static_cast<int>(y), radius, radius * 0.6f, Fade(ROOT_DARK, 0.85f));

        const float time = m_wigglePhase + m_elapsed * 6.f;
        for (int32_t i = 0; i < TENDRIL_COUNT; ++i)
        {
            const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(TENDRIL_COUNT) + m_wigglePhase;
            const float wiggle = 0.25f * std::sin(time + static_cast<float>(i));
            const raylib::Vector2 mid { x + std::cos(angle + wiggle) * radius * 0.55f, y + std::sin(angle + wiggle) * radius * 0.33f };
            const raylib::Vector2 tip { x + std::cos(angle + wiggle * 1.6f) * radius * 1.05f, y + std::sin(angle + wiggle * 1.6f) * radius * 0.63f };
            DrawLineEx({ x, y }, mid, 4.f, ROOT_LIGHT);
            DrawLineEx(mid, tip, 2.f, ROOT_LIGHT);
        }
        DrawEllipse(static_cast<int>(x), static_cast<int>(y), radius * 0.35f, radius * 0.22f, ROOT_DARK);

        // Warning ring: starts faint and brightens, pulsing faster, as the spike gets closer.
        if (m_phase == RootPhase::Landed)
        {
            const float progress = m_spikeDelay > 0.f ? std::clamp(m_elapsed / m_spikeDelay, 0.f, 1.f) : 1.f;
            const float pulse = 0.5f + 0.5f * std::sin(m_elapsed * (6.f + 18.f * progress));
            const float alpha = 0.25f + 0.45f * progress + 0.3f * pulse * progress;
            DrawEllipseLines(static_cast<int>(x), static_cast<int>(y), HIT_RADIUS, HIT_RADIUS * GROUND_SQUASH, Fade(WARNING_COLOR, std::min(alpha, 1.f)));
        }
    }

    void TreantRoot::drawSpikes() const
    {
        const float growth = SpikeFraction(m_elapsed);
        if (growth <= 0.f)
            return;

        // Spikes at the ground ring and one in the middle; back ones are drawn first so front ones overlap them.
        struct Spike
        {
            float offsetX;
            float offsetY;
            float height;
        };
        std::array<Spike, SPIKE_COUNT> spikes {};
        for (int32_t i = 0; i < SPIKE_COUNT - 1; ++i)
        {
            const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(SPIKE_COUNT - 1) + m_wigglePhase;
            // Heights vary per spike so the crown looks jagged rather than like a fence.
            const float height = SPIKE_HEIGHT * (0.7f + 0.3f * std::abs(std::sin(angle * 2.3f + 1.f)));
            spikes[static_cast<size_t>(i)] = { std::cos(angle) * ROOT_RADIUS * 0.8f, std::sin(angle) * ROOT_RADIUS * 0.5f, height };
        }
        spikes[SPIKE_COUNT - 1] = { 0.f, 0.f, SPIKE_HEIGHT * 1.15f };
        std::sort(spikes.begin(), spikes.end(), [](const Spike& a, const Spike& b) { return a.offsetY < b.offsetY; });

        for (const Spike& spike : spikes)
        {
            const float baseX = m_position.x + spike.offsetX;
            const float baseY = m_position.y + spike.offsetY;
            const float halfWidth = 6.f;
            const float tipHeight = spike.height * growth;
            const raylib::Vector2 tip { baseX, baseY - tipHeight };
            // Counter-clockwise for DrawTriangle: tip, bottom left, bottom right.
            DrawTriangle(tip, { baseX - halfWidth, baseY }, { baseX + halfWidth, baseY }, SPIKE_COLOR);
            // Pale tip so the sharp end reads clearly.
            const float tipLength = tipHeight * 0.4f;
            const float tipHalfWidth = halfWidth * 0.4f;
            DrawTriangle(tip, { baseX - tipHalfWidth, baseY - tipHeight + tipLength }, { baseX + tipHalfWidth, baseY - tipHeight + tipLength }, SPIKE_TIP_COLOR);
        }
    }
} // namespace ggj
