#version 430

struct Vertex
{
    vec4 color;
    vec2 uv;
};

layout (location = 0) in Vertex In;
layout (location = 0) out vec4 out_color;

const float EPSILON = 0.00001;

layout(std140, push_constant) uniform PushConstants
{
    float u_borderWidth;
    float u_alpha;
    vec4 u_borderColor;
};

void main()
{
    out_color = In.color;

    if (u_borderWidth > EPSILON)
    out_color = mix(u_borderColor, In.color, min(min(In.uv.x, min(In.uv.y, min((1.0 - In.uv.x), (1.0 - In.uv.y)))) / u_borderWidth, 1.0));
    // out_color.xyz  = vec3(1);
    out_color.a = u_alpha;
    out_color = In.color;

}
