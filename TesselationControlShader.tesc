#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(vertices = 4) out;

layout(location = 0) in VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
} vertexData[];

layout(location = 0) out VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
} vertexDataOut[];

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 1.0f;
        gl_TessLevelInner[1] = 1.0f;

        gl_TessLevelOuter[0] = 1.0f;
        gl_TessLevelOuter[1] = 1.0f;
        gl_TessLevelOuter[2] = 1.0f;
        gl_TessLevelOuter[3] = 1.0f;
    }
    vertexDataOut[gl_InvocationID].coords = vertexData[gl_InvocationID].coords;
    vertexDataOut[gl_InvocationID].normal = vertexData[gl_InvocationID].normal;
    vertexDataOut[gl_InvocationID].uv = vertexData[gl_InvocationID].uv;
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}