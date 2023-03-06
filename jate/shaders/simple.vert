#version 450
//#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform Push
{
    mat4 transform;
} push;

void main()
{
    gl_Position = vec4(push.transform * vec4(position, 1.0));
    //gl_Position = vec4(position, 1.0);
    outColor = color;
}