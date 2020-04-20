#version 460
//#extension GL_ARB_shading_language_include : require

#include "/vertex_load.glsl"

uniform int NUM_DRAWS_PER_VIEW;

layout(std140) uniform Vs
{
	mat4 V[128];
};

uniform int uVsOffset = 0;

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

out layout(location = 0) int g_draw;
#ifdef PASS_ON_GEOMETRY_ID
out layout(location = 1) int g_geometry;
#endif

void main()
{
	vec3 NewPosition = SimulateVertexLoad(position);
	g_draw = gl_DrawID / NUM_DRAWS_PER_VIEW;
#ifdef PASS_ON_GEOMETRY_ID
	// View1 -> n geometries -> View2 -> n geometries -> etc.
	g_geometry = gl_DrawID - (g_draw * NUM_DRAWS_PER_VIEW);
#endif
	gl_Position = V[g_draw + uVsOffset] * vec4(NewPosition, 1.0f);

#ifdef GBUFFER
	v_out.normal = normalIn;
	v_out.triangleid = triangleidIn;
#endif
}
