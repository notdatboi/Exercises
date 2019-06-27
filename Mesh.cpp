#include"Mesh.hpp"

void Mesh::create(const std::vector<Vertex>& vertices, 
    const std::vector<uint32_t>& indices, 
    const std::vector<const vk::DescriptorSet*>& descriptorSets,
    const spk::Pipeline* pipeline)
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    this->pipeline = pipeline;
    this->descriptorSets = descriptorSets;

    vk::FenceCreateInfo fenceInfo;
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences + 1) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    
    writeBuffers(vertices, indices);
}

void Mesh::writeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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

const Mesh& Mesh::bindVertexBuffer(spk::Subpass& subpass) const
{
    std::vector<vk::Buffer> vertexBuffers;
    vertexBuffers.push_back(vertexBuffer.getBuffer());
    subpass.bindVertexBuffers(vertexBuffers);

    return *this;
}

const Mesh& Mesh::bindIndexBuffer(spk::Subpass& subpass) const
{
    subpass.bindIndexBuffer(indexBuffer.getBuffer(), vk::IndexType::eUint32);

    return *this;
}

const Mesh& Mesh::bindPipeline(spk::Subpass& subpass) const
{
    subpass.bindPipeline(pipeline->getPipeline());

    return *this;
}

const Mesh& Mesh::bindDescriptorSets(spk::Subpass& subpass, const uint32_t firstSetToUpdate = 0) const
{
    std::vector<vk::DescriptorSet> sets;
    for(const auto* setPtr : descriptorSets)
    {
        sets.push_back(*setPtr);
    }
    subpass.bindDescriptorSets(pipeline->getLayout(), sets, firstSetToUpdate);

    return *this;
}

const Mesh& Mesh::draw(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const
{
    subpass.draw(vertexCount, instanceCount, 0, firstInstance);

    return *this;
}

const Mesh& Mesh::drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount = 1, const uint32_t firstInstance = 0) const
{
    if(indexCount == 0) throw std::runtime_error("There are no indices to draw.\n");
    subpass.drawIndexed(indexCount, instanceCount, 0, firstInstance);

    return *this;
}

void Mesh::destroy()
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