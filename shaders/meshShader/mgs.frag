#version 460
struct FragmentInput {
    float height;
    float root_height;
    vec3 world_pos;
    vec3 normal;
    vec3 ground_normal;
    vec3 clip_pos;
};
layout(binding = 0,set = 1) uniform sampler2D perlinNoise;

layout(location = 0) in FragmentInput fragmentInput;
layout(location = 0) out vec4 fragColor;

float PerlinNoise2D(vec2 P){
    return texture(perlinNoise, P).r;
}

void main() {
    float selfshadow = clamp(pow((fragmentInput.world_pos.z - fragmentInput.root_height)/fragmentInput.height, 1.5), 0, 1);
    vec3 baseColor;
    baseColor.rgb = vec3(0.163, 0.186, 0.059) * selfshadow;
    baseColor.rgb *= 0.75 + 0.25 * PerlinNoise2D(0.25 * fragmentInput.world_pos.xy);
    fragColor = vec4(baseColor, 1.0);
}
