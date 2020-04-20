#version 460

layout(std140) uniform Vs
{
	mat4 V[128];
};

#ifdef PLANE_CLIPPING
// I have no f**** idea what's going on. If this is declared as a uniform (just like Vs)
// then the result will always be all black. If this is declared as a buffer, it's fine.
// WT*, OpenGL?!
// TODO: Ask Bernhard if he has an idea what's going on here.
//  => No idea what's going on here.
layout(std430, binding = 5) buffer CLIP_PLANES 
{
	vec4 CLIP_PLANE[]; // 128 view matrixes * 4 clipping planes
};
#endif

uniform int uIdOffset = 0;

uniform int uLayerOffset = 0;

layout(triangles, invocations = NUM_INVOCATIONS) in;
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

void main()
{
    const int voc = gl_InvocationID + uIdOffset;
	const int lyr = gl_InvocationID + uLayerOffset;
	const int id = gl_PrimitiveIDIn;
    const mat4 view = V[voc];
	#ifdef PLANE_CLIPPING
	const int NP = 4; // number of planes per view
	#endif

	#ifdef GEOM_CULLING
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
		gl_PrimitiveID = id;
		gl_Position = tmp0;
		#ifdef PLANE_CLIPPING
		gl_ClipDistance[0] = dot(CLIP_PLANE[NP*voc + 0], gl_in[0].gl_Position);
		gl_ClipDistance[1] = dot(CLIP_PLANE[NP*voc + 1], gl_in[0].gl_Position);
		gl_ClipDistance[2] = dot(CLIP_PLANE[NP*voc + 2], gl_in[0].gl_Position);
		gl_ClipDistance[3] = dot(CLIP_PLANE[NP*voc + 3], gl_in[0].gl_Position);
		#endif
#ifdef GBUFFER
		v_out.normal = v_in[0].normal;
		v_out.triangleid = v_in[0].triangleid;
#endif
		EmitVertex();
		gl_Layer = lyr;
		gl_PrimitiveID = id;
		gl_Position = tmp1;
		#ifdef PLANE_CLIPPING
		gl_ClipDistance[0] = dot(CLIP_PLANE[NP*voc + 0], gl_in[1].gl_Position);
		gl_ClipDistance[1] = dot(CLIP_PLANE[NP*voc + 1], gl_in[1].gl_Position);
		gl_ClipDistance[2] = dot(CLIP_PLANE[NP*voc + 2], gl_in[1].gl_Position);
		gl_ClipDistance[3] = dot(CLIP_PLANE[NP*voc + 3], gl_in[1].gl_Position);
		#endif
#ifdef GBUFFER
		v_out.normal = v_in[1].normal;
		v_out.triangleid = v_in[1].triangleid;
#endif
		EmitVertex();
		gl_Layer = lyr;
		gl_PrimitiveID = id;
		gl_Position = tmp2;
		#ifdef PLANE_CLIPPING
		gl_ClipDistance[0] = dot(CLIP_PLANE[NP*voc + 0], gl_in[2].gl_Position);
		gl_ClipDistance[1] = dot(CLIP_PLANE[NP*voc + 1], gl_in[2].gl_Position);
		gl_ClipDistance[2] = dot(CLIP_PLANE[NP*voc + 2], gl_in[2].gl_Position);
		gl_ClipDistance[3] = dot(CLIP_PLANE[NP*voc + 3], gl_in[2].gl_Position);
		#endif
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
		gl_Layer = lyr;
		gl_PrimitiveID = id;
		gl_Position = view * gl_in[i].gl_Position;
		#ifdef PLANE_CLIPPING
		gl_ClipDistance[0] = dot(CLIP_PLANE[NP*voc + 0], gl_in[i].gl_Position);
		gl_ClipDistance[1] = dot(CLIP_PLANE[NP*voc + 1], gl_in[i].gl_Position);
		gl_ClipDistance[2] = dot(CLIP_PLANE[NP*voc + 2], gl_in[i].gl_Position);
		gl_ClipDistance[3] = dot(CLIP_PLANE[NP*voc + 3], gl_in[i].gl_Position);
		#endif
#ifdef GBUFFER
		v_out.normal = v_in[i].normal;
		v_out.triangleid = v_in[i].triangleid;
#endif
		EmitVertex();
	}
	EndPrimitive();
	#endif
}
