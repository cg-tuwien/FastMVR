#version 460

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

in layout(location = 0) int g_instance[];

uniform int uLayerOffset = 0;

void main()
{
	#ifdef GEOM_CULLING
	vec4 tmp0 = gl_in[0].gl_Position;
	vec4 tmp1 = gl_in[1].gl_Position;
	vec4 tmp2 = gl_in[2].gl_Position;
	if (! (	(tmp0.x < -tmp0.w && tmp1.x < -tmp1.w && tmp2.x < -tmp2.w) || (tmp0.x > tmp0.w && tmp1.x > tmp1.w && tmp2.x > tmp2.w)
			|| (tmp0.y < -tmp0.w && tmp1.y < -tmp1.w && tmp2.y < -tmp2.w) || (tmp0.y > tmp0.w && tmp1.y > tmp1.w && tmp2.y > tmp2.w)
			|| (tmp0.z < -tmp0.w && tmp1.z < -tmp1.w && tmp2.z < -tmp2.w) || (tmp0.z > tmp0.w && tmp1.z > tmp1.w && tmp2.z > tmp2.w)
			)
		#ifdef BACKFACE_CULLING
			&& dot(vec3(0, 0, 1), cross(tmp1.xyz/tmp1.w - tmp0.xyz/tmp0.w, tmp2.xyz/tmp2.w - tmp0.xyz/tmp0.w)) > 0.0
		#endif
		)
	{
		gl_Layer = g_instance[0] + uLayerOffset;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = tmp0;
#ifdef GBUFFER
		v_out.normal = v_in[0].normal;
		v_out.triangleid = v_in[0].triangleid;
#endif
		EmitVertex();
		gl_Layer = g_instance[0] + uLayerOffset;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = tmp1;
#ifdef GBUFFER
		v_out.normal = v_in[1].normal;
		v_out.triangleid = v_in[1].triangleid;
#endif
		EmitVertex();
		gl_Layer = g_instance[0] + uLayerOffset;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = tmp2;
#ifdef GBUFFER
		v_out.normal = v_in[2].normal;
		v_out.triangleid = v_in[2].triangleid;
#endif
		EmitVertex();
		EndPrimitive();
	}
	#else
	for(int i = 0; i < 3; i++)
	{
		gl_Layer = g_instance[0] + uLayerOffset;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[i].gl_Position;
#ifdef GBUFFER
		v_out.normal = v_in[i].normal;
		v_out.triangleid = v_in[i].triangleid;
#endif
		EmitVertex();
	}
	EndPrimitive();
	#endif
}
