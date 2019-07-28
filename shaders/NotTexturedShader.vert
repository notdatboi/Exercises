#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(location = 0) out VertexData
{
    vec3 coords;
    vec3 normal;
} vertexData;

void main(void)
{
    gl_Position = vec4(pos + vec3(0, 0, 0.1), 1.0);

    vertexData.coords = pos;
    vertexData.normal = normal;
}
