layout(set = 0, binding = 0) uniform _GlobalUniform {
    mat4 view_proj;
    mat4 view_proj_inv;
} per_frame;

layout(set = 0, binding = 1) uniform _GlobalUniform {
    mat4 model;
} per_primitve;
