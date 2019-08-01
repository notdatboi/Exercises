#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(triangles, fractional_odd_spacing, cw) in;

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
    vec4 pos = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
    vec3 norm = vertexData[0].normal * gl_TessCoord.x + vertexData[1].normal * gl_TessCoord.y + vertexData[2].normal * gl_TessCoord.z;
    vec2 uv = vertexData[0].uv * gl_TessCoord.x + vertexData[1].uv * gl_TessCoord.y + vertexData[2].uv * gl_TessCoord.z;

    gl_Position = pos;
    vertexDataOut.coords = pos.xyz;
    vertexDataOut.normal = normalize(norm);
    vertexDataOut.uv = uv;
}