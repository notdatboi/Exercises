#ifndef TEXTURE_HOLDER_HPP
#define TEXTURE_HOLDER_HPP

#include<Spark.hpp>

struct Texture
{
    spk::Image image;
    spk::ImageView view;
    vk::CommandBuffer layoutChangeCommandBuffer;
    vk::CommandBuffer imageUpdateCommandBuffer;
    vk::Fence layoutChangedFence;
    vk::Fence imageUpdatedFence;
    vk::Semaphore layoutChangedSemaphore;
    vk::Semaphore imageUpdatedSemaphore;
};

class TextureHolder
{
public:
    TextureHolder& addTexture(const vk::Format format, 
        const std::string filename,
        const std::string key = ""); // if the key must be different
    const vk::Image& getImage(const std::string id) const;
    const vk::ImageLayout getImageLayout(const std::string id) const;
    const vk::ImageView& getImageView(const std::string id) const;

    void destroy();
    ~TextureHolder();
private:
    std::map<std::string, Texture> textures;
};

#endif