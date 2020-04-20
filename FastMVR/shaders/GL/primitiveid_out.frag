#version 460

// Force early fragment test:
layout(early_fragment_tests) in;

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

#ifdef MULTIPLE_MAPPING_BUFFERS
layout(std430, binding = 1) buffer PrimitiveIdMappingOffsets
{
	uint primMappingOffsets[];
};
#else
uniform int uPrimitiveIdMappingBufferOffset;
#endif

#endif

#ifdef PASS_ON_GEOMETRY_ID
flat in int g_geometry_id;
#endif

void main()
{
#ifdef PRIMITIVE_ID_MAPPING_ENABLED
	#ifdef MULTIPLE_MAPPING_BUFFERS
		#ifdef PASS_ON_GEOMETRY_ID
			d_color = primMapping[gl_PrimitiveID + primMappingOffsets[g_geometry_id]];
		#else
			d_color = 0xFF00FF; // ERROR
		#endif
	#else
		d_color = primMapping[gl_PrimitiveID + uPrimitiveIdMappingBufferOffset];
	#endif
#else 
	d_color = gl_PrimitiveID;
#endif

#ifdef GBUFFER
	d_normal = v_in.normal;
	d_triangleid = v_in.triangleid;
#endif
}
