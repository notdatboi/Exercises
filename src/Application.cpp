#include<Application.hpp>

Application::Application() : lampPositionsAndPowers(MAX_LAMPS_ON_SCENE)
{
    lampPositionsAndPowers[0] = {0, 0.5, 0, 1};
    mvp.model = glm::mat4(1);
    mvp.proj = glm::perspective(glm::radians<float>(60), windowWidth / (float)windowHeight, 0.0001f, 10.f);
    mvp.proj[1][1] = -mvp.proj[1][1];

    createSyncObjects();
    createWindowAndSurface();
    createSwapchain();
    loadDescriptorSets();
    createDescriptorBuffers();
    updateMVPDescriptorSet();
    updateCameraDescriptorSet();
    updateLampDescriptorSet();
    loadAndWritePlaneTexture();
    createDepthMaps();
    createGBufferPass();
    createRenderPass();
    loadMeshes();
    createQueryPool();
    recordRenderPass();

    camera.setPosition({0, 1, 0});
    camera.setRotation(0, 89);
}

void Application::run()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    const auto& graphicsQueue = spk::system::Executives::getInstance()->getGraphicsQueue();
    const auto& presentQueue =  spk::system::Executives::getInstance()->getPresentQueue(surface);

    auto startClock = std::chrono::system_clock::now();
    double timeCounter = 0;
    float timeFromTheStart = 0;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto endClock = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = endClock - startClock;
        timeCounter += elapsed.count();
        timeFromTheStart += elapsed.count();
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
            uint32_t queryResult;
            if(logicalDevice.getQueryPoolResults(queryPool, nextImageIndex, 1, 4, reinterpret_cast<void*>(&queryResult), 4, vk::QueryResultFlagBits::eWait) != vk::Result::eSuccess) spk::system::yeet("Failed to fetch query pool results!\n");
            //std::cout << queryResult << '\n';
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
        .setDescriptorCount(2);
    descriptorPool.create(5, poolSizes, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    std::vector<vk::DescriptorSetLayoutBinding> uniformVertGeomB0C1(1);   // uniform buffer, geometry and vertex shader stages, binding = 0, count = 1; usage: mvp
    uniformVertGeomB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eVertex);
    
    std::vector<vk::DescriptorSetLayoutBinding> uniformVertFragB0C1(1);
    uniformVertFragB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    std::vector<vk::DescriptorSetLayoutBinding> uniformFragmentB0C1(1);               // usage: Camera
    uniformFragmentB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::vector<vk::DescriptorSetLayoutBinding> combinedImageSamplerFragmentB0C1(1);    // usage: texture, normal map
    combinedImageSamplerFragmentB0C1[0].setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    descriptorPool.addDescriptorSetLayout(uniformVertGeomB0C1)
        .addDescriptorSetLayout(uniformVertFragB0C1)
        .addDescriptorSetLayout(uniformFragmentB0C1)
        .addDescriptorSetLayout(combinedImageSamplerFragmentB0C1);

    std::vector<uint32_t> layoutIndices = {mvpLayoutIndex, lampLayoutIndex, cameraLayoutIndex, planeTextureLayoutIndex, normalMapLayoutIndex};

    descriptorPool.allocateDescriptorSets(layoutIndices);
}

void Application::loadAndWritePlaneTexture()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    textureHolder.addTexture(vk::Format::eR8G8B8A8Unorm, "resources/brickwall.jpg", "Wall");
    textureHolder.addTexture(vk::Format::eR8G8B8A8Unorm, "resources/brickwall_normal.jpg", "WallNormals");

    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
        .setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
        .setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
        .setMipLodBias(0.0f)
        .setAnisotropyEnable(false)
        //.setMaxAnisotropy()
        .setCompareEnable(false)
        //.setCompareOp()
        .setMinLod(0.0f)
        .setMaxLod(10.0f)
        .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
        .setUnnormalizedCoordinates(false);
    if(logicalDevice.createSampler(&samplerInfo, nullptr, &planeSampler) != vk::Result::eSuccess) throw std::runtime_error("Failed to create sampler!\n");

    vk::DescriptorImageInfo wallImageInfo;
    wallImageInfo.setImageLayout(textureHolder.getImageLayout("Wall"))
        .setImageView(textureHolder.getImageView("Wall"))
        .setSampler(planeSampler);
    descriptorPool.writeDescriptorSetImage(textureSetIndex, 0, vk::DescriptorType::eCombinedImageSampler, wallImageInfo);

    vk::DescriptorImageInfo wallNormalMapInfo;
    wallNormalMapInfo.setImageLayout(textureHolder.getImageLayout("WallNormals"))
        .setImageView(textureHolder.getImageView("WallNormals"))
        .setSampler(planeSampler);
    descriptorPool.writeDescriptorSetImage(normalMapSetIndex, 0, vk::DescriptorType::eCombinedImageSampler, wallNormalMapInfo);
}

void Application::createDepthMaps()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    const auto& commandPool = spk::system::Executives::getInstance()->getPool();

    depthMaps.resize(swapchain.getImageViews().size());
    depthMapImageViews.resize(depthMaps.size());
    std::vector<vk::CommandBuffer> layoutChangeCBs(depthMaps.size());
    std::vector<vk::Fence> layoutChangedFences(layoutChangeCBs.size());
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandBufferCount(layoutChangeCBs.size())
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary);
    if(logicalDevice.allocateCommandBuffers(&allocInfo, layoutChangeCBs.data()) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate command buffers!\n");
    vk::FenceCreateInfo fenceInfo;
    for(auto& fence : layoutChangedFences)
    {
        if(logicalDevice.createFence(&fenceInfo, nullptr, &fence) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate fence!\n");
    }

    auto fmt = spk::Image::getSupportedFormat({vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32SfloatS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    if(!fmt.has_value()) throw std::runtime_error("Can't choose format for depth map.\n");
    depthMapFormat = fmt.value();

    for(auto& depthMap : depthMaps)
    {
        depthMap.create({windowWidth, windowHeight, 1}, depthMapFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
    }
    int index = 0;
    for(auto& depthMap : depthMaps)
    {
        depthMap.bindMemory();
        depthMap.changeLayout(layoutChangeCBs[index], vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::Semaphore(), vk::Semaphore(), vk::Fence(), layoutChangedFences[index], true);
        depthMapImageViews[index].create(depthMap.getImage(), depthMapFormat, depthMap.getSubresource());
        ++index;
    }

    logicalDevice.waitForFences(layoutChangedFences.size(), layoutChangedFences.data(), true, ~0U);
    logicalDevice.freeCommandBuffers(commandPool, layoutChangeCBs.size(), layoutChangeCBs.data());
    for(auto& fence : layoutChangedFences)
    {
        logicalDevice.destroyFence(fence, nullptr);
    }
}

void Application::createGBufferPass()
{
    std::vector<vk::AttachmentReference> colorAttachmentReferences(1);
    colorAttachmentReferences[0].setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    
    vk::AttachmentReference depthAttachmentReference;
    depthAttachmentReference.setAttachment(1)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    gBufferPass.create(gBufferPassID, 
        {}, 
        colorAttachmentReferences, 
        &depthAttachmentReference,
        {}, 
        vk::PipelineStageFlagBits::eAllGraphics, 
        vk::AccessFlagBits::eColorAttachmentWrite);
}

void Application::createRenderPass()        // and subpass dependency (l8r)
{
    std::vector<vk::AttachmentDescription> attachments(2);
    attachments[0].setFormat(swapchainImageFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    attachments[1].setFormat(depthMapFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::SubpassDescription> descriptions(1);
    descriptions[0] = gBufferPass.getDescription();
    std::vector<vk::SubpassDependency> dependencies;
    renderPass.create(attachments, descriptions, dependencies);

    const auto& swapchainImageViews = swapchain.getImageViews();
    int index = 0;
    for(const auto& view : swapchainImageViews)
    {
        std::vector<vk::ImageView> framebufferAttachments(2);
        framebufferAttachments[0] = view.getView();
        framebufferAttachments[1] = depthMapImageViews[index].getView();
        renderPass.addFramebuffer(framebufferAttachments, {windowWidth, windowHeight});
        ++index;
    }
}

void Application::loadMeshes()
{
    Assimp::Importer importer;
    const aiScene* planeScene = importer.ReadFile("resources/plane.obj", aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
    if(!planeScene) throw std::runtime_error("Failed to load scene.\n");
    std::vector<vk::DescriptorSet> planeDescriptorSets = descriptorPool.getDescriptorSets({mvpSetIndex, lampSetIndex, cameraSetIndex, textureSetIndex, normalMapSetIndex});
    plane.create(*(*(planeScene->mMeshes)), planeDescriptorSets, 1);

    //std::vector<spk::ShaderInfo> planeShaderInfos(5);
    //planeShaderInfos[0].filename = "shaders/PlaneVert.spv";
    //planeShaderInfos[0].type = vk::ShaderStageFlagBits::eVertex;
    //planeShaderInfos[1].filename = "shaders/PlaneTesc.spv";
    //planeShaderInfos[1].type = vk::ShaderStageFlagBits::eTessellationControl;
    //planeShaderInfos[2].filename = "shaders/PlaneTese.spv";
    //planeShaderInfos[2].type = vk::ShaderStageFlagBits::eTessellationEvaluation;
    //planeShaderInfos[3].filename = "shaders/PlaneFrag.spv";
    //planeShaderInfos[3].type = vk::ShaderStageFlagBits::eFragment;
    //planeShaderInfos[4].filename = "shaders/PlaneGeom.spv";
    //planeShaderInfos[4].type = vk::ShaderStageFlagBits::eGeometry;
    std::vector<std::string> planeShaderInfos{"shaders/Plane.vert", "shaders/Plane.tesc", "shaders/Plane.tese", "shaders/Plane.geom", "shaders/Plane.frag"};

    planePipelineLayout = descriptorPool.getPipelineLayout({mvpLayoutIndex, lampLayoutIndex, cameraLayoutIndex, planeTextureLayoutIndex, normalMapLayoutIndex});

    spk::AdditionalInfo planeAdditionalInfo;
    planeAdditionalInfo.layout = planePipelineLayout;
    planeAdditionalInfo.renderPass = renderPass.getRenderPass();
    planeAdditionalInfo.subpassIndex = gBufferPassID;

    plane.createPipeline(0, planeShaderInfos, {windowWidth, windowHeight}, planeAdditionalInfo);

    const aiScene* lampScene = importer.ReadFile("resources/lamp.obj", aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
    if(!lampScene) throw std::runtime_error("Failed to load scene.\n");
    lamp.create(*(*(lampScene->mMeshes)), {}, 1);

    //std::vector<spk::ShaderInfo> lampShaderInfos(2);
    //lampShaderInfos[0].filename = "shaders/LampVert.spv";
    //lampShaderInfos[0].type = vk::ShaderStageFlagBits::eVertex;
    //lampShaderInfos[1].filename = "shaders/LampFrag.spv";
    //lampShaderInfos[1].type = vk::ShaderStageFlagBits::eFragment;
    std::vector<std::string> lampShaderInfos{"shaders/Lamp.vert", "shaders/Lamp.frag"};

    lampPipelineLayout = descriptorPool.getPipelineLayout({mvpLayoutIndex, lampLayoutIndex});

    spk::AdditionalInfo lampAdditionalInfo;
    lampAdditionalInfo.layout = lampPipelineLayout;
    lampAdditionalInfo.renderPass = renderPass.getRenderPass();
    lampAdditionalInfo.subpassIndex = gBufferPassID;

    lamp.createPipeline(0, lampShaderInfos, {windowWidth, windowHeight}, lampAdditionalInfo);
}

void Application::createQueryPool()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    vk::QueryPoolCreateInfo info;
    info.setQueryType(vk::QueryType::ePipelineStatistics)
        .setQueryCount(renderPass.getFramebufferCount())
        .setPipelineStatistics(vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations);
    
    if(logicalDevice.createQueryPool(&info, nullptr, &queryPool) != vk::Result::eSuccess) throw std::runtime_error("Failed to create query pool!\n");
}

void Application::recordRenderPass()
{
    const auto framebufferCount = renderPass.getFramebufferCount();
    for(auto currentFramebufferIndex = 0; currentFramebufferIndex < framebufferCount; ++currentFramebufferIndex)
    {
        const auto& framebuffer = renderPass.getFramebuffer(currentFramebufferIndex);
        gBufferPass.bindCommandBuffer(currentFramebufferIndex)
            .beginRecording(renderPass.getRenderPass(), framebuffer);
        plane.bindDescriptorSets(gBufferPass, 0)
            .bindPipeline(gBufferPass, 0)
            .bindIndexBuffer(gBufferPass)
            .bindVertexBuffer(gBufferPass)
            .drawIndexed(gBufferPass);
        lamp.bindPipeline(gBufferPass, 0)
            .bindIndexBuffer(gBufferPass)
            .bindVertexBuffer(gBufferPass)
            .drawIndexed(gBufferPass);
        gBufferPass.endRecording();
        vk::Rect2D renderArea;
        renderArea.setExtent({windowWidth, windowHeight})
            .setOffset({0, 0});
        std::vector<vk::ClearValue> clearVals(2);
        clearVals[0].setColor(vk::ClearColorValue());
        clearVals[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f));
        renderPass.beginRecording(currentFramebufferIndex)
            .resetQueries(queryPool, currentFramebufferIndex, 1)
            .beginQuery(queryPool, currentFramebufferIndex, vk::QueryControlFlags())
            .beginPass(clearVals, renderArea)
            //.nextSubpass(gBufferPass.getSecondaryCommandBuffer(currentFramebufferIndex))
            .executeCommandBuffer(gBufferPass.getSecondaryCommandBuffer(currentFramebufferIndex))
            .endPass()
            .endQuery(queryPool, currentFramebufferIndex)
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
    static const glm::vec3 up(0, 1, 0);
    int WState = glfwGetKey(window, GLFW_KEY_W);
    int AState = glfwGetKey(window, GLFW_KEY_A);
    int SState = glfwGetKey(window, GLFW_KEY_S);
    int DState = glfwGetKey(window, GLFW_KEY_D);
    int EscState = glfwGetKey(window, GLFW_KEY_ESCAPE);
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
    if(EscState == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    camera.move(cameraShift.x, cameraShift.y, cameraShift.z);
    mvp.view = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getNormalizedDirection(), up);
}

void Application::createDescriptorBuffers()
{
    mvpBuffer.create(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    cameraBuffer.create(sizeof(glm::vec4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    lampBuffer.create(sizeof(glm::vec4) * MAX_LAMPS_ON_SCENE, vk::BufferUsageFlagBits::eUniformBuffer, false, false);
    mvpBuffer.bindMemory();
    cameraBuffer.bindMemory();
    lampBuffer.bindMemory();
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

void Application::updateLampDescriptorSet()
{
    updateLampBuffer();

    vk::DescriptorBufferInfo info;
    info.setBuffer(lampBuffer.getBuffer())
        .setOffset(0)
        .setRange(VK_WHOLE_SIZE);
    
    descriptorPool.writeDescriptorSetBuffer(lampSetIndex, 0, vk::DescriptorType::eUniformBuffer, info);
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

void Application::updateMVPBuffer()
{
    mvpBuffer.updateCPUAccessible(reinterpret_cast<const void*>(&mvp));
}

void Application::updateLampBuffer()
{
    lampBuffer.updateCPUAccessible(reinterpret_cast<const void*>(lampPositionsAndPowers.data()));
}

void Application::updateCameraBuffer()
{
    std::vector<glm::vec4> data(2);
    data[0] = glm::vec4(camera.getPosition(), 0);
    data[1] = glm::vec4(camera.getNormalizedDirection(), 0);
    cameraBuffer.updateCPUAccessible(reinterpret_cast<const void*>(data.data()));
}


void Application::destroy()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    if(planePipelineLayout)
    {
        descriptorPool.destroyPipelineLayout(planePipelineLayout);
    }
    if(lampPipelineLayout)
    {
        descriptorPool.destroyPipelineLayout(lampPipelineLayout);
    }
    if(planeSampler)
    {
        logicalDevice.destroySampler(planeSampler, nullptr);
        planeSampler = vk::Sampler();
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
    if(queryPool)
    {
        logicalDevice.destroyQueryPool(queryPool, nullptr);
        queryPool = vk::QueryPool();
    }
}

Application::~Application()
{
    destroy();
}