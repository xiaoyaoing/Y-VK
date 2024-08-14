//#if defined(SHADOW_GLSL)
//#define SHADOW_GLSL
#include "lighting.glsl"
#include "perFrameShading.glsl"

//uniform sampler2D shadow_maps[4];

float  calcute_shadow(in Light light, vec3 world_pos){
    int shadow_map_index = int(light.info.z);
    mat4 mvp = light.matrix;
    vec4 clipPos = mvp * vec4(world_pos, 1.0);
    vec3 ndcPos = clipPos.xyz / clipPos.w;
    vec3 shadowCoord = ndcPos * 0.5 + 0.5;
    float  shadow = 1.0;
    
   // if(shadowCoord.x > 1 || shadowCoord.y > 1 || shadowCoord.z > 1){
   // return vec3(1,0,0);
  //  }
   // return shadowCoord;
    if (shadowCoord.z > 1.0)
    {
        shadow = 0.0;
    }
    else
    {
        float depth = texture(scene_textures[shadow_map_index], shadowCoord.xy).r;
        if (shadowCoord.z < depth + 0.001f)
        {
            shadow = 1.0;
        }
        
        else {
        shadow = 0.0;
        }
        
      //  return (shadowCoord.z-depth) * 10.f;
        //return depth;
    }
    return shadow;
}




//#endif