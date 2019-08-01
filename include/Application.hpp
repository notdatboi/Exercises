#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include<BasicMesh.hpp>
#include<NotTexturedMesh.hpp>
//#include<map>
#include<string>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
//#include<Spark.hpp>
#include<TextureHolder.hpp>
#include<Camera.hpp>
#include<chrono>

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

const unsigned int MAX_LAMPS_ON_SCENE = 1;

class Application
{
public:
    Application();
    void run();
    ~Application();
private:
    void destroy();

    void createSyncObjects();
    void createWindowAndSurface();
    void createSwapchain();
    void loadDescriptorSets();
    void createDescriptorBuffers();
    void loadAndWritePlaneTexture();
    void createDepthMaps();
    //void createDepthPrepass();
    void createGBufferPass();
    void createRenderPass();        // and subpass dependency (l8r)
    void loadMeshes();
    void createQueryPool();
    void recordRenderPass();

    void processInput(const float elapsedTime);
    void updateMVPDescriptorSet();
    void updateLampDescriptorSet();
    void updateCameraDescriptorSet();
    void updateMVPBuffer();
    void updateLampBuffer();
    void updateCameraBuffer();

    const unsigned int windowWidth = 1024;
    const unsigned int windowHeight = 720;
    const double sensitivity = 0.05;
    const double fps = 60;
    const double frameTime = 1 / fps;

    std::vector<glm::vec4> lampPositionsAndPowers;

    MVP mvp;
    Camera camera;
    spk::Buffer mvpBuffer;
    spk::Buffer lampBuffer;
    spk::Buffer cameraBuffer;

    const uint32_t mvpSetIndex = 0;
    const uint32_t lampSetIndex = 1;
    const uint32_t cameraSetIndex = 2;
    const uint32_t textureSetIndex = 3;
    const uint32_t normalMapSetIndex = 4;

    const uint32_t mvpLayoutIndex = 0;
    const uint32_t lampLayoutIndex = 1;
    const uint32_t cameraLayoutIndex = 2;
    const uint32_t planeTextureLayoutIndex = 3;
    const uint32_t normalMapLayoutIndex = 3;

    const uint32_t gBufferPassID = 0;
    vk::Format swapchainImageFormat = vk::Format::eR8G8B8Unorm;
    vk::Format depthMapFormat;
    
    GLFWwindow* window;
    vk::SurfaceKHR surface;
    spk::Swapchain swapchain;
    TextureHolder textureHolder;
    spk::DescriptorPool descriptorPool;
    vk::PipelineLayout planePipelineLayout;
    vk::PipelineLayout lampPipelineLayout;
    std::vector<spk::Image> depthMaps;
    std::vector<spk::ImageView> depthMapImageViews;
    spk::Subpass gBufferPass;
    spk::RenderPass renderPass;
    vk::Sampler planeSampler;
    vk::QueryPool queryPool;
    BasicMesh plane;
    NotTexturedMesh lamp;

    vk::Semaphore imageAcquiredSemaphore;
    vk::Semaphore imageRenderedSemaphore;
    vk::Fence imageAcquiredFence;
    vk::Fence imageRenderedFence;
};

#endif