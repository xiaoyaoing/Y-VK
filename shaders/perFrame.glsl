layout(set = 0, binding = 0) uniform _GlobalFrameUniform {
    mat4 view_proj;
    mat4 inv_view_proj;

    ivec2 resolution;
    ivec2 inv_resolution;

    uint light_count;
} per_frame;

layout(set = 0, binding = 1) uniform _GlobalPrimitiveUniform {
    mat4 model;
} per_primitive;
