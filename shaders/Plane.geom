#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices = 3) out;

layout(set = 0, binding = 0) uniform MVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
} vertexData[];

layout(location = 0) out VertexDataOut
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
} vertexDataOut;

void main()
{
    for(int i = 0; i < 3; i++)
    {
        vec4 pos = gl_in[i].gl_Position;
        gl_Position = mvp.proj * mvp.view * mvp.model * pos;
        vertexDataOut.coords = pos.xyz;
        vertexDataOut.normal = vertexData[i].normal;
        vertexDataOut.uv = vertexData[i].uv;
        EmitVertex();
    }
    EndPrimitive();
}