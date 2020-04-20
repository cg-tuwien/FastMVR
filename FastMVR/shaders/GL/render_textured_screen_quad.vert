#version 460

in layout(location = 0) vec3 position;
out vec2 uv;

void main()
{
	uv = position.xy * 0.5 + vec2(0.5, 0.5);
	gl_Position = vec4(position, 1.0f);
}
