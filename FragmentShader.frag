#version 460 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 coords;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(set = 2, binding = 0) uniform sampler2D txt;
layout(set = 3, binding = 0) uniform Camera
{
    vec3 pos;
} camera;

layout(location = 0) out vec4 outColor;

void main(void)
{
    vec3 norm = normalize(normal);
    vec3 lightEmitter = vec3(3, 0, 0);
    float lightPower = 2;
    vec3 vecToLight = normalize(lightEmitter - coords);
    float cosineDiffuse = max(dot(norm, vecToLight), 0.0);
    vec3 rayReflection = reflect(-vecToLight, norm);
    vec3 viewDir = normalize(camera.pos - coords);
    float cosineSpecular = max(dot(viewDir, rayReflection), 0.0);
    float dist = distance(lightEmitter, coords);
    vec4 textureColor = texture(txt, uv);
    vec4 ambientLight = vec4(0.1, 0.1, 0.1, 1) * textureColor;
    vec4 diffuseLight = textureColor * (cosineDiffuse / (dist * dist)) * lightPower;
    vec4 specularLight = textureColor * pow(cosineSpecular, 32) * 0.5;
    outColor = diffuseLight + ambientLight + specularLight;
}