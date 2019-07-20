#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(set = 2, binding = 0) uniform Instances
{
    vec3 data[2];
} instances;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

/*layout(location = 0) out vec3 coords;
layout(location = 1) out vec2 uvOut;
layout(location = 2) out vec3 normalOut;*/

layout(location = 0) out VertexData
{
    vec3 coords;
    vec3 normal;
    vec2 uv;
} vertexData;

void main(void)
{
    vec3 instancePos = pos + instances.data[gl_InstanceIndex];
    gl_Position = vec4(instancePos, 1.0);

    vertexData.coords = instancePos;
    vertexData.normal = normal;
    vertexData.uv = vec2(uv.x, 1 - uv.y);
}
