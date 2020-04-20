#version 460

#extension GL_NV_geometry_shader_passthrough : require

layout(triangles) in;

layout(passthrough) in gl_PerVertex {
	vec4 gl_Position;
} gl_in[];

layout(location = 0) in int g_draw[];
#ifdef PASS_ON_GEOMETRY_ID
layout(location = 1) in int g_geometry[];
flat out int g_geometry_id;
#endif

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

void main()
{
	gl_Layer = g_draw[0];
#ifdef PASS_ON_GEOMETRY_ID
	g_geometry_id = g_geometry[0];
#endif
#ifdef GBUFFER
		v_out.normal = v_in[0].normal;
		v_out.triangleid = v_in[0].triangleid;
#endif
	gl_PrimitiveID = gl_PrimitiveIDIn;
}
