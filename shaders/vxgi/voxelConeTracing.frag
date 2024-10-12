#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "../perFrame.glsl"
#include "../shadow.glsl"
#include "../brdf.glsl"

precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(set=1, binding = 0)  uniform sampler3D radiance_map;


layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

#define DEBUG_DIFFUSE_ONLY                0
#define DEBUG_SPECULAR_ONLY                1
#define DEBUG_NORMAL_ONLY                    2
#define DEBUG_MIN_LEVEL_ONLY                3
#define DEBUG_DIRECT_CONTRIBUTION_ONLY        4
#define DEBUG_INDIRECT_DIFFUSE_ONLY        5
#define DEBUG_INDIRECT_SPECULAR_ONLY        6
#define DEBUG_AMBIENT_OCCLUSION_ONLY        7
#define DEBUG_GI_OUTPUT                    8

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
    float   uDirectLighting;//56
    float fopacityScale;
    float fmaxTraceDistance;
    uint  debugMode;
//    bool useLowerLevel;
//    bool useHigherLevel;
//    bool usemix;
//    bool padding;
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
const float MIN_SPECULAR_APERTURE = 0.05;



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

const float MIN_SPECULAR_FACTOR  = 0.05f;


const vec3 SIX_LEVEL_DEBUG_COLORS[6] = {
vec3(1, 0, 0),
vec3(0, 1, 0),
vec3(0, 0, 1),
vec3(1, 1, 0),
vec3(1, 0, 1),
vec3(0, 1, 1)
};


vec3  worldPosFromDepth    (float depth);
vec4  traceCone            (vec3 startPos, vec3 direction, float aperture,
float maxDistance, float startLevel, float stepFactor);
//float calcmin_level        (vec3 worldPos);
vec4  minLevelToColor    (float min_level);

float getMinLevel(vec3 posW)
{
    float distanceToCenter = length(volume_center - posW);
    //    float distanceToCenter = length(posW);
    float minRadius = voxel_size * clip_map_resoultion * 0.5;
    float minLevel = log2(distanceToCenter / minRadius);
    minLevel = max(0.0, minLevel);

    //    return minLevel;

    float radius = minRadius * exp2(ceil(minLevel));
    float f = distanceToCenter / radius;

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

    if (depth == 1.0)
    {
        out_color = vec4(0.0);
        return;
    }

    vec3 world_pos = worldPosFromDepth(in_uv, depth);

    //diffuse cone 
    vec4 indirect_diffuse_contribution = vec4(0, 0, 0, 1);
    float min_level = getMinLevel(world_pos);
    float cur_voxel_size = voxel_size * exp2(min_level);

    vec3 start_pos = world_pos + cur_voxel_size * normal * uTraceStartOffset;// * 3;


    if (uEnable32Cones > 0)
    { for (int i = 0; i < DIFFUSE_CONE_COUNT_32; ++i)
    {
        float cos_theta = dot(normal, DIFFUSE_CONE_DIRECTIONS_32[i]);

        //cos_theta = abs(cos_theta);
        if (cos_theta < 0.0)
        continue;

        indirect_diffuse_contribution +=
        traceCone(start_pos, DIFFUSE_CONE_DIRECTIONS_32[i], DIFFUSE_CONE_APERTURE_32,
        fmaxTraceDistance, min_level, 1) *
        cos_theta;// / 3.141592;
    }
        indirect_diffuse_contribution /= DIFFUSE_CONE_COUNT_32; }
    else {
        for (int i = 0; i < DIFFUSE_CONE_COUNT_16; ++i)
        {
            float cos_theta = dot(normal, DIFFUSE_CONE_DIRECTIONS_16[i]);
            //  cos_theta = abs(cos_theta);

            if (cos_theta < 0.0)
            continue;

            indirect_diffuse_contribution += traceCone(start_pos, DIFFUSE_CONE_DIRECTIONS_16[i], DIFFUSE_CONE_APERTURE_16,
            fmaxTraceDistance, min_level, 1) * cos_theta;// / 3.141592;
        }
        indirect_diffuse_contribution /= DIFFUSE_CONE_COUNT_16;
    }

    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;
    float roughness = sqrt(2.0 / (metallic + 2.0));


    vec3 view_dir = per_frame.camera_pos - world_pos;
    vec3 specular_reflect_dir = reflect(-view_dir, normal);
    specular_reflect_dir = normalize(specular_reflect_dir);

    vec4 indirect_specular_contribution= traceCone(start_pos, specular_reflect_dir, max(roughness, MIN_SPECULAR_APERTURE), fmaxTraceDistance, min_level, 1) * uIndirectSpecularIntensity;
    indirect_specular_contribution.rgb *=  mix(vec3(0.04), diffuse_color, metallic);





    //specular cone 
    //  indirect_contribution += indirect_specular_contribution;

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
            vec3 light_contribution = apply_light(lights_info.lights[i], world_pos, normal) * microfacetBRDF(pbr_info) * calcute_shadow(lights_info.lights[i], world_pos);
            //            light_contribution = calcute_shadow(lights_info.lights[i], world_pos);
            direct_contribution += light_contribution;
        }
        //  direct_contribution = vec3(1);
    }
    direct_contribution *= indirect_diffuse_contribution.a;
    direct_contribution *= uDirectLighting;

    indirect_diffuse_contribution.rgb *= uIndirectDiffuseIntensity;
    indirect_diffuse_contribution.rgb *= diffuse_color;

    if (debugMode == DEBUG_DIFFUSE_ONLY)
    {
        out_color = vec4(indirect_diffuse_contribution.rgb, 1);
    }
    else if (debugMode == DEBUG_SPECULAR_ONLY)
    {
        out_color = vec4(direct_contribution, 1);
    }
    else if (debugMode == DEBUG_NORMAL_ONLY)
    {
        out_color = vec4(normal * 0.5 + 0.5, 1);
    }
    else if (debugMode == DEBUG_MIN_LEVEL_ONLY)
    {
        out_color = minLevelToColor(min_level);
    }
    else if (debugMode == DEBUG_DIRECT_CONTRIBUTION_ONLY)
    {
        out_color = vec4(direct_contribution, 1);
    }
    else if (debugMode == DEBUG_INDIRECT_DIFFUSE_ONLY)
    {
        out_color = vec4(indirect_diffuse_contribution.rgb, 1);
    }
    else if (debugMode == DEBUG_INDIRECT_SPECULAR_ONLY)
    {
        out_color = vec4(indirect_specular_contribution.rgb, 1);
    }
    else if (debugMode == DEBUG_AMBIENT_OCCLUSION_ONLY)
    {
        out_color = vec4(indirect_diffuse_contribution.a);
    }
    else if (debugMode == DEBUG_GI_OUTPUT)
    {
        out_color = vec4(direct_contribution + indirect_diffuse_contribution.rgb + indirect_specular_contribution.rgb, 1);
    }

    //    out_color.rgb = world_pos/20.f;
}



vec4 sampleClipmap(sampler3D clipmap, vec3 worldPos, int clipmapLevel, vec3 faceOffset, vec3 weight)
{


    float cur_level_voxel_size = voxel_size * exp2(clipmapLevel);
    float extent    =  cur_level_voxel_size * clip_map_resoultion;

    vec3 samplePos = (fract(worldPos / extent) * clip_map_resoultion) / (float(clip_map_resoultion));

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

    start_pos += direction * cur_voxel_size  * 0.5 * uTraceStartOffset;

    //   start_pos += direction * cur_voxel_size  * 0.5;
    float step         = 0.0;
    float diameter     = max(step * coneCoefficient, voxel_size);
    float occlusion  = 0.0;

    ivec3 faceIndex  = calculateVoxelFaceIndex(direction);

    vec3  weight     = direction * direction;

    float curSegmentLength    = cur_voxel_size;
    float minRadius        = voxel_size  * 0.5 * clip_map_resoultion;

    //    maxDistance = 1000.f;
    while ((step < maxDistance) && (occlusion < 0.95))
    {
        vec3  position                = start_pos + direction * step;
        float distanceToVoxelCenter = length(volume_center - position);
        float min_level                = ceil(log2(distanceToVoxelCenter / minRadius));

        curLevel = log2(diameter / voxel_size);
        //curLevel = min_level
        curLevel = min(max(max(startLevel, curLevel), min_level), CLIP_LEVEL_COUNT - 1);


        min_level = curLevel;
        //        if (min_level_int == 0) return vec4(1, 0, 0, 1);
        //        if (min_level_int ==1) return vec4(0, 1, 0, 1);
        //        if (min_level_int ==2) return vec4(0, 0, 1, 1);
        //        if (min_level_int ==3) return vec4(1, 1, 0, 1);
        //        if (min_level_int ==4) return vec4(1, 0, 1, 1);
        //        if (min_level_int ==5) return vec4(1, 1, 1, 1);
        vec4 clipmapSample = sampleClipmapLinear(radiance_map, position, curLevel, faceIndex, weight);


        // return clipmapSample;

        vec3 radiance = clipmapSample.rgb;
        //        clipmapSample.a = 1;

        float opacity = clipmapSample.a * fopacityScale;

        cur_voxel_size = voxel_size * exp2(curLevel);

        float correction = curSegmentLength / cur_voxel_size;
        //todo what does correction do?
        radiance = radiance * correction;

        //        if(step>2.f && radiance.x > 0)
        //            debugPrintfEXT("radiance %f %f %f\n", radiance.r, radiance.g, radiance.b); 
        //        if(step>2.f)
        //            debugPrintfEXT("curlevel %f step %f cur_voxel_size %f\n", curLevel, step, cur_voxel_size);

        float dist           = max(step * 0.3f, 1);
        float atten          = 1.0 / (dist * dist);
        // radiance.rgb *= atten;

        //  return clipmapSample;

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
    //    debugPrintfEXT("result rgb occlusion %f %f %f %f\n", result.r, result.g, result.b, occlusion);
    //    step = step/30.f;
    //    return vec4(step,step,step,1.0);
    return vec4(result.rgb, 1.0 - occlusion);
}

vec4 minLevelToColor(float minLevel)
{
    vec4 colors[] = {
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(1.0, 1.0, 0.0, 1.0),
    vec4(0.0, 1.0, 1.0, 1.0),
    vec4(1.0, 0.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0)
    };

    int lowerLevel = int(floor(minLevel));
    vec4 minLevelColor = vec4(mix(colors[lowerLevel], colors[lowerLevel + 1], fract(minLevel)));
    return minLevelColor * 0.5;
}