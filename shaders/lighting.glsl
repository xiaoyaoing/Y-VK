#define MAX_LIGHTS 256
#define Directional_LIGHT_TYPE 0
#define Point_LIGHT_TYPE 1
#define Spot_LIGHT_TYPE 2

struct  Light
{
    vec4 color;// color.w represents light intensity
    vec4 position;// position.w represents type of light
    vec4 direction;// direction.w represents range
    vec4 info;// (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
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
    return smoothstep(0.0, 1.0, intensity) * light.color.w * light.color.rgb;
}

vec3 apply_light(Light light, vec3 pos, vec3 normal)
{
    if (light.info.x == 1)
    {
        return apply_directional_light(light, normal);
    }
    else if (light.info.x == 2)
    {
        return apply_point_light(light, pos, normal);
    }
    else if(light.info.x == 3){
        return apply_point_light(light, pos, normal);
    }
    return vec3(0);
}
