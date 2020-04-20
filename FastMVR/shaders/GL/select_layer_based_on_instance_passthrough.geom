#version 460

#extension GL_NV_geometry_shader_passthrough : require

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

layout(location = 0) in int g_instance[];

void main()
{
	gl_Layer = g_instance[0];
	gl_PrimitiveID = gl_PrimitiveIDIn;
#ifdef GBUFFER
	v_out.normal = v_in[0].normal;
	v_out.triangleid = v_in[0].triangleid;
#endif
}
