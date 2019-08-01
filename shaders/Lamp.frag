#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform Lamps
{
    vec4 posAndPower;
} lamps[1];

void main()
{
    outColor = vec4(1.0, 1.0, 1.0, 1.0) * lamps[0].posAndPower.w;
}