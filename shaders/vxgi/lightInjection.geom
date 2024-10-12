#version 450

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;


layout( location = 0 ) in VS_OUT {
	vec2 texCoord;
	vec3 normal;
	uint i_primID;
} gs_in[];


layout(location = 0) out vec3 o_position;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out vec3 o_normal;
layout(location = 3 ) out flat uint o_primID;

//layout ( std140, set = 3, binding = 0 ) uniform ViewportSize {
//	uvec2 uViewportSize[3];
//};

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

vec2 project(vec3 vertex, uint axis)
{
	return axis == 0 ? vertex.yz : (axis == 1 ? vertex.xz : vertex.xy);
}

void main()
{
	int axis = getDominantAxis(gl_in[0].gl_Position.xyz,
	gl_in[1].gl_Position.xyz,
	gl_in[2].gl_Position.xyz);

//	axis = 0;
	for (int i = 0; i < 3; ++i)
	{
		gl_ViewportIndex 	= axis;
		gl_Position 		= uViewProj[axis] * gl_in[i].gl_Position;
		o_position 			= gl_in[i].gl_Position.xyz;
		o_uv 				= gs_in[i].texCoord;
		o_normal 			= gs_in[i].normal;
		o_primID 			= gs_in[i].i_primID;
		EmitVertex();
	}
	EndPrimitive();
}