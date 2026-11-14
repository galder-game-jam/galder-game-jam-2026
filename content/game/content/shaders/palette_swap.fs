#version 330

// Replaces exact sprite colours with player-picked ones. Used to recolour the player characters.
// Keep MAX_COLORS in sync with PaletteSwap::MAX_COLORS in data/CharacterColors.hpp.
const int MAX_COLORS = 24;
// Half a step of an 8-bit channel: sprites use point filtering, so texels hit the palette exactly.
const float TOLERANCE = 0.5 / 255.0;

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 fromColors[MAX_COLORS];
uniform vec3 toColors[MAX_COLORS];
uniform int colorCount;

out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 color = texel.rgb;

    for (int i = 0; i < MAX_COLORS; ++i)
    {
        if (i >= colorCount)
            break;

        if (all(lessThan(abs(texel.rgb - fromColors[i]), vec3(TOLERANCE))))
        {
            color = toColors[i];
            break;
        }
    }

    finalColor = vec4(color, texel.a) * colDiffuse * fragColor;
}
