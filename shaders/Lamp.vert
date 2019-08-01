#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(set = 0, binding = 0) uniform MVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(set = 1, binding = 0) uniform Lamps
{
    vec4 posAndPower;
} lamps[1];

void main(void)
{
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(pos + lamps[0].posAndPower.xyz, 1.0);
}