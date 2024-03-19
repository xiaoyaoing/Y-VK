layout(set = 0, binding = 0) uniform _GlobalFrameUniform {
    mat4 view_proj;
    mat4 inv_view_proj;

    vec3 camera_pos;

    uint light_count;


    ivec2 resolution;
    ivec2 inv_resolution;

} per_frame;


layout(set = 0, binding = 1) uniform _GlobalPrimitiveUniform {
    mat4 model;
} per_primitive;


struct PerPrimitive {
    mat4 model;
    uint material_index;
    uvec3 padding1;
    uvec4 padding2;
};







