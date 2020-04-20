#version 460

layout(std140) uniform Vs
{
	mat4 V[128];
};

uniform int uIdOffset = 0;

uniform int uLayerOffset = 0;

layout(triangles) in;
layout(triangle_strip, max_vertices = NUM_VIEWS * 3) out;

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
	const int num_views = NUM_VIEWS;

	#ifdef GEOM_CULLING
	for(int m = 0; m < num_views; m++)
	{
		const int vid = m + uIdOffset;
		const int lyr = m + uLayerOffset;
		const mat4 view = V[vid];

		vec4 tmp0 = view * gl_in[0].gl_Position;
		vec4 tmp1 = view * gl_in[1].gl_Position;
		vec4 tmp2 = view * gl_in[2].gl_Position;
		if (! (	(tmp0.x < -tmp0.w && tmp1.x < -tmp1.w && tmp2.x < -tmp2.w) || (tmp0.x > tmp0.w && tmp1.x > tmp1.w && tmp2.x > tmp2.w)
				|| (tmp0.y < -tmp0.w && tmp1.y < -tmp1.w && tmp2.y < -tmp2.w) || (tmp0.y > tmp0.w && tmp1.y > tmp1.w && tmp2.y > tmp2.w)
				|| (tmp0.z < -tmp0.w && tmp1.z < -tmp1.w && tmp2.z < -tmp2.w) || (tmp0.z > tmp0.w && tmp1.z > tmp1.w && tmp2.z > tmp2.w)
				)
		#ifdef BACKFACE_CULLING
			&& dot(vec3(0, 0, 1), cross(tmp1.xyz/tmp1.w - tmp0.xyz/tmp0.w, tmp2.xyz/tmp2.w - tmp0.xyz/tmp0.w)) > 0.0
		#endif
			)
		{
			gl_Layer = lyr;
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = tmp0;
#ifdef GBUFFER
			v_out.normal = v_in[0].normal;
			v_out.triangleid = v_in[0].triangleid;
#endif
			EmitVertex();
			gl_Layer = lyr;
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = tmp1;
#ifdef GBUFFER
			v_out.normal = v_in[1].normal;
			v_out.triangleid = v_in[1].triangleid;
#endif
			EmitVertex();
			gl_Layer = lyr;
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = tmp2;
#ifdef GBUFFER
			v_out.normal = v_in[2].normal;
			v_out.triangleid = v_in[2].triangleid;
#endif
			EmitVertex();
			EndPrimitive();
		}
	}
	#else
	for(int m = 0; m < num_views; m++)
	{
		const int vid = m + uIdOffset;
		const int lyr = m + uLayerOffset;

		gl_Layer = lyr;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[vid] * gl_in[0].gl_Position;
#ifdef GBUFFER
		v_out.normal = v_in[0].normal;
		v_out.triangleid = v_in[0].triangleid;
#endif
		EmitVertex();
		gl_Layer = lyr;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[vid] * gl_in[1].gl_Position;
#ifdef GBUFFER
		v_out.normal = v_in[1].normal;
		v_out.triangleid = v_in[1].triangleid;
#endif
		EmitVertex();
		gl_Layer = lyr;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = V[vid] * gl_in[2].gl_Position;
#ifdef GBUFFER
		v_out.normal = v_in[2].normal;
		v_out.triangleid = v_in[2].triangleid;
#endif
		EmitVertex();	
		EndPrimitive();
	}
	EndPrimitive();
	#endif
}
