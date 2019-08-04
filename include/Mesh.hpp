#ifndef MESH_HPP
#define MESH_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include<VertexClasses.hpp>
#include<Spark.hpp>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

class Mesh                  // abstract mesh class
{
public:
    void create(const aiMesh& mesh,
        const std::vector<vk::DescriptorSet>& descriptorSets,
        const uint32_t pipelineCount);
    virtual void createPipeline(const uint32_t pipelineIndex, const std::vector</*spk::ShaderInfo*/std::string> shaderInfos, const vk::Extent2D extent, const vk::PipelineLayout layout, const vk::RenderPass renderPass, const uint32_t subpassIndex) = 0;
    const Mesh& bindVertexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindIndexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindPipeline(spk::Subpass& subpass, const uint32_t index) const;
    const Mesh& bindDescriptorSets(spk::Subpass& subpass, const uint32_t pipelineIndex, const uint32_t firstSetToUpdate = 0) const;
    const Mesh& draw(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;
    const Mesh& drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;

    void destroy();
    ~Mesh();
protected:
    void loadShaders(const uint32_t shaderSetIndex, const std::vector</*spk::ShaderInfo*/std::string> shaderInfos);
    void load(const aiMesh& mesh);

    std::vector<spk::Pipeline> pipelines;
    std::vector<spk::ShaderSet> shaderSets;
    std::vector<vk::DescriptorSet> descriptorSets;

    VertexDescription vertexDescription;
    uint32_t stride;

    vk::Fence bufferUpdateFences[2];            // vertex, index
    vk::CommandBuffer bufferUpdateCBs[2];       // vertex, index
    spk::Buffer vertexBuffer;
    uint32_t vertexCount;
    spk::Buffer indexBuffer;
    uint32_t indexCount;
};

#endif