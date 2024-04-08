layout(set = 0, binding = 0) uniform _GlobalFrameUniform {
    mat4 view_proj;
    mat4 inv_view_proj;

    vec3 camera_pos;

    uint light_count;


    ivec2 resolution;
    ivec2 inv_resolution;

} per_frame;


//layout(set = 0, binding = 1) uniform _GlobalPrimitiveUniform {
//    mat4 model;
//} per_primitive;


struct PerPrimitive {
    mat4 model;
    mat4 modelIT;
    uint material_index;
//    uvec3 padding1;
//    uvec4 padding2;
    uint padding1;
    uint padding2;
    uint padding3;
};


vec3  worldPosFromDepth    (vec2 uv, float depth){
    vec4  clip         = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 world_w = per_frame.inv_view_proj * clip;
    vec3 pos     = world_w.xyz / world_w.w;

    return pos;
}




