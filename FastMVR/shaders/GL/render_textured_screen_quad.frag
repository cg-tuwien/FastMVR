#version 460

uniform sampler2D u_sampler;
#ifdef GBUFFER
uniform sampler2D u_samplerNormals;
uniform usampler2D u_samplerTriangleId;
#endif

in vec2 uv;
out layout(location = 0) vec4 d_color;

void main()
{
//	d_color = vec4(1, 0, 0, 1);
	
	float d = texture(u_sampler, uv).r;
	d = pow(d, 20);

#ifdef GBUFFER
	vec3 n = texture(u_samplerNormals, uv).xyz;

	uint t = texture(u_samplerTriangleId, uv).r;
	
	d_color = vec4(float(t & 0xFF) / 255.0, float((t >> 8) & 0xFF) / 255.0, float((t >> 16) & 0xFF) / 255.0, 1.0);
	if ((10.0 * uv.x - 6.166 - uv.y) < 0.0) {
		d_color = vec4(d, d, d, 1.0);
	}
	if ((10.0 * uv.x - 2.833 - uv.y) < 0.0) {
		d_color = vec4(n, 1.0);
	}
#else

	d_color = vec4(d, d, d, 1.0);	

#endif
}
