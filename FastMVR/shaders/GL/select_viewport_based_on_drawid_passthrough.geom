#version 460

#extension GL_NV_geometry_shader_passthrough : require
#ifdef OVR_VIEWPORT
#extension GL_EXT_multiview_tessellation_geometry_shader : enable
#endif

#ifdef OVR_VIEWPORT
layout (num_views = NUM_VIEWS) in;
#endif
layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

#ifdef GBUFFER
in VertexData
{
	vec3 normal;
	flat uint triangleid;
} v_in[];

out VertexData
{
	vec3 normal;
	flat uint triangleid;
} v_out;
#endif

layout(location = 0) in int g_draw[]; // OpenGL requires geometry inputs to be arrays... but it's only a single value, set by vertex shader

void main()
{
	gl_ViewportIndex = g_draw[0];
	gl_PrimitiveID = gl_PrimitiveIDIn;
#ifdef GBUFFER
	v_out.normal = v_in[0].normal;
	v_out.triangleid = v_in[0].triangleid;
#endif
}
