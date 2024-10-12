#version 430
#extension GL_EXT_debug_printf : enable

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

layout(binding = 0, set= 1) uniform sampler3D u_3dTexture;
layout(std140, binding = 0) uniform GlobalParams {
    mat4 u_viewProj;

    ivec3 u_imageMin;
    int u_hasPrevClipmapLevel;

    ivec3 u_regionMin;
    int u_clipmapResolution;

    ivec3 u_prevRegionMin;
    int u_clipmapLevel;

    ivec3 u_prevRegionMax;
    float u_voxelSize;

    int u_hasMultipleFaces;
    int u_numColorComponents;

    vec3  u_prevRegionMinWorld;
    int       u_padding;
    vec3  u_prevRegionMaxWorld;
    int       u_padding2;
};


const float EPSILON = 0.00001;

#define VOXEL_TEXTURE_WITH_BORDER

const int BORDER_WIDTH = 1;

struct Geometry
{
    vec4 color;
    vec2 uv;
};
layout(location =0) out Geometry  Out;

vec3 toWorld(ivec3 p)
{
    //  return p;
    return p * u_voxelSize;
}

void createQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3, vec4 color)
{

    if(color.x <= EPSILON && color.y <= EPSILON && color.z <= EPSILON){
        return;
        color.xyz = vec3(0,1,0);
    }
    
//    if(u_clipmapLevel == 0 ){
//        color.xyz = vec3(1,0,0);
//    }
//    if(u_clipmapLevel == 1 ){
//        color.xyz = vec3(0,1,0);
//    }
//    if(u_clipmapLevel == 2 ){
//        color.xyz = vec3(0,0,1);
//    }
//    if(u_clipmapLevel == 3 ){
//        color.xyz = vec3(1,1,0);
//    }
//    if(u_clipmapLevel == 4 ){
//        color.xyz = vec3(1,0,1);
//    }
    
    gl_Position = v0;
    Out.color = color;
    Out.uv = vec2(0.0, 0.0);
    EmitVertex();
    gl_Position = v1;    Out.color = color;

    Out.uv = vec2(0.0, 1.0);
    EmitVertex();
    gl_Position = v3;    Out.color = color;

    Out.uv = vec2(1.0, 0.0);
    EmitVertex();
    gl_Position = v2;    Out.color = color;
        
    Out.uv = vec2(1.0, 1.0);
    EmitVertex();
    EndPrimitive();
    

}

bool GenerateVoxel(vec4 color){
    return color.a >= EPSILON || color.r >= EPSILON || color.g >= EPSILON || color.b >= EPSILON;
}

void main()
{
    ivec3 pos = ivec3(gl_in[0].gl_Position.xyz);
    ivec3 posV = pos + u_regionMin;

    // Expecting u_prevRegionMin and u_prevVolumeMax in the voxel coordinates of the current region
//    if (u_hasPrevClipmapLevel > 0 && (all(greaterThanEqual(posV, u_prevRegionMin)) && all(lessThan(posV, u_prevRegionMax))))
//        return;

    if (u_clipmapLevel == 2){
        //  debugPrintfEXT("pos %d %d %d\n", pos.x, pos.y, pos.z);
    }

    // Same as: (u_imageMin + pos) % u_clipmapResolution - the bitwise version is faster than %
    ivec3 samplePos = (u_imageMin + pos) % u_clipmapResolution;
//    samplePos = pos;

    int resolution = u_clipmapResolution;

    // Target correct clipmap level
    samplePos.y += u_clipmapLevel * resolution;

    vec4 colors[6];

    if (u_hasMultipleFaces > 0)
    {
        colors = vec4[6] (
        texelFetch(u_3dTexture, samplePos, 0),
        texelFetch(u_3dTexture, samplePos + ivec3(resolution, 0, 0), 0),
        texelFetch(u_3dTexture, samplePos + ivec3(2 * resolution, 0, 0), 0),
        texelFetch(u_3dTexture, samplePos + ivec3(3 * resolution, 0, 0), 0),
        texelFetch(u_3dTexture, samplePos + ivec3(4 * resolution, 0, 0), 0),
        texelFetch(u_3dTexture, samplePos + ivec3(5 * resolution, 0, 0), 0));
    }
    else
    {
        vec4 color = texelFetch(u_3dTexture, samplePos, 0);
//        color = vec4(0.5,0.5,0.5,1);
        colors = vec4[6] (color, color, color, color, color, color);
    }
    
//    if(colors[0].r !=1.0f && colors[0].r != 0.0f)
//        debugPrintfEXT("color %f %f %f %f\n", colors[0].r, colors[0].g, colors[0].b, colors[0].a);

    vec4 v0 = u_viewProj * vec4(toWorld(posV), 1.0);
    vec4 v1 = u_viewProj * vec4(toWorld(posV + ivec3(1, 0, 0)), 1.0);
    vec4 v2 = u_viewProj * vec4(toWorld(posV + ivec3(0, 1, 0)), 1.0);
    vec4 v3 = u_viewProj * vec4(toWorld(posV + ivec3(1, 1, 0)), 1.0);
    vec4 v4 = u_viewProj * vec4(toWorld(posV + ivec3(0, 0, 1)), 1.0);
    vec4 v5 = u_viewProj * vec4(toWorld(posV + ivec3(1, 0, 1)), 1.0);
    vec4 v6 = u_viewProj * vec4(toWorld(posV + ivec3(0, 1, 1)), 1.0);
    vec4 v7 = u_viewProj * vec4(toWorld(posV + ivec3(1, 1, 1)), 1.0);


    // X Axis left face of the cube
    if (GenerateVoxel(colors[0]))
    {
        //   Out.color = colors[0];
        createQuad(v0, v2, v6, v4, colors[0]);
    }


    // X Axis right face of the cube
    if (GenerateVoxel(colors[1]))
    {
        //  Out.color = colors[1];
        createQuad(v1, v5, v7, v3, colors[1]);
    }

    // Y Axis bottom face of the cube
    if (GenerateVoxel(colors[2]))
    {
        //  Out.color = colors[2];
        createQuad(v0, v4, v5, v1, colors[2]);
    }

    // Y Axis top face of the cube
    if (GenerateVoxel(colors[3]))
    {
        //  Out.color = colors[3];
        createQuad(v2, v3, v7, v6, colors[3]);
    }

    // Z Axis front face of the cube
    if (GenerateVoxel(colors[4]))
    {
        // Out.color = colors[4];
        createQuad(v0, v1, v3, v2, colors[4]);
    }

    // Z Axis back face of the cube
    if (GenerateVoxel(colors[5]))
    {
        // Out.color = colors[5];
        createQuad(v4, v6, v7, v5, colors[5]);
    }
}
