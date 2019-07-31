#include<TextureHolder.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

TextureHolder& TextureHolder::addTexture(const vk::Format format, 
    const std::string filename,
    const std::string key)
{
    const auto& pool = spk::system::Executives::getInstance()->getPool();
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    std::string name = (key != "") ? key : filename;
    if(textures.count(name)) throw std::invalid_argument("Trying to create existing texture.\n");

    vk::CommandBufferAllocateInfo commandBufferAllocInfo;
    commandBufferAllocInfo.setCommandBufferCount(1)
        .setCommandPool(pool)
        .setLevel(vk::CommandBufferLevel::ePrimary);
    if(logicalDevice.allocateCommandBuffers(&commandBufferAllocInfo, &textures[name].imageUpdateCommandBuffer) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate command buffer!\n");
    if(logicalDevice.allocateCommandBuffers(&commandBufferAllocInfo, &textures[name].layoutChangeCommandBuffer) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate command buffer!\n");

    vk::FenceCreateInfo fenceInfo;
    if(logicalDevice.createFence(&fenceInfo, nullptr, &textures[name].layoutChangedFence) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    if(logicalDevice.createFence(&fenceInfo, nullptr, &textures[name].imageUpdatedFence) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");

    vk::SemaphoreCreateInfo semaphoreInfo;
    if(logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &textures[name].layoutChangedSemaphore) != vk::Result::eSuccess) throw std::runtime_error("Failed to create semaphore!\n");
    if(logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &textures[name].imageUpdatedSemaphore) != vk::Result::eSuccess) throw std::runtime_error("Failed to create semaphore!\n");

    int width, height, channels;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    channels = 4; // because channel count returns 3

    spk::Buffer temporaryStorage;
    temporaryStorage.create(width * height * channels, vk::BufferUsageFlagBits::eTransferSrc, false, true);
    temporaryStorage.bindMemory();
    temporaryStorage.updateCPUAccessible(reinterpret_cast<const void*>(imageData));

    textures[name].image.create({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}, format, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageAspectFlagBits::eColor, static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1));
    textures[name].image.bindMemory();
    textures[name].image.changeLayout(textures[name].layoutChangeCommandBuffer, 
        vk::ImageLayout::eTransferDstOptimal, 
        vk::Semaphore(), 
        textures[name].layoutChangedSemaphore, 
        vk::Fence(), 
        textures[name].layoutChangedFence, 
        true);
    textures[name].image.update(textures[name].imageUpdateCommandBuffer, 
        temporaryStorage.getBuffer(), 
        textures[name].layoutChangedSemaphore, 
        textures[name].imageUpdatedSemaphore, 
        textures[name].layoutChangedFence, 
        textures[name].imageUpdatedFence, 
        vk::PipelineStageFlagBits::eTopOfPipe, 
        true);
    if(textures[name].layoutChangeCommandBuffer.reset(vk::CommandBufferResetFlags()) != vk::Result::eSuccess) throw std::runtime_error("Failed to reset command buffer!\n");
    textures[name].image.generateMipmap(textures[name].layoutChangeCommandBuffer, 
        vk::Filter::eLinear,
        textures[name].imageUpdatedSemaphore,
        textures[name].layoutChangedSemaphore,
        textures[name].imageUpdatedFence,
        textures[name].layoutChangedFence,
        true);
    if(logicalDevice.waitForFences(1, &textures[name].layoutChangedFence, true, ~0U) != vk::Result::eSuccess) throw std::runtime_error("Failed to wait for fence!\n");

    textures[name].imageUpdateCommandBuffer.reset(vk::CommandBufferResetFlags());
    textures[name].layoutChangeCommandBuffer.reset(vk::CommandBufferResetFlags());

    textures[name].view.create(textures[name].image.getImage(), textures[name].image.getFormat(), textures[name].image.getSubresource());

    delete imageData;

    return *this;
}

const vk::ImageLayout TextureHolder::getImageLayout(const std::string id) const
{
    return textures.at(id).image.getLayout();
}

const vk::Image& TextureHolder::getImage(const std::string id) const
{
    return textures.at(id).image.getImage();
}

const vk::ImageView& TextureHolder::getImageView(const std::string id) const
{
    return textures.at(id).view.getView();
}

void TextureHolder::destroy()
{
    const auto& pool = spk::system::Executives::getInstance()->getPool();
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    for(auto& pair : textures)
    {
        pair.second.view.destroy();
        pair.second.image.destroy();
        if(pair.second.imageUpdatedSemaphore)
        {
            logicalDevice.destroySemaphore(pair.second.imageUpdatedSemaphore, nullptr);
            pair.second.imageUpdatedSemaphore = vk::Semaphore();
        }
        if(pair.second.layoutChangedSemaphore)
        {
            logicalDevice.destroySemaphore(pair.second.layoutChangedSemaphore, nullptr);
            pair.second.layoutChangedSemaphore = vk::Semaphore();
        }
        if(pair.second.imageUpdatedFence)
        {
            logicalDevice.destroyFence(pair.second.imageUpdatedFence, nullptr);
            pair.second.imageUpdatedFence = vk::Fence();
        }
        if(pair.second.layoutChangedFence)
        {
            logicalDevice.destroyFence(pair.second.layoutChangedFence, nullptr);
            pair.second.layoutChangedFence = vk::Fence();
        }
        if(pair.second.imageUpdateCommandBuffer && pair.second.layoutChangeCommandBuffer)
        {
            vk::CommandBuffer cbs[] = {pair.second.imageUpdateCommandBuffer, pair.second.layoutChangeCommandBuffer};
            logicalDevice.freeCommandBuffers(pool, 2, cbs);
            pair.second.imageUpdateCommandBuffer = vk::CommandBuffer();
            pair.second.layoutChangeCommandBuffer = vk::CommandBuffer();
        }
    }
}

TextureHolder::~TextureHolder()
{
    destroy();
}