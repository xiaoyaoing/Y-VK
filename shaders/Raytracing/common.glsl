#include "commons.h"
#include "PT/pt_commons.glsl"


layout(binding = 0, set = 0) uniform accelerationStructureEXT top_level_as;
layout(binding = 1, set = 0, rgba8) uniform image2D image;
layout(binding = 2, set = 0) uniform SceneUbo scene_ubo;
layout(binding = 3, set = 0) buffer SceneDesc_ {SceneDesc scene_desc;};
layout(binding = 4, set = 0) readonly buffer Lights {Light lights[];};
layout(binding = 5, set = 0) uniform sampler2D scene_textures;



layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Vertices { vec3 v[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Indices { uint i[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Normals { vec3 n[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer TexCoords { vec2 t[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Materials { Material m[]; };


Indices indices = Indices(scene_desc.index_addr);
Vertices vertices = Vertices(scene_desc.vertex_addr);
Normals normals = Normals(scene_desc.normal_addr);
Materials materials = Materials(scene_desc.material_addr);
InstanceInfo prim_infos = InstanceInfo(scene_desc.prim_info_addr);
TexCoords tex_coords = TexCoords(scene_desc.uv_addr);

vec3 eval_bsdf(uint material_idx,vec3 n,vec3 wi,vec3 wo){
    
}

vec3 sample_li(uint light_idx,vec3 p,vec3 n,vec3 wo){
    Light light = lights[light_idx];
    vec3 wi = light.position - p;
    float dist = length(wi);
    wi = normalize(wi);
    float cos_theta = dot(n,wi);
    if(cos_theta <= 0.0){
        return vec3(0.0);
    }
    float cos_theta_light = dot(light.normal,-wi);
    if(cos_theta_light <= 0.0){
        return vec3(0.0);
    }
    float attenuation = 1.0 / (dist * dist);
    return light.intensity * cos_theta_light * cos_theta * attenuation;
}


vec3 uniform_sample_one_light(uint material_idx,vec3 p,vec3 n,vec3 wo){
    
}