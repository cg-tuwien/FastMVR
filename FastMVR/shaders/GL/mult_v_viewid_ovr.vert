#version 460
#extension GL_OVR_multiview : require
//#extension GL_ARB_shading_language_include : require

#include "/vertex_load.glsl"

layout (num_views = NUM_VIEWS) in;
layout(location = 0) in vec3 position;
#ifdef GBUFFER
in layout(location = 1) vec3 normalIn;
in layout(location = 2) uint triangleidIn;
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
// uniform int uNumTiles; <-- This shit doesn't work => "invalid value for this parameter". Why? WTF, OpenGL?
#endif

#ifdef OVR_VIEWPORT
out layout(location = 0) int g_draw;
#endif

uniform int uIdOffset = 0;

void main()
{
	vec3 NewPosition = SimulateVertexLoad(position);
	int vIndex = uIdOffset + gl_InstanceID * NUM_VIEWS + int(gl_ViewID_OVR);
	gl_Position = V[vIndex] * vec4(NewPosition, 1.0f);

#ifdef OVR_VIEWPORT
	g_draw = vIndex / NUM_VIEWS;
#endif

#ifdef PLANE_CLIPPING
	const int num_tiles = NUM_TILES;
	const int NP = 4; // number of planes per view
	int i = gl_InstanceID;

	int numPerRow = int(ceil(sqrt(float(num_tiles))));
	int numPerCol = num_tiles % numPerRow == 0 ? num_tiles / numPerRow : num_tiles / numPerRow + 1;
	int fullRows = i / numPerRow; // ATTENTION: m, not i, in order to support multiple giant textures
	int remainderInRow = i - (fullRows * numPerRow);

	// Scale down one view to the right fraction of the screen
	vec2 screensplit = 1.0 / vec2(numPerRow, numPerCol);
	vec2 scale = screensplit;
	vec2 scale2 = 2.0 * scale;
	vec2 trans = vec2(0, 0); // vec2(-1.0 + 1.0 / screensplit, 0.0);
	mat4 TS  = mat4(scale.x, 0.0,     0.0, 0.0,
					0.0,     scale.y, 0.0, 0.0, 
					0.0,     0.0,     1.0, 0.0,
					0.0,     0.0,     0.0, 1.0);

	gl_ClipDistance[0] = dot(CLIP_PLANE[NP * vIndex + 0], vec4(NewPosition, 1.0f));
	gl_ClipDistance[1] = dot(CLIP_PLANE[NP * vIndex + 1], vec4(NewPosition, 1.0f));
	gl_ClipDistance[2] = dot(CLIP_PLANE[NP * vIndex + 2], vec4(NewPosition, 1.0f));
	gl_ClipDistance[3] = dot(CLIP_PLANE[NP * vIndex + 3], vec4(NewPosition, 1.0f));
	
	vec4 tmp = TS * gl_Position;
	gl_Position = tmp;
	gl_Position.x += (-1.0 + scale.x + scale2.x * remainderInRow) * tmp.w;
	gl_Position.y += (-1.0 + scale.y + scale2.y * fullRows) * tmp.w;
#endif

#ifdef GBUFFER
	v_out.normal = normalIn;
	v_out.triangleid = triangleidIn;
#endif
}
