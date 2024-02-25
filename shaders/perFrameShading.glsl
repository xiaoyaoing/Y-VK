#include "gltfMaterial.glsl"

#define MAX_SCENE_TEXTURES 128

layout ( std430,  binding = 2) readonly buffer MaterialBuffer
{
    GltfMaterial scene_materials[];
};

layout(set =1,binding =0) uniform sampler2D scene_textures[MAX_SCENE_TEXTURES];                         