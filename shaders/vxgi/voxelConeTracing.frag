#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable


#include "../perFrame.glsl"
//#include "../shadow.glsl"
#include "../lighting.glsl"
#include "../brdf.glsl"

precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(set=1, binding = 0)  uniform sampler3D radiance_map;


layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;


layout (push_constant) uniform PushConstants
{
    vec3  volume_center;// 12
    uint  uRenderingMode;// 16
    float voxel_size;// 20
    float clip_map_resoultion;// 24
    float uTraceStartOffset;// 28
    float uIndirectDiffuseIntensity;// 32
    float uAmbientOcclusionFactor;// 36
    float uMinTraceStepFactor;// 40
    float uIndirectSpecularIntensity;// 44
    float uOcclusionDecay;// 48
    int   uEnable32Cones;// 52
    int       uDirectLighting;
    int       uIndirectLighting;
};

//layout(push_constant) uniform PushConstants
//{
//    vec3  volume_center;// 12
//    uint  uRenderingMode;// 16
//    float voxel_size;// 20
//    float clip_map_resoultion;// 24
//    float uTraceStartOffset;// 28
//    float uIndirectDiffuseIntensity;// 32
//    float uAmbientOcclusionFactor;// 36
//    float uMinTraceStepFactor;// 40
//    float uIndirectSpecularIntensity;// 44
//    float uOcclusionDecay;// 48
//    int   uEnable32Cones;// 52
//} push_constants;



#define  CLIP_LEVEL_COUNT 6
#define  VOXEL_FACE_COUNT 6



layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;
//layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_specular;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_normal_metalic;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_emission;
layout(input_attachment_index = 3, binding = 3, set=2) uniform subpassInput gbuffer_depth;


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

float getMinLevel(vec3 posW)
{
    float distanceToCenter = length(volume_center - posW);
    float minRadius = voxel_size * clip_map_resoultion * 0.5;
    float minLevel = log2(distanceToCenter / minRadius);
    minLevel = max(0.0, minLevel);

    float radius = minRadius * exp2(ceil(minLevel));
    float f = distanceToCenter / radius;

    //debugPrintfEXT("My float is %f %f %f", f, minLevel, minRadius);
    //   debugPrintfEXT("posw is %f %f %f", distanceToCenter / minRadius, minLevel, minRadius);
    // debugPrintfEXT("posw is %f %f %f", distanceToCenter, minRadius, volume_center.x);
    vec3 l = volume_center - posW;
    // l = vec3(1.f);
    //   debugPrintfEXT("posw is %f %f %f %f", l.x, l.y, l.z, length(l));
    //    debugPrintfEXT("posw is %f %f %f %f", l.x, l.y, l.z, length(l));
    //  debugPrintfEXT("center is %f %f %f", volume_center.x, volume_center.y, volume_center.z);


    // Smoothly transition from current level to the next level
    float transitionStart = 0.5;
    float c = 1.0 / (1.0 - transitionStart);

    return f > transitionStart ? ceil(minLevel) + (f - transitionStart) * c : ceil(minLevel);
}

void main(){
    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);
    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = subpassLoad(gbuffer_depth).x;

    vec3 world_pos = worldPosFromDepth(in_uv, depth);
    vec3 start_pos = world_pos + voxel_size * normal;

    //diffuse cone 
    vec3 indirect_contribution = vec3(0.0);
    float min_level = getMinLevel(world_pos);


    //    out_color = vec4(length(world_pos) / 500.f);
    //    .//out_color = vec4(world_pos / 500.f,1.f);
    //    return;



    for (int i = 0; i < DIFFUSE_CONE_COUNT_16; ++i)
    {
        float cos_theta = dot(normal, DIFFUSE_CONE_DIRECTIONS_16[i]);
        if (cos_theta < 0.0)
        continue;

        indirect_contribution += traceCone(start_pos, DIFFUSE_CONE_DIRECTIONS_16[i], DIFFUSE_CONE_APERTURE_16,
        MAX_TRACE_DISTANCE, min_level, 1).rgb * cos_theta;// / 3.141592;

        out_color = vec4(indirect_contribution, 1);
        // out_color = vec4(world_pos,1);
        //        return;
        //validConeCount += cos_theta;
    }
    indirect_contribution /= DIFFUSE_CONE_COUNT_16;


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    vec3 view_dir = per_frame.camera_pos - world_pos;
    vec3 indirect_specular_contribution = vec3(0.0);
    float roughness = sqrt(2.0 / (metallic + 2.0));
    //    if (metallic > 1e-6)
    //    {
    //        vec3 specular_cone_direction = reflect(-view_dir, normal);
    //        indirect_specular_contribution += traceCone(
    //        start_pos, specular_cone_direction,
    //        MIN_SPECULAR_FACTOR,
    //        MAX_TRACE_DISTANCE, min_level, voxel_size
    //        ).rgb;
    //    }
    //specular cone 
    indirect_contribution += indirect_specular_contribution;

    //calcuate sppecular contribution
    vec3 direct_contribution = vec3(0.0);

    bool has_emission = any(greaterThan(emission, vec3(1e-6)));
    //    if (has_emission)
    //    {
    //        direct_contribution+= emission;
    //    }
    //    else
    {
        // calculate Microfacet BRDF model
        // Roughness is authored as perceptual roughness; as is convention
        // convert to material roughness by squaring the perceptual roughness [2].
        // for 

        vec3 view_dir = normalize(per_frame.camera_pos - world_pos);

        PBRInfo pbr_info;
        // why use abs here?
        pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);

        pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
        pbr_info.F90 = vec3(1.0);
        pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
        //  pbr_info.alphaRoughness = 0.01f;
        pbr_info.diffuseColor = diffuse_color;




        for (uint i = 0U; i < per_frame.light_count; ++i)
        {
            vec3 light_dir = calcuate_light_dir(lights_info.lights[i], world_pos);

            vec3 half_vector = normalize(light_dir + view_dir);

            pbr_info.NdotL = clamp(dot(normal, light_dir), 0.001, 1.0);
            pbr_info.NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
            pbr_info.LdotH = clamp(dot(light_dir, half_vector), 0.0, 1.0);
            pbr_info.VdotH = clamp(dot(view_dir, half_vector), 0.0, 1.0);

            // vec3 light_contribution = microfacetBRDF(pbr_info) * calcuate_light_intensity(lights_info.lights[i], world_pos) * calcute_shadow(lights_info.lights[i], world_pos);
            vec3 light_contribution = apply_light(lights_info.lights[i], world_pos, normal) * pbr_info.diffuseColor;

            direct_contribution += light_contribution;
        }
        //  direct_contribution = vec3(1);
    }
    out_color = vec4(direct_contribution * uDirectLighting + indirect_contribution * uIndirectLighting, 1);
    out_color = vec4(direct_contribution, 1);
    //    out_color = vec4(indirect_contribution, 1);
    // out_color += vec4(diffuse_color,1);
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



vec4 traceCone(vec3 start_pos, vec3 direction, float aperture, float maxDistance, float startLevel, float stepFactor)
{
    vec4 result = vec4(0.0);
    float coneCoefficient = 2.0 * tan(aperture * 0.5);

    float curLevel = startLevel;
    float cur_voxel_size = voxel_size * exp2(curLevel);


    start_pos += direction * cur_voxel_size  * 0.5;



    float step         = 0.0;
    float diameter     = max(step * coneCoefficient, voxel_size);
    float occlusion  = 0.0;

    ivec3 faceIndex  = calculateVoxelFaceIndex(direction);

    vec3  weight     = direction * direction;

    float curSegmentLength    = cur_voxel_size;
    float minRadius        = voxel_size  * 0.5;




    while ((step < maxDistance) && (occlusion < 1.0))
    {
        vec3  position                = start_pos + direction * step;
        float distanceToVoxelCenter = length(volume_center - position);
        float min_level                = ceil(log2(distanceToVoxelCenter / minRadius));


        curLevel = log2(diameter / voxel_size);
        curLevel = min(max(max(startLevel, curLevel), min_level), CLIP_LEVEL_COUNT - 1);

        //  debugPrintfEXT("My float is %f %f %f", distanceToVoxelCenter, min_level, diameter);


        min_level = curLevel;

        //        return vec4(log2(distanceToVoxelCenter)/20.f);

        //        if (min_level == 0) return vec4(1, 0, 0, 1);
        //        if (min_level ==1) return vec4(0, 1, 0, 1);
        //        if (min_level ==2) return vec4(0, 0, 1, 1);
        //        if (min_level ==3) return vec4(1, 1, 0, 1);
        //        if (min_level ==4) return vec4(1, 0, 1, 1);
        //        if (min_level ==5) return vec4(1, 1, 1, 1);

        vec4 clipmapSample = sampleClipmapLinear(radiance_map, position, curLevel, faceIndex, weight);
        vec3 radiance = clipmapSample.rgb;

        // return clipmapSample;

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