#version 460

in layout(location = 0) vec3 position;

uniform mat4 V;
uniform mat4 P;

void main()
{
	gl_Position = P * V * vec4(position, 1.0f);
}
