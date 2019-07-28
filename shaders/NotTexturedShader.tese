#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(quads, fractional_odd_spacing, cw) in;

layout(location = 0) in VertexData
{
    vec3 coords;
    vec3 normal;
} vertexData[];

layout(location = 0) out VertexDataOut
{
    vec3 coords;
    vec3 normal;
} vertexDataOut;

void main()
{
    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 mid2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);
    vec2 fromCenterToTessCoords = gl_TessCoord.xy - vec2(0.5, 0.5);
    
    vec3 normalMid1 = mix(vertexData[0].normal, vertexData[1].normal, gl_TessCoord.x);
    vec3 normalMid2 = mix(vertexData[3].normal, vertexData[2].normal, gl_TessCoord.x);
    vec3 norm = mix(normalMid1, normalMid2, gl_TessCoord.y);

    gl_Position = pos;
    vertexDataOut.coords = pos.xyz;
    vertexDataOut.normal = normalize(norm);
}