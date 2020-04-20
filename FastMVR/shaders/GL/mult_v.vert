#version 460
//#extension GL_ARB_shading_language_include : require

#include "/vertex_load.glsl"

in layout(location = 0) vec3 position;
#ifdef GBUFFER
in layout(location = 1) vec3 normalIn;
in layout(location = 2) uint triangleidIn;
out VertexData
{
	vec3 normal;
	flat uint triangleid;
} v_out;
#endif

uniform mat4 V;

void main()
{
	vec3 NewPosition = SimulateVertexLoad(position);
	gl_Position = V * vec4(NewPosition, 1.0f);
#ifdef GBUFFER
	v_out.normal = normalIn;
	v_out.triangleid = triangleidIn;
#endif
}
