#version 450
// #extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 outColor;

// layout (push_constant) uniform Push
// {
//     mat2 transform;
//     vec2 offset;
//     vec3 color;
// } push;

void main()
{
    //gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
    gl_Position = vec4(position, 0.0, 1.0);
    outColor = color;
}