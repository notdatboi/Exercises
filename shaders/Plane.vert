#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 0) out VertexData
{
    vec3 coords;
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
    vec2 uv;
} vertexData;

void main(void)
{
    gl_Position = vec4(pos, 1.0);

    vertexData.coords = pos;
    vertexData.tangent = tangent;
    vertexData.bitangent = bitangent;
    vertexData.normal = normal;
    vertexData.uv = vec2(uv.x, 1 - uv.y);
}
