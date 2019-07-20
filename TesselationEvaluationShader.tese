#version 460 core
#extension GL_ARB_separate_shader_objects : enable

struct VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
};

layout(quads, fractional_odd_spacing) in;

layout(location = 0) in VertexData vertexData[];

layout(location = 0) out VertexData vertexDataOut;

float len(vec3 cds)
{
    return sqrt(cds.x * cds.x + cds.y * cds.y + cds.z * cds.z);
}

void main()
{
    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 mid2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);
    vec2 fromCenterToTessCoords = gl_TessCoord.xy - vec2(0.5, 0.5);
    pos.z += 1 - sqrt(fromCenterToTessCoords.x * fromCenterToTessCoords.x + fromCenterToTessCoords.y * fromCenterToTessCoords.y);
    
    vec3 normalMid1 = mix(vertexData[0].normal, vertexData[1].normal, gl_TessCoord.x);
    vec3 normalMid2 = mix(vertexData[2].normal, vertexData[3].normal, gl_TessCoord.x);
    vec3 norm = mix(normalMid1, normalMid1, gl_TessCoord.y);

    vec2 uvMid1 = mix(vertexData[0].uv, vertexData[1].uv, gl_TessCoord.x);
    vec2 uvMid2 = mix(vertexData[2].uv, vertexData[3].uv, gl_TessCoord.x);
    vec2 uv = mix(uvMid1, uvMid1, gl_TessCoord.y);

    gl_Position = pos;
    vertexDataOut.coords = pos.xyz;
    vertexDataOut.normal = normalize(norm);
    vertexDataOut.uv = uv;
}