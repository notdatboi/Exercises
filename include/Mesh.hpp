#ifndef MESH_HPP
#define MESH_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include"VertexClasses.hpp"
#include"../../Vulkan-wrapper/include/Spark.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include<string>

template<typename Vertex>
class Mesh                  // abstract mesh class
{
public:
    void create(const std::vector<Vertex>& vertices, 
        const std::vector<uint32_t>& indices, 
        const std::vector<vk::DescriptorSet>& descriptorSets,
        const uint32_t pipelineCount);
    virtual void createPipeline(const uint32_t pipelineIndex, const VertexDescription& vertexDescription, const std::vector<spk::ShaderInfo> shaderInfos, const vk::Extent2D extent, const spk::AdditionalInfo& info) = 0;
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

template<typename Vertex>
void Mesh<Vertex>::create(const std::vector<Vertex>& vertices, 
    const std::vector<uint32_t>& indices, 
    const std::vector<vk::DescriptorSet>& descriptorSets,
    const uint32_t pipelineCount)
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    this->descriptorSets = descriptorSets;
    pipelines.resize(pipelineCount);
    shaderSets.resize(pipelineCount);

    vk::FenceCreateInfo fenceInfo;
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences + 1) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    
    writeBuffers(vertices, indices);
}

template<typename Vertex>
void Mesh<Vertex>::writeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    const auto& pool = spk::system::Executives::getInstance()->getPool();
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    vertexCount = vertices.size();
    indexCount = indices.size();

    vk::CommandBufferAllocateInfo CBAllocInfo;
    CBAllocInfo.setCommandBufferCount(2)
        .setCommandPool(pool)
        .setLevel(vk::CommandBufferLevel::ePrimary);
    if(logicalDevice.allocateCommandBuffers(&CBAllocInfo, bufferUpdateCBs) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate command buffer!\n");
    
    vertexBuffer.create(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, true, false);
    indexBuffer.create(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, true, false);
    vertexBuffer.bindMemory();
    indexBuffer.bindMemory();

    spk::Buffer vertexBufferOnCPU, indexBufferOnCPU;
    vertexBufferOnCPU.create(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc, false, false);
    indexBufferOnCPU.create(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, false, false);
    vertexBufferOnCPU.bindMemory();
    indexBufferOnCPU.bindMemory();
    vertexBufferOnCPU.updateCPUAccessible(reinterpret_cast<const void *>(vertices.data()));
    indexBufferOnCPU.updateCPUAccessible(reinterpret_cast<const void *>(indices.data()));

    vertexBuffer.updateDeviceLocal(bufferUpdateCBs[0], 
        vertexBufferOnCPU.getBuffer(),
        0,
        vk::Semaphore(),
        vk::Semaphore(),
        vk::Fence(),
        bufferUpdateFences[0],
        vk::PipelineStageFlagBits::eTopOfPipe,
        true);
    indexBuffer.updateDeviceLocal(bufferUpdateCBs[1],
        indexBufferOnCPU.getBuffer(),
        0,
        vk::Semaphore(),
        vk::Semaphore(),
        vk::Fence(),
        bufferUpdateFences[1],
        vk::PipelineStageFlagBits::eTopOfPipe,
        true);
    
    if(logicalDevice.waitForFences(2, bufferUpdateFences, true, ~0U) != vk::Result::eSuccess) throw std::runtime_error("Failed to wait for fences!\n");

    vertexBufferOnCPU.destroy();
    indexBufferOnCPU.destroy();
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::bindVertexBuffer(spk::Subpass& subpass) const
{
    std::vector<vk::Buffer> vertexBuffers;
    vertexBuffers.push_back(vertexBuffer.getBuffer());
    subpass.bindVertexBuffers(vertexBuffers);

    return *this;
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::bindIndexBuffer(spk::Subpass& subpass) const
{
    subpass.bindIndexBuffer(indexBuffer.getBuffer(), vk::IndexType::eUint32);

    return *this;
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::bindPipeline(spk::Subpass& subpass, const uint32_t index) const
{
    subpass.bindPipeline(pipelines[index].getPipeline());

    return *this;
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::bindDescriptorSets(spk::Subpass& subpass, const uint32_t pipelineIndex, const uint32_t firstSetToUpdate) const
{
    subpass.bindDescriptorSets(pipelines[pipelineIndex].getLayout(), descriptorSets, firstSetToUpdate);

    return *this;
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::draw(spk::Subpass& subpass, const uint32_t instanceCount, const uint32_t firstInstance) const
{
    subpass.draw(vertexCount, instanceCount, 0, firstInstance);

    return *this;
}

template<typename Vertex>
const Mesh<Vertex>& Mesh<Vertex>::drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount, const uint32_t firstInstance) const
{
    if(indexCount == 0) throw std::runtime_error("There are no indices to draw.\n");
    subpass.drawIndexed(indexCount, instanceCount, 0, firstInstance);

    return *this;
}

template<typename Vertex>
void Mesh<Vertex>::loadShaders(const uint32_t shaderSetIndex, const std::vector<spk::ShaderInfo> shaderInfos)
{
    if(shaderSetIndex >= shaderSets.size())
    {
        throw std::invalid_argument("Shader set index is out of range.\n");
    }
    shaderSets[shaderSetIndex].create(shaderInfos);
}

template<typename Vertex>
void Mesh<Vertex>::destroy()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    const auto& pool = spk::system::Executives::getInstance()->getPool();

    if(bufferUpdateFences[0])
    {
        logicalDevice.destroyFence(bufferUpdateFences[0], nullptr);
        logicalDevice.destroyFence(bufferUpdateFences[1], nullptr);
        bufferUpdateFences[0] = bufferUpdateFences[1] = vk::Fence();
    }
    if(bufferUpdateCBs[0])
    {
        logicalDevice.freeCommandBuffers(pool, 2, bufferUpdateCBs);
    }
}
    
template<typename Vertex>
Mesh<Vertex>::~Mesh<Vertex>()
{
    destroy();
}

#endif