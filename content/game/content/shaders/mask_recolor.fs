#version 330

// Recolours the parts of a painted image (the HUD portraits) that are marked in a slot mask, keeping
// the painted shading. Unlike palette_swap.fs it does not need exact colours, so it works on art with
// hundreds of shades.
//
// Mask colours: red = slot 0, green = slot 1, blue = slot 2, yellow = slot 3, transparent = untouched.
// Keep MAX_SLOTS in sync with MaskRecolor::MAX_SLOTS in data/CharacterColors.hpp.
const int MAX_SLOTS = 4;
const vec3 LUMA = vec3(0.299, 0.587, 0.114); // Same weights as CharacterPalette.cpp

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform sampler2D maskTexture;
uniform vec3 baseColors[MAX_SLOTS];   // The sprite's base shade per slot; a texel this bright gets exactly the target colour
uniform vec3 targetColors[MAX_SLOTS]; // The picked colour per slot
uniform float slotWeights[MAX_SLOTS]; // 0 = leave the slot as painted, 1 = fully recoloured

out vec4 finalColor;

int slotFromMask(vec4 mask)
{
    if (mask.a < 0.5)
        return -1;

    bool hasRed = mask.r > 0.5;
    bool hasGreen = mask.g > 0.5;
    if (hasRed && hasGreen)
        return 3;
    if (hasRed)
        return 0;
    if (hasGreen)
        return 1;
    if (mask.b > 0.5)
        return 2;
    return -1;
}

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 color = texel.rgb;

    int slot = slotFromMask(texture(maskTexture, fragTexCoord));
    if (slot >= 0)
    {
        // Same idea as the sprite palette swap: scale the picked colour by how much darker (or
        // lighter) this texel is than the slot's base shade, so the portrait keeps its shading and
        // matches the recoloured sprite.
        float baseLuminance = dot(baseColors[slot], LUMA);
        float factor = (baseLuminance > 0.0) ? dot(texel.rgb, LUMA) / baseLuminance : 1.0;
        vec3 recolored = targetColors[slot] * factor;

        // Highlights brighter than the base shade overflow a channel. Spill the excess into every
        // channel so they fade towards white instead of flattening into the clipped colour.
        float overflow = max(max(recolored.r, max(recolored.g, recolored.b)) - 1.0, 0.0);
        recolored = clamp(recolored + vec3(overflow), 0.0, 1.0);

        color = mix(color, recolored, slotWeights[slot]);
    }

    finalColor = vec4(color, texel.a) * colDiffuse * fragColor;
}
