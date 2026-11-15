#ifndef GALDER_GAME_JAM_2026_BLOODSPLATTER_H
#define GALDER_GAME_JAM_2026_BLOODSPLATTER_H

#include "raylib-cpp.hpp"
#include "../interfaces/graphics/IBloodEmitter.h"
#include "../interfaces/manager/IOptionsManager.h"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <random>
#include <vector>

namespace ggj
{
    /*!
     * Blood particle system. Droplets fly out of a wound, slow down and land as stains that stay on
     * the ground for a while before fading. How much blood a hit makes follows
     * GraphicsOptions::bloodAmount, read on every hit so changing the option takes effect at once.
     */
    class BloodSplatter : public IBloodEmitter
    {
        public:
        /*!
         * @param options Options manager to read the blood amount from; must outlive this object
         */
        explicit BloodSplatter(const IOptionsManager& options) : m_options { options }, m_random { std::random_device {}() } {}

        void emitBlood(const raylib::Vector2& position, const raylib::Vector2& direction, float strength) override;

        /*!
         * Advances flying droplets and ages stains.
         *
         * @param timeDelta Time since last frame in seconds
         */
        void update(float timeDelta);

        /*! Draws the stains lying on the ground. Call before drawing characters so they walk over the blood. */
        void drawStains() const;

        /*! Draws the droplets still in the air. Call after drawing characters. */
        void drawDroplets() const;

        /*! Removes every droplet and stain. */
        void clear();

        /*!
         * How many droplets one burst makes.
         *
         * @param bloodAmount Blood amount option, clamped into its valid range
         * @param strength    Burst strength; see IBloodEmitter::emitBlood()
         * @return Droplet count, at least 1 and at most MAX_DROPLETS_PER_BURST
         */
        [[nodiscard]] static int32_t dropletCountFor(int32_t bloodAmount, float strength);

        /*!
         * How long a stain stays visible. More blood means a longer-lasting mess.
         *
         * @param bloodAmount Blood amount option, clamped into its valid range
         * @return Lifetime in seconds
         */
        [[nodiscard]] static float stainLifetimeFor(int32_t bloodAmount);

        [[nodiscard]] size_t getDropletCount() const { return m_droplets.size(); }
        [[nodiscard]] size_t getStainCount() const { return m_stains.size(); }

        static constexpr int32_t MAX_DROPLETS_PER_BURST = 200;
        static constexpr size_t MAX_STAINS = 3000;   ///< Oldest stains go first when the floor is this full.
        static constexpr size_t MAX_DROPLETS = 2000; ///< Cap on droplets in the air at once.

        private:
        struct Droplet
        {
            raylib::Vector2 position;
            raylib::Vector2 velocity;
            float timeLeft { 0.f }; ///< Seconds until it lands.
            float size { 1.f };     ///< Radius in pixels.
            Color color { 0, 0, 0, 255 };
        };

        struct Stain
        {
            raylib::Vector2 position;
            float radius { 1.f };
            float age { 0.f };
            float lifetime { 0.f };
            Color color { 0, 0, 0, 255 };
        };

        static constexpr float FADE_SECONDS = 4.f; ///< How long a stain takes to fade out at the end of its life.
        static constexpr float DROPLET_DRAG = 3.f; ///< Air drag in 1/s; higher slows droplets faster.

        void land(const Droplet& droplet);

        [[nodiscard]] float randomFloat(float min, float max);

        const IOptionsManager& m_options;
        std::mt19937 m_random;
        std::vector<Droplet> m_droplets;
        std::deque<Stain> m_stains;
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_BLOODSPLATTER_H
