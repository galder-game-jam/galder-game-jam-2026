#ifndef GALDER_GAME_JAM_2026_HUDOPTIONS_HPP
#define GALDER_GAME_JAM_2026_HUDOPTIONS_HPP

namespace ggj
{
    /*!
     * HUD display settings loaded from the "hud" object in options.json.
     *
     * The user-declared constructor prevents Boost.DI aggregate-injection from
     * silently discarding the member initializers (see CLAUDE.md).
     */
    struct HudOptions
    {
        HudOptions() = default;

        float hudAlpha { 0.706f }; ///< HUD transparency in [0.0, 1.0]; 0 = fully transparent, 1 = fully opaque. Default matches legacy HUD_ALPHA 180/255.
    };
} // namespace ggj

#endif // GALDER_GAME_JAM_2026_HUDOPTIONS_HPP
