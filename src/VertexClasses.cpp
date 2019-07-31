#include<VertexClasses.hpp>

VertexDescription::VertexDescription(const std::vector<AttributeInfo> attributes)
{
    create(attributes);
}

void VertexDescription::create(const std::vector<AttributeInfo> attributes)
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

const std::vector<vk::VertexInputAttributeDescription>& VertexDescription::getAttributeDescriptions() const
{
    return attributeDescriptions;
}

const VertexDescription BasicVertexDescriptions::VertexPDescription = VertexDescription({{vk::Format::eR32G32B32A32Sfloat, offsetof(VertexP, position)}});
const VertexDescription BasicVertexDescriptions::VertexPNDescription = VertexDescription({{vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPN, position)}, 
    {vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPN, normal)}});
const VertexDescription BasicVertexDescriptions::VertexPNUDescription = VertexDescription({{vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNU, position)}, 
    {vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNU, normal)}, 
    {vk::Format::eR32G32Sfloat, offsetof(VertexPNU, uv)}});
const VertexDescription BasicVertexDescriptions::VertexPNUTBDescription = VertexDescription({{vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNUTB, position)}, 
    {vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNUTB, normal)}, 
    {vk::Format::eR32G32Sfloat, offsetof(VertexPNUTB, uv)}, 
    {vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNUTB, tangent)}, 
    {vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPNUTB, bitangent)}});