#version 460 core

#extension GL_GOOGLE_include_directive : enable 

#include "vxgi.glsl"
#include "../perFrame.glsl"
#include "../shadow.glsl"

precision mediump float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(set=1, binding = 0)  uniform sampler3D radiance_map;


layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;



#define  CLIP_LEVEL_COUNT 6
#define  VOXEL_FACE_COUNT 6



layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_specular;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_normal;
layout(input_attachment_index = 3, binding = 3, set=2) uniform subpassInput gbuffer_emission;
layout(input_attachment_index = 4, binding = 4, set=2) uniform subpassInput gbuffer_depth;


// Get Fixed voxel cone directions from 
// https://www.gamasutra.com/view/news/286023/Graphics_Deep_Dive_Cascaded_voxel_cone_tracing_in_The_Tomorrow_Children.php

// 32 Cones for higher quality (16 on average per hemisphere)
const int    DIFFUSE_CONE_COUNT_32      = 32;
const float DIFFUSE_CONE_APERTURE_32  = 0.628319;
const vec3 DIFFUSE_CONE_DIRECTIONS_32[32] = {
vec3(0.898904, 0.435512, 0.0479745),
vec3(0.898904, -0.435512, -0.0479745),
vec3(0.898904, 0.0479745, -0.435512),
vec3(0.898904, -0.0479745, 0.435512),
vec3(-0.898904, 0.435512, -0.0479745),
vec3(-0.898904, -0.435512, 0.0479745),
vec3(-0.898904, 0.0479745, 0.435512),
vec3(-0.898904, -0.0479745, -0.435512),
vec3(0.0479745, 0.898904, 0.435512),
vec3(-0.0479745, 0.898904, -0.435512),
vec3(-0.435512, 0.898904, 0.0479745),
vec3(0.435512, 0.898904, -0.0479745),
vec3(-0.0479745, -0.898904, 0.435512),
vec3(0.0479745, -0.898904, -0.435512),
vec3(0.435512, -0.898904, 0.0479745),
vec3(-0.435512, -0.898904, -0.0479745),
vec3(0.435512, 0.0479745, 0.898904),
vec3(-0.435512, -0.0479745, 0.898904),
vec3(0.0479745, -0.435512, 0.898904),
vec3(-0.0479745, 0.435512, 0.898904),
vec3(0.435512, -0.0479745, -0.898904),
vec3(-0.435512, 0.0479745, -0.898904),
vec3(0.0479745, 0.435512, -0.898904),
vec3(-0.0479745, -0.435512, -0.898904),
vec3(0.57735, 0.57735, 0.57735),
vec3(0.57735, 0.57735, -0.57735),
vec3(0.57735, -0.57735, 0.57735),
vec3(0.57735, -0.57735, -0.57735),
vec3(-0.57735, 0.57735, 0.57735),
vec3(-0.57735, 0.57735, -0.57735),
vec3(-0.57735, -0.57735, 0.57735),
vec3(-0.57735, -0.57735, -0.57735)
};

const int    DIFFUSE_CONE_COUNT_16        = 16;
const float DIFFUSE_CONE_APERTURE_16    = 0.872665;
const vec3 DIFFUSE_CONE_DIRECTIONS_16[16] = {
vec3(0.57735, 0.57735, 0.57735),
vec3(0.57735, -0.57735, -0.57735),
vec3(-0.57735, 0.57735, -0.57735),
vec3(-0.57735, -0.57735, 0.57735),
vec3(-0.903007, -0.182696, -0.388844),
vec3(-0.903007, 0.182696, 0.388844),
vec3(0.903007, -0.182696, 0.388844),
vec3(0.903007, 0.182696, -0.388844),
vec3(-0.388844, -0.903007, -0.182696),
vec3(0.388844, -0.903007, 0.182696),
vec3(0.388844, 0.903007, -0.182696),
vec3(-0.388844, 0.903007, 0.182696),
vec3(-0.182696, -0.388844, -0.903007),
vec3(0.182696, 0.388844, -0.903007),
vec3(-0.182696, 0.388844, 0.903007),
vec3(0.182696, -0.388844, 0.903007)
};

const float MAX_TRACE_DISTANCE = 30.f;
const float MIN_SPECULAR_FACTOR  = 0.05f;



vec3  worldPosFromDepth    (float depth);
vec4  traceCone            (vec3 startPos, vec3 direction, float aperture,
float maxDistance, float startLevel, float stepFactor);
float calcmin_level        (vec3 worldPos);
vec4  min_levelToColor    (float min_level);

void main(){
    vec4  diffuse  = subpassLoad(gbuffer_diffuse);
    vec4  specular = subpassLoad(gbuffer_specular);
    vec3  normal   = subpassLoad(gbuffer_normal).xyz;
    normal      = normalize(2.0 * normal - 1.0);
    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = subpassLoad(gbuffer_depth).x;

    vec3 world_pos = worldPosFromDepth(depth);
    vec3 start_pos = world_pos + voxel_size * normal;

    //diffuse cone 
    vec3 indirect_contribution = vec3(0.0);
    uint min_level = 0;

    for (int i = 0; i < DIFFUSE_CONE_COUNT_16; ++i)
    {
        float cos_theta = dot(normal, DIFFUSE_CONE_DIRECTIONS_16[i]);
        if (cos_theta < 0.0)
        continue;

        indirect_contribution += traceCone(start_pos, DIFFUSE_CONE_DIRECTIONS_16[i], DIFFUSE_CONE_APERTURE_16,
        MAX_TRACE_DISTANCE, min_level, 1).rgb * cos_theta;// / 3.141592;
        //validConeCount += cos_theta;
    }
    indirect_contribution /= DIFFUSE_CONE_COUNT_16;

    vec3 specular_color = specular.xyz;
    float metallic = specular.a;

    vec3 diffuse_color = diffuse.xyz;
    float perceptual_roughness = diffuse.a;

    vec3 view_dir = per_frame.camera_pos - world_pos;
    vec3 indirect_specular_contribution = vec3(0.0);
    float roughness = sqrt(2.0 / (metallic + 2.0));
    if (any(greaterThan(specular_color, vec3(1e-6))) && metallic > 1e-6)
    {
        vec3 specular_cone_direction = reflect(-view_dir, normal);
        indirect_specular_contribution += traceCone(
        start_pos, specular_cone_direction,
        MIN_SPECULAR_FACTOR,
        MAX_TRACE_DISTANCE, min_level, voxel_size
        ).rgb * specular_color;
    }
    //specular cone 
    indirect_contribution += indirect_specular_contribution;

    //calcuate sppecular contribution
    vec3 direct_contribution = vec3(0.0);

    bool has_emission = any(greaterThan(emission, vec3(1e-6)));
    if (has_emission)
    {
        direct_contribution+= emission;
    }
    else
    {
        // calculate Microfacet BRDF model
        // Roughness is authored as perceptual roughness; as is convention
        // convert to material roughness by squaring the perceptual roughness [2].
        // for 
        for (uint i = 0U; i < per_frame.light_count; ++i)
        {
            direct_contribution += apply_light(lights_info.lights[i], world_pos, normal);
        }
        //        float alphaRoughness = perceptualRoughness * perceptualRoughness;
        //
        //        // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
        //        // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflectance to 0%;
        //        float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
        //        vec3 specularEnvironmentR0 = specularColor.rgb;
        //        vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));
        //
        //        vec3 light = normalize(-uDirectionalLight.direction);
        //        vec3 h = normalize(light + view);
        //        vec3 reflection = -normalize(reflect(view, normal));
        //        reflection.y *= -1.0f;
        //
        //        float NdotL = clamp(dot(normal, light),		0.001, 1.0);
        //        float NdotV = clamp(abs(dot(normal, view)), 0.001, 1.0);
        //        float NdotH = clamp(dot(normal, h),			  0.0, 1.0);
        //        float LdotH = clamp(dot(light, h),			  0.0, 1.0);
        //        float VdotH = clamp(dot(view, h),			  0.0, 1.0);
        //
        //        PBRInfo pbr = PBRInfo(NdotL, NdotV, NdotH, LdotH, VdotH, perceptualRoughness,
        //                              metallic, specularEnvironmentR0, specularEnvironmentR90,
        //                              alphaRoughness, diffuseColor, specularColor);
        //
        //        float visibility = calcute_shadow()
        //        directContribution += microfacetBRDF(pbr) * visibility;
    }
    out_color = vec4(direct_contribution + indirect_contribution, 1);
}

vec3  worldPosFromDepth    (float depth){
    vec4  clip         = vec4(in_uv * 2.0 - 1.0, subpassLoad(gbuffer_depth).x, 1.0);
    vec4 world_w = per_frame.inv_view_proj * clip;
    vec3 pos     = world_w.xyz / world_w.w;
    return pos;
}

vec4 sampleClipmap(sampler3D clipmap, vec3 worldPos, int clipmapLevel, vec3 faceOffset, vec3 weight)
{
    float cur_level_voxel_size = voxel_size * exp2(clipmapLevel);
    float extent    =  cur_level_voxel_size * clip_map_resoultion;

    vec3 samplePos = (fract(worldPos / extent) * clip_map_resoultion) / (float(clip_map_resoultion) + 2.0);

    samplePos.y += clipmapLevel;
    samplePos.y /= CLIP_LEVEL_COUNT;
    samplePos.x /= VOXEL_FACE_COUNT;

    return texture(clipmap, samplePos + vec3(faceOffset.x, 0.0, 0.0)) * weight.x +
    texture(clipmap, samplePos + vec3(faceOffset.y, 0.0, 0.0)) * weight.y +
    texture(clipmap, samplePos + vec3(faceOffset.z, 0.0, 0.0)) * weight.z;
}

vec4 sampleClipmapLinear(sampler3D clipmap, vec3 worldPos, float curLevel, ivec3 faceIndex, vec3 weight)
{
    int lowerLevel = int(floor(curLevel));
    int upperLevel = int(ceil(curLevel));

    vec3 faceOffset  = vec3(faceIndex) / VOXEL_FACE_COUNT;
    vec4 lowerSample = sampleClipmap(clipmap, worldPos, lowerLevel, faceOffset, weight);
    vec4 upperSample = sampleClipmap(clipmap, worldPos, upperLevel, faceOffset, weight);

    return mix(lowerSample, upperSample, fract(curLevel));
}

ivec3 calculateVoxelFaceIndex(vec3 normal)
{
    return ivec3(
    normal.x > 0.0 ? 0 : 1,
    normal.y > 0.0 ? 2 : 3,
    normal.z > 0.0 ? 4 : 5
    );
}



vec4 traceCone(vec3 startPos, vec3 direction, float aperture, float maxDistance, float startLevel, float stepFactor)
{
    vec4 result = vec4(0.0);
    float coneCoefficient = 2.0 * tan(aperture * 0.5);

    float curLevel = startLevel;
    float cur_voxel_size = voxel_size * exp2(curLevel);

    startPos += direction * cur_voxel_size  * 0.5;

    float step         = 0.0;
    float diameter     = max(step * coneCoefficient, voxel_size);
    float occlusion  = 0.0;

    ivec3 faceIndex  = calculateVoxelFaceIndex(direction);

    vec3  weight     = direction * direction;

    float curSegmentLength    = cur_voxel_size;
    float minRadius        = voxel_size  * 0.5;

    vec3 clip_map_center = (clipmap_max_pos + clipmap_min_pos) * 0.5;

    while ((step < maxDistance) && (occlusion < 1.0))
    {
        vec3  position                = startPos + direction * step;
        float distanceToVoxelCenter = length(clipmap_level - position);
        float min_level                = ceil(log2(distanceToVoxelCenter / minRadius));

        curLevel = log2(diameter / voxel_size);
        curLevel = min(max(max(startLevel, curLevel), min_level), CLIP_LEVEL_COUNT - 1);

        vec4 clipmapSample = sampleClipmapLinear(radiance_map, position, curLevel, faceIndex, weight);
        vec3 radiance = clipmapSample.rgb;
        float opacity = clipmapSample.a;

        cur_voxel_size = voxel_size * exp2(curLevel);

        float correction = curSegmentLength / cur_voxel_size;
        //todo what does correction do?
        radiance = radiance * correction;
        opacity  = clamp(1.0 - pow(1.0 - opacity, correction), 0.0, 1.0);

        vec4 src = vec4(radiance.rgb, opacity);
        // Alpha blending
        result      += clamp(1.0 - result.a, 0.0, 1.0) * src;
        occlusion += (1.0 - occlusion) * opacity / (1.0 + (step + cur_voxel_size) * 2);

        float prevStep = step;
        step += max(diameter, voxel_size) * stepFactor;
        curSegmentLength = (step - prevStep);
        diameter = step * coneCoefficient;
    }

    return vec4(result.rgb, 1.0 - occlusion);
}

float calcmin_level        (vec3 worldPos);
vec4  min_levelToColor    (float min_level);