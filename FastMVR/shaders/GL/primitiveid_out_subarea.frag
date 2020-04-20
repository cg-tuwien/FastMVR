#version 460

// Force early fragment test:
//layout(early_fragment_tests) in;
//
// ^ ATTENTION: Early Fragment Test messes up the results!
//

uniform ivec2 RESOLUTION; // Resolution of the target, e.g., 800x600

flat in ivec2 subarea;

out layout(location = 0) uint d_color;
#ifdef GBUFFER
out layout(location = 1) vec3 d_normal;
out layout(location = 2) uint d_triangleid;
in VertexData
{
	vec3 normal;
	flat uint triangleid;
} v_in;
#endif

#ifdef PRIMITIVE_ID_MAPPING_ENABLED
layout(std430, binding = 0) buffer PrimitiveIdMapping
{
	uint primMapping[];
};
uniform int uPrimitiveIdMappingBufferOffset;
#endif

void main()
{
#ifndef PLANE_CLIPPING
	// Clipping:
	if (   gl_FragCoord.x <  subarea.x 
		|| gl_FragCoord.x > (subarea.x + RESOLUTION.x)
		|| gl_FragCoord.y <  subarea.y 
		|| gl_FragCoord.y > (subarea.y + RESOLUTION.y)) 
	{
		discard;
	}
	else 
#endif
	{
#ifdef PRIMITIVE_ID_MAPPING_ENABLED
		d_color = primMapping[gl_PrimitiveID + uPrimitiveIdMappingBufferOffset];
#else 
		d_color = gl_PrimitiveID;
#endif
	}

#ifdef GBUFFER
	d_normal = v_in.normal;
	d_triangleid = v_in.triangleid;
#endif
}
