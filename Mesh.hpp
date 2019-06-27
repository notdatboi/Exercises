#ifndef MESH_HPP
#define MESH_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include<Spark.hpp>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include<string>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh
{
public:
    void create(const std::vector<Vertex>& vertices, 
        const std::vector<uint32_t>& indices, 
        const std::vector<const vk::DescriptorSet*>& descriptorSets,
        const spk::Pipeline* pipeline);
    const Mesh& bindVertexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindIndexBuffer(spk::Subpass& subpass) const;
    const Mesh& bindPipeline(spk::Subpass& subpass) const;
    const Mesh& bindDescriptorSets(spk::Subpass& subpass, const uint32_t firstSetToUpdate = 0) const;
    const Mesh& draw(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;
    const Mesh& drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const;

    void destroy();
    ~Mesh();
private:
    const spk::Pipeline* pipeline;
    std::vector<const vk::DescriptorSet*> descriptorSets;
    void writeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    vk::Fence bufferUpdateFences[2];            // vertex, index
    vk::CommandBuffer bufferUpdateCBs[2];       // vertex, index
    spk::Buffer vertexBuffer;
    uint32_t vertexCount;
    spk::Buffer indexBuffer;
    uint32_t indexCount;
};

#endif