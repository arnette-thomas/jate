#version 450

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 outColor;

// layout (push_constant) uniform Push
// {
// 	mat2 transform;
// 	vec2 offset;
// 	vec3 color;
// } push;

void main()
{
	// Gradient
//	vec2 coords = gl_FragCoord.xy / vec2(800., 600.);
//	float xGradient = clamp(coords.x, 0., 1.);
//	outColor = vec4(xGradient, 1. - xGradient, 0.5, 1.0);

	outColor = vec4(inColor, 1.0f);
}