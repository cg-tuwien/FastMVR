#version 460
//#pragma optionNV unroll all
//#pragma optionNV unroll 1 
// ^ to prevent stupid "lvalue in array access too complex" error
//   ^ doesn't work

uniform int uVsOffset = 0;

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

uniform ivec2 RESOLUTION; // Resolution of the target, e.g., 800x600 (not the texture resolution)

// Output to fragment shader
flat out ivec2 subarea;

void main()
{
	const int num_views = NUM_VIEWS;
	for(int m = 0; m < num_views; m++)
	{
		const int NP = 4; // number of planes per view
		int i = m + uVsOffset;

//		auto perRow = cfg.width / width();
//		auto fullRows = static_cast<uint32_t>(matrixId) / perRow;
//		auto remainderInRow = static_cast<uint32_t>(matrixId) - (fullRows * perRow);
//		return { remainderInRow * width(), fullRows * height() };
		
		int numPerRow = int(ceil(sqrt(float(num_views))));
		int numPerCol = num_views % numPerRow == 0 ? num_views / numPerRow : num_views / numPerRow + 1;
		int fullRows = m / numPerRow; // ATTENTION: m, not i, in order to support multiple giant textures
		int remainderInRow = m - (fullRows * numPerRow);

		// Scale down one view to the right fraction of the screen
		vec2 screensplit = 1.0 / vec2(numPerRow, numPerCol);
		vec2 scale = screensplit;
		vec2 scale2 = 2.0 * scale;
		vec2 trans = vec2(0, 0); // vec2(-1.0 + 1.0 / screensplit, 0.0);
		mat4 TS  = mat4(scale.x, 0.0,     0.0, 0.0,
						0.0,     scale.y, 0.0, 0.0, 
						0.0,     0.0,     1.0, 0.0,
						0.0,     0.0,     0.0, 1.0);

		subarea.x = RESOLUTION.x * remainderInRow;
		subarea.y = RESOLUTION.y * fullRows;

		#ifdef GEOM_CULLING
		vec4 tmp0 = V[i] * gl_in[0].gl_Position;
		vec4 tmp1 = V[i] * gl_in[1].gl_Position;
		vec4 tmp2 = V[i] * gl_in[2].gl_Position;
		if (! (	(tmp0.x < -tmp0.w && tmp1.x < -tmp1.w && tmp2.x < -tmp2.w) || (tmp0.x > tmp0.w && tmp1.x > tmp1.w && tmp2.x > tmp2.w)
			 || (tmp0.y < -tmp0.w && tmp1.y < -tmp1.w && tmp2.y < -tmp2.w) || (tmp0.y > tmp0.w && tmp1.y > tmp1.w && tmp2.y > tmp2.w)
			 || (tmp0.z < -tmp0.w && tmp1.z < -tmp1.w && tmp2.z < -tmp2.w) || (tmp0.z > tmp0.w && tmp1.z > tmp1.w && tmp2.z > tmp2.w)
			 )
		#ifdef BACKFACE_CULLING
			&& dot(vec3(0, 0, 1), cross(tmp1.xyz/tmp1.w - tmp0.xyz/tmp0.w, tmp2.xyz/tmp2.w - tmp0.xyz/tmp0.w)) > 0.0
		#endif
			)
		{
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = TS * tmp0;
			gl_Position.x += (-1.0 + scale.x + scale2.x * remainderInRow) * tmp0.w;
			gl_Position.y += (-1.0 + scale.y + scale2.y * fullRows) * tmp0.w;
			#ifdef PLANE_CLIPPING
			gl_ClipDistance[0] = dot(CLIP_PLANE[NP*i + 0], gl_in[0].gl_Position);
			gl_ClipDistance[1] = dot(CLIP_PLANE[NP*i + 1], gl_in[0].gl_Position);
			gl_ClipDistance[2] = dot(CLIP_PLANE[NP*i + 2], gl_in[0].gl_Position);
			gl_ClipDistance[3] = dot(CLIP_PLANE[NP*i + 3], gl_in[0].gl_Position);
			#endif
#ifdef GBUFFER
			v_out.normal = v_in[0].normal;
			v_out.triangleid = v_in[0].triangleid;
#endif
			EmitVertex();
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = TS * tmp1;
			gl_Position.x += (-1.0 + scale.x + scale2.x * remainderInRow) * tmp1.w;
			gl_Position.y += (-1.0 + scale.y + scale2.y * fullRows) * tmp1.w;
			#ifdef PLANE_CLIPPING
			gl_ClipDistance[0] = dot(CLIP_PLANE[NP*i + 0], gl_in[1].gl_Position);
			gl_ClipDistance[1] = dot(CLIP_PLANE[NP*i + 1], gl_in[1].gl_Position);
			gl_ClipDistance[2] = dot(CLIP_PLANE[NP*i + 2], gl_in[1].gl_Position);
			gl_ClipDistance[3] = dot(CLIP_PLANE[NP*i + 3], gl_in[1].gl_Position);
			#endif
#ifdef GBUFFER
			v_out.normal = v_in[1].normal;
			v_out.triangleid = v_in[1].triangleid;
#endif
			EmitVertex();
			gl_PrimitiveID = gl_PrimitiveIDIn;
			gl_Position = TS * tmp2;
			gl_Position.x += (-1.0 + scale.x + scale2.x * remainderInRow) * tmp2.w;
			gl_Position.y += (-1.0 + scale.y + scale2.y * fullRows) * tmp2.w;
			#ifdef PLANE_CLIPPING
			gl_ClipDistance[0] = dot(CLIP_PLANE[NP*i + 0], gl_in[2].gl_Position);
			gl_ClipDistance[1] = dot(CLIP_PLANE[NP*i + 1], gl_in[2].gl_Position);
			gl_ClipDistance[2] = dot(CLIP_PLANE[NP*i + 2], gl_in[2].gl_Position);
			gl_ClipDistance[3] = dot(CLIP_PLANE[NP*i + 3], gl_in[2].gl_Position);
			#endif
#ifdef GBUFFER
			v_out.normal = v_in[2].normal;
			v_out.triangleid = v_in[2].triangleid;
#endif
			EmitVertex();
			EndPrimitive();
		}
		#else
		for (int v = 0; v < 3; ++v)
		{
			gl_PrimitiveID = gl_PrimitiveIDIn;
			vec4 tmp = TS * V[i] * gl_in[v].gl_Position;
			gl_Position = tmp;
			gl_Position.x += (-1.0 + scale.x + scale2.x * remainderInRow) * tmp.w;
			gl_Position.y += (-1.0 + scale.y + scale2.y * fullRows) * tmp.w;
			#ifdef PLANE_CLIPPING
			gl_ClipDistance[0] = dot(CLIP_PLANE[NP*i + 0], gl_in[v].gl_Position);
			gl_ClipDistance[1] = dot(CLIP_PLANE[NP*i + 1], gl_in[v].gl_Position);
			gl_ClipDistance[2] = dot(CLIP_PLANE[NP*i + 2], gl_in[v].gl_Position);
			gl_ClipDistance[3] = dot(CLIP_PLANE[NP*i + 3], gl_in[v].gl_Position);
//			gl_ClipDistance[0] = dot(vec4( 1, 0, 0, 1), tmp);
//			gl_ClipDistance[1] = dot(vec4(-1, 0, 0, 1), tmp);
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
}
