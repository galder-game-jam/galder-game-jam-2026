#version 330

// Recolours a sprite towards a single tint while keeping its shading. Used for the treant's enrage.

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 tintColor;
uniform float tintStrength; // 0 = untouched sprite, 1 = fully tinted

out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    float luma = dot(texel.rgb, vec3(0.299, 0.587, 0.114));

    // Map brightness onto the tint so dark pixels stay dark and highlights stay bright: the sprite
    // keeps its detail instead of turning into a flat silhouette.
    vec3 tinted = clamp(tintColor * (0.35 + 1.15 * luma), 0.0, 1.0);

    finalColor = vec4(mix(texel.rgb, tinted, tintStrength), texel.a) * colDiffuse * fragColor;
}
