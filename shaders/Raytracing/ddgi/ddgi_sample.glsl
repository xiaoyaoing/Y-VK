#define NUM_THREADS_X 16
#define NUM_THREADS_Y 16
#define PROBE_SIDE_LENGTH 16
#define PROBE_WITH_BORDER_SIDE 18
#define CACHE_SIZE 32

uvec3 get_probe_coord_by_index(uint probe_idx){
    uvec3 probe_counts  = ddgi_ubo.probe_counts;
    return uvec3(probe_idx % probe_counts.x, (probe_idx / probe_counts.x) % probe_counts.y, probe_idx / (probe_counts.x * probe_counts.y));
}

vec3 get_position_by_grid(uvec3 grid){
    float  probe_grid_size = 0.5f;
    vec3 probe_grid_origin =ddgi_ubo.probe_start_position;
    return probe_grid_origin + vec3(grid) * probe_grid_size;
}

//vec3 probe_location(uint probe_idx){
//    return get_position_by_grid(get_probe_coord_by_index(probe_idx));
//}

uvec2 get_pixel_coord_top_left(uvec3 probe_coord){
    return probe_coord.xz * PROBE_WITH_BORDER_SIDE + uvec2(1, 1) + uvec2(probe_coord.y * ddgi_ubo.probe_counts.x * PROBE_WITH_BORDER_SIDE, 0);
}


vec2 normalized_oct_coord(uvec2 oct_coord) {
    // Local oct texture coords
    return (vec2(oct_coord + 0.5)) * (2.0f / float(PROBE_SIDE_LENGTH)) -
    vec2(1.0f, 1.0f);
}

// Returns +/-1
vec2  sign_not_zero(vec2 v)
{
    return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

vec3 oct_decode(vec2 oct_coord) {
    vec3 v = vec3(oct_coord.xy, 1.0 - abs(oct_coord.x) - abs(oct_coord.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
    return normalize(v);
}

vec2 encode_to_oct(vec3 dir){ 
    vec2 oct = vec2(dir.xy / (abs(dir.x) + abs(dir.y) + abs(dir.z)));
    return oct;
}

vec2 get_probe_color_uv(uvec3 probe_coord,vec3 dir){
    vec2 pixel_coord = vec2(get_pixel_coord_top_left(probe_coord));
    pixel_coord += (encode_to_oct(dir) + vec2(1.0)) * 0.5 * float(PROBE_SIDE_LENGTH);
    return pixel_coord / vec2(ddgi_ubo.irradiance_width, ddgi_ubo.irradiance_height);
}
