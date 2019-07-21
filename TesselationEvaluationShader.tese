#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(set = 0, binding = 0) uniform MVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(quads, fractional_odd_spacing, cw) in;

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

/*float len(vec3 cds)
{
    return sqrt(cds.x * cds.x + cds.y * cds.y + cds.z * cds.z);
}*/

void main()
{
    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 mid2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);
    vec2 fromCenterToTessCoords = gl_TessCoord.xy - vec2(0.5, 0.5);
    
    vec3 normalMid1 = mix(vertexData[0].normal, vertexData[1].normal, gl_TessCoord.x);
    vec3 normalMid2 = mix(vertexData[3].normal, vertexData[2].normal, gl_TessCoord.x);
    vec3 norm = mix(normalMid1, normalMid2, gl_TessCoord.y);

    pos += vec4((-sqrt(fromCenterToTessCoords.x * fromCenterToTessCoords.x + fromCenterToTessCoords.y * fromCenterToTessCoords.y)) * norm, 0) * 0.05;

    vec2 uvMid1 = mix(vertexData[0].uv, vertexData[1].uv, gl_TessCoord.x);
    vec2 uvMid2 = mix(vertexData[3].uv, vertexData[2].uv, gl_TessCoord.x);
    vec2 uv = mix(uvMid1, uvMid2, gl_TessCoord.y);

    gl_Position = mvp.proj * mvp.view * mvp.model * pos;
    vertexDataOut.coords = pos.xyz;
    vertexDataOut.normal = normalize(norm);
    vertexDataOut.uv = uv;
    //for (int i = 0; i < 4; ++i)
    //    if (gl_in[i].gl_Position.xyz != vertexData[i].coords)
    //        uv = vec2(0, 0);
}