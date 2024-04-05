#include "gltfMaterial.glsl"

#define MAX_SCENE_TEXTURES  25
#define MAX_SCENE_MATERIALS 25

layout (std430, set=0, binding = 3) readonly buffer  MaterialBuffer
{
    GltfMaterial scene_materials[MAX_SCENE_MATERIALS];
};

layout(set =0, binding =6) uniform sampler2D scene_textures[MAX_SCENE_TEXTURES];                         