#ifndef VERTEX_CLASSES_HPP
#define VERTEX_CLASSES_HPP
#include"../../Vulkan-wrapper/include/Spark.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class VertexP
{
public:
    alignas(16) glm::vec3 position;
};

class VertexPN
{
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
};

class VertexPNU
{
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 uv;
};

class VertexPNUTB
{
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 uv;
    alignas(16) glm::vec3 tangent;
    alignas(16) glm::vec3 bitangent;
};

class AttributeInfo
{
public:
    vk::Format format;
    uint32_t offset;
private:
};

class VertexDescription
{
public:
    VertexDescription() = default;
    VertexDescription(const std::vector<AttributeInfo> attributes);
    void create(const std::vector<AttributeInfo> attributes);
    const std::vector<vk::VertexInputAttributeDescription>& getAttributeDescriptions() const;
private:
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
};

struct BasicVertexDescriptions
{
    static const VertexDescription VertexPDescription;
    static const VertexDescription VertexPNDescription;
    static const VertexDescription VertexPNUDescription;
    static const VertexDescription VertexPNUTBDescription;
};

#endif