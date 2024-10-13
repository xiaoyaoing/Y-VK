//#if defined(SHADOW_GLSL)
//#define SHADOW_GLSL
#include "lighting.glsl"
#include "perFrameShading.glsl"

//uniform sampler2D shadow_maps[4];

#define ambient 0

float textureProj(vec4 shadowCoord, vec2 off,int shadowMapTexture)
{
    float shadow = 1.0;
    float dist = texture(scene_textures[shadowMapTexture], shadowCoord.xy + off).r;
    
//    return dist;
//    return shadowCoord.z;
//    return dist - shadowCoord.z + 0.001f> 0? 1:0;
  //  return dist < shadowCoord.z ? 0.0 : a'b's
  //  return abs(dist - shadowCoord.z);// * 10.f;
        if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 )
    {
//         dist = texture(scene_textures[shadowMapTexture], shadowCoord.xy + off).r;

        if ( shadowCoord.w > 0.0 && dist < shadowCoord.z - 0.03f)
        {
//                    debugPrintfEXT("dist shadowcoord.z %f %f\n",dist,shadowCoord.z);

            shadow = ambient;
        }
    }
    else {
            //debugPrintfEXT("shadowCoord x y z w %f %f %f %f\n",shadowCoord.x,shadowCoord.y,shadowCoord.z,shadowCoord.w);
        }
    return shadow;
}

float filterPCF(vec4 sc,int texture_idx)
{
    ivec2 texDim;
    texDim = textureSize(scene_textures[texture_idx], 0);
    
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += textureProj(sc, vec2(dx*x, dy*y),texture_idx);
            count++;
        }

    }
    return shadowFactor / count;
}
                   
const mat4 biasMat = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0 );

vec3  calcute_shadow(in Light light, vec3 world_pos){
    int shadow_map_index = int(light.info.z);
    
    if(shadow_map_index <0){
        return vec3(1.0);
    }
//    return vec3(1,0,0);
    mat4 mvp = light.matrix;
    
//    vec4 shadowCoord = biasMat * mvp * vec4(world_pos, 1.0);
    //shadowCoord = shadowCoord / shadowCoord.w;
//
    DirectionalLightShadowDesc lightShadowDesc = light.shadow_desc;
    vec3 lightSpacePos = (lightShadowDesc.view * vec4(world_pos, 1.0)).xyz;
//    debugPrintfEXT("lightSpacePos1 %f %f %f1\n",lightSpacePos.x,lightSpacePos.y,lightSpacePos.z);
    lightSpacePos.z /= lightShadowDesc.zFar - lightShadowDesc.zNear;
   // debugPrintfEXT("lightSpacePos %f %f %f\n",lightSpacePos.x,lightSpacePos.y,lightSpacePos.z);
    lightSpacePos.xy = (lightShadowDesc.proj * vec4(lightSpacePos.xy, 0.0, 1.0)).xy;
    lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5;
//    lightSpacePos.z = abs(lightSpacePos.z);
    vec4 shadowCoord = vec4(lightSpacePos, 1);
    //shadowCoord.z = abs(shadowCoord.z);
    
//    debugPrintfEXT("shadowCoord %f %f %f %f\n",shadowCoord.x,shadowCoord.y,shadowCoord.z,shadowCoord.w);
//    shadowCoord.y = 1.0 - shadowCoord.y;
    
  //  return world_pos / 10.f;
  //  return 2 * vec3(shadowCoord.z - 0.9f);
    //    return shadowCoord.xyz;
    return vec3(filterPCF(shadowCoord, shadow_map_index));
    
//    if (shadowCoord.z > 1.0)
//    {
//        shadow = 0.0;
//    }
//    else
//    {
//        float depth = texture(scene_textures[shadow_map_index], shadowCoord.xy).r;
//        if (shadowCoord.z < depth + 0.001f)
//        {
//            shadow = 1.0;
//        }
//        
//        else {
//        shadow = 0.0;
//        }
//        
//      //  return (shadowCoord.z-depth) * 10.f;
//        //return depth;
//    }
//    return shadow;
}




//#endif