#include <jate/rendering/vulkan/vulkan_renderer.h>
#include <jate/rendering/vulkan/exceptions.h>
#include <jate/rendering/data_structs.h>

#include <spdlog/spdlog.h>
#include <vector>

namespace jate::rendering::vulkan
{
    VulkanRenderer::VulkanRenderer(Window& window) :
        ARenderer(window),
        m_vulkanInstance("My app"),
        m_vulkanDevice(m_vulkanInstance, m_window)
    {
        init_createSwapChain();
        init_createCommandManager();
        init_createPipelineLayout();
        init_createPipeline();
        init_createSyncObjects();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkDeviceWaitIdle(m_vulkanDevice.getVkDevice());

        m_vulkanCommandManager = nullptr;

        // Sync objects
        for (auto imageAvailableSemaphore : m_imageAvailableSemaphores)
        {
            vkDestroySemaphore(m_vulkanDevice.getVkDevice(), imageAvailableSemaphore, nullptr);
        }
        for (auto renderFinishedSemaphore : m_renderFinishedSemaphores)
        {
            vkDestroySemaphore(m_vulkanDevice.getVkDevice(), renderFinishedSemaphore, nullptr);
        }
        for (auto inFlightFence : m_inFlightFences)
        {
            vkDestroyFence(m_vulkanDevice.getVkDevice(), inFlightFence, nullptr);
        }

        vkDestroyPipelineLayout(m_vulkanDevice.getVkDevice(), m_pipelineLayout, nullptr);
    }

    void VulkanRenderer::init_createSwapChain()
    {
        m_vulkanSwapChain = std::make_unique<VulkanSwapChain>(m_window, m_vulkanDevice, std::move(m_vulkanSwapChain));
    }

    void VulkanRenderer::init_createCommandManager()
    {
        m_vulkanCommandManager = std::make_unique<VulkanCommandManager>(m_vulkanDevice, *m_vulkanSwapChain, MAX_FRAMES_IN_FLIGHT);
        m_vulkanDevice.attachCommandManager(m_vulkanCommandManager.get());
    }

    void VulkanRenderer::init_createPipelineLayout()
    {
        // Push constant
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_vulkanDevice.getVkDevice(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
            spdlog::error("failed to create pipeline layout");
            return;
		}
    }

    void VulkanRenderer::init_createPipeline()
    {
        VulkanPipeline::PipelineConfigInfo pipelineConfig {};
        VulkanPipeline::PipelineConfigInfo::defaultConfig(pipelineConfig);
        pipelineConfig.renderPass = m_vulkanSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_vulkanPipeline = std::make_unique<vulkan::VulkanPipeline>(m_vulkanDevice, "jate_resources/shaders/simple.vert.spv", "jate_resources/shaders/simple.frag.spv", pipelineConfig);
    }

    void VulkanRenderer::init_createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_vulkanDevice.getVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_vulkanDevice.getVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_vulkanDevice.getVkDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                spdlog::error("failed to create semaphores at index {}", i);
                return;
            }
        }
    }

    void VulkanRenderer::recreateSwapChain()
    {
        // If one dimension is zero, pause the current thread until both dimensions are positive
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window.getWindowPtr(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window.getWindowPtr(), &width, &height);
            glfwWaitEvents();
        }

		// Pause current thread until device is not busy
		vkDeviceWaitIdle(m_vulkanDevice.getVkDevice());

        // Recreate swap chain, and everything that needs the swap chain
        init_createSwapChain();
        init_createCommandManager();
        init_createPipeline();
    }

    void VulkanRenderer::beginFrame()
    {
        vkWaitForFences(m_vulkanDevice.getVkDevice(), 1, &m_inFlightFences[m_currentFrameInFlight], VK_TRUE, UINT64_MAX);
        vkResetFences(m_vulkanDevice.getVkDevice(), 1, &m_inFlightFences[m_currentFrameInFlight]);

        try
        {
            m_currentImageIndex = m_vulkanSwapChain->acquireNextImage(m_imageAvailableSemaphores[m_currentFrameInFlight]);
        }
        catch(const SwapChainOutOfDateException e)
        {
            recreateSwapChain();
            m_window.resetFrameBufferResizedFlag();
            m_currentImageIndex = m_vulkanSwapChain->acquireNextImage(m_imageAvailableSemaphores[m_currentFrameInFlight]);
        }

        m_currentFrameCommandBuffer = m_vulkanCommandManager->getMainCommandBuffer(static_cast<size_t>(m_currentFrameInFlight));

        m_currentFrameCommandBuffer->startRecording();
        m_currentFrameCommandBuffer->cmdStartRenderPass(*m_vulkanSwapChain, m_currentImageIndex);
        m_currentFrameCommandBuffer->cmdBindPipeline(*m_vulkanPipeline);

        auto swapChainExtent = m_vulkanSwapChain->getExtent();
        m_currentFrameCommandBuffer->cmdSetViewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height));
        m_currentFrameCommandBuffer->cmdSetScissor({0, 0}, swapChainExtent);
    }

    void VulkanRenderer::endFrame()
    {
        m_currentFrameCommandBuffer->cmdEndRenderPass();
        m_currentFrameCommandBuffer->endRecording();

        m_currentFrameCommandBuffer->submit(m_imageAvailableSemaphores[m_currentFrameInFlight], m_renderFinishedSemaphores[m_currentFrameInFlight], m_inFlightFences[m_currentFrameInFlight]);
        m_currentFrameCommandBuffer->present(*m_vulkanSwapChain, &m_currentImageIndex, m_renderFinishedSemaphores[m_currentFrameInFlight]);

        m_currentFrameInFlight = (m_currentFrameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    renderer_memory_slot_id VulkanRenderer::allocateVertexData(const std::vector<VertexData> &vertices)
    {
        static renderer_memory_slot_id s_nextVertexSlot = 0;
        if (m_vertexBufferSlots.size() > m_vertexBufferSlots.max_size())
        {
            throw std::runtime_error("[Vulkan Renderer] Could not allocate vertex data : no memory slot available");
        }

        auto insertedElementInfo = m_vertexBufferSlots.insert({s_nextVertexSlot, std::make_unique<VulkanVertexBuffer>(m_vulkanDevice, vertices)});

        s_nextVertexSlot++;

        return (insertedElementInfo.first)->first;
    }

    void VulkanRenderer::freeVertexData(renderer_memory_slot_id slotId)
    {
        m_vertexBufferSlots.erase(slotId);
    }
    
    renderer_memory_slot_id VulkanRenderer::allocateIndexData(const std::vector<uint32_t> &indices)
    {
        static renderer_memory_slot_id s_nextIndexSlot = 0;
        if (m_indexBufferSlots.size() > m_indexBufferSlots.max_size())
        {
            throw std::runtime_error("[Vulkan Renderer] Could not allocate index data : no memory slot available");
        }

        auto insertedElementInfo = m_indexBufferSlots.insert({s_nextIndexSlot, std::make_unique<VulkanIndexBuffer>(m_vulkanDevice, indices)});

        s_nextIndexSlot++;

        return (insertedElementInfo.first)->first;
    }

    void VulkanRenderer::freeIndexData(renderer_memory_slot_id slotId)
    {
        m_indexBufferSlots.erase(slotId);
    }

    void VulkanRenderer::drawIndexed(renderer_memory_slot_id verticesSlotId, renderer_memory_slot_id indicesSlotId, const PushConstantData &pushConstantData)
    {
        auto vertexBufferIt = m_vertexBufferSlots.find(verticesSlotId);
        if (vertexBufferIt == m_vertexBufferSlots.end())
        {
            spdlog::error("[Vulkan Renderer] Using vertex slot id {}, but memory is not allocated", verticesSlotId);
            return;
        }

        auto indexBufferIt = m_indexBufferSlots.find(indicesSlotId);
        if (indexBufferIt == m_indexBufferSlots.end())
        {
            spdlog::error("[Vulkan Renderer] Using index slot id {}, but memory is not allocated", indicesSlotId);
            return;
        }

        m_currentFrameCommandBuffer->cmdPushConstants(pushConstantData, m_pipelineLayout);
        m_currentFrameCommandBuffer->cmdDrawIndexedVertexBuffer(*(vertexBufferIt->second), *(indexBufferIt->second));
    }
}