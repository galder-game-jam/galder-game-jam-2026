#ifndef GALDER_GAME_JAM_2026_GRAPHICSOPTIONS_HPP
#define GALDER_GAME_JAM_2026_GRAPHICSOPTIONS_HPP

#include <cstdint>

namespace ggj
{
    /*!
     * Visual effect settings loaded from the "graphics" object in options.json.
     *
     * The user-declared constructor prevents Boost.DI aggregate-injection from
     * silently discarding the member initializers (see CLAUDE.md).
     */
    struct GraphicsOptions
    {
        GraphicsOptions() = default;

        static constexpr int32_t MIN_BLOOD_AMOUNT = 1;  ///< Slightest splatter.
        static constexpr int32_t MAX_BLOOD_AMOUNT = 10; ///< "Gorefest".

        int32_t bloodAmount { MAX_BLOOD_AMOUNT }; ///< How much blood flies when something takes damage, in [MIN_BLOOD_AMOUNT, MAX_BLOOD_AMOUNT].
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_GRAPHICSOPTIONS_HPP
