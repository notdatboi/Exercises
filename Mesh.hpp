#ifndef MESH_HPP
#define MESH_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include"../Vulkan-wrapper/include/Spark.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include<string>

struct Vertex
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 uv;
};

class Mesh                  // abstract mesh class
{
public:
    void create(const std::vector<Vertex>& vertices, 
        const std::vector<uint32_t>& indices, 
        const std::vector<vk::DescriptorSet>& descriptorSets);
    virtual void createPipeline(const uint32_t pipelineIndex, const std::vector<spk::ShaderInfo> shaderInfos, const vk::Extent2D extent, const spk::AdditionalInfo& info) = 0;
    const Mesh& bindVertexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindIndexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindPipeline(spk::Subpass& subpass, const uint32_t index) const;
    const Mesh& bindDescriptorSets(spk::Subpass& subpass, const uint32_t pipelineIndex, const uint32_t firstSetToUpdate = 0) const;
    const Mesh& draw(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;
    const Mesh& drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;

    void destroy();
    ~Mesh();
protected:
    void writeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void loadShaders(const uint32_t shaderSetIndex, const std::vector<spk::ShaderInfo> shaderInfos);

    std::vector<spk::Pipeline> pipelines;
    std::vector<spk::ShaderSet> shaderSets;
    std::vector<vk::DescriptorSet> descriptorSets;

    vk::Fence bufferUpdateFences[2];            // vertex, index
    vk::CommandBuffer bufferUpdateCBs[2];       // vertex, index
    spk::Buffer vertexBuffer;
    uint32_t vertexCount;
    spk::Buffer indexBuffer;
    uint32_t indexCount;
};

#endif