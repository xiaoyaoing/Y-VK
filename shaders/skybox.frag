#version 450

#extension GL_EXT_debug_printf : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "RayTracing/commons.h"

layout (set = 1, binding = 0) uniform samplerCube samplerEnv;


layout (set = 0, binding = 0) buffer UBOParams {
    RTPrimitive primitives[1];
} uboParams1;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform UBOParams {
    vec4 _pad0;
    float exposure;
    float gamma;
} uboParams;

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
    vec3 outcol = Uncharted2Tonemap(color.rgb * uboParams.exposure);
    outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
    return vec4(pow(outcol, vec3(1.0f / uboParams.gamma)), color.a);
}

#define MANUAL_SRGB 1

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 bLess = step(vec3(0.04045), srgbIn.xyz);
    vec3 linOut = mix(srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055), vec3(2.4)), bLess);
    return vec4(linOut, srgbIn.w);;
}

void main()
{
    vec3 color = SRGBtoLINEAR(tonemap(textureLod(samplerEnv, inUVW, 1.5))).rgb;
    outColor = vec4(color * 1.0, 1.0);
}