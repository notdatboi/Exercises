#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(vertices = 4) out;

struct VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
};

layout(location = 0) in VertexData vertexData[];

layout(location = 0) out VertexData vertexDataOut[];

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 3.0f;
        gl_TessLevelInner[1] = 3.0f;

        gl_TessLevelOuter[0] = 3.0f;
        gl_TessLevelOuter[1] = 3.0f;
        gl_TessLevelOuter[2] = 3.0f;
        gl_TessLevelOuter[3] = 3.0f;
    }
    vertexDataOut[gl_InvocationID] = vertexData[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}