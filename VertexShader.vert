#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform MVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(set = 2, binding = 0) uniform Instances
{
    vec3 data[2];
} instances;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 coords;
layout(location = 1) out vec2 uvOut;
layout(location = 2) out vec3 normalOut;

void main(void)
{
    vec3 instancePos = pos + instances.data[gl_InstanceIndex];
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(instancePos, 1.0);

    uvOut = vec2(uv.x, 1 - uv.y);
    normalOut = normal;
    coords = instancePos;
}
