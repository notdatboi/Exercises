#include"Application.hpp"

Application::Application()
{
    mvp.model = glm::mat4(1);
    mvp.proj = glm::perspective(glm::radians<float>(60), windowWidth / (float)windowHeight, 0.1f, 10.f);
    mvp.proj[1][1] = -mvp.proj[1][1];

    createSyncObjects();
    createWindowAndSurface();
    createSwapchain();
    loadDescriptorSets();
    createDescriptorBuffers();
    updateDonutInstancesDescriptorSet();
    updateMVPDescriptorSet();
    updateCameraDescriptorSet();
    loadAndWriteDonutTexture();
    loadShaders();
    createGBufferPass();
    createRenderPass();
    createGPassPipeline();
//    createFramebuffers();
    loadDonutMesh("DonutWithStoneTexture.obj");
    recordRenderPass();

    camera.setPosition({3, 3, 0});
    camera.setRotation(0, 0);
}

void Application::run()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    const auto& graphicsQueue = spk::system::Executives::getInstance()->getGraphicsQueue();
    const auto& presentQueue =  spk::system::Executives::getInstance()->getPresentQueue(surface);

    auto startClock = std::chrono::system_clock::now();
    double timeCounter = 0;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto endClock = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = endClock - startClock;
        timeCounter += elapsed.count();
        startClock = endClock;

        processInput(static_cast<float>(elapsed.count()));

        if(timeCounter >= frameTime)
        {
            timeCounter -= frameTime;
            updateMVPBuffer();
            updateCameraBuffer();
            const auto nextImageIndex = swapchain.acquireNextImageIndex(imageAcquiredSemaphore, imageAcquiredFence);
            logicalDevice.waitForFences(1, &imageAcquiredFence, true, ~0U);
            logicalDevice.resetFences(1, &imageAcquiredFence);

            const auto& renderPassCommandBuffer = renderPass.getCommandBuffer(nextImageIndex);
            vk::SubmitInfo renderPassSubmitInfo;
            vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            renderPassSubmitInfo.setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&imageAcquiredSemaphore)
                .setCommandBufferCount(1)
                .setPCommandBuffers(&renderPassCommandBuffer)
                .setSignalSemaphoreCount(1)
                .setPSignalSemaphores(&imageRenderedSemaphore)
                .setPWaitDstStageMask(&waitDstStageMask);
            if(graphicsQueue.submit(1, &renderPassSubmitInfo, imageRenderedFence) != vk::Result::eSuccess) throw std::runtime_error("Failed to submit graphics commands.\n");
            logicalDevice.waitForFences(1, &imageRenderedFence, true, ~0U);
            logicalDevice.resetFences(1, &imageRenderedFence);
            
            std::vector<vk::Result> presentResults(1);
            vk::PresentInfoKHR presentInfo;
            presentInfo.setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&imageRenderedSemaphore)
                .setSwapchainCount(1)
                .setPSwapchains(&swapchain.getSwapchain())
                .setPImageIndices(&nextImageIndex)
                .setPResults(presentResults.data());
            for(auto result : presentResults)
            {
                if(result != vk::Result::eSuccess) throw std::runtime_error("Failed to perform presentation.\n");
            }
            if(presentQueue.second->presentKHR(&presentInfo) != vk::Result::eSuccess) throw std::runtime_error("Failed to submit presentation commands.\n");
            //std::cout << "Camera: "
            //    << camera.getPosition().x << ' ' << camera.getPosition().y << ' ' << camera.getPosition().z << '\n'
            //    << camera.getNormalizedDirection().x << ' ' << camera.getNormalizedDirection().y << ' ' << camera.getNormalizedDirection().z << '\n';
        }
    }
}

void Application::createSyncObjects()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    vk::SemaphoreCreateInfo semaphoreInfo;
    if(logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &imageAcquiredSemaphore) != vk::Result::eSuccess) throw std::runtime_error("Failed to create semaphore!\n");
    if(logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &imageRenderedSemaphore) != vk::Result::eSuccess) throw std::runtime_error("Failed to create semaphore!\n");

    vk::FenceCreateInfo unsignaledFenceInfo;
    if(logicalDevice.createFence(&unsignaledFenceInfo, nullptr, &imageAcquiredFence) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    if(logicalDevice.createFence(&unsignaledFenceInfo, nullptr, &imageRenderedFence) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
}

void Application::createWindowAndSurface()
{
    const vk::Instance& instance = spk::system::System::getInstance()->getvkInstance();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(windowWidth, windowHeight, "TEST", nullptr, nullptr);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    VkSurfaceKHR tmpSurface;
    if(glfwCreateWindowSurface(instance, window, nullptr, &tmpSurface) != VK_SUCCESS) throw std::runtime_error("Failed to create surface!\n");
    surface = tmpSurface;
}

void Application::createSwapchain()
{
    swapchain.create(surface, 
        3, 
        swapchainImageFormat, 
        {windowWidth, windowHeight}, 
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::ImageAspectFlagBits::eColor,
        vk::PresentModeKHR::eFifo,
        true);
}

void Application::loadDescriptorSets()
{
    std::vector<vk::DescriptorPoolSize> poolSizes(2);
    poolSizes[0].setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(3);
    poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1);
    descriptorPool.create(4, poolSizes, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    std::vector<vk::DescriptorSetLayoutBinding> uniformVertexB0C1(1);   // uniform buffer, vertex stage, binding = 0, count = 1; usage: mvp, Instances
    uniformVertexB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    
    std::vector<vk::DescriptorSetLayoutBinding> combinedImageSamplerFragmentB0C1(1);    // usage: texture
    combinedImageSamplerFragmentB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::vector<vk::DescriptorSetLayoutBinding> uniformFragmentB0C1(1);               // usage: Camera
    uniformFragmentB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    descriptorPool.addDescriptorSetLayout(uniformVertexB0C1)
        .addDescriptorSetLayout(combinedImageSamplerFragmentB0C1)
        .addDescriptorSetLayout(uniformFragmentB0C1);

    std::vector<uint32_t> layoutIndices = {0, 2, 0, 1};

    descriptorPool.allocateDescriptorSets(layoutIndices);
}

void Application::loadAndWriteDonutTexture()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    textureHolder.addTexture(vk::Format::eR8G8B8A8Unorm, "textureWALL.jpg", "Wall");
    vk::DescriptorImageInfo imageInfo;

    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(vk::Filter::eNearest)
        .setMinFilter(vk::Filter::eNearest)
        .setMipmapMode(vk::SamplerMipmapMode::eNearest)
        .setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
        .setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
        .setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
        .setMipLodBias(0.0f)
        .setAnisotropyEnable(false)
        //.setMaxAnisotropy()
        .setCompareEnable(false)
        //.setCompareOp()
        .setMinLod(0.0f)
        .setMaxLod(1.0f)
        .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
        .setUnnormalizedCoordinates(false);
    if(logicalDevice.createSampler(&samplerInfo, nullptr, &donutSampler) != vk::Result::eSuccess) throw std::runtime_error("Failed to create sampler!\n");

    imageInfo.setImageLayout(textureHolder.getImageLayout("Wall"));
    imageInfo.setImageView(textureHolder.getImageView("Wall"));
    imageInfo.setSampler(donutSampler);
    descriptorPool.writeDescriptorSetImage(textureSetIndex, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo);
}

void Application::loadShaders()
{
    std::vector<spk::ShaderInfo> shaderInfos(2);
    shaderInfos[0].filename = "vert.spv";
    shaderInfos[0].type = vk::ShaderStageFlagBits::eVertex;
    shaderInfos[1].filename = "frag.spv";
    shaderInfos[1].type = vk::ShaderStageFlagBits::eFragment;
    gPassShaders.create(shaderInfos);
}

void Application::createGBufferPass()
{
    std::vector<vk::AttachmentReference> colorAttachmentReferences(1);
    colorAttachmentReferences[0].setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    gBufferPass.create(gBufferPassID, 
        std::vector<vk::AttachmentReference>(), 
        colorAttachmentReferences, 
        nullptr,
        {}, 
        vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader, 
        vk::AccessFlagBits::eColorAttachmentWrite);
}

void Application::createRenderPass()        // and subpass dependency (l8r)
{
    std::vector<vk::AttachmentDescription> attachments(1);
    attachments[0].setFormat(swapchainImageFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    std::vector<vk::SubpassDescription> descriptions(1);
    descriptions[0] = gBufferPass.getDescription();
    std::vector<vk::SubpassDependency> dependencies;
    renderPass.create(attachments, descriptions, dependencies);

    const auto& swapchainImageViews = swapchain.getImageViews();
    for(const auto& view : swapchainImageViews)
    {
        std::vector<vk::ImageView> framebufferAttachments(1);
        framebufferAttachments[0] = view.getView();
        renderPass.addFramebuffer(framebufferAttachments, {windowWidth, windowHeight});
    }

    // continue later
}

void Application::createGPassPipeline()
{
    spk::ShaderStages shaderStages;
    shaderStages.stages = gPassShaders.getShaderStages();

    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(Vertex));

    vk::VertexInputAttributeDescription positionAttribute, normalAttribute, uvAttribute;
    positionAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setLocation(0)
        .setOffset(offsetof(Vertex, position));
    normalAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setLocation(1)
        .setOffset(offsetof(Vertex, normal));
    uvAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setLocation(2)
        .setOffset(offsetof(Vertex, uv));

    spk::VertexInputState vertexInputState;
    vertexInputState.bindingDescriptions = {bindingDesc};
    vertexInputState.attributeDescriptions = {positionAttribute, normalAttribute, uvAttribute};

    spk::InputAssemblyState assemblyState;
    assemblyState.enablePrimitiveRestart = false;
    assemblyState.topology = vk::PrimitiveTopology::eTriangleList;

    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent({windowWidth, windowHeight});

    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(windowWidth)
        .setHeight(windowHeight)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);

    spk::ViewportState viewportState;
    viewportState.scissor = scissor;
    viewportState.viewport = viewport;

    spk::RasterizationState rasterizationState;
    rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationState.enableDepthClamp = false;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;

    spk::MultisampleState multisampleState;
    multisampleState.rasterizationSampleCount = vk::SampleCountFlagBits::e1;

    spk::DepthStencilState depthStencilState;
    depthStencilState.enableDepthTest = false;        // change later
    depthStencilState.depthCompareOp = vk::CompareOp::eLess;
    depthStencilState.writeTestResults = false;       // too

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.setBlendEnable(false)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    spk::ColorBlendState colorBlendState;
    colorBlendState.attachmentStates = {colorBlendAttachmentState};

    spk::DynamicState dynamicState;

    gPassPipelineLayout = descriptorPool.getPipelineLayout({0, 2, 0, 1});

    spk::AdditionalInfo additionalInfo;
    additionalInfo.layout = gPassPipelineLayout;
    additionalInfo.renderPass = renderPass.getRenderPass();
    additionalInfo.subpassIndex = gBufferPassID;

    gPassPipeline.create(shaderStages, 
        vertexInputState,
        assemblyState,
        viewportState,
        rasterizationState,
        multisampleState,
        depthStencilState,
        colorBlendState,
        dynamicState,
        additionalInfo);
}

void Application::getMeshVertexData(const aiMesh* mesh, std::vector<Vertex>& vertices) const
{
    auto vertexCount = mesh->mNumVertices;
    vertices.resize(vertexCount);
    for(auto currentVertexIndex = 0; currentVertexIndex < vertexCount; ++currentVertexIndex)
    {
        const auto& currentAiVertex = *(mesh->mVertices + currentVertexIndex);
        const auto& currentAiNormal = *(mesh->mNormals + currentVertexIndex);
        const auto& currentAiUV = *(mesh->mTextureCoords[0] + currentVertexIndex);
        Vertex vertex;
        vertex.position = {currentAiVertex.x, currentAiVertex.y, currentAiVertex.z};
        vertex.normal = {currentAiNormal.x, currentAiNormal.y, currentAiNormal.z};
        vertex.uv = {currentAiUV.x, currentAiUV.y};
        vertices[currentVertexIndex] = vertex;
    }
}

const std::vector<Vertex> Application::getMeshVertexData(const aiMesh* mesh) const
{
    std::vector<Vertex> vertices;
    getMeshVertexData(mesh, vertices);
    return vertices;
}

void Application::getMeshIndexData(const aiMesh* mesh, std::vector<uint32_t>& indices) const
{
    const auto faceCount = mesh->mNumFaces;
    indices.resize(faceCount * 3);
    for(auto currentFaceIndex = 0; currentFaceIndex < faceCount; ++currentFaceIndex)
    {
        const auto& face = *(mesh->mFaces + currentFaceIndex);
        for(auto faceIndexIndex = 0; faceIndexIndex < face.mNumIndices; ++faceIndexIndex)
        {
            indices[currentFaceIndex * 3 + faceIndexIndex] = *(face.mIndices + faceIndexIndex);
        }
    }
}

const std::vector<uint32_t> Application::getMeshIndexData(const aiMesh* mesh) const
{
    std::vector<uint32_t> indices;
    getMeshIndexData(mesh, indices);
    return indices;
}

void Application::loadDonutMesh(const std::string donutFilename)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(donutFilename, aiProcess_JoinIdenticalVertices/*aiProcess_Triangulate*/);
    if(!scene) throw std::runtime_error("Failed to load scene.\n");
    std::vector<Vertex> donutVertices = getMeshVertexData(*(scene->mMeshes + 1));
    std::vector<uint32_t> donutIndices = getMeshIndexData(*(scene->mMeshes + 1));
    std::vector<vk::DescriptorSet> descriptorSets = descriptorPool.getDescriptorSets({mvpSetIndex, cameraSetIndex, donutInstancesSetIndex, textureSetIndex});
    donut.create(donutVertices, donutIndices, descriptorSets, &gPassPipeline);
}


void Application::recordRenderPass()
{
    const auto framebufferCount = renderPass.getFramebufferCount();
    for(auto currentFramebufferIndex = 0; currentFramebufferIndex < framebufferCount; ++currentFramebufferIndex)
    {
        const auto& framebuffer = renderPass.getFramebuffer(currentFramebufferIndex);
        gBufferPass.bindCommandBuffer(currentFramebufferIndex);
        gBufferPass.beginRecording(renderPass.getRenderPass(), framebuffer);
        donut.bindDescriptorSets(gBufferPass)
            .bindPipeline(gBufferPass)
            .bindIndexBuffer(gBufferPass)
            .bindVertexBuffer(gBufferPass)
            .drawIndexed(gBufferPass, 2);
        gBufferPass.endRecording();
        vk::Rect2D renderArea;
        renderArea.setExtent({windowWidth, windowHeight})
            .setOffset({0, 0});
        renderPass.beginRecording(currentFramebufferIndex, 1, renderArea)
            //.nextSubpass(gBufferPass.getSecondaryCommandBuffer(currentFramebufferIndex))
            .executeCommandBuffer(gBufferPass.getSecondaryCommandBuffer(currentFramebufferIndex))
            .endRecording();
    }
}

void Application::processInput(const float elapsedTime)
{
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    static double prevCursorX = windowWidth / 2;
    static double prevCursorY = windowHeight / 2;
    double offsetX = (cursorX - prevCursorX) * sensitivity, offsetY = (cursorY - prevCursorY) * sensitivity;
    camera.rotate(offsetX, offsetY);
    prevCursorX = cursorX;
    prevCursorY = cursorY;

    const auto direction = camera.getNormalizedDirection();
    static const glm::vec3 up(0, 0, 1);
    int WState = glfwGetKey(window, GLFW_KEY_W);
    int AState = glfwGetKey(window, GLFW_KEY_A);
    int SState = glfwGetKey(window, GLFW_KEY_S);
    int DState = glfwGetKey(window, GLFW_KEY_D);
    glm::vec3 cameraShift({0, 0, 0});
    if(WState == GLFW_PRESS)
    {
        cameraShift += direction * elapsedTime;
    }
    if(SState == GLFW_PRESS)
    {
        cameraShift -= direction * elapsedTime;
    }
    if(AState == GLFW_PRESS)
    {
        cameraShift += glm::normalize(glm::cross(up, direction)) * elapsedTime;
    }
    if(DState == GLFW_PRESS)
    {
        cameraShift -= glm::normalize(glm::cross(up, direction)) * elapsedTime;
    }
    camera.move(cameraShift.x, cameraShift.y, cameraShift.z);
    mvp.view = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getNormalizedDirection(), up);
}

void Application::createDescriptorBuffers()
{
    mvpBuffer.create(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    cameraBuffer.create(sizeof(glm::vec4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    donutInstancesBuffer.create(sizeof(glm::vec4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    mvpBuffer.bindMemory();
    cameraBuffer.bindMemory();
    donutInstancesBuffer.bindMemory();
}

void Application::updateMVPDescriptorSet()
{
    updateMVPBuffer();

    vk::DescriptorBufferInfo info;
    info.setBuffer(mvpBuffer.getBuffer())
        .setOffset(0)
        .setRange(VK_WHOLE_SIZE);

    descriptorPool.writeDescriptorSetBuffer(mvpSetIndex, 0, vk::DescriptorType::eUniformBuffer, info);
}

void Application::updateCameraDescriptorSet()
{
    updateCameraBuffer();

    vk::DescriptorBufferInfo info;
    info.setBuffer(cameraBuffer.getBuffer())
        .setOffset(0)
        .setRange(VK_WHOLE_SIZE);

    descriptorPool.writeDescriptorSetBuffer(cameraSetIndex, 0, vk::DescriptorType::eUniformBuffer, info);
}

void Application::updateDonutInstancesDescriptorSet()
{
    updateDonutInstancesBuffer();

    vk::DescriptorBufferInfo info;
    info.setBuffer(donutInstancesBuffer.getBuffer())
        .setOffset(0)
        .setRange(VK_WHOLE_SIZE);

    descriptorPool.writeDescriptorSetBuffer(donutInstancesSetIndex, 0, vk::DescriptorType::eUniformBuffer, info);
}

void Application::updateMVPBuffer()
{
    mvpBuffer.updateCPUAccessible(reinterpret_cast<const void*>(&mvp));
}

void Application::updateCameraBuffer()
{
    std::vector<glm::vec4> data(2);
    data[0] = glm::vec4(camera.getPosition(), 0);
    data[1] = glm::vec4(camera.getNormalizedDirection(), 0);
    cameraBuffer.updateCPUAccessible(reinterpret_cast<const void*>(data.data()));
}

void Application::updateDonutInstancesBuffer()
{
    donutInstancesBuffer.updateCPUAccessible(reinterpret_cast<const void*>(donutInstances.data()));
}

void Application::destroy()
{
    // TODO: destroy everything else too
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    if(gPassPipelineLayout)
    {
        descriptorPool.destroyPipelineLayout(gPassPipelineLayout);
    }
    if(donutSampler)
    {
        logicalDevice.destroySampler(donutSampler, nullptr);
        donutSampler = vk::Sampler();
    }
    if(imageAcquiredFence)
    {
        logicalDevice.destroyFence(imageAcquiredFence, nullptr);
        imageAcquiredFence = vk::Fence();
    }
    if(imageRenderedFence)
    {
        logicalDevice.destroyFence(imageRenderedFence, nullptr);
        imageRenderedFence = vk::Fence();
    }
    if(imageAcquiredSemaphore)
    {
        logicalDevice.destroySemaphore(imageAcquiredSemaphore, nullptr);
        imageAcquiredSemaphore = vk::Semaphore();
    }
    if(imageRenderedSemaphore)
    {
        logicalDevice.destroySemaphore(imageRenderedSemaphore, nullptr);
        imageRenderedSemaphore = vk::Semaphore();
    }
}

Application::~Application()
{
    destroy();
}