#version 450

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout( location = 0 )  in VS_OUT {
    vec3 normal;
} gs_in[];

layout( location = 0 ) out GS_OUT {
    vec3 position;
} gs_out;


layout ( std140, set = 3, binding = 1 ) uniform ViewProjMatrix {
    mat4 uViewProj[3];
    mat4 uViewProjIt[3];
};

int getDominantAxis(vec3 pos0, vec3 pos1, vec3 pos2)
{
    vec3 normal = abs(cross(pos1 - pos0, pos2 - pos0));
    return (normal.x > normal.y && normal.x > normal.z) ? 0 :
    (normal.y > normal.z) ? 1 : 2;
}

void main()
{
    int axis = getDominantAxis(gl_in[0].gl_Position.xyz,
    gl_in[1].gl_Position.xyz,
    gl_in[2].gl_Position.xyz);

//    axis = 0;
    for (int i = 0; i < 3; ++i)
    {
        gl_ViewportIndex 	= axis;
        gl_Position 		= uViewProj[axis] * gl_in[i].gl_Position;
        gs_out.position 	= gl_in[i].gl_Position.xyz;
        EmitVertex();
    }
    EndPrimitive();
}