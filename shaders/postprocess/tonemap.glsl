//From https://github.com/GPUOpen-Effects/FidelityFX-SSSR/tree/master

float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut) {
    return
    -((-pow(midIn, contrast) + (midOut*(pow(hdrMax, contrast*shoulder)*pow(midIn, contrast) -
    pow(hdrMax, contrast)*pow(midIn, contrast*shoulder)*midOut)) /
    (pow(hdrMax, contrast*shoulder)*midOut - pow(midIn, contrast*shoulder)*midOut)) /
    (pow(midIn, contrast*shoulder)*midOut));
}

float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut) {
    return (pow(hdrMax, contrast*shoulder)*pow(midIn, contrast) - pow(hdrMax, contrast)*pow(midIn, contrast*shoulder)*midOut) /
    (pow(hdrMax, contrast*shoulder)*midOut - pow(midIn, contrast*shoulder)*midOut);
}

float ColTone(float x, vec4 p) {
    float z = pow(x, p.r);
    return z / (pow(z, p.g)*p.b + p.a);
}

vec3 pow(vec3 x, float y) {
    return vec3(pow(x.r, y), pow(x.g, y), pow(x.b, y));
}

vec3 AMDTonemapper(vec3 color) {
    float hdrMax = 16.0;
    float contrast = 2.0;
    float shoulder = 1.0;
    float midIn = 0.18;
    float midOut = 0.18;

    float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
    float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

    float EPS = 1e-6;
    float peak = max(max(color.r, color.g), color.b);
    peak = max(EPS, peak);

    vec3 ratio = color / peak;
    peak = ColTone(peak, vec4(contrast, shoulder, b, c));

    float crosstalk = 4.0;
    float saturation = contrast;
    float crossSaturation = contrast*16.0;

    vec3 white = vec3(1.0);

    ratio = pow(abs(ratio), saturation / crossSaturation);
    ratio = mix(ratio, white, pow(peak, crosstalk));
    ratio = pow(abs(ratio), crossSaturation);

    color = peak * ratio;
    return color;
}

vec3 DX11DSK(vec3 color) {
    float MIDDLE_GRAY = 0.72;
    float LUM_WHITE = 1.5;

    color.rgb *= MIDDLE_GRAY;
    color.rgb *= (1.0 + color/LUM_WHITE);
    color.rgb /= (1.0 + color);

    return color;
}

vec3 Reinhard(vec3 color) {
    return color / (1.0 + color);
}

vec3 Uncharted2TonemapOp(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 Uncharted2Tonemap(vec3 color) {
    float W = 11.2;
    return Uncharted2TonemapOp(2.0 * color) / Uncharted2TonemapOp(vec3(W));
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
}
