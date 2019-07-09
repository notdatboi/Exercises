#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include"Mesh.hpp"
//#include<map>
#include<string>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
//#include<Spark.hpp>
#include"TextureHolder.hpp"
#include"Camera.hpp"
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include<chrono>

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
    void loadAndWriteDonutTexture();
    //void updateDescriptorSets();
    void loadShaders();
    //void createDepthPrepass();
    void createGBufferPass();
    void createRenderPass();        // and subpass dependency (l8r)
    void createGPassPipeline();
    //void createFramebuffers();
    void loadDonutMesh(const std::string donutFilename);
    void recordRenderPass();

    void getMeshVertexData(const aiMesh* mesh, std::vector<Vertex>& vertices) const;
    const std::vector<Vertex> getMeshVertexData(const aiMesh* mesh) const;
    void getMeshIndexData(const aiMesh* mesh, std::vector<uint32_t>& indices) const;
    const std::vector<uint32_t> getMeshIndexData(const aiMesh* mesh) const;

    void processInput(const float elapsedTime);
    void updateMVPDescriptorSet();
    void updateCameraDescriptorSet();
    void updateDonutInstancesDescriptorSet();
    void updateMVPBuffer();
    void updateCameraBuffer();
    void updateDonutInstancesBuffer();

    const unsigned int windowWidth = 1024;
    const unsigned int windowHeight = 720;
    const double sensitivity = 0.05;
    const double fps = 60;
    const double frameTime = 1 / fps;

    MVP mvp;
    Camera camera;
    std::vector<glm::vec4> donutInstances = {{0, 0, 0, 0}, {2, 2, 2, 0}};
    spk::Buffer mvpBuffer;
    spk::Buffer cameraBuffer;
    spk::Buffer donutInstancesBuffer;
    const uint32_t mvpSetIndex = 0;
    const uint32_t cameraSetIndex = 1;
    const uint32_t donutInstancesSetIndex = 2;
    const uint32_t textureSetIndex = 3;

    const uint32_t gBufferPassID = 0;
    vk::Format swapchainImageFormat = vk::Format::eR8G8B8A8Snorm;
    
    GLFWwindow* window;
    vk::SurfaceKHR surface;
    spk::Swapchain swapchain;
    TextureHolder textureHolder;
    spk::DescriptorPool descriptorPool;
    //spk::ShaderSet depthPrepassShaders;
    spk::ShaderSet gPassShaders;
    vk::PipelineLayout gPassPipelineLayout;
    spk::Pipeline gPassPipeline;
    //std::vector<spk::Image> depthMaps;
    //spk::Subpass depthPrepass;
    spk::Subpass gBufferPass;
    //vk::SubpassDependency depthToGBufferDependency;
    spk::RenderPass renderPass;
    spk::Image donutTexture;
    vk::Sampler donutSampler;
    spk::ImageView donutTextureView;
    Mesh donut;

    vk::Semaphore imageAcquiredSemaphore;
    vk::Semaphore imageRenderedSemaphore;
    vk::Fence imageAcquiredFence;
    vk::Fence imageRenderedFence;
};

#endif