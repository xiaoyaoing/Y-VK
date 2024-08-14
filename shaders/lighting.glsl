#if !defined(LIGHTING_GLSL)
#define LIGHTING_GLSL

#define MAX_LIGHTS 256
#define Directional_LIGHT_TYPE 1
#define Point_LIGHT_TYPE 2
#define Spot_LIGHT_TYPE 3

struct  Light
{
    vec4 color;// color.w represents light intensity
    vec4 position;// position.w represents type of light
    vec4 direction;// direction.w represents range
    vec4 info;// (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
    mat4 matrix;
};

vec3 apply_directional_light(Light light, vec3 normal)
{
    vec3 world_to_light = -light.direction.xyz;
    world_to_light      = normalize(world_to_light);
    float ndotl         = clamp(dot(normal, world_to_light), 0.0, 1.0);
    return ndotl * light.color.w * light.color.rgb;
}

vec3 apply_point_light(Light light, vec3 pos, vec3 normal)
{
    vec3  world_to_light = light.position.xyz - pos;
    float dist           = length(world_to_light) * 0.005f;
    float atten          = 1.0 / (dist * dist);
    world_to_light       = normalize(world_to_light);
    float ndotl          = clamp((dot(normal, world_to_light)), 0.0, 1.0);
    return ndotl * light.color.w * atten * light.color.rgb;
}

vec3 apply_spot_light(Light light, vec3 pos, vec3 normal)
{
    vec3  light_to_pixel   = normalize(pos - light.position.xyz);
    float theta            = dot(light_to_pixel, normalize(light.direction.xyz));
    float inner_cone_angle = light.info.x;
    float outer_cone_angle = light.info.y;
    float intensity        = (theta - outer_cone_angle) / (inner_cone_angle - outer_cone_angle);
    return light.color.w * light.color.rgb;
    return smoothstep(0.0, 1.0, intensity) * light.color.w * light.color.rgb;
}


vec3 calcuate_light_dir(Light light, vec3 pos)
{
    if (light.info.x == Directional_LIGHT_TYPE)
    {
        return -light.direction.xyz;
    }
    else if (light.info.x == Point_LIGHT_TYPE)
    {
        return normalize(light.position.xyz - pos);
    }
    else if (light.info.x == Spot_LIGHT_TYPE){
        return light.position.xyz - pos;
    }
    return vec3(0);
}

vec3 calcuate_light_intensity(Light light, vec3 pos)
{
    if (light.info.x == Directional_LIGHT_TYPE)
    {
        return light.color.w * light.color.rgb;
    }
    else if (light.info.x == Point_LIGHT_TYPE)
    {
        vec3  world_to_light = light.position.xyz - pos;
        // float dist           = length(world_to_light) * 0.005f;
        float dist = 1.f;
        float atten          = 1.0 / (dist * dist);
        return atten * light.color.w * light.color.rgb;
    }
    else if (light.info.x == Spot_LIGHT_TYPE){
        return light.color.w * light.color.rgb;
    }
    return vec3(0);
}

vec3 apply_light(Light light, vec3 pos, vec3 normal)
{
    if (light.info.x == Directional_LIGHT_TYPE)
    {
        return apply_directional_light(light, normal);
    }
    else if (light.info.x == Point_LIGHT_TYPE)
    {
        return apply_point_light(light, pos, normal);
    }
    else if (light.info.x == Spot_LIGHT_TYPE){
        return apply_spot_light(light, pos, normal);
    }
    return vec3(0);
}


//float calcute_shadow(in Light light, vec3 world_pos){
//    //todo handle shadow 
//    return 1;
//
//}
#endif