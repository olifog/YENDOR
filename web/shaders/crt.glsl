vec2 crtCurve(vec2 uv)
{
    //uv = uv * 2.0 - 1.0;
    //uv *= 1.05;
    //uv.x *= 1.0 + pow(abs(uv.y) / 4.0, 2.0);
    //uv.y *= 1.0 + pow(abs(uv.x) / 3.0, 2.0);
    //uv = uv * 0.5 + 0.5;
    return uv;
}

vec3 sampleChromatic(sampler2D tex, vec2 uv)
{
    // Subtle CRT-style aberration
    float strength = 0.0001;

    vec2 center = vec2(0.5);
    vec2 dir = uv - center;
    float dist = length(dir);

    vec2 offset = normalize(dir + 1e-6) * dist * strength;

    float r = texture(tex, uv - offset).r;
    float g = texture(tex, uv + offset).g;
    float b = texture(tex, uv - offset).b;

    return vec3(r, g, b);
}

vec3 applyCRT(vec2 fragCoord, vec3 color)
{
    vec2 uv = fragCoord / iResolution.xy;

    /* GAMMA */
    color = pow(color, vec3(1.0 / 1.3));

    /* =========================
       STRONGER SCANLINES
       ========================= */

    float scanFreq = iResolution.y * 2.5;
    float scan = sin(uv.y * scanFreq);
    float scanMask = 0.75 + 0.25 * scan;     // deeper contrast
    color *= scanMask;

    /* =========================
       RGB SHADOW MASK
       ========================= */

    float mask = sin(uv.x * iResolution.x * 3.14159);
    color.r *= 1.05 + 0.05 * mask;
    color.g *= 0.95;
    color.b *= 1.05 - 0.05 * mask;

    /* =========================
       GLOW (EMISSIVE BLEED)
       ========================= */

    // Luminance
    float lum = dot(color, vec3(0.299, 0.587, 0.114));

    // Only glow bright areas
    float glowStrength = smoothstep(0.4, 0.9, lum);

    // Horizontal blur approximation (cheap CRT bleed)
    float glow = 0.0;
    glow += sin(uv.x * iResolution.x * 0.5) * 0.5 + 0.5;

    vec3 glowColor = color * glowStrength * 0.6;
    color += glowColor * glow;

    /* =========================
       VIGNETTE
       ========================= */

    float dist = distance(uv, vec2(0.5));
    color *= smoothstep(0.85, 0.35, dist);

    /* =========================
       FLICKER
       ========================= */

    color *= 0.97 + 0.03 * sin(iTime * 60.0);

    return color;
}

