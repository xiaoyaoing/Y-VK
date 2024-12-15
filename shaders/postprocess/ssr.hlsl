// Constants
static const float PI = 3.14159265359;
static const float PI2 = 6.28318530718;
static const float INV_PI = 0.3183098861837697;
static const float INV_TWO_PI = 0.1591549430918953;
static const float INF = 1e10;
static const float EPS = 1e-3;
static const float SHADOW_EPS = 2.0 / 65536.0;
static const float sqrt2 = 1.41421356237309504880;

// Structures
struct Frame {
    float3 tangent;
    float3 bitTangent;
    float3 n;
};

struct GlobalFrameUniform {
    matrix view_proj;
    matrix inv_view_proj;
    matrix proj;
    matrix view;
    matrix inv_proj;
    matrix inv_view;
    float3 camera_pos;
    uint light_count;
    int2 resolution;
    int2 inv_resolution;
    float roughness_scale;
    float normal_scale;
    float roughness_override;
    int use_roughness_override;
};

struct PushConstant {
    uint use_inverse_depth;
    uint use_hiz;
    uint2 screen_size;
    float depth_buffer_thickness;
    uint hiz_mip_count;
    uint show_origin;
    float padding3;
};

// Bindings
ConstantBuffer<GlobalFrameUniform> per_frame : register(b0, space0);
ConstantBuffer<PushConstant> pc : register(b1, space0);

Texture2D gbuffer_diffuse_roughness : register(t0, space1);
Texture2D gbuffer_normal_metalic : register(t1, space1);
Texture2D gbuffer_emission : register(t2, space1);
Texture2D gbuffer_depth : register(t3, space1);
Texture2D frame_color_attach : register(t4, space1);
Texture2D blue_noise : register(t5, space1);
Texture2D hiz_depth : register(t6, space1);

RWTexture2D<float4> out_image : register(u0, space2);

SamplerState samplerLinear : register(s0);

// Utility functions
float sqr(float x) {
    return x * x;
}

float3 to_local(Frame frame, float3 v) {
    return float3(
        dot(v, frame.tangent),
        dot(v, frame.bitTangent),
        dot(v, frame.n)
    );
}

float3 to_world(Frame frame, float3 v) {
    return frame.tangent * v.x + frame.bitTangent * v.y + frame.n * v.z;
}

Frame make_frame(float3 n) {
    Frame frame;
    frame.n = n;
    if (abs(n.z) < 0.999) {
        frame.tangent = normalize(cross(n, float3(0, 0, 1)));
    } else {
        frame.tangent = normalize(cross(n, float3(0, 1, 0)));
    }
    frame.bitTangent = normalize(cross(n, frame.tangent));
    return frame;
}

// GGX functions
float ggx_g1(float cos_theta, float alpha) {
    float alphaSq = alpha * alpha;
    float cosThetaSq = cos_theta * cos_theta;
    float tanThetaSq = max(1.0 - cosThetaSq, 0.0) / cosThetaSq;
    return 2.0 / (1.0 + sqrt(1.0 + alphaSq * tanThetaSq));
}

float ggx_g(float alpha, float3 wi, float3 wo, float3 wh) {
    return ggx_g1(dot(wh, wo), alpha) * ggx_g1(dot(wh, wi), alpha);
}

float ggx_d(float alpha, float3 wh, float3 wi) {
    float cos_theta = wh.z;
    float alphaSq = alpha * alpha;
    float cosThetaSq = cos_theta * cos_theta;
    float tanThetaSq = max(1.0 - cosThetaSq, 0.0) / cosThetaSq;
    float cosThetaQu = cosThetaSq * cosThetaSq;
    return alphaSq * INV_PI / (cosThetaQu * sqr(alphaSq + tanThetaSq));
}

float3 ggx_sample(float alpha, float2 rand) {
    float phi = 2.0 * PI * rand.x;
    float cosThetaSq = 1.0 / (1.0 + alpha * alpha * rand.y / (1.0 - rand.y));
    float cosTheta = sqrt(cosThetaSq);
    float sinTheta = sqrt(max(1.0 - cosThetaSq, 0.0));
    return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// SSR specific functions
float3 worldPosFromDepth(float2 uv, float depth) {
    float4 clip = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 world_w = mul(per_frame.inv_view_proj, clip);
    return world_w.xyz / world_w.w;
}

float3 MyReflect(float3 I, float3 N) {
    return -I + 2.0 * dot(I, N) * N;
}

float3 sample_reflection_dir(float3 normal, float3 world_wo, float roughness, int dispatch_thread_id) {
    if (roughness < 0.01) {
        return MyReflect(world_wo, normal);
    }
    Frame frame = make_frame(normal);
    float3 wo = to_local(frame, world_wo);
    float2 xi = blue_noise.SampleLevel(samplerLinear, float2(dispatch_thread_id.xy) / 128.0, 0).rg;
    float3 wh = ggx_sample(roughness, xi);
    float3 wi = MyReflect(wo, wh);
    return to_world(frame, wi);
}

float3 InvProjectPosition(float3 coord, matrix mat) {
    coord.y = (1 - coord.y);
    coord.xy = 2 * coord.xy - 1;
    float4 projected = mul(mat, float4(coord, 1.0));
    projected.xyz /= projected.w;
    return projected.xyz;
}

float3 ScreenSpaceToViewSpace(float3 screen_space) {
    return InvProjectPosition(screen_space, per_frame.inv_proj);
}

float3 ProjectDirection(float3 view_origin, float3 view_dir, float3 proj_origin) {
    float3 view_target = view_origin + view_dir;
    float4 proj_target = mul(per_frame.proj, float4(view_target, 1.0));
    proj_target.xyz /= proj_target.w;
    
    proj_target.xy = proj_target.xy * 0.5 + 0.5;
    proj_target.y = 1 - proj_target.y;
    
    return normalize(proj_target.xyz - proj_origin);
}

// Add these functions before the main() function

uint jenkinsHash(uint a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

float3 pseudocolor(uint value) {
    if (value == 0) return float3(1, 0, 0);
    if (value == 1) return float3(0, 1, 0);
    if (value == 2) return float3(0, 0, 1);
    if (value == 3) return float3(1, 1, 0);
    if (value == 4) return float3(1, 0, 1);
    uint h = jenkinsHash(value);
    return float3(h & 0xff, (h >> 8) & 0xff, (h >> 16) & 0xff) / 255.0;
}

void ssr_initial_advance_ray(float3 origin, float3 direction, float3 inv_direction, 
                           float2 current_mip_resolution, float2 current_mip_resolution_inv, 
                           float2 floor_offset, float2 uv_offset, 
                           out float3 position, out float current_t) {
    float2 current_mip_position = current_mip_resolution * origin.xy;
    float2 xy_plane = floor(current_mip_position) + floor_offset;
    xy_plane = xy_plane * current_mip_resolution_inv + uv_offset;
    
    float2 t = xy_plane * inv_direction.xy - origin.xy * inv_direction.xy;
    current_t = min(t.x, t.y);
    position = origin + current_t * direction;
}

static const float FFX_SSSR_FLOAT_MAX = 3.402823466e+38;

bool ssr_advance_ray(float3 origin, float3 direction, float3 inv_direction,
                    float2 current_mip_position, float2 current_mip_resolution_inv,
                    float2 floor_offset, float2 uv_offset, float surface_z,
                    out float3 position, out float current_t) {
    float2 xy_plane = floor(current_mip_position) + floor_offset;
    xy_plane = xy_plane * current_mip_resolution_inv + uv_offset;
    float3 boundary_planes = float3(xy_plane, surface_z);
    
    float3 t = boundary_planes * inv_direction - origin * inv_direction;
    
    if (pc.use_inverse_depth > 0)
        t.z = direction.z < 0 ? t.z : FFX_SSSR_FLOAT_MAX;
    else
        t.z = direction.z > 0 ? t.z : FFX_SSSR_FLOAT_MAX;
    
    float t_min = min(min(t.x, t.y), t.z);
    
    bool above_surface;
    if (pc.use_inverse_depth > 0)
        above_surface = surface_z < position.z;
    else
        above_surface = surface_z > position.z;
    
    bool skipped_tile = asuint(t_min) != asuint(t.z) && above_surface;
    current_t = above_surface ? t_min : current_t;
    position = origin + current_t * direction;
    
    return skipped_tile;
}

float2 get_mip_resolution(float2 screen_size, int mip) {
    float2 mip_resolution = screen_size;
    mip_resolution = max(mip_resolution / exp2(float(mip)), 1.0);
    return mip_resolution;
}

float GetHizDepth(float2 uv, int mip) {
    return hiz_depth.SampleLevel(samplerLinear, uv, mip).x;
}

float3 ssr_hierarch_ray_march(float3 origin, float3 direction, float2 screen_size, out bool valid_hit) {
    const float3 inv_direction = 1.0 / direction;
    int current_mip = 0;
    
    float2 current_mip_resolution = get_mip_resolution(screen_size, current_mip);
    float2 current_mip_resolution_inv = 1.0 / current_mip_resolution;
    
    float current_t;
    float3 position;
    
    float2 floor_offset = float2(1, 1);
    float2 uv_offset = 0.005 * exp2(0) / screen_size;
    
    if (direction.x < 0) {
        floor_offset.x = 0;
        uv_offset.x = -uv_offset.x;
    }
    if (direction.y < 0) {
        floor_offset.y = 0;
        uv_offset.y = -uv_offset.y;
    }
    
    ssr_initial_advance_ray(origin, direction, inv_direction, current_mip_resolution,
                          current_mip_resolution_inv, floor_offset, uv_offset,
                          position, current_t);
    
    uint max_traversal_intersections = 64;
    uint i = 0;
    while (i < max_traversal_intersections && current_mip >= 0) {
        float2 current_mip_position = current_mip_resolution * position.xy;
        float surface_z = GetHizDepth(current_mip_position, current_mip);
        
        bool skipped_tile = ssr_advance_ray(origin, direction, inv_direction,
                                          current_mip_position, current_mip_resolution_inv,
                                          floor_offset, uv_offset, surface_z,
                                          position, current_t);
        
        current_mip += skipped_tile ? 1 : -1;
        current_mip_resolution *= skipped_tile ? 0.5 : 2;
        current_mip_resolution_inv *= skipped_tile ? 2 : 0.5;
        
        ++i;
    }
    
    valid_hit = (i <= max_traversal_intersections);
    return position;
}

float ValidHit(float3 hit, float2 uv, float3 world_space_ray_direction, float depth_buffer_thickness) {
    if (any(hit.xy > 1.0 + 1e-4) || any(hit.xy < -1e-4))
        return 0;
        
    float2 distance_TS = abs(uv - hit.xy);
    if (all(distance_TS < (4.0/pc.screen_size)))
        return 0;
        
    float surface_depth = gbuffer_depth.SampleLevel(samplerLinear, uv, 0).x;
    if (surface_depth == 1)
        return 0;
        
    float3 hit_normal = normalize(2.0 * gbuffer_normal_metalic.SampleLevel(samplerLinear, uv, 0).xyz - 1.0);
    
    if (dot(hit_normal, world_space_ray_direction) < 0)
        return 0;
        
    float3 view_space_surface_pos = ScreenSpaceToViewSpace(float3(uv, surface_depth));
    float3 view_space_hit_pos = ScreenSpaceToViewSpace(hit);
    
    float distance = length(view_space_hit_pos - view_space_surface_pos);
    
    float2 fov = 0.05 * float2(pc.screen_size.y / pc.screen_size.x, 1);
    
    float2 border;
    border.x = smoothstep(0, fov.x, hit.x) * (1 - smoothstep(1 - fov.x, 1, hit.x));
    border.y = smoothstep(0, fov.y, hit.y) * (1 - smoothstep(1 - fov.y, 1, hit.y));
    float vignette = border.x * border.y;
    
    float confidence = 1 - smoothstep(0, 1.5, distance);
    confidence *= confidence;
    
    return vignette * confidence;
}

// Main compute shader
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= pc.screen_size.x || DTid.y >= pc.screen_size.y) {
        return;
    }

    float2 uv = float2(DTid.xy) / float2(pc.screen_size);
    float4 diffuse_roughness = gbuffer_diffuse_roughness.SampleLevel(samplerLinear, uv, 0);
    float4 normal_metalic = gbuffer_normal_metalic.SampleLevel(samplerLinear, uv, 0);
    float3 normal = normalize(2.0 * normal_metalic.xyz - 1.0);
    float depth = gbuffer_depth.SampleLevel(samplerLinear, uv, 0).x;

    if (depth == 1) {
        float4 color = frame_color_attach.SampleLevel(samplerLinear, uv, 0);
        out_image[DTid.xy] = float4(color.rgb, 1.0);
        return;
    }

    float3 world_pos = worldPosFromDepth(float2(uv.x, 1-uv.y), depth);
    float3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    float3 screen_origin = float3(uv, depth);
    float3 view_normal = normalize(mul(per_frame.view, float4(normal, 0.0)).xyz);
    float3 view_origin = InvProjectPosition(screen_origin, per_frame.inv_proj);
    float3 view_ray_dir = normalize(view_origin);
    float3 reflect_dir_view = sample_reflection_dir(view_normal, -view_ray_dir, perceptual_roughness, DTid.x);
    float3 ray_dir_TS = ProjectDirection(view_origin, reflect_dir_view, screen_origin);

    bool valid_hit;
    float3 position;
    
    if (pc.use_hiz == 2) {
        float max_trace_distance = ray_dir_TS.x >= 0 ? 
            (1-screen_origin.x)/ray_dir_TS.x : (-screen_origin.x)/ray_dir_TS.x;
        max_trace_distance = min(max_trace_distance, 
            ray_dir_TS.y >= 0 ? (1-screen_origin.y)/ray_dir_TS.y : (-screen_origin.y)/ray_dir_TS.y);
        max_trace_distance = min(max_trace_distance,
            ray_dir_TS.z >= 0 ? (1-screen_origin.z)/ray_dir_TS.z : (-screen_origin.z)/ray_dir_TS.z);
        position = ssr_hierarch_ray_march(screen_origin, ray_dir_TS, pc.screen_size, valid_hit);
    }
    else {
        // Implement linear ray march if needed
        position = screen_origin;
        valid_hit = false;
    }
    
    float3 world_space_hit = worldPosFromDepth(float2(position.x, 1-position.y), position.z);
    float3 world_space_ray_direction = normalize(world_space_hit - world_pos);
    
    float confidence = valid_hit ? 
        ValidHit(position, uv, world_space_ray_direction, pc.depth_buffer_thickness) : 0;
    
    float3 reflection_radiance = 0;
    if (confidence > 0) {
        reflection_radiance = frame_color_attach.SampleLevel(samplerLinear, position.xy, 0).rgb;
    }
    
    if (pc.show_origin > 0) {
        screen_origin.x = 0;
        screen_origin.y = 0;
        reflection_radiance = screen_origin;
    }
    
    out_image[DTid.xy] = float4(reflection_radiance, 1.0);
} 