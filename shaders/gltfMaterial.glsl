#if !defined(GLTF_GLSL)
#define GLTF_GLSL


#if defined(__cplusplus)
#include <glm.hpp>
using vec4 = glm:: vec4;
using vec3 = glm:: vec3;
#endif

struct GltfMaterial
{
    vec4  pbrBaseColorFactor;// 16
    int   pbrBaseColorTexture;// 20
    float pbrMetallicFactor;// 24
    float pbrRoughnessFactor;// 28 
    int   pbrMetallicRoughnessTexture;// 32
    int   emissiveTexture;// 36
    int   alphaMode;// 40
    float alphaCutoff;// 56
    int   doubleSided;// 60
    vec3  emissiveFactor;// 52
    int   normalTexture;// 64
    float normalTextureScale;// 68
    int   occlusionTexture;// 72
    float occlusionTextureStrength;// 76
    int padding;// 80 
};

#endif
