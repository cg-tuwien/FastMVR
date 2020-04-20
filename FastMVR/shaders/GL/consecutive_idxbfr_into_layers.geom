#version 460

uniform int NUM_PRIMITIVES;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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

layout(std140) uniform Vs
{
	mat4 V[128];
};

void main()
{
	const int layer = gl_PrimitiveIDIn / NUM_PRIMITIVES;
	const int id = gl_PrimitiveIDIn - (layer * NUM_PRIMITIVES);

	for(int i = 0; i < 3; i++)
	{
		gl_Layer = layer;
		gl_PrimitiveID = id;
		gl_Position = V[layer] * gl_in[i].gl_Position;
#ifdef GBUFFER
		v_out.normal = v_in[i].normal;
		v_out.triangleid = v_in[i].triangleid;
#endif
		EmitVertex();
	}
	EndPrimitive();
}
