#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_tessellation_shader : enable

layout(location = 0) in VertexData
{
    vec3 coords;
    vec3 normal;
} vertexData;

layout(set = 1, binding = 0) uniform Camera
{
    vec3 pos;
    vec3 viewDir;
} camera;

layout(location = 0) out vec4 outColor;

struct Flashlight
{
    float innerCutoffCos;
    float outerCutoffCos;
} flashlight;

struct DirectionalLight
{
    vec3 direction;
} directional;

struct PointLight
{
    vec3 emitterPosition;
    float power;

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
} light;

vec4 calculateDiffuse(vec4 textureColor, vec3 normal, vec3 fragToLight, float lightPower)
{
    float cosineDiffuse = max(dot(normal, fragToLight), 0.0);
    vec4 diffuseLight = textureColor * light.power * cosineDiffuse;
    return diffuseLight;
}

vec4 calculateSpecular(vec4 textureColor, vec3 normal, vec3 fragToLight, vec3 fragToCamera, float dispersion, float lightPower)
{
    vec3 rayReflection = reflect(-fragToLight, normal);
    float cosineSpecular = max(dot(fragToCamera, rayReflection), 0.0);
    vec4 specularLight = textureColor * pow(cosineSpecular, dispersion) * lightPower;
    return specularLight;
}

vec4 calculateDirectional(vec4 textureColor, vec3 normal, vec3 direction, float power)
{
    vec3 vecToDirectionalLight = normalize(-direction);
    float cosineDirectional = max(dot(normal, vecToDirectionalLight), 0.0);
    vec4 directionalLight = textureColor * cosineDirectional * power;
    return directionalLight;
}

void main(void)
{
    flashlight.innerCutoffCos = cos(radians(12.0f));
    flashlight.outerCutoffCos = cos(radians(20.0f));

    directional.direction = vec3(0, 0, -1);

    light.emitterPosition = vec3(3, 0, 0);
    light.power = 0.5;
    light.attenuationConstant = 1.0f;
    light.attenuationLinear = 0.09f;
    light.attenuationQuadratic = 0.032f;

    vec3 norm = normalize(vertexData.normal);
    vec3 fragToLight = normalize(light.emitterPosition - vertexData.coords);
    vec3 fragToCamera = normalize(camera.pos - vertexData.coords);

    float dist = distance(light.emitterPosition, vertexData.coords);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * dist + light.attenuationQuadratic * dist * dist);

    vec4 textureColor = vec4(1, 0.5, 0.55, 1);
    vec4 ambientLight = vec4(0.1, 0.1, 0.1, 1) * textureColor;
    vec4 diffuseLight = calculateDiffuse(textureColor, norm, fragToLight, light.power);
    vec4 specularLight = calculateSpecular(textureColor, norm, fragToLight, fragToCamera, 32, light.power);
    vec4 directionalLight = calculateDirectional(textureColor, norm, directional.direction, 0.1);

    float cosineFlashlight = max(dot(normalize(-camera.viewDir), fragToCamera), 0.0);
    vec4 flashlightLight = vec4(0, 0, 0, 0);
    float flashlightIntensity = clamp((cosineFlashlight - flashlight.outerCutoffCos) / (flashlight.innerCutoffCos - flashlight.outerCutoffCos), 0.0, 1.0);
    if(flashlightIntensity > 0)
    {
        vec4 flashlightDiffuse = calculateDiffuse(textureColor, norm, fragToCamera, 0.2);
        vec4 flashlightSpecular = calculateSpecular(textureColor, norm, fragToCamera, fragToCamera, 32, 0.2);
        float distToFlashlight = distance(vertexData.coords, camera.pos);
        float flashlightAttenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * distToFlashlight + light.attenuationQuadratic * distToFlashlight * distToFlashlight);
        flashlightLight = (flashlightDiffuse + flashlightSpecular) * flashlightAttenuation * flashlightIntensity;
    }

    outColor = (diffuseLight + ambientLight + specularLight) * attenuation + directionalLight + flashlightLight;
    //outColor.w = 1;
    //outColor = vec4(1, 1, 1, 1) * gl_FragCoord.z;
    //outColor = vec4(1, 1, 1, 1);
}