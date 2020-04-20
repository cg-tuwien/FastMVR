#version 460
out layout(location = 0) vec4 d_color;

layout(std430, binding = 2) buffer PrimitiveEnabledBuffer
{
	uint primitiveEnabled[];
};

layout(std430, binding = 0) buffer PrimitiveIdMapping
{
	uint primMapping[];
};
uniform int uPrimitiveIdMappingEnabled;
uniform int uPrimitiveIdMappingBufferOffset;

void main()
{
	if (primitiveEnabled[gl_PrimitiveID] == 0) {
		discard;
	}
	if (1 == uPrimitiveIdMappingEnabled) {
		d_color = vec4(
			float((primMapping[gl_PrimitiveID + uPrimitiveIdMappingBufferOffset] >>  0) & 0xFF) / 255.0,
			float((primMapping[gl_PrimitiveID + uPrimitiveIdMappingBufferOffset] >>  8) & 0xFF) / 255.0,
			float((primMapping[gl_PrimitiveID + uPrimitiveIdMappingBufferOffset] >> 16) & 0xFF) / 255.0,
			1.0
		);
	}
	else {
		d_color = vec4(
			float((gl_PrimitiveID >>  0) & 0xFF) / 255.0,
			float((gl_PrimitiveID >>  8) & 0xFF) / 255.0,
			float((gl_PrimitiveID >> 16) & 0xFF) / 255.0,
			1.0
		);
	}
}
