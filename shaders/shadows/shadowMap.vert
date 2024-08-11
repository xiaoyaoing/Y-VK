layout(location = 0) in vec3 position;
layout(location = 1) in uint primitive_id;

layout(pushing) uniform PushConstants {
    mat4 mvp;
};

struct PerPrimitive {
    mat4 model;
    mat4 modelIT;
    uint material_index;
    uint padding1;
    uint padding2;
    uint padding3;
};


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    gl_Position = mvp * primitive_infos[primitive_id].model * vec4(position, 1.0f);
}