#include <jate/rendering/vulkan/vulkan_renderer.h>

#include <spdlog/spdlog.h>

namespace jate::rendering::vulkan
{
    VulkanRenderer::VulkanRenderer(Window& window) :
        ARenderer(window),
        m_vulkanInstance("My app"),
        m_vulkanDevice(m_vulkanInstance, m_window),
        m_vulkanSwapChain(m_window, m_vulkanDevice),
        m_vulkanCommandManager(m_vulkanDevice, m_vulkanSwapChain)
    {
        init_createPipelineLayout();
        init_createPipeline();
        init_createSyncObjects();

        // TEMPORARY
        std::vector<VulkanVertexBuffer::Vertex> vertices(3);
        vertices[0] = VulkanVertexBuffer::Vertex {{0, 0.5}, {1, 0, 0}};
        vertices[1] = VulkanVertexBuffer::Vertex {{-0.5, -0.5}, {0, 1, 0}};
        vertices[2] = VulkanVertexBuffer::Vertex {{0.5, -0.5}, {0, 0, 1}};

        m_testingVertexBuffer = std::make_unique<VulkanVertexBuffer>(m_vulkanDevice, vertices);
    }

    VulkanRenderer::~VulkanRenderer()
    {
        // Sync objects
        vkDestroySemaphore(m_vulkanDevice.getVkDevice(), m_imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(m_vulkanDevice.getVkDevice(), m_renderFinishedSemaphore, nullptr);
        vkDestroyFence(m_vulkanDevice.getVkDevice(), m_inFlightFence, nullptr);

        vkDestroyPipelineLayout(m_vulkanDevice.getVkDevice(), m_pipelineLayout, nullptr);
    }

    void VulkanRenderer::init_createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

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
        pipelineConfig.renderPass = m_vulkanSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_vulkanPipeline = std::make_unique<vulkan::VulkanPipeline>(m_vulkanDevice, "shaders/simple.vert.spv", "shaders/simple.frag.spv", pipelineConfig);
    }

    void VulkanRenderer::init_createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(m_vulkanDevice.getVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_vulkanDevice.getVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(m_vulkanDevice.getVkDevice(), &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
            spdlog::error("failed to create semaphores!");
            return;
        }
    }

    void VulkanRenderer::beginFrame()
    {
        vkWaitForFences(m_vulkanDevice.getVkDevice(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_vulkanDevice.getVkDevice(), 1, &m_inFlightFence);

        m_currentImageIndex = m_vulkanSwapChain.acquireNextImage(m_imageAvailableSemaphore);

        m_vulkanCommandManager.startRecording();
        m_vulkanCommandManager.cmdStartRenderPass(m_vulkanSwapChain.getRenderPass(), 0); // TODO
        m_vulkanCommandManager.cmdBindPipeline(*m_vulkanPipeline);

        auto swapChainExtent = m_vulkanSwapChain.getExtent();
        m_vulkanCommandManager.cmdSetViewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height));
        m_vulkanCommandManager.cmdSetScissor({0, 0}, swapChainExtent);

        m_vulkanCommandManager.cmdDrawVertexBuffer(*m_testingVertexBuffer);
    }

    void VulkanRenderer::endFrame()
    {
        m_vulkanCommandManager.cmdEndRenderPass();
        m_vulkanCommandManager.endRecording();

        m_vulkanCommandManager.submit(m_imageAvailableSemaphore, m_renderFinishedSemaphore, m_inFlightFence);
        m_vulkanCommandManager.present(m_currentImageIndex, m_renderFinishedSemaphore);
    }
}