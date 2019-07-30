#ifndef VERTEX_CLASSES_HPP
#define VERTEX_CLASSES_HPP
#include"../../Vulkan-wrapper/include/Spark.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class BasicVertex
{
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 uv;
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
    VertexDescription(const std::vector<AttributeInfo> attributes)
    {
        create(attributes);
    }
    void create(const std::vector<AttributeInfo> attributes)
    {
        attributeDescriptions.resize(attributes.size());
        for(auto index = 0; index < attributes.size(); ++index)
        {
            attributeDescriptions[index].setBinding(0)
                .setFormat(attributes[index].format)
                .setLocation(index)
                .setOffset(attributes[index].offset);
        }
    }
    const std::vector<vk::VertexInputAttributeDescription>& getAttributeDescriptions() const
    {
        return attributeDescriptions;
    }
private:
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
};

#endif